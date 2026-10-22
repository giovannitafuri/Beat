#include "Camera.h"
#include "Components.h"

void camera_movement(World* world, Camera* main_camera) {
    for (size_t i = 1; i <= world->entityCount; ++i) {
        if (world->componentsMaskList[i] & COMPONENT_PLAYER) {
            LSMVECTOR pos;
            LSMLoadVector(&world->positionList[i], 2, &pos);
            main_camera->x = pos.x;
            main_camera->y = pos.y;
        }
    }
}
