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
    memset(CellHeads, 0, sizeof(CellHeads));

    constexpr float inv_cell = 1.0f / (float)CELL_SIZE;

    for (size_t e = 1; e <= w->entityCount; e++) {
        if (!(w->componentsMaskList[e] & COMPONENT_COLLIDER)) continue;
        int cx = (int)(w->positionList[e].data[0] * inv_cell);
        int cy = (int)(w->positionList[e].data[1] * inv_cell);

        if ((unsigned)cx >= GRID_COLS) cx = (cx < 0) ? 0 : (int)GRID_COLS - 1;
        if ((unsigned)cy >= GRID_ROWS) cy = (cy < 0) ? 0 : (int)GRID_ROWS - 1;

        uint32_t cell_idx = (uint32_t)(cy * GRID_COLS + cx);

        EntityNext[e] = CellHeads[cell_idx];
        CellHeads[cell_idx] = (uint32_t)e;
    }

    for (size_t e1 = 1; e1 <= w->entityCount; e1++) {
        if (!(w->componentsMaskList[e1] & COMPONENT_COLLIDER)) continue;

        float e1_x = w->positionList[e1].data[0];
        float e1_y = w->positionList[e1].data[1];
        float e1_w = w->colliderList[e1].width;
        float e1_h = w->colliderList[e1].height;
        float e1_r = e1_x + e1_w;
        float e1_b = e1_y + e1_h;

        int cx = (int)(e1_x * inv_cell);
        int cy = (int)(e1_y * inv_cell);

        if ((unsigned)cx >= GRID_COLS) cx = (cx < 0) ? 0 : (int)GRID_COLS - 1;
        if ((unsigned)cy >= GRID_ROWS) cy = (cy < 0) ? 0 : (int)GRID_ROWS - 1;

        for (int dy = -1; dy <= 1; ++dy) {
            int ny = cy + dy;
            if ((unsigned)ny >= GRID_ROWS) continue;
            uint32_t row_offset = (uint32_t)ny * GRID_COLS;

            for (int dx = -1; dx <= 1; ++dx) {
                int nx = cx + dx;
                if ((unsigned)nx >= GRID_COLS) continue;

                uint32_t neighbor_idx = row_offset + (uint32_t)nx;
                uint32_t e2 = CellHeads[neighbor_idx];

                while (e2 != 0) {
                    if (e2 > e1) {
                        float e2_x = w->positionList[e2].data[0];
                        float e2_y = w->positionList[e2].data[1];
                        float e2_w = w->colliderList[e2].width;
                        float e2_h = w->colliderList[e2].height;

                        if (e1_x < e2_x + e2_w && e1_r > e2_x &&
                            e1_y < e2_y + e2_h && e1_b > e2_y) {
                            if (CollisionsCount < MAX_COLLISIONS) {
                                CollisionsList[CollisionsCount++] = { (uint32_t)e1, (uint32_t)e2 };
                            }
                        }
                    }
                    e2 = EntityNext[e2];
                }
            }
        }
    }
}
