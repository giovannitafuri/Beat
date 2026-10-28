#ifndef LEVEL_H
#define LEVEL_H

#include "Entity.h"
#include <cstdio>

constexpr float CELL_SIZE = 64.0f;
constexpr uint32_t MAX_ENTITY_PER_CELL = 16;

struct Collision {
    Entity e1;
    Entity e2;
};

constexpr uint32_t MAX_COLLISIONS = 100000;
extern Collision CollisionsList[MAX_COLLISIONS];
extern uint32_t CollisionsCount;

inline int aabb_intersect(Position p1, Collider c1, Position p2, Collider c2) {
    LSMVECTOR pos1, pos2;
    LSMLoadVector(&p1, 2, &pos1);
    LSMLoadVector(&p2, 2, &pos2);

    return (pos1.x < pos2.x + c2.width  && pos1.x + c1.width  > pos2.x &&
            pos1.y < pos2.y + c2.height && pos1.y + c1.height > pos2.y);
}

void collisions_list_clear();
void collision_system(World *w);

#endif // LEVEL_H
