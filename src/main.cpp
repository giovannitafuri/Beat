#include <SDL3/SDL.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "entity.h"
#include "beat_time.h"
#include "assets.h"
#include "level.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForSDLRenderer(ctx.window, ctx.renderer);
    ImGui_ImplSDLRenderer3_Init(ctx.renderer);

    bool is_editor_mode = true;
    bool draw_colliders_mode = false;
    uint32_t selected_entity_id = 0;
    
    SpatialGrid grid = { (GridCell*)calloc(64 * 64, sizeof(GridCell)), 64, 64 };
    
    float camera_x = 0;
    float camera_y = 0;
    
    // Rectangle drawing state
    bool is_drawing_rect = false;
    float rect_start_x = 0;
    float rect_start_y = 0;

    // Track the currently dragged asset
    const char* dragged_asset = nullptr;

    Entity player = create_entity(world);
    add_position_component(world, player.id, 400.0f, 300.0f);

    uint64_t last_time = SDL_GetPerformanceCounter();
    uint64_t freq = SDL_GetPerformanceFrequency();

    SDL_Event event;
    while (ctx.is_running) {
        uint64_t current_time = SDL_GetPerformanceCounter();
        Time::delta = (float)((double)(current_time - last_time) / (double)freq);
        last_time = current_time;

        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT) {
                ctx.is_running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_TAB) {
                is_editor_mode = !is_editor_mode;
            }

            // Custom manual drawing for colliders (rectangles)
            if (is_editor_mode && !io.WantCaptureMouse) {
                // Panning with Middle Mouse Button
                if (event.type == SDL_EVENT_MOUSE_MOTION) {
                    if (event.motion.state & SDL_BUTTON_MMASK) {
                        camera_x -= event.motion.xrel;
                        camera_y -= event.motion.yrel;
                    }
                }
                
                // Custom manual drawing for colliders (rectangles)
                if (draw_colliders_mode) {
                    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT) {
                        is_drawing_rect = true;
                        rect_start_x = event.button.x + camera_x;
                        rect_start_y = event.button.y + camera_y;
                    }
                    else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_LEFT) {
                        if (is_drawing_rect) {
                            is_drawing_rect = false;
                            float end_x = event.button.x + camera_x;
                            float end_y = event.button.y + camera_y;
                            
                            float x = (rect_start_x < end_x) ? rect_start_x : end_x;
                            float y = (rect_start_y < end_y) ? rect_start_y : end_y;
                            float w = (rect_start_x < end_x) ? end_x - rect_start_x : rect_start_x - end_x;
                            float h = (rect_start_y < end_y) ? end_y - rect_start_y : rect_start_y - end_y;
                            
                            if (w > 5.0f && h > 5.0f) {
                                Entity e = create_entity(world);
                                add_position_component(world, e.id, x, y);
                                add_collider_component(world, e.id, w, h);
                            }
                        }
                    }
                }
            }
        }

        // Handle Drag & Drop release from ImGui onto the game view
        if (is_editor_mode) {
            if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                // The mouse was just released or is up
                if (dragged_asset) {
                    // Check if released over game view (not hovering any ImGui window)
                    if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
                        Entity e = create_entity(world);
                        // Convert mouse coords to world coords
                        add_position_component(world, e.id, io.MousePos.x + camera_x, io.MousePos.y + camera_y);
                        add_sprite_component(world, e.id, dragged_asset, 0);
                    }
                }
                dragged_asset = nullptr; // Clear it out when mouse is released
            }
        }
        
        // --- Physics & Logic Update ---
        if (!is_editor_mode) {
            grid_rebuild(&grid, world);
            collision_system_grid(world, &grid);
            
            for (uint32_t i = 0; i < collisionsCount; i++) {
                // Collision c = collisionsList[i];
                // TODO: Handle entity specific resolution (e.g. wall sliding, damage)
            }
            
            collisions_list_clear();
        }

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        if (is_editor_mode) {
            ImGui::Begin("Beat Engine - Editor");
            ImGui::Text("Editor Mode Active");
            ImGui::Text("Press TAB to toggle Play mode");
            ImGui::Text("Delta Time: %.4f", (double)Time::delta);
            ImGui::Text("Entities: %zu", world->entityCount);
            
            ImGui::Separator();
            ImGui::Checkbox("Draw colliders mode", &draw_colliders_mode);
            ImGui::Separator();
            ImGui::Text("Asset Browser");
            ImGui::Text("Drag images to place them in the world!");

            for (int i = 0; i < Assets::loaded_atlas_count; i++) {
                const char* atlas_name = Assets::loaded_atlas_names[i];
                Assets::Atlas* a = Assets::atlases.find(atlas_name);
                if (a && a->texture) {
                    ImGui::PushID(i);
                    // Use a reasonable thumbnail size
                    ImGui::ImageButton(atlas_name, (ImTextureID)(intptr_t)a->texture, ImVec2(64, 64));
                    
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                        ImGui::SetDragDropPayload("DND_ASSET", atlas_name, strlen(atlas_name) + 1);
                        ImGui::Text("Spawning %s", atlas_name);
                        dragged_asset = atlas_name;
                        ImGui::EndDragDropSource();
                    }

                    if ((i + 1) % 4 != 0) ImGui::SameLine();
                    ImGui::PopID();
                }
            }

            ImGui::End();
        }
        
        // Context menu logic outside of the main ImGui Window but still within is_editor_mode
        if (is_editor_mode) {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !io.WantCaptureMouse) {
                float mx = io.MousePos.x + camera_x;
                float my = io.MousePos.y + camera_y;
                for (size_t i = 1; i <= world->entityCount; ++i) {
                    if (world->componentsMaskList[i] & COMPONENT_POSITION) {
                        float x = world->positionList[i].x;
                        float y = world->positionList[i].y;
                        float w = 32, h = 32;
                        
                        if (world->componentsMaskList[i] & COMPONENT_SPRITE) {
                            Sprite& spr = world->spriteList[i];
                            Assets::Atlas* a = Assets::atlases.find(spr.atlas_name);
                            if (a) { w = a->sprite_width; h = a->sprite_height; }
                        }
                        if (world->componentsMaskList[i] & COMPONENT_COLLIDER) {
                            w = world->colliderList[i].width;
                            h = world->colliderList[i].height;
                        }

                        if (mx >= x && mx <= x + w && my >= y && my <= y + h) {
                            selected_entity_id = i;
                            ImGui::OpenPopup("EntityContextMenu");
                            break;
                        }
                    }
                }
            }

            if (ImGui::BeginPopup("EntityContextMenu")) {
                ImGui::Text("Entity #%u", selected_entity_id);
                ImGui::Separator();
                bool has_collider = (world->componentsMaskList[selected_entity_id] & COMPONENT_COLLIDER) != 0;
                if (ImGui::MenuItem(has_collider ? "Remove Collider" : "Add Collider")) {
                    if (has_collider) {
                        world->componentsMaskList[selected_entity_id] &= ~COMPONENT_COLLIDER;
                    } else {
                        float w = 32, h = 32;
                        if (world->componentsMaskList[selected_entity_id] & COMPONENT_SPRITE) {
                            Sprite& spr = world->spriteList[selected_entity_id];
                            Assets::Atlas* a = Assets::atlases.find(spr.atlas_name);
                            if (a) { w = a->sprite_width; h = a->sprite_height; }
                        }
                        add_collider_component(world, selected_entity_id, w, h);
                    }
                }
                ImGui::EndPopup();
            }
        }

        SDL_SetRenderDrawColor(ctx.renderer, 30, 30, 30, 255);
        SDL_RenderClear(ctx.renderer);

        // --- Render Game Entities ---
        
        // 1. Draw Sprites
        for (size_t i = 1; i <= world->entityCount; ++i) {
            if (world->componentsMaskList[i] & COMPONENT_SPRITE) {
                float x = 0, y = 0;
                if (world->componentsMaskList[i] & COMPONENT_POSITION) {
                    x = world->positionList[i].x;
                    y = world->positionList[i].y;
                }
                
                Sprite& spr = world->spriteList[i];
                Assets::Atlas* a = Assets::atlases.find(spr.atlas_name);
                if (a && a->texture) {
                    SDL_FRect dst;
                    dst.x = x - camera_x;
                    dst.y = y - camera_y;
                    dst.w = a->sprite_width;
                    dst.h = a->sprite_height;
                    
                    SDL_FRect src;
                    src.x = spr.frame_index * a->sprite_width;
                    src.y = 0;
                    src.w = a->sprite_width;
                    src.h = a->sprite_height;
                    
                    SDL_RenderTexture(ctx.renderer, a->texture, &src, &dst);
                }
            }
        }

        // 2. Draw Colliders (only visible in Editor Mode)
        if (is_editor_mode && draw_colliders_mode) {
            for (size_t i = 1; i <= world->entityCount; ++i) {
                if (world->componentsMaskList[i] & COMPONENT_COLLIDER) {
                    float x = 0, y = 0;
                    if (world->componentsMaskList[i] & COMPONENT_POSITION) {
                        x = world->positionList[i].x;
                        y = world->positionList[i].y;
                    }
                    SDL_FRect rect;
                    rect.x = x - camera_x;
                    rect.y = y - camera_y;
                    rect.w = world->colliderList[i].width;
                    rect.h = world->colliderList[i].height;
                    
                    SDL_SetRenderDrawColor(ctx.renderer, 255, 0, 0, 255);
                    SDL_RenderRect(ctx.renderer, &rect);
                }
            }

            // Draw the temporary rectangle being dragged
            if (is_drawing_rect) {
                float mouse_x, mouse_y;
                SDL_GetMouseState(&mouse_x, &mouse_y);
                
                float world_mouse_x = mouse_x + camera_x;
                float world_mouse_y = mouse_y + camera_y;
                
                SDL_FRect r;
                r.x = ((rect_start_x < world_mouse_x) ? rect_start_x : world_mouse_x) - camera_x;
                r.y = ((rect_start_y < world_mouse_y) ? rect_start_y : world_mouse_y) - camera_y;
                r.w = (rect_start_x < world_mouse_x) ? world_mouse_x - rect_start_x : rect_start_x - world_mouse_x;
                r.h = (rect_start_y < world_mouse_y) ? world_mouse_y - rect_start_y : rect_start_y - world_mouse_y;
                
                SDL_SetRenderDrawColor(ctx.renderer, 0, 255, 0, 255);
                SDL_RenderRect(ctx.renderer, &r);
            }
        }

        // Render ImGui on top of everything
        ImGui::Render();
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), ctx.renderer);
        
        SDL_RenderPresent(ctx.renderer);
    }

    free(world->componentsMaskList);
    free(world->positionList);
    free(world->colliderList);
    free(world->spriteList);
    free(world->status);
    free(world);
    free(grid.cells);

    Assets::shutdown();

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    engine_shutdown(&ctx);
    return 0;
}
