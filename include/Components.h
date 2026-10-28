#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <stdint.h>
#include "LSMath.h"

typedef enum {
    COMPONENT_POSITION      = 1 << 0,
    COMPONENT_COLLIDER      = 1 << 1,
    COMPONENT_SPRITE        = 1 << 2,
    COMPONENT_VELOCITY      = 1 << 3,
    COMPONENT_PLAYER        = 1 << 4,
    COMPONENT_PREV_POSITION = 1 << 5
} ComponentType;

typedef LSMFLOAT2 Position;

struct Collider {
    float width;
    float height;
};

namespace Assets {
    struct Atlas;
}

struct Sprite {
    Assets::Atlas* atlas;
    uint32_t frame_index;
};

typedef LSMFLOAT2 Velocity;

#endif // COMPONENTS_H
