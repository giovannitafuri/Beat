#include <SDL3/SDL.h>
#include <cstdio>
#include <cstdlib>
#include "Entity.h"
#include "BeatTime.h"
#include "Assets.h"
#include "Level.h"
#include "Event.h"
#include "Editor.h"
#include "RenderSystem.h"
#include "Camera.h"
#include "Input.h"
#include "Systems.h"
#include "Settings.h"
#include "Assert.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

float Time::Delta = 0.0f;

struct EngineContext {
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool is_running;
};

bool engine_init(EngineContext* ctx) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        ASSERT(false, "SDL_Init Error");
        return false;
    }

    ctx->window = SDL_CreateWindow("Beat Engine", GlobalSettings.screenWidth, GlobalSettings.screenHeight, 0);
    if (!ctx->window) {
        ASSERT(false, "SDL_CreateWindow Error");
        SDL_Quit();
        return false;
    }

    ctx->renderer = SDL_CreateRenderer(ctx->window, nullptr);
    if (!ctx->renderer) {
        ASSERT(false, "SDL_CreateRenderer Error");
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
    init_event_machine();

    EditorState editor_state;
    editor_init(ctx.window, ctx.renderer, &editor_state);

    Camera main_camera = {0.0f, 0.0f, 1.0f};

    Entity player = create_entity(world);
    add_position_component(world, player.id, 400.0f, 300.0f);
    world->componentsMaskList[player.id] |= COMPONENT_PLAYER;
    add_velocity_component(world, player.id, 0.0f, 0.0f);
    add_collider_component(world, player.id, 32.0f, 32.0f);

    uint64_t last_time = SDL_GetPerformanceCounter();
    uint64_t freq = SDL_GetPerformanceFrequency();

    SDL_Event event;
    while (ctx.is_running) {
        uint64_t current_time = SDL_GetPerformanceCounter();
        Time::Delta = (float)((double)(current_time - last_time) / (double)freq);
        last_time = current_time;

        while (SDL_PollEvent(&event)) {
            editor_process_event(&event, &editor_state, world, &main_camera);
            
            if (event.type == SDL_EVENT_QUIT) {
                ctx.is_running = false;
            }
        }

        input_update();

        editor_update(&editor_state, world, &main_camera);
        
        // --- Physics & Logic Update ---
        if (!editor_state.is_editor_mode) {
            movement_system(world);
            
            collision_system(world);
            resolve_collision_system(world);
            camera_movement(world, &main_camera);
            collisions_list_clear();
        }

        // --- Render ---
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        editor_draw_ui(&editor_state, world, &main_camera);

        SDL_SetRenderDrawColor(ctx.renderer, 30, 30, 30, 255);
        SDL_RenderClear(ctx.renderer);

        render_system(world, ctx.renderer, &main_camera);
        
        editor_draw_overlay(&editor_state, world, ctx.renderer, &main_camera);

        // Render ImGui on top of everything
        ImGui::Render();
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), ctx.renderer);
        
        SDL_RenderPresent(ctx.renderer);
        
        // --- End of Frame ---
        events_machine_clear();
    }

    world_free(world);

    editor_shutdown();
    Assets::shutdown();

    engine_shutdown(&ctx);
    return 0;
}
