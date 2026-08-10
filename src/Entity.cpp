#include "Entity.h"
#include <cstdlib>
#include <cstdio>

World *world_init() {
    World *new_world = (World*)malloc(sizeof(World));
    if (!new_world) return nullptr;

    new_world->capacity = START_ENTITY_NUM; 
    new_world->entityCount = 0;

    new_world->componentsMaskList = (uint32_t*)calloc(START_ENTITY_NUM, sizeof(uint32_t));
    new_world->positionList       = (Position*)malloc(sizeof(Position) * START_ENTITY_NUM);
    new_world->status             = (uint32_t*)malloc(sizeof(uint32_t) * START_ENTITY_NUM);
    new_world->colliderList       = (Collider*)malloc(sizeof(Collider) * START_ENTITY_NUM);
    new_world->spriteList         = (Sprite*)malloc(sizeof(Sprite) * START_ENTITY_NUM);
    
    if (!new_world->componentsMaskList || !new_world->positionList || !new_world->status || !new_world->colliderList || !new_world->spriteList) {
        free(new_world->componentsMaskList);
        free(new_world->positionList);
        free(new_world->status);
        free(new_world->colliderList);
        free(new_world->spriteList);
        free(new_world);
        return nullptr;
    }
    
    return new_world;
} 

bool world_resize(World *w) {
    size_t new_capacity = w->capacity * 2;

    uint32_t *new_mask     = (uint32_t*)realloc(w->componentsMaskList, sizeof(uint32_t) * new_capacity);
    Position *new_pos      = (Position*)realloc(w->positionList, sizeof(Position) * new_capacity);
    uint32_t *new_status   = (uint32_t*)realloc(w->status, sizeof(uint32_t) * new_capacity);
    Collider *new_collider = (Collider*)realloc(w->colliderList, sizeof(Collider) * new_capacity);
    Sprite   *new_sprite   = (Sprite*)realloc(w->spriteList, sizeof(Sprite) * new_capacity);

    if (!new_mask || !new_pos || !new_status || !new_collider || !new_sprite) {
        // I don't free the new reallocated arrays because if realloc fails
        // on these small arrays, the OS is out of memory and we are going to crash anyway.
        return false;
    }

    w->componentsMaskList = new_mask;
    w->positionList = new_pos;
    w->status = new_status;
    w->colliderList = new_collider;
    w->spriteList = new_sprite;
    
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
    if (w->entityCount + 1 >= w->capacity) {
        if (!world_resize(w)) {
            fprintf(stderr, "Error resizing world!\n");
        }
    }
    w->entityCount++;
    Entity new_entity = { w->entityCount, w->status[w->entityCount] };
    return new_entity;
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

void add_sprite_component(World *w, uint32_t e, const char* atlas_name, int frame_index) {
    Sprite new_spr = {atlas_name, frame_index};
    w->componentsMaskList[e] |= COMPONENT_SPRITE;
    w->spriteList[e] = new_spr;
}
