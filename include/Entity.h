#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>
#include <stddef.h>

#include "Components.h"

constexpr size_t START_ENTITY_NUM = 128;

struct Entity {
    uint64_t id;
    uint32_t status;
};

struct World {
    size_t      capacity;
    size_t      entityCount;
    size_t      *freeList;
    size_t      freeListCount;
    uint32_t    *status;
    uint64_t    *componentsMaskList;
    Position    *positionList;
    Position    *prevPositionList;
    Collider    *colliderList;
    Sprite      *spriteList;
    Velocity    *velocityList;
};

World *world_init();
bool world_resize(World *w);
Entity get_entity(World *w, uint64_t id);
Entity create_entity(World *w);
void delete_entity(World *w, Entity e);
void add_position_component(World *w, uint32_t e, float x, float y);
void add_prev_position_component(World *w, uint32_t e, float x, float y);
void add_collider_component(World *w, uint32_t e, float width, float height);
void add_sprite_component(World *w, uint32_t e, Assets::Atlas* atlas, uint32_t frame_index);
void add_sprite_component(World *w, uint32_t e, const char* atlas_name, uint32_t frame_index);
void add_velocity_component(World *w, uint32_t e, float vx, float vy);
void world_clear(World *w);
void world_free(World *w);

#endif // ENTITY_H
