#ifndef BEATMAP_H
#define BEATMAP_H

#include <stdint.h>
#include <stddef.h>
#include "Assert.h"

// ============================================================================
// BeatMap<Value>
// Open-addressing hash map using Robin Hood hashing and backward-shift deletion.
//
// Key:   64-bit FNV-1a hash of C-string (hash-as-identity, no string storage).
// Value: Trivially copyable handles, indices, or pointers.
// Alloc: Zero STL dependencies. Flat dynamic array (power-of-two capacity).
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
        ASSERT(data != nullptr, "BeatMap memory allocation failed");
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

    // Returns pointer to value, or nullptr (invalidated on map resize/insert).
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

    // Rehash and reinsert all active entries into the new table.
    void resize() {
        Item*  old_data     = data;
        size_t old_capacity = capacity;

        capacity = old_capacity * 2;
        mask     = capacity - 1;
        count    = 0;
        data     = new Item[capacity];
        ASSERT(data != nullptr, "BeatMap resize allocation failed");

        for (size_t i = 0; i < old_capacity; i++) {
            if (old_data[i].hash != 0) {
                raw_insert(old_data[i].hash, old_data[i].value);
            }
        }

        delete[] old_data;
    }

    // Direct placement by precomputed hash (used during table resize).
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
