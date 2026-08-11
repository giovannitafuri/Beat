#ifndef INPUT_H
#define INPUT_H

#include <SDL3/SDL.h>

struct InputState {
    bool up;
    bool down;
    bool left;
    bool right;
};

extern InputState CurrentInput;

// Must be called every frame to update the keyboard state
void input_update();

#endif // INPUT_H
