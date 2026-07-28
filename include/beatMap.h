#ifndef BEATMAP_H
#define BEATMAP_H

constexpr size_t START_SIZE  = 16;
constexpr float  LOAD_FACTOR = 0.8;

template<typename Value>
struct BeatMap { 
    struct Item {
        uint64_t key; /* key == 0 is occupied */
        Value    value;  
        uint32_t dist; /* Distance from ideal position */ 
    };
    size_t  capacity;
    Item    *data;
};

#endif BEATMAP_H
