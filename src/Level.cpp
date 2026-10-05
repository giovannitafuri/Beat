#include "Level.h"
#include <cstdio>
#include <cstdlib>
#include "Assert.h"

Collision CollisionsList[MAX_COLLISIONS];
uint32_t CollisionsCount = 0;

void collisions_list_clear() {
    CollisionsCount = 0;
}

void collision_system(World *w) {
    CollisionsCount = 0;
    for (size_t i = 1; i <= w->entityCount; ++i) {
        if (!(w->componentsMaskList[i] & COMPONENT_COLLIDER)) continue;

        for (size_t j = i + 1; j <= w->entityCount; ++j) {
            if (!(w->componentsMaskList[j] & COMPONENT_COLLIDER)) continue;

            if (aabb_intersect(w->positionList[i], w->colliderList[i],
                               w->positionList[j], w->colliderList[j])) {
                if (CollisionsCount < MAX_COLLISIONS) {
                    Collision c = { get_entity(w, i), get_entity(w, j) };
                    CollisionsList[CollisionsCount++] = c;
                }
            }
        }
    }
}
