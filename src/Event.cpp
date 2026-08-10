#include "Event.h"

EventsMachine eventsMachine;

void init_event_machine() {
    eventsMachine.currentList = 1; // A
    eventsMachine.countA = 0;
    eventsMachine.countB = 0;
}

void add_event(Event e) {
    if (eventsMachine.currentList == 1) { // A
        if (eventsMachine.countB < MAX_EVENTS) {
            eventsMachine.eventsListB[eventsMachine.countB++] = e;
        }
    } else {
        if (eventsMachine.countA < MAX_EVENTS) {
            eventsMachine.eventsListA[eventsMachine.countA++] = e;
        }
    }
}

Event* get_events(uint32_t *out_count) {
    if (eventsMachine.currentList == 1) {
        *out_count = eventsMachine.countA;
        return eventsMachine.eventsListA;
    } else {
        *out_count = eventsMachine.countB;
        return eventsMachine.eventsListB;
    }
}

void events_machine_clear() {
    if (eventsMachine.currentList == 1) {
        eventsMachine.currentList = 2; // Swap to B
        eventsMachine.countA = 0;
    } else {
        eventsMachine.currentList = 1; // Swap to A
        eventsMachine.countB = 0;
    }
}
