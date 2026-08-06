#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>
#include <stddef.h>

constexpr size_t START_ENTITY_NUM = 124;

typedef enum {
    COMPONENT_POSITION = 1 << 0,
    COMPONENT_COLLIDER = 1 << 1,
    COMPONENT_SPRITE   = 1 << 2
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

struct Entity {
    uint64_t id;
    uint32_t status;
};

struct World {
    size_t      capacity;
    size_t      entityCount;
    uint32_t     *status; 
    uint32_t    *componentsMaskList; 
    Position    *positionList;
    Collider    *colliderList;
    Sprite      *spriteList;
};

World *world_init();
Entity get_entity(World *w, uint64_t id);
Entity create_entity(World *w);
void add_position_component(World *w, uint32_t e, float x, float y);
void add_collider_component(World *w, uint32_t e, float width, float height);
void add_sprite_component(World *w, uint32_t e, const char* atlas_name, int frame_index);

#endif // ENTITY_H
