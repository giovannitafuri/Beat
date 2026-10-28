#ifndef BATCH_RENDERER_H
#define BATCH_RENDERER_H

#include <stddef.h>
#include <stdint.h>
#include <SDL3/SDL.h>
#include "LSMath.h"

struct BatchRenderer {
    SDL_Vertex*  vertices;
    int*         indices;
    size_t       quad_count;
    size_t       max_quads;
    SDL_Texture* current_texture;
};

BatchRenderer* batch_create(size_t max_quads);
void batch_shutdown(BatchRenderer* batch);
void batch_flush(BatchRenderer* batch, SDL_Renderer* renderer);
void batch_draw_sprite(BatchRenderer* batch, SDL_Renderer* renderer,
                       LSMVECTOR pos_size, SDL_Texture* texture, LSMVECTOR uv, SDL_FColor color);
void batch_draw_rect(BatchRenderer* batch, SDL_Renderer* renderer,
                     LSMVECTOR pos_size, SDL_FColor color);

#endif
