#ifndef CAMERA_H
#define CAMERA_H

#include "Entity.h"

struct Camera {
    float x;
    float y;
    float zoom;
};

void camera_movement(World* world, Camera* main_camera);

#endif // CAMERA_H
