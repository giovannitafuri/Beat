#ifndef EVENT_H
#define EVENT_H

#include <cstdint>
#include "Entity.h"

constexpr size_t MAX_EVENTS = 2048;

enum EventType {
    EVENT_DEATH,
    EVENT_SHOOT,
    EVENT_ASTEROID_HIT
};

// Tagged union that represents an Event
struct Event {
    uint32_t type;
    union {
        struct { Entity entity; } death;
        struct { float x, y; float dir_x, dir_y; } shoot;
        struct { uint32_t asteroid_id; uint32_t projectile_id; } asteroid_hit;
    } data; 
};

struct EventsMachine {
    Event eventsListA[MAX_EVENTS]; // List of events to be processed in the current frame.
    Event eventsListB[MAX_EVENTS]; // List for event generated from current frame processing, to be swapped.
    uint8_t currentList;
    uint32_t countA;
    uint32_t countB;
};

extern EventsMachine GlobalEventsMachine;

void init_event_machine();
void add_event(Event e);
Event* get_events(uint32_t *out_count);
void events_machine_clear();
void event_system(World* world, uint32_t* score);

#endif // EVENT_H
