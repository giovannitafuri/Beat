#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include <SDL3/SDL.h>
#include "Entity.h"
#include "Camera.h"

struct BatchRenderer;

void render_system(World* world, SDL_Renderer* renderer, Camera* camera, BatchRenderer* batch);

#endif // RENDER_SYSTEM_H
