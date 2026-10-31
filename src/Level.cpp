#include "Level.h"
#include <cstdio>
#include <cstdlib>
#include "Assert.h"

Collision CollisionsList[MAX_COLLISIONS];
uint32_t CollisionsCount = 0;

uint32_t CellHeads[TOTAL_CELLS];
uint32_t EntityNext[ENTITIES_NUM];

void collisions_list_clear() {
    CollisionsCount = 0;
}

void collision_system(World *w) {
    CollisionsCount = 0;
    // Celar Grid
    memset(CellHeads, 0, sizeof(CellHeads));

    for (size_t e = 1; e <= w->entityCount; e++) {
        if (!(w->componentsMaskList[e] & COMPONENT_COLLIDER)) continue;
        int cx = (int)(w->positionList[e].data[0] / CELL_SIZE);
        int cy = (int)(w->positionList[e].data[1] / CELL_SIZE);

        if (cx < 0) cx = 0;
        if (cx >= (int)GRID_COLS) cx = (int)GRID_COLS - 1;

        if (cy < 0) cy = 0;
        if (cy >= (int)GRID_ROWS) cy = (int)GRID_ROWS - 1;

        uint32_t cell_idx = cy * GRID_COLS + cx;

        // add new entity
        EntityNext[e] = CellHeads[cell_idx];
        CellHeads[cell_idx] = e;
    }

    for (size_t e1 = 1; e1 <= w->entityCount; e1++) {

        int cx = (int)(w->positionList[e1].data[0] / CELL_SIZE);
        int cy = (int)(w->positionList[e1].data[1] / CELL_SIZE);

        if (!(w->componentsMaskList[e1] & COMPONENT_COLLIDER)) continue;
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                int nx = cx + dx;
                int ny = cy + dy;

                // Check borders
                if (nx < 0 || nx >= (int)GRID_COLS || ny < 0 || ny >= (int)GRID_ROWS) continue;

                uint32_t neighbor_idx = ny * GRID_COLS + nx;
                uint32_t e2 = CellHeads[neighbor_idx];

                // Loop over the linked list
                while (e2 != 0) {
                    if (e2 > e1) {
                        if (aabb_intersect(w->positionList[e1], w->colliderList[e1], w->positionList[e2], w->colliderList[e2])) {
                                if (CollisionsCount < MAX_COLLISIONS) {
                                    CollisionsList[CollisionsCount++] = { get_entity(w, e1), get_entity(w, e2) };
                                }
                            }
                        }
                    e2 = EntityNext[e2]; // Next cell
                }
            }
        }
    }
}
