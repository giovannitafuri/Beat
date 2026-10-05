#ifndef LEVEL_H
#define LEVEL_H

#include "Entity.h"
#include <cstdio>

constexpr float CELL_SIZE = 64.0f;
constexpr uint32_t MAX_ENTITY_PER_CELL = 16;

// THINK: should i save time os simpli print it in debug?
struct Collision {
    Entity e1;
    Entity e2;
};

constexpr uint32_t MAX_COLLISIONS = 1024;
extern Collision CollisionsList[MAX_COLLISIONS];
extern uint32_t CollisionsCount;

inline int aabb_intersect(Position p1, Collider c1, Position p2, Collider c2) {
    return (p1.x < p2.x + c2.width  && p1.x + c1.width  > p2.x &&
            p1.y < p2.y + c2.height && p1.y + c1.height > p2.y);
}

void collisions_list_clear();
void collision_system(World *w);

#endif // LEVEL_H
