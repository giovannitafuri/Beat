#include "entity.h"
#include <cstdlib>

World *world_init() {
    World *new_world = (World*)malloc(sizeof(World));
    if (!new_world) return nullptr;

    new_world->capacity = START_ENTITY_NUM; 
    new_world->entityCount = 0;

    new_world->componentsMaskList = (uint32_t*)calloc(START_ENTITY_NUM, sizeof(uint32_t));
    new_world->positionList       = (Position*)malloc(sizeof(Position) * START_ENTITY_NUM);
    new_world->status             = (uint8_t*)malloc(sizeof(uint8_t) * START_ENTITY_NUM);
    new_world->colliderList       = (Collider*)malloc(sizeof(Collider) * START_ENTITY_NUM);
    
    if (!new_world->componentsMaskList || !new_world->positionList || !new_world->status || !new_world->colliderList) {
        free(new_world->componentsMaskList);
        free(new_world->positionList);
        free(new_world->status);
        free(new_world->colliderList);
        free(new_world);
        return nullptr;
    }
    
    return new_world;
} 

Entity get_entity(World *w, uint64_t id) {
    Entity new_entity = { id, w->status[id] };
    return new_entity;
}

Entity create_entity(World *w) { 
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
