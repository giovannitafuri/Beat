#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include <SDL3/SDL.h>
#include "Entity.h"

void render_system(World* world, SDL_Renderer* renderer, float camera_x, float camera_y);

#endif // RENDER_SYSTEM_H
