#include "Event.h"

EventsMachine GlobalEventsMachine;

void init_event_machine() {
    GlobalEventsMachine.currentList = 1; // A
    GlobalEventsMachine.countA = 0;
    GlobalEventsMachine.countB = 0;
}

void add_event(Event e) {
    if (GlobalEventsMachine.currentList == 1) { // A
        if (GlobalEventsMachine.countB < MAX_EVENTS) {
            GlobalEventsMachine.eventsListB[GlobalEventsMachine.countB++] = e;
        }
    } else {
        if (GlobalEventsMachine.countA < MAX_EVENTS) {
            GlobalEventsMachine.eventsListA[GlobalEventsMachine.countA++] = e;
        }
    }
}

Event* get_events(uint32_t *out_count) {
    if (GlobalEventsMachine.currentList == 1) {
        *out_count = GlobalEventsMachine.countA;
        return GlobalEventsMachine.eventsListA;
    } else {
        *out_count = GlobalEventsMachine.countB;
        return GlobalEventsMachine.eventsListB;
    }
}

void events_machine_clear() {
    if (GlobalEventsMachine.currentList == 1) {
        GlobalEventsMachine.currentList = 2; // Swap to B
        GlobalEventsMachine.countA = 0;
    } else {
        GlobalEventsMachine.currentList = 1; // Swap to A
        GlobalEventsMachine.countB = 0;
    }
}
