#include "Systems.h"
#include "Input.h"
#include "BeatTime.h"

void movement_system(World *w) {
    for (size_t id = 1; id <= w->entityCount; id++) {
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

void resolve_collision_system(World *w) {
    for (uint32_t i = 0; i < CollisionsCount; i++) {
        Collision current_coll = CollisionsList[i];
        uint64_t id1 = current_coll.e1.id;
        uint64_t id2 = current_coll.e2.id;

        Position e1pos = w->positionList[id1];
        Collider e1collider = w->colliderList[id1];
        Position e2pos = w->positionList[id2];
        Collider e2collider = w->colliderList[id2];

        float overlap_x = 0.0f;
        float direction_x = 0.0f;
        if (e1pos.x < e2pos.x) {
            overlap_x = (e1pos.x + e1collider.width) - e2pos.x;
            direction_x = -1.0f;
        } else {
            overlap_x = (e2pos.x + e2collider.width) - e1pos.x;
            direction_x = 1.0f;
        }

        float overlap_y = 0.0f;
        float direction_y = 0.0f;
        if (e1pos.y < e2pos.y) {
            overlap_y = (e1pos.y + e1collider.height) - e2pos.y;
            direction_y = -1.0f;
        } else {
            overlap_y = (e2pos.y + e2collider.height) - e1pos.y;
            direction_y = 1.0f;
        }

        bool e1_dynamic = (w->componentsMaskList[id1] & COMPONENT_VELOCITY) != 0;
        bool e2_dynamic = (w->componentsMaskList[id2] & COMPONENT_VELOCITY) != 0;

        if (e1_dynamic && !e2_dynamic) {
            if (overlap_x <= overlap_y) {
                w->positionList[id1].x += overlap_x * direction_x;
                w->velocityList[id1].vx = 0.0f;
            } else {
                w->positionList[id1].y += overlap_y * direction_y;
                w->velocityList[id1].vy = 0.0f;
            }
        } else if (!e1_dynamic && e2_dynamic) {
            if (overlap_x <= overlap_y) {
                w->positionList[id2].x -= overlap_x * direction_x;
                w->velocityList[id2].vx = 0.0f;
            } else {
                w->positionList[id2].y -= overlap_y * direction_y;
                w->velocityList[id2].vy = 0.0f;
            }
        } else {
            if (overlap_x <= overlap_y) {
                w->positionList[id1].x += overlap_x * direction_x;
                if (e1_dynamic) w->velocityList[id1].vx = 0.0f;
            } else {
                w->positionList[id1].y += overlap_y * direction_y;
                if (e1_dynamic) w->velocityList[id1].vy = 0.0f;
            }
        }
    }
}
