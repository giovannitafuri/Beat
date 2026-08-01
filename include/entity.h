#ifndef ENTITY_H
#define ENTITY_H

#include <cstdio>
#include <cstring>
#include <cstdlib>

constexpr size_t START_ENTITY_NUM = 100;

typedef enum {
    COMPONENT_POSITION = 1 << 0
} ComponentType;

struct Position {
    float x;
    float y;
};

// THINK: Resolve deleted entities
struct World {
    size_t      entityCount;
    uint32_t    *componentsMaskList; 
    Position    *positionList;
};

World *world_init() {
    World *new_world = (World*)malloc(sizeof(World));
    if (!new_world) return nullptr;
    
    new_world->entityCount = 0;
    
    new_world->componentsMaskList = (uint32_t*)calloc(START_ENTITY_NUM, sizeof(uint32_t));
    new_world->positionList = (Position*)malloc(sizeof(Position) * START_ENTITY_NUM);
    
    if (!new_world->componentsMaskList || !new_world->positionList) {
        free(new_world->componentsMaskList);
        free(new_world->positionList);
        free(new_world);
        return nullptr;
    }
    
    return new_world;
} 

uint32_t create_entity(World *w) { return ++w->entityCount; }
    
void add_position_component(World *w, uint32_t e, float x, float y) {
    Position new_pos = {x, y};
    w->componentsMaskList[e] |= COMPONENT_POSITION; 
    w->positionList[e] = new_pos;
}

#endif // ENTITY_H
