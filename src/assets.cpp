#include "assets.h"
#include <SDL3/SDL.h>

namespace Assets {
    BeatMap<Atlas> atlases(32);
    SDL_Renderer* g_renderer = nullptr;

    const char* loaded_atlas_names[64];
    int loaded_atlas_count = 0;

    void init(SDL_Renderer* renderer) {
        g_renderer = renderer;
    }

    void shutdown() {
    }
}
