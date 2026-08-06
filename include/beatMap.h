#ifndef BEATMAP_H
#define BEATMAP_H

#include <stdint.h>
#include <stddef.h>

// ============================================================================
// BeatMap<Value>
//
// Open-addressing hash map using Robin Hood hashing, keyed by C strings.
//
// - Keys are hashed internally to a 64-bit integer (FNV-1a + a splitmix64
//   style finalizer). That 64-bit hash IS the identity used for lookups:
//   the original string is never stored or compared again. This means two
//   different strings that happen to hash to the same value would be
//   treated as the same key. With a 64-bit hash this risk is negligible
//   for typical engine use (asset names, tags, event ids), but it is a
//   conscious trade-off, not a guarantee of correctness against adversarial
//   input.
//
// - Value should stay lightweight (a handle, an index, a raw pointer, an
//   integer...). Robin Hood hashing physically moves entries between slots
//   during insert (stealing a slot from a "richer" entry) and during
//   remove (backward-shift of the following cluster), so Value is copied
//   around frequently. Heavy owned data does not belong here directly;
//   store a handle/index into a separate pool instead.
//
// - No tombstones on delete: removal uses backward-shift deletion, so
//   probe sequences never degrade the way they do with tombstone-based
//   linear probing.
//
// - No STL dependency: storage is a raw Item[] managed with new[]/delete[].
//   Copying a BeatMap is disabled (see deleted copy ctor/assignment) since
//   a shallow copy of the raw pointer would lead to double-free.
// ============================================================================

constexpr float LOAD_FACTOR = 0.85f;

template<typename Value>
class BeatMap {
public:
    struct Item {
        uint64_t hash  = 0;   // 0 == empty slot (sentinel); hash_string() never returns 0
        Value    value{};
        uint32_t dist  = 0;   // distance from the ideal (hashed) position
    };

    explicit BeatMap(size_t initial_capacity = 16) {
        capacity = initial_capacity;
        mask     = capacity - 1;   // NOTE: initial_capacity must be a power of 2
        count    = 0;
        data     = new Item[capacity];
    }

    ~BeatMap() {
        delete[] data;
    }

    // Copy disabled: 'data' is a raw pointer, a default shallow copy would
    // cause double-free / unintended sharing between instances.
    BeatMap(const BeatMap&) = delete;
    BeatMap& operator=(const BeatMap&) = delete;

    void insert(const char *key, Value val) {
        if (needs_resize()) resize();

        uint64_t hash = hash_string(key);
        uint64_t idx  = mix(hash) & mask;
        uint32_t dist = 0;
        Value    v    = val;

        while (true) {
            // Empty slot
            if (data[idx].hash == 0) {
                data[idx].hash  = hash;
                data[idx].value = v;
                data[idx].dist  = dist;
                count++;
                return;
            }
            // Same key: update value
            if (data[idx].hash == hash) {
                data[idx].value = v;
                return;
            }
            // Robin Hood
            if (data[idx].dist < dist) {
                uint64_t tmp_h = data[idx].hash;
                Value    tmp_v = data[idx].value;
                uint32_t tmp_d = data[idx].dist;
                data[idx].hash  = hash;
                data[idx].value = v;
                data[idx].dist  = dist;
                hash = tmp_h; v = tmp_v; dist = tmp_d;
            }
            idx = (idx + 1) & mask;   // wraparound
            dist++;
        }
    }

    // Safe variant: copies the value out, no dangling-pointer risk.
    bool find(const char *key, Value *out_value) {
        uint64_t h   = hash_string(key);
        size_t   idx = mix(h) & mask;
        uint32_t dist = 0;

        while (true) {
            if (data[idx].hash == 0) return false;
            if (data[idx].dist < dist) return false;   // Robin Hood early-exit
            if (data[idx].hash == h) {
                *out_value = data[idx].value;
                return true;
            }
            idx = (idx + 1) & mask;
            dist++;
        }
    }

