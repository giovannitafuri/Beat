#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "Entity.h"
#include "Settings.h"
#include "Camera.h"

void movement_system(World *w);
inline void camera_system(Camera *c, World *w, Entity *e) {
    // For now i don't check if e->status == w->status because if player dies the game stops
    Position p = w->positionList[e->id];
    c->x = p.x;
    c->y = p.y;
}

#endif // SYSTEMS_H
