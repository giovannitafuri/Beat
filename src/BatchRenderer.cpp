#include "BatchRenderer.h"
#include "Assert.h"
#include <cstdlib>

BatchRenderer* batch_create(size_t max_quads) {
    BatchRenderer* batch = (BatchRenderer*)malloc(sizeof(BatchRenderer));
    ASSERT(batch != nullptr, "batch_create malloc failed");
    if (!batch) return nullptr;

    batch->vertices = (SDL_Vertex*)malloc(max_quads * 4 * sizeof(SDL_Vertex));
    batch->indices = (int*)malloc(max_quads * 6 * sizeof(int));
    ASSERT(batch->vertices != nullptr, "batch vertices malloc failed");
    ASSERT(batch->indices != nullptr, "batch indices malloc failed");

    if (!batch->vertices || !batch->indices) {
        free(batch->vertices);
        free(batch->indices);
        free(batch);
        return nullptr;
    }

    batch->quad_count = 0;
    batch->max_quads = max_quads;
    batch->current_texture = nullptr;

    for (size_t i = 0; i < max_quads; ++i) {
        size_t v = i * 4;
        size_t idx = i * 6;

        batch->indices[idx + 0] = (int)(v + 0);
        batch->indices[idx + 1] = (int)(v + 1);
        batch->indices[idx + 2] = (int)(v + 2);

        batch->indices[idx + 3] = (int)(v + 2);
        batch->indices[idx + 4] = (int)(v + 3);
        batch->indices[idx + 5] = (int)(v + 0);
    }

    return batch;
}

void batch_shutdown(BatchRenderer* batch) {
    if (!batch) return;
    free(batch->vertices);
    free(batch->indices);
    free(batch);
}

void batch_flush(BatchRenderer* batch, SDL_Renderer* renderer) {
    if (!batch || batch->quad_count == 0) return;

    SDL_RenderGeometry(renderer,
                       batch->current_texture,
                       batch->vertices,
                       (int)(batch->quad_count * 4),
                       batch->indices,
                       (int)(batch->quad_count * 6));

    batch->quad_count = 0;
}

void batch_draw_sprite(BatchRenderer* batch, SDL_Renderer* renderer,
                       LSMVECTOR pos_size, SDL_Texture* texture, LSMVECTOR uv, SDL_FColor color) {
    if (batch->current_texture != texture) {
        if (batch->quad_count > 0) {
            batch_flush(batch, renderer);
        }
        batch->current_texture = texture;
    }

    if (batch->quad_count >= batch->max_quads) {
        batch_flush(batch, renderer);
    }

    size_t idx = batch->quad_count * 4;

    batch->vertices[idx + 0] = { { pos_size.x, pos_size.y }, color, { uv.x, uv.y } };
    batch->vertices[idx + 1] = { { pos_size.x + pos_size.z, pos_size.y }, color, { uv.z, uv.y } };
    batch->vertices[idx + 2] = { { pos_size.x + pos_size.z, pos_size.y + pos_size.w }, color, { uv.z, uv.w } };
    batch->vertices[idx + 3] = { { pos_size.x, pos_size.y + pos_size.w }, color, { uv.x, uv.w } };

    batch->quad_count++;
}

void batch_draw_rect(BatchRenderer* batch, SDL_Renderer* renderer,
                     LSMVECTOR pos_size, SDL_FColor color) {
    LSMVECTOR dummy_uv = { 0.0f, 0.0f, 0.0f, 0.0f };
    batch_draw_sprite(batch, renderer, pos_size, nullptr, dummy_uv, color);
}
