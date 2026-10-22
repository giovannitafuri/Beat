#include "Systems.h"
#include "Components.h"
#include "Input.h"
#include "BeatTime.h"
#include "LSMath.h"

void movement_system(World *w) {
    for (size_t id = 1; id <= w->entityCount; id++) {
        uint64_t mask = w->componentsMaskList[id];

        if (mask & COMPONENT_PREV_POSITION) {
            w->prevPositionList[id] = w->positionList[id];
        }

        if (mask & COMPONENT_PLAYER) {
            float speed = 300.0f; // pixels per second
            LSMVECTOR vel = LSMVectorZero();
            
            if (CurrentInput.up)    vel.y = -speed;
            if (CurrentInput.down)  vel.y = speed;
            if (CurrentInput.left)  vel.x = -speed;
            if (CurrentInput.right) vel.x = speed;

            w->velocityList[id] = LSMStoreFloat2(vel);
        }

        if ((mask & COMPONENT_POSITION) && (mask & COMPONENT_VELOCITY)) {
            LSMVECTOR pos, vel;
            LSMLoadVector(&w->positionList[id], 2, &pos);
            LSMLoadVector(&w->velocityList[id], 2, &vel);

            pos = pos + vel * Time::Delta;

            w->positionList[id] = LSMStoreFloat2(pos);
        }
    }
}

void resolve_collision_system(World *w) {
    for (uint32_t i = 0; i < CollisionsCount; i++) {
        Collision current_coll = CollisionsList[i];
        uint64_t id1 = current_coll.e1.id;
        uint64_t id2 = current_coll.e2.id;

        LSMVECTOR e1pos, e2pos;
        LSMLoadVector(&w->positionList[id1], 2, &e1pos);
        LSMLoadVector(&w->positionList[id2], 2, &e2pos);

        Collider e1collider = w->colliderList[id1];
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
            LSMVECTOR e1vel;
            LSMLoadVector(&w->velocityList[id1], 2, &e1vel);
            if (overlap_x <= overlap_y) {
                e1pos.x += overlap_x * direction_x;
                e1vel.x = 0.0f;
            } else {
                e1pos.y += overlap_y * direction_y;
                e1vel.y = 0.0f;
            }
            w->positionList[id1] = LSMStoreFloat2(e1pos);
            w->velocityList[id1] = LSMStoreFloat2(e1vel);
        } else if (!e1_dynamic && e2_dynamic) {
            LSMVECTOR e2vel;
            LSMLoadVector(&w->velocityList[id2], 2, &e2vel);
            if (overlap_x <= overlap_y) {
                e2pos.x -= overlap_x * direction_x;
                e2vel.x = 0.0f;
            } else {
                e2pos.y -= overlap_y * direction_y;
                e2vel.y = 0.0f;
            }
            w->positionList[id2] = LSMStoreFloat2(e2pos);
            w->velocityList[id2] = LSMStoreFloat2(e2vel);
        } else {
            LSMVECTOR e1vel;
            if (e1_dynamic) LSMLoadVector(&w->velocityList[id1], 2, &e1vel);
            if (overlap_x <= overlap_y) {
                e1pos.x += overlap_x * direction_x;
                if (e1_dynamic) e1vel.x = 0.0f;
            } else {
                e1pos.y += overlap_y * direction_y;
                if (e1_dynamic) e1vel.y = 0.0f;
            }
            w->positionList[id1] = LSMStoreFloat2(e1pos);
            if (e1_dynamic) w->velocityList[id1] = LSMStoreFloat2(e1vel);
        }
    }
}
