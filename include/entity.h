#ifndef ENTITY_H
#define ENTITY_H

#include <cstdio>
#include <cstring>

constexpr size_t START_ENTITY_NUM = 100;

typedef enum {
    COMPONENT_POSITION = 1 << 0
} ComponentType;

struct Position {
    float x;
    float y;
}

// RESOLVE: Resolve deleted entities
struct World {
    size_t      entityCount;
    uint32_t    *componentsMaskList; 
    Position    *positionList;
}

World *world_init() {
    World *new_world = malloc(sizeof(world));
    // TODO: check for error
    new_world->entityCount = 0;
    // TODO: use calloc for componentsMaskList
    new_world->componentsMaskList = malloc(sizeof(new_world->componentsMaskList) * START_ENTITY_NUM);
    new_world->positionList = malloc(sizeof(positionList->componentsMaskList) * START_ENTITY_NUM);
    
    return new_world;
} 

uint32_t create_entity(World *w) { return ++w->entityCount; }
    
void add_position_component(World *w, uint32_t e, float x, float y) {
    Position new_pos = {x, y};
    w->componentsMaskList[e] |= COMPONENT_POSITION; 
    w->positionList[e] = new_pos;
}

#endif ENTITY_H
