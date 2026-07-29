#include <SDL3/SDL.h>
#include <cstdio>

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

    SDL_Event event;
    while (ctx.is_running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                ctx.is_running = false;
            }
        }

        // --- Render ---
        // Gray Background 
        SDL_SetRenderDrawColor(ctx.renderer, 30, 30, 30, 255);
        SDL_RenderClear(ctx.renderer);

        // TODO: Update and Draw 

        SDL_RenderPresent(ctx.renderer);
    }

    engine_shutdown(&ctx);
    return 0;
}
