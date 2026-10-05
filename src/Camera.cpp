#include "Camera.h"
#include "Components.h"

void camera_movement(World* world, Camera* main_camera) {
    for (size_t i = 1; i <= world->entityCount; ++i) {
        if (world->componentsMaskList[i] & COMPONENT_PLAYER) {
            main_camera->x = world->positionList[i].x;
            main_camera->y = world->positionList[i].y;
        }
    }
}
