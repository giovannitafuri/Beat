#include "Editor.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "Assets.h"
#include "BeatTime.h"
#include "Settings.h"
#include "Level.h"
#include <cstring>
#include <cmath>
#include <algorithm>

void editor_init(SDL_Window* window, SDL_Renderer* renderer, EditorState* state) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    state->renderer = renderer;
    state->is_editor_mode = false;
    state->draw_colliders_mode = false;
    state->vsync_enabled = true;
    SDL_SetRenderVSync(renderer, 1);
    state->selected_entity_id = 0;
    state->is_drawing_rect = false;
    state->rect_start_x = 0;
    state->rect_start_y = 0;
    state->dragged_asset = nullptr;
    state->score = 0;
    benchmark_init(&state->benchmark);
}

void editor_shutdown() {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

void editor_process_event(SDL_Event* event, EditorState* state, World* world, Camera* camera) {
    ImGui_ImplSDL3_ProcessEvent(event);
    
    if (event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_TAB) {
        state->is_editor_mode = !state->is_editor_mode;
    }

    ImGuiIO& io = ImGui::GetIO();
    if (state->is_editor_mode && !io.WantCaptureMouse) {
        // Panning with Middle Mouse Button
        if (event->type == SDL_EVENT_MOUSE_MOTION) {
            if (event->motion.state & SDL_BUTTON_MMASK) {
                camera->x -= event->motion.xrel;
                camera->y -= event->motion.yrel;
            }
        }
        
        // Custom manual drawing for colliders (rectangles)
        if (state->draw_colliders_mode) {
            if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN && event->button.button == SDL_BUTTON_LEFT) {
                state->is_drawing_rect = true;
                state->rect_start_x = event->button.x + camera->x - (GlobalSettings.screenWidth / 2.0f);
                state->rect_start_y = event->button.y + camera->y - (GlobalSettings.screenHeight / 2.0f);
            }
            else if (event->type == SDL_EVENT_MOUSE_BUTTON_UP && event->button.button == SDL_BUTTON_LEFT) {
                if (state->is_drawing_rect) {
                    state->is_drawing_rect = false;
                    float end_x = event->button.x + camera->x - (GlobalSettings.screenWidth / 2.0f);
                    float end_y = event->button.y + camera->y - (GlobalSettings.screenHeight / 2.0f);
                    
                    float x = (state->rect_start_x < end_x) ? state->rect_start_x : end_x;
                    float y = (state->rect_start_y < end_y) ? state->rect_start_y : end_y;
                    float w = (state->rect_start_x < end_x) ? end_x - state->rect_start_x : state->rect_start_x - end_x;
                    float h = (state->rect_start_y < end_y) ? end_y - state->rect_start_y : state->rect_start_y - end_y;
                    
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

void editor_update(EditorState* state, World* world, Camera* camera) {
    ImGuiIO& io = ImGui::GetIO();
    if (state->is_editor_mode) {
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            // The mouse was just released or is up
            if (state->dragged_asset) {
                // Check if released over game view (not hovering any ImGui window)
                if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
                    Entity e = create_entity(world);
                    add_position_component(world, e.id, io.MousePos.x + camera->x - (GlobalSettings.screenWidth / 2.0f), io.MousePos.y + camera->y - (GlobalSettings.screenHeight / 2.0f));
                    add_sprite_component(world, e.id, state->dragged_asset, 0);
                }
            }
            state->dragged_asset = nullptr; // Clear it out when mouse is released
        }
    }
}

static uint32_t xorshift32_state = 0x12345678;
static uint32_t fast_rand() {
    uint32_t x = xorshift32_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    xorshift32_state = x;
    return x;
}

static float fast_randf(float min, float max) {
    return min + (float)fast_rand() / 4294967295.0f * (max - min);
}

void spawn_stress_entities(World* world, size_t count) {
    if (!world) return;

    constexpr size_t RESERVED_GAMEPLAY_SLOTS = 8192;
    size_t max_benchmark = (world->capacity > RESERVED_GAMEPLAY_SLOTS) ? (world->capacity - RESERVED_GAMEPLAY_SLOTS) : 0;
    size_t available = (max_benchmark > world->entityCount) ? (max_benchmark - world->entityCount) : 0;
    if (count > available) count = available;
    if (count == 0) return;

    Assets::Atlas* atlas16 = Assets::Atlases.find("particles_16.png");
    if (!atlas16) atlas16 = Assets::Atlases.find("assets/particles_16.png");

    Assets::Atlas* atlas32 = Assets::Atlases.find("particles_32.png");
    if (!atlas32) atlas32 = Assets::Atlases.find("assets/particles_32.png");

    Assets::Atlas* atlas64 = Assets::Atlases.find("particles_64.png");
    if (!atlas64) atlas64 = Assets::Atlases.find("assets/particles_64.png");

    float min_x = 64.0f;
    float max_x = (float)WORLD_WIDTH - 64.0f;
    float min_y = 64.0f;
    float max_y = (float)WORLD_HEIGHT - 64.0f;

    for (size_t i = 0; i < count; ++i) {
        Entity e = create_entity(world);
        if (e.id == 0) break;

        uint32_t type = fast_rand() % 3;
        float size = 16.0f;
        Assets::Atlas* selected_atlas = atlas16;
        if (type == 1) {
            size = 32.0f;
            selected_atlas = atlas32;
        } else if (type == 2) {
            size = 64.0f;
            selected_atlas = atlas64;
        }

        float x = fast_randf(min_x, max_x - size);
        float y = fast_randf(min_y, max_y - size);

        float vx = fast_randf(-150.0f, 150.0f);
        float vy = fast_randf(-150.0f, 150.0f);
        if (fabsf(vx) < 30.0f) vx = (vx < 0.0f) ? -60.0f : 60.0f;
        if (fabsf(vy) < 30.0f) vy = (vy < 0.0f) ? -60.0f : 60.0f;

        uint32_t frame = fast_rand() % 8;

        add_position_component(world, e.id, x, y);
        add_prev_position_component(world, e.id, x, y);
        add_velocity_component(world, e.id, vx, vy);
        add_collider_component(world, e.id, size, size);
        if (selected_atlas) {
            add_sprite_component(world, e.id, selected_atlas, frame);
        }
    }
}

void editor_draw_ui(EditorState* state, World* world, Camera* camera) {
    static float frame_times[120] = {};
    static int frame_time_idx = 0;
    static float displayed_fps = 0.0f;
    static float displayed_avg = 0.0f;
    static float displayed_p99 = 0.0f;
    static float hud_timer = 0.0f;

    float frame_ms = Time::RealDelta * 1000.0f;
    frame_times[frame_time_idx] = frame_ms;
    frame_time_idx = (frame_time_idx + 1) % 120;

    hud_timer += Time::RealDelta;
    if (hud_timer >= 0.15f) {
        hud_timer = 0.0f;
        float sorted[120];
        memcpy(sorted, frame_times, sizeof(sorted));
        std::sort(sorted, sorted + 120);
        displayed_p99 = sorted[118];

        float sum = 0.0f;
        for (int i = 0; i < 120; i++) sum += frame_times[i];
        displayed_avg = sum / 120.0f;
        displayed_fps = (displayed_avg > 0.00001f) ? (1000.0f / displayed_avg) : 0.0f;
    }

    ImGui::Begin("Stress Test & Benchmark");
    ImGui::Text("FPS: %.1f | Avg Frametime: %.2f ms | p99: %.2f ms", (double)displayed_fps, (double)displayed_avg, (double)displayed_p99);
    float plot_max = (displayed_p99 > 33.3f) ? (displayed_p99 * 1.25f) : 33.3f;
    ImGui::PlotLines("Frametime (ms)", frame_times, 120, frame_time_idx, nullptr, 0.0f, plot_max, ImVec2(0, 80));

    if (ImGui::Checkbox("VSync (Lock to Monitor Hz)", &state->vsync_enabled)) {
        if (state->renderer) {
            SDL_SetRenderVSync(state->renderer, state->vsync_enabled ? 1 : 0);
        }
    }

    ImGui::SeparatorText("Gameplay Stats");
    ImGui::Text("Asteroids Destroyed (Score): %u", state->score);

    ImGui::Separator();
    size_t active_count = (world->entityCount >= world->freeListCount) ? (world->entityCount - world->freeListCount) : 0;
    ImGui::Text("Active Entities: %zu / %zu", active_count, world->capacity - 1);
    ImGui::Text("Spatial Partitioning: Dense Grid 512x512 (262,144 cells)");

    ImGui::Separator();
    ImGui::Text("Spawn Benchmark Entities:");
    if (ImGui::Button("Spawn 1,000")) {
        spawn_stress_entities(world, 1000);
    }
    ImGui::SameLine();
    if (ImGui::Button("Spawn 10,000")) {
        spawn_stress_entities(world, 10000);
    }
    ImGui::SameLine();
    if (ImGui::Button("Spawn 100,000")) {
        spawn_stress_entities(world, 100000);
    }

    if (ImGui::Button("Clear All Entities")) {
        clear_stress_entities(world);
    }

    ImGui::Separator();
    ImGui::SeparatorText("DoD Benchmark Session (10k Frames)");
    BenchmarkContext* bench = &state->benchmark;
    if (bench->state == BenchmarkState::IDLE) {
        if (ImGui::Button("Run Benchmark (10,000 frames)")) {
            benchmark_start(bench, 10000);
        }
    } else if (bench->state == BenchmarkState::RECORDING) {
        float progress = (bench->target_samples > 0) ? ((float)bench->current_sample / (float)bench->target_samples) : 0.0f;
        ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));
        ImGui::Text("Sampling: %zu / %zu frames", bench->current_sample, bench->target_samples);
    } else if (bench->state == BenchmarkState::COMPLETED) {
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Benchmark Completed (%u frames)", bench->result.total_samples);
        ImGui::Text("Min:  %.3f ms", (double)bench->result.min_ms);
        ImGui::Text("Max:  %.3f ms", (double)bench->result.max_ms);
        ImGui::Text("Avg:  %.3f ms", (double)bench->result.avg_ms);
        ImGui::Text("p50:  %.3f ms", (double)bench->result.p50_ms);
        ImGui::Text("p99:  %.3f ms", (double)bench->result.p99_ms);
        ImGui::Text("Missed 144Hz (> 6.94ms):  %u", bench->result.missed_144hz);
        ImGui::Text("Missed 60Hz  (> 16.67ms): %u", bench->result.missed_60hz);

        if (ImGui::Button("Export to CSV")) {
            benchmark_export_csv(bench, "benchmark_results.csv");
        }
        ImGui::SameLine();
        if (ImGui::Button("Run Again")) {
            benchmark_start(bench, 10000);
        }
    }

    ImGui::Separator();
    if (ImGui::Button(state->is_editor_mode ? "Switch to PLAY MODE (TAB)" : "Switch to EDITOR MODE (TAB)")) {
        state->is_editor_mode = !state->is_editor_mode;
    }
    ImGui::End();

    if (!state->is_editor_mode) {
        return;
    }

    ImGui::Begin("Beat Engine - Editor");
    ImGui::Text("Editor Mode Active");
    ImGui::Text("Press TAB to toggle Play mode");
    ImGui::Text("Target Delta Time: %.4f", (double)Time::TargetDelta);
    size_t active_entities = (world->entityCount >= world->freeListCount) ? (world->entityCount - world->freeListCount) : 0;
    ImGui::Text("Entities: %zu", active_entities);
    
    ImGui::Separator();
    ImGui::Checkbox("Draw colliders mode", &state->draw_colliders_mode);
    ImGui::Separator();
    ImGui::Text("Asset Browser");
    ImGui::Text("Drag images to place them in the world!");

    for (int i = 0; i < Assets::LoadedAtlasCount; i++) {
        const char* atlas_name = Assets::LoadedAtlasNames[i];
        Assets::Atlas* a = Assets::Atlases.find(atlas_name);
        if (a && a->texture) {
            ImGui::PushID(i);
            ImGui::ImageButton(atlas_name, (ImTextureID)(intptr_t)a->texture, ImVec2(64, 64));
            
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                ImGui::SetDragDropPayload("DND_ASSET", atlas_name, strlen(atlas_name) + 1);
                ImGui::Text("Spawning %s", atlas_name);
                state->dragged_asset = atlas_name;
                ImGui::EndDragDropSource();
            }

            if ((i + 1) % 4 != 0) ImGui::SameLine();
            ImGui::PopID();
        }
    }

    ImGui::End();

    ImGuiIO& io = ImGui::GetIO();
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !io.WantCaptureMouse) {
        float mx = io.MousePos.x + camera->x - (GlobalSettings.screenWidth / 2.0f);
        float my = io.MousePos.y + camera->y - (GlobalSettings.screenHeight / 2.0f);
        for (size_t i = 1; i <= world->entityCount; ++i) {
            if (world->componentsMaskList[i] & COMPONENT_POSITION) {
                LSMVECTOR pos;
                LSMLoadVector(&world->positionList[i], 2, &pos);
                float x = pos.x;
                float y = pos.y;
                float w = 32, h = 32;
                
                if (world->componentsMaskList[i] & COMPONENT_SPRITE) {
                    Sprite& spr = world->spriteList[i];
                    Assets::Atlas* a = spr.atlas;
                    if (a) { w = a->sprite_width; h = a->sprite_height; }
                }
                if (world->componentsMaskList[i] & COMPONENT_COLLIDER) {
                    w = world->colliderList[i].width;
                    h = world->colliderList[i].height;
                }

                if (mx >= x && mx <= x + w && my >= y && my <= y + h) {
                    state->selected_entity_id = i;
                    ImGui::OpenPopup("EntityContextMenu");
                    break;
                }
            }
        }
    }

    if (ImGui::BeginPopup("EntityContextMenu")) {
        ImGui::Text("Entity #%u", state->selected_entity_id);
        ImGui::Separator();
        bool has_collider = (world->componentsMaskList[state->selected_entity_id] & COMPONENT_COLLIDER) != 0;
        if (ImGui::MenuItem(has_collider ? "Remove Collider" : "Add Collider")) {
            if (has_collider) {
                world->componentsMaskList[state->selected_entity_id] &= ~COMPONENT_COLLIDER;
            } else {
                float w = 32, h = 32;
                if (world->componentsMaskList[state->selected_entity_id] & COMPONENT_SPRITE) {
                    Sprite& spr = world->spriteList[state->selected_entity_id];
                    Assets::Atlas* a = spr.atlas;
                    if (a) { w = a->sprite_width; h = a->sprite_height; }
                }
                add_collider_component(world, state->selected_entity_id, w, h);
            }
        }
        if (ImGui::MenuItem("Delete Entity")) {
            delete_entity(world, get_entity(world, state->selected_entity_id));
            state->selected_entity_id = 0;
        }
        ImGui::EndPopup();
    }
}

