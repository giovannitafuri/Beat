#include "Systems.h"
#include "Input.h"
#include "BeatTime.h"

void movement_system(World *w) {
    if (w->entityCount == (size_t)-1) return;

    for (size_t id = 0; id <= w->entityCount; id++) {
        uint64_t mask = w->componentsMaskList[id];
        
        if (mask & COMPONENT_PLAYER) {
            float speed = 300.0f; // pixels per second
            w->velocityList[id].vx = 0;
            w->velocityList[id].vy = 0;
            
            if (CurrentInput.up)    w->velocityList[id].vy = -speed;
            if (CurrentInput.down)  w->velocityList[id].vy = speed;
            if (CurrentInput.left)  w->velocityList[id].vx = -speed;
            if (CurrentInput.right) w->velocityList[id].vx = speed;
        }

        if ((mask & COMPONENT_POSITION) && (mask & COMPONENT_VELOCITY)) {
            w->positionList[id].x += w->velocityList[id].vx * Time::Delta;
            w->positionList[id].y += w->velocityList[id].vy * Time::Delta;
        }
    }
}
