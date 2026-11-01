#include "Camera.h"
#include "Components.h"
#include "BeatTime.h"
#include "LSMath.h"

void camera_movement(World* world, Camera* main_camera) {
    for (size_t i = 1; i <= world->entityCount; ++i) {
        if (world->componentsMaskList[i] & COMPONENT_PLAYER) {
            LSMVECTOR pos;
            LSMLoadVector(&world->positionList[i], 2, &pos);
            if (world->componentsMaskList[i] & COMPONENT_PREV_POSITION) {
                LSMVECTOR prev_pos;
                LSMLoadVector(&world->prevPositionList[i], 2, &prev_pos);
                pos = prev_pos * (1.0f - Time::Alpha) + pos * Time::Alpha;
            }
            main_camera->x = pos.x;
            main_camera->y = pos.y;
            break;
        }
    }
}
