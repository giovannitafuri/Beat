#include "Entity.h"
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

    new_world->capacity = START_ENTITY_NUM; 
    // Entity ID 0 is reserved as the sentinel (NULL_ENTITY / no selection).
    // Initializing entityCount to 0 ensures the first created entity receives ID 1.
    new_world->entityCount = 0;
    new_world->freeListCount = 0;

    new_world->componentsMaskList = (uint64_t*)calloc(START_ENTITY_NUM, sizeof(uint64_t));
    new_world->positionList       = (Position*)malloc(sizeof(Position) * START_ENTITY_NUM);
    new_world->status             = (uint32_t*)malloc(sizeof(uint32_t) * START_ENTITY_NUM);
    new_world->colliderList       = (Collider*)malloc(sizeof(Collider) * START_ENTITY_NUM);
    new_world->spriteList         = (Sprite*)malloc(sizeof(Sprite) * START_ENTITY_NUM);
    new_world->velocityList       = (Velocity*)malloc(sizeof(Velocity) * START_ENTITY_NUM);
    new_world->freeList           = (size_t*)malloc(sizeof(size_t) * START_ENTITY_NUM);
    
    if (!new_world->componentsMaskList || !new_world->positionList || !new_world->status || !new_world->colliderList || !new_world->spriteList || !new_world->velocityList || !new_world->freeList) {
        free(new_world->componentsMaskList);
        free(new_world->positionList);
        free(new_world->status);
        free(new_world->colliderList);
        free(new_world->spriteList);
        free(new_world->velocityList);
        free(new_world->freeList);
        free(new_world);
        ASSERT(false, "Failed to allocate World component arrays");
        return nullptr;
    }
    
    return new_world;
} 

bool world_resize(World *w) {
    size_t new_capacity = w->capacity * 2;

    uint64_t *new_mask      = (uint64_t*)realloc(w->componentsMaskList, sizeof(uint64_t) * new_capacity);
    Position *new_pos       = (Position*)realloc(w->positionList, sizeof(Position) * new_capacity);
    uint32_t *new_status    = (uint32_t*)realloc(w->status, sizeof(uint32_t) * new_capacity);
    Collider *new_collider  = (Collider*)realloc(w->colliderList, sizeof(Collider) * new_capacity);
    Sprite   *new_sprite    = (Sprite*)realloc(w->spriteList, sizeof(Sprite) * new_capacity);
    Velocity *new_velocity  = (Velocity*)realloc(w->velocityList, sizeof(Velocity) * new_capacity);
    size_t   *new_free_list = (size_t*)realloc(w->freeList, sizeof(size_t) * new_capacity);

    if (!new_mask || !new_pos || !new_status || !new_collider || !new_sprite || !new_velocity || !new_free_list) {
        // I don't free the new reallocated arrays because if realloc fails
        // on these small arrays, the OS is out of memory and we are going to crash anyway.
        ASSERT(false, "Failed to reallocate World arrays");
        return false;
    }

    w->componentsMaskList = new_mask;
    w->positionList = new_pos;
    w->status = new_status;
    w->colliderList = new_collider;
    w->spriteList = new_sprite;
    w->velocityList = new_velocity;
    w->freeList = new_free_list;
    
    // realloc doesn't zero-initialize the new memory. We only really need to zero the mask.
    for (size_t i = w->capacity; i < new_capacity; i++) {
        w->componentsMaskList[i] = 0;
        w->status[i] = 0;
    }

    w->capacity = new_capacity;
    return true;
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
    if (w->entityCount + 1 >= w->capacity) {
        bool resized = world_resize(w);
        ASSERT(resized, "Error resizing world!");
        if (!resized) {
            // Keep going, although we might crash soon if we exceed capacity.
            // Ideally we'd return a null entity here, but for now we just log an assert.
        }
    }
    w->entityCount++;
    Entity new_entity = { w->entityCount, w->status[w->entityCount] };
    return new_entity;
}
/* the compenents remain in their respective lists for performance
 * status and components are set as 0 (INDICATOR OF FREE)
 * the id is set in a free list entity;
*/
void delete_entity(World* w, Entity e) {
    w->componentsMaskList[e.id] = 0;
    w->status[e.id]++;
    free_list_add(w, e.id);
}
    
void add_position_component(World *w, uint32_t e, float x, float y) {
    Position new_pos = {x, y};
    w->componentsMaskList[e] |= COMPONENT_POSITION; 
    w->positionList[e] = new_pos;
}

void add_collider_component(World *w, uint32_t e, float width, float height) {
    Collider new_col = {width, height};
    w->componentsMaskList[e] |= COMPONENT_COLLIDER;
    w->colliderList[e] = new_col;
}

void add_sprite_component(World *w, uint32_t e, const char* atlas_name, uint64_t frame_index) {
    Sprite new_spr = {atlas_name, frame_index};
    w->componentsMaskList[e] |= COMPONENT_SPRITE;
    w->spriteList[e] = new_spr;
}

void add_velocity_component(World *w, uint32_t e, float vx, float vy) {
    Velocity new_vel = {vx, vy};
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
    free(w->colliderList);
    free(w->spriteList);
    free(w->velocityList);
    free(w->status);
    free(w->freeList);
    free(w);
}
