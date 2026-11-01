#ifndef EDITOR_H
#define EDITOR_H

#include <SDL3/SDL.h>
#include "Entity.h"
#include "Camera.h"

struct EditorState {
    SDL_Renderer* renderer;
    bool is_editor_mode;
    bool draw_colliders_mode;
    bool vsync_enabled;
    uint32_t selected_entity_id;
    bool is_drawing_rect;
    float rect_start_x;
    float rect_start_y;
    const char* dragged_asset;
};

void editor_init(SDL_Window* window, SDL_Renderer* renderer, EditorState* state);
void editor_shutdown();
void editor_process_event(SDL_Event* event, EditorState* state, World* world, Camera* camera);
void editor_update(EditorState* state, World* world, Camera* camera);
void editor_draw_ui(EditorState* state, World* world, Camera* camera);
void editor_draw_overlay(EditorState* state, World* world, SDL_Renderer* renderer, Camera* camera);
void spawn_stress_entities(World* world, size_t count);

#endif // EDITOR_H
