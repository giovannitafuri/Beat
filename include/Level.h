#ifndef LEVEL_H
#define LEVEL_H



#include "Entity.h"


constexpr float CELL_SIZE = 64.0f;
constexpr uint32_t MAX_ENTITY_PER_CELL = 16;

// THINK: should i save time?
struct Collision {
    Entity e1;
    Entity e2;
};

constexpr uint32_t MAX_COLLISIONS = 1024;
extern Collision collisionsList[MAX_COLLISIONS];
extern uint32_t collisionsCount;


struct GridCell {
    Entity entities[MAX_ENTITY_PER_CELL];
    uint32_t count;
};

// Spatial grid for broad-phase collision detection.
// Divides the world into uniform cells to avoid O(N^2) checks.
struct SpatialGrid {
    GridCell *cells; // 1D array representing a 2D grid
    uint32_t width;
    uint32_t height;
};

inline int aabb_intersect(Position p1, Collider c1, Position p2, Collider c2) {
    return (p1.x < p2.x + c2.width  && p1.x + c1.width  > p2.x &&
            p1.y < p2.y + c2.height && p1.y + c1.height > p2.y);
}

inline void world_to_cell(float wx, float wy, uint32_t *cx, uint32_t *cy) {
    *cx = (uint32_t)(wx / CELL_SIZE);
    *cy = (uint32_t)(wy / CELL_SIZE);
}

void collisions_list_clear();
void clear_grid(SpatialGrid *g);
void grid_insert(SpatialGrid *g, World *w, Entity e);
void grid_rebuild(SpatialGrid *g, World *w);
void collision_system_grid(World *w, SpatialGrid *g);
void collisions_list_clear();
bool load_level(const char* filepath, SpatialGrid* grid);

#endif // LEVEL_H
