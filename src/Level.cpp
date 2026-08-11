#include "Level.h"
#include <cstdio>
#include <cstdlib>

Collision CollisionsList[MAX_COLLISIONS];
uint32_t CollisionsCount = 0;

void collisions_list_clear() {
    CollisionsCount = 0;
}

void clear_grid(SpatialGrid *g) {
    for (uint32_t i = 0; i < g->width; i++) {
        for (uint32_t j = 0; j < g->height; j++) {
            g->cells[j * g->width + i].count = 0;
        }
    }
}

void grid_insert(SpatialGrid *g, World *w, Entity e) {
    uint32_t cx = 0;
    uint32_t cy = 0;
    Position entityPos = w->positionList[e.id];
    world_to_cell(entityPos.x, entityPos.y, &cx, &cy);

    // Unsigned comparison handles < 0 automatically
    if (cx >= g->width || cy >= g->height) return;
    
    GridCell *cell = &g->cells[cy * g->width + cx];
    if (cell->count < MAX_ENTITY_PER_CELL) {
        cell->entities[cell->count++] = e;
    }
}    

void grid_rebuild(SpatialGrid *g, World *w) {
    clear_grid(g);
    for (size_t i = 1; i <= w->entityCount; i++) {
        // Only insert entities that actually have a collider
        if (w->componentsMaskList[i] & COMPONENT_COLLIDER) {
            grid_insert(g, w, get_entity(w, i)); 
        }
    }
}

void collision_system_grid(World *w, SpatialGrid *g) {
    for (uint32_t cx = 0; cx < g->width; cx++) {
        for (uint32_t cy = 0; cy < g->height; cy++) {
            
            // Check current cell and its 8 neighbors
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    int nx = (int)cx + dx;
                    int ny = (int)cy + dy;
                    
                    if (nx < 0 || nx >= (int)g->width || ny < 0 || ny >= (int)g->height)
                        continue;

                    GridCell *cellA = &g->cells[cy * g->width + cx];
                    GridCell *cellB = &g->cells[ny * g->width + nx];

                    for (uint32_t i = 0; i < cellA->count; i++) {
                        for (uint32_t j = 0; j < cellB->count; j++) {
                            uint64_t idA = cellA->entities[i].id;
                            uint64_t idB = cellB->entities[j].id;

                            // Prevent comparing an entity against itself or doing duplicate checks
                            if (idA >= idB) continue; 

                            if (aabb_intersect(w->positionList[idA], w->colliderList[idA],
                                               w->positionList[idB], w->colliderList[idB])) {
                                if (CollisionsCount < MAX_COLLISIONS) {
                                    Collision c = {get_entity(w, idA), get_entity(w, idB)};
                                    CollisionsList[CollisionsCount++] = c;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// Parses a minimal text file containing:
// TITLE: "My Level"
// WIDTH: n HEIGHT: n
bool load_level(const char* filepath, SpatialGrid* grid) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open level file %s\n", filepath);
        return false;
    }

    char title[256] = {0};
    if (fscanf(file, "TITLE: \"%255[^\"]\"\n", title) != 1) {
        fprintf(stderr, "Error: Expected TITLE in %s\n", filepath);
        fclose(file);
        return false;
    }

    uint32_t width = 0, height = 0;
    if (fscanf(file, "WIDTH: %u HEIGHT: %u\n", &width, &height) != 2) {
        fprintf(stderr, "Error: Expected WIDTH and HEIGHT in %s\n", filepath);
        fclose(file);
        return false;
    }

    fclose(file);

    grid->width = width;
    grid->height = height;
    grid->cells = (GridCell*)calloc(width * height, sizeof(GridCell));

    if (!grid->cells) {
        fprintf(stderr, "Error: Failed to allocate grid cells\n");
        return false;
    }

    printf("Level '%s' loaded. Grid size: %ux%u\n", title, width, height);
    return true;
}
