#ifndef EDITOR_H
#define EDITOR_H

#include <SDL3/SDL.h>
#include "Entity.h"

struct EditorState {
    bool is_editor_mode;
    bool draw_colliders_mode;
    uint32_t selected_entity_id;
    float camera_x;
    float camera_y;
    bool is_drawing_rect;
    float rect_start_x;
    float rect_start_y;
    const char* dragged_asset;
};

void editor_init(SDL_Window* window, SDL_Renderer* renderer, EditorState* state);
void editor_shutdown();
void editor_process_event(SDL_Event* event, EditorState* state, World* world);
void editor_update(EditorState* state, World* world);
void editor_draw_ui(EditorState* state, World* world);
void editor_draw_overlay(EditorState* state, World* world, SDL_Renderer* renderer);

#endif // EDITOR_H
