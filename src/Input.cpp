#include "Input.h"

InputState CurrentInput = {false, false, false, false};

void input_update() {
    int numkeys;
    const bool* state = SDL_GetKeyboardState(&numkeys);
    
    // Support for WASD and arrows
    CurrentInput.up = state[SDL_SCANCODE_W] || state[SDL_SCANCODE_UP];
    CurrentInput.down = state[SDL_SCANCODE_S] || state[SDL_SCANCODE_DOWN];
    CurrentInput.left = state[SDL_SCANCODE_A] || state[SDL_SCANCODE_LEFT];
    CurrentInput.right = state[SDL_SCANCODE_D] || state[SDL_SCANCODE_RIGHT];
}
