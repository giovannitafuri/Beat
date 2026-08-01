#include <SDL3/SDL.h>
#include <cstdio>
#include "entity.h"
#include "beat_time.h"
#include "assets.h"

float Time::delta = 0.0f;

struct EngineContext {
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool is_running;
};

bool engine_init(EngineContext* ctx) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return false;
    }

    ctx->window = SDL_CreateWindow("Beat Engine", 800, 600, 0);
    if (!ctx->window) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    ctx->renderer = SDL_CreateRenderer(ctx->window, nullptr);
    if (!ctx->renderer) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(ctx->window);
        SDL_Quit();
        return false;
    }

    ctx->is_running = true;
    return true;
}

void engine_shutdown(EngineContext* ctx) {
    if (ctx->renderer) {
        SDL_DestroyRenderer(ctx->renderer);
        ctx->renderer = nullptr;
    }
    if (ctx->window) {
        SDL_DestroyWindow(ctx->window);
        ctx->window = nullptr;
    }
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    EngineContext ctx = {nullptr, nullptr, false};

    if (!engine_init(&ctx)) {
        return 1;
    }

    World* world = world_init();
    if (!world) {
        engine_shutdown(&ctx);
        return 1;
    }

    Assets::init(ctx.renderer);

    uint32_t player = create_entity(world);
    add_position_component(world, player, 400.0f, 300.0f);

    uint64_t last_time = SDL_GetPerformanceCounter();
    uint64_t freq = SDL_GetPerformanceFrequency();

    SDL_Event event;
    while (ctx.is_running) {
        uint64_t current_time = SDL_GetPerformanceCounter();
        Time::delta = (float)((double)(current_time - last_time) / (double)freq);
        last_time = current_time;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                ctx.is_running = false;
            }
        }

        SDL_SetRenderDrawColor(ctx.renderer, 30, 30, 30, 255);
        SDL_RenderClear(ctx.renderer);

        for (size_t i = 1; i <= world->entityCount; ++i) {
            if (world->componentsMaskList[i] & COMPONENT_POSITION) {
                SDL_FRect rect;
                rect.x = world->positionList[i].x - 10.0f;
                rect.y = world->positionList[i].y - 10.0f;
                rect.w = 20.0f;
                rect.h = 20.0f;
                SDL_SetRenderDrawColor(ctx.renderer, 255, 0, 0, 255);
                SDL_RenderFillRect(ctx.renderer, &rect);
            }
        }

        SDL_RenderPresent(ctx.renderer);
    }

    free(world->componentsMaskList);
    free(world->positionList);
    free(world);

    Assets::shutdown();
    engine_shutdown(&ctx);
    return 0;
}
