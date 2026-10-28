#include "Editor.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "Assets.h"
#include "BeatTime.h"
#include "Settings.h"
#include <cstring>

void editor_init(SDL_Window* window, SDL_Renderer* renderer, EditorState* state) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    state->is_editor_mode = true;
    state->draw_colliders_mode = false;
    state->selected_entity_id = 0;
    state->is_drawing_rect = false;
    state->rect_start_x = 0;
    state->rect_start_y = 0;
    state->dragged_asset = nullptr;
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

void editor_draw_ui(EditorState* state, World* world, Camera* camera) {
    if (!state->is_editor_mode) {
        return;
    }

    ImGui::Begin("Beat Engine - Editor");
    ImGui::Text("Editor Mode Active");
    ImGui::Text("Press TAB to toggle Play mode");
    ImGui::Text("Delta Time: %.4f", (double)Time::Delta);
    ImGui::Text("Entities: %zu", world->entityCount);
    
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
