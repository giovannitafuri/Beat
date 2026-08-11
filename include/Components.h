#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <stdint.h>

typedef enum {
    COMPONENT_POSITION  = 1 << 0,
    COMPONENT_COLLIDER  = 1 << 1,
    COMPONENT_SPRITE    = 1 << 2,
    COMPONENT_VELOCITY  = 1 << 3,
    COMPONENT_PLAYER    = 1 << 4
} ComponentType;

struct Position {
    float x;
    float y;
};

struct Collider {
    float width;
    float height;
};

struct Sprite {
    const char* atlas_name;
    uint64_t frame_index;
};

struct Velocity {
    float vx;
    float vy;
};

#endif // COMPONENTS_H
