#include "Entity.h"
#include "Assets.h"
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include "Assert.h"

static uint64_t free_list_pop(World *w) {
    if (w->freeListCount > 0) {
        return w->freeList[--w->freeListCount];
    }
    return 0;
}

static void free_list_add(World *w, uint64_t id) {
    w->freeList[w->freeListCount++] = id;
}

World *world_init() {
    World *new_world = (World*)malloc(sizeof(World));
    ASSERT(new_world != nullptr, "Failed to allocate World");
    if (!new_world) return nullptr;

    new_world->capacity = ENTITIES_NUM;
    // Entity ID 0 is reserved as the sentinel (NULL_ENTITY / no selection).
    // Initializing entityCount to 0 ensures the first created entity receives ID 1.
    new_world->entityCount = 0;
    new_world->freeListCount = 0;

    new_world->componentsMaskList = (uint64_t*)calloc(ENTITIES_NUM, sizeof(uint64_t));
    new_world->positionList       = (Position*)malloc(sizeof(Position) * ENTITIES_NUM);
    new_world->prevPositionList       = (Position*)malloc(sizeof(Position) * ENTITIES_NUM);
    new_world->status             = (uint32_t*)malloc(sizeof(uint32_t) * ENTITIES_NUM);
    new_world->colliderList       = (Collider*)malloc(sizeof(Collider) * ENTITIES_NUM);
    new_world->spriteList         = (Sprite*)malloc(sizeof(Sprite) * ENTITIES_NUM);
    new_world->velocityList       = (Velocity*)malloc(sizeof(Velocity) * ENTITIES_NUM);
    new_world->freeList           = (size_t*)malloc(sizeof(size_t) * ENTITIES_NUM);
    
    bool arrays_allocated = new_world->componentsMaskList && new_world->positionList &&
                            new_world->prevPositionList && new_world->status &&
                            new_world->colliderList && new_world->spriteList &&
                            new_world->velocityList && new_world->freeList;
    ASSERT(arrays_allocated, "Failed to allocate World component arrays");
    if (!arrays_allocated) {
        world_free(new_world);
        return nullptr;
    }
    
    return new_world;
} 

Entity get_entity(World *w, uint64_t id) {
    Entity new_entity = { id, w->status[id] };
    return new_entity;
}

Entity create_entity(World *w) {
    uint64_t id = free_list_pop(w);
    if (id != 0) {
        Entity new_entity = { id, w->status[id] };
        return new_entity;
    }
    ASSERT(w->entityCount + 1 < w->capacity, "World entity capacity exceeded");
    w->entityCount++;
    Entity new_entity = { w->entityCount, w->status[w->entityCount] };
    return new_entity;
}

// Mark entity as inactive and add its ID to the free list for reuse.
void delete_entity(World* w, Entity e) {
    w->componentsMaskList[e.id] = 0;
    w->status[e.id]++;
    free_list_add(w, e.id);
}
    
void add_position_component(World *w, uint32_t e, float x, float y) {
    Position new_pos = {{x, y}};
    w->componentsMaskList[e] |= COMPONENT_POSITION; 
    w->positionList[e] = new_pos;
}

void add_prev_position_component(World *w, uint32_t e, float x, float y) {
    Position new_pos = {{x, y}};
    w->componentsMaskList[e] |= COMPONENT_PREV_POSITION;
    w->prevPositionList[e] = new_pos;
}

void add_collider_component(World *w, uint32_t e, float width, float height) {
    Collider new_col = {width, height};
    w->componentsMaskList[e] |= COMPONENT_COLLIDER;
    w->colliderList[e] = new_col;
}

void add_sprite_component(World *w, uint32_t e, Assets::Atlas* atlas, uint32_t frame_index) {
    Sprite new_spr = {atlas, frame_index};
    w->componentsMaskList[e] |= COMPONENT_SPRITE;
    w->spriteList[e] = new_spr;
}

void add_sprite_component(World *w, uint32_t e, const char* atlas_name, uint32_t frame_index) {
    Assets::Atlas* atlas = Assets::Atlases.find(atlas_name);
    add_sprite_component(w, e, atlas, frame_index);
}

void add_velocity_component(World *w, uint32_t e, float vx, float vy) {
    Velocity new_vel = {{vx, vy}};
    w->componentsMaskList[e] |= COMPONENT_VELOCITY;
    w->velocityList[e] = new_vel;
}

void world_clear(World *w) {
    w->entityCount = 0;
    w->freeListCount = 0;
    for (size_t i = 0; i < w->capacity; i++) {
        w->componentsMaskList[i] = 0;
        w->status[i] = 0;
    }
}

void world_free(World *w) {
    if (!w) return;
    free(w->componentsMaskList);
    free(w->positionList);
    free(w->prevPositionList);
    free(w->colliderList);
    free(w->spriteList);
    free(w->velocityList);
    free(w->status);
    free(w->freeList);
    free(w);
}
