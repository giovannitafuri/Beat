#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "Entity.h"
#include "Settings.h"
#include "Camera.h"
#include "Level.h"

void movement_system(World *w);
inline void camera_system(Camera *c, World *w, Entity *e) {
    // Player status check skipped: the game stops when the player dies.
    LSMVECTOR p;
    LSMLoadVector(&w->positionList[e->id], 2, &p);
    c->x = p.x;
    c->y = p.y;
}

void resolve_collision_system(World *w);

#endif // SYSTEMS_H