    // Convenience variant: returns a pointer into the slot, or nullptr.
    // WARNING: only valid until the next insert/remove/resize on this map,
    // since Robin Hood moves entries between slots. Use immediately, do not
    // hold onto it across other map operations.
    Value *find(const char *key) {
        uint64_t h   = hash_string(key);
        size_t   idx = mix(h) & mask;
        uint32_t dist = 0;

        while (true) {
            if (data[idx].hash == 0) return nullptr;
            if (data[idx].dist < dist) return nullptr;
            if (data[idx].hash == h) return &data[idx].value;
            idx = (idx + 1) & mask;
            dist++;
        }
    }

    bool remove(const char *key) {
        uint64_t h   = hash_string(key);
        size_t   idx = mix(h) & mask;
        uint32_t dist = 0;

        while (true) {
            if (data[idx].hash == 0) return false;
            if (data[idx].dist < dist) return false;
            if (data[idx].hash == h) break;
            idx = (idx + 1) & mask;
            dist++;
        }

        // Backward-shift deletion: pull the following cluster back by one
        // slot instead of leaving a tombstone.
        size_t next = (idx + 1) & mask;
        while (data[next].hash != 0 && data[next].dist > 0) {
            data[idx] = data[next];
            data[idx].dist--;
            idx = next;
            next = (next + 1) & mask;
        }
        data[idx] = Item{};
        count--;
        return true;
    }

    size_t size()      const { return count; }
    size_t capacity_() const { return capacity; }

private:
    // splitmix64-style finalizer, used to spread the low bits of the hash
    // in case hash_string() alone doesn't distribute them well enough.
    static uint64_t mix(uint64_t x) {
        x ^= x >> 33;
        x *= 0xff51afd7ed558ccdULL;
        x ^= x >> 33;
        x *= 0xc4ceb9fe1a85ec53ULL;
        x ^= x >> 33;
        return x;
    }

    // FNV-1a over a C string.
    static uint64_t hash_string(const char *s) {
        uint64_t h = 14695981039346656037ULL;   // offset basis
        while (*s) {
            h ^= (unsigned char)(*s);
            h *= 1099511628211ULL;               // prime
            s++;
        }
        return h == 0 ? 1 : h;   // 0 is reserved for EMPTY
    }

    bool needs_resize() {
        return ((float)(count + 1) / (float)capacity) >= LOAD_FACTOR;
    }

    // A resize can't just copy the old array into a bigger one: each
    // entry's target slot depends on 'mask' (= capacity - 1), so a bigger
    // capacity means (almost) every entry needs a different index. Every
    // live entry has to be reinserted from scratch.
    void resize() {
        Item*  old_data     = data;
        size_t old_capacity = capacity;

        capacity = old_capacity * 2;
        mask     = capacity - 1;
        count    = 0;
        data     = new Item[capacity];

        for (size_t i = 0; i < old_capacity; i++) {
            if (old_data[i].hash != 0) {
                raw_insert(old_data[i].hash, old_data[i].value);
            }
        }

        delete[] old_data;
    }

    // Used only by resize(): the hash and value are already known (the
    // original string is gone), so this skips hash_string() entirely and
    // just runs the Robin Hood placement logic.
    void raw_insert(uint64_t hash, Value val) {
        uint64_t idx  = mix(hash) & mask;
        uint32_t dist = 0;
        Value    v    = val;

        while (true) {
            if (data[idx].hash == 0) {
                data[idx].hash  = hash;
                data[idx].value = v;
                data[idx].dist  = dist;
                count++;
                return;
            }
            if (data[idx].dist < dist) {
                uint64_t tmp_h = data[idx].hash;
                Value    tmp_v = data[idx].value;
                uint32_t tmp_d = data[idx].dist;
                data[idx].hash  = hash;
                data[idx].value = v;
                data[idx].dist  = dist;
                hash = tmp_h; v = tmp_v; dist = tmp_d;
            }
            idx = (idx + 1) & mask;
            dist++;
        }
    }

    size_t count;
    size_t capacity;
    size_t mask;
    Item*  data;
};

#endif // BEATMAP_H
