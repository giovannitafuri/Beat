#ifndef EVENT_H
#define EVENT_H

#include <cstdint>
#include "Entity.h"

constexpr size_t MAX_EVENTS = 256;

enum EventType {
    EVENT_DEATH
};

// Tagged union that represents an Event
struct Event {
    uint32_t type;
    union {
        struct { Entity entity; } death;
    } data; 
};

struct EventsMachine {
    Event eventsListA[MAX_EVENTS]; // List of events to be processed in the current frame.
    Event eventsListB[MAX_EVENTS]; // List for event generated from current frame processing, to be swapped.
    uint8_t currentList;
    uint32_t countA;
    uint32_t countB;
};

extern EventsMachine eventsMachine;

void init_event_machine();
void add_event(Event e);
Event* get_events(uint32_t *out_count);
void events_machine_clear();

#endif // EVENT_H