void editor_draw_overlay(EditorState* state, World* world, SDL_Renderer* renderer, Camera* camera) {
    if (!state->is_editor_mode || !state->draw_colliders_mode) {
        return;
    }

    for (size_t i = 1; i <= world->entityCount; ++i) {
        if (world->componentsMaskList[i] & COMPONENT_COLLIDER) {
            float x = 0, y = 0;
            if (world->componentsMaskList[i] & COMPONENT_POSITION) {
                LSMVECTOR pos;
                LSMLoadVector(&world->positionList[i], 2, &pos);
                x = pos.x;
                y = pos.y;
            }
            SDL_FRect rect;
            rect.x = x - camera->x + (GlobalSettings.screenWidth / 2.0f);
            rect.y = y - camera->y + (GlobalSettings.screenHeight / 2.0f);
            rect.w = world->colliderList[i].width;
            rect.h = world->colliderList[i].height;
            
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderRect(renderer, &rect);
        }
    }

    if (state->is_drawing_rect) {
        float mouse_x, mouse_y;
        SDL_GetMouseState(&mouse_x, &mouse_y);
        
        float world_mouse_x = mouse_x + camera->x - (GlobalSettings.screenWidth / 2.0f);
        float world_mouse_y = mouse_y + camera->y - (GlobalSettings.screenHeight / 2.0f);
        
        SDL_FRect r;
        r.x = ((state->rect_start_x < world_mouse_x) ? state->rect_start_x : world_mouse_x) - camera->x + (GlobalSettings.screenWidth / 2.0f);
        r.y = ((state->rect_start_y < world_mouse_y) ? state->rect_start_y : world_mouse_y) - camera->y + (GlobalSettings.screenHeight / 2.0f);
        r.w = (state->rect_start_x < world_mouse_x) ? world_mouse_x - state->rect_start_x : state->rect_start_x - world_mouse_x;
        r.h = (state->rect_start_y < world_mouse_y) ? world_mouse_y - state->rect_start_y : state->rect_start_y - world_mouse_y;
        
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderRect(renderer, &r);
    }
}
