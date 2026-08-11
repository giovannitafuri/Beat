#include "Assets.h"
#include <SDL3/SDL.h>

namespace Assets {
    BeatMap<Atlas> Atlases(32);
    SDL_Renderer* g_renderer = nullptr;

    const char* LoadedAtlasNames[64];
    int LoadedAtlasCount = 0;

    void init(SDL_Renderer* renderer) {
        g_renderer = renderer;
    }

    void shutdown() {
    }
}
