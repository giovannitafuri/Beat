#include "Assets.h"
#include <SDL3/SDL.h>
#include <cstdlib>

namespace Assets {
    BeatMap<Atlas> Atlases(32);
    SDL_Renderer* g_renderer = nullptr;

    const char* LoadedAtlasNames[64];
    int LoadedAtlasCount = 0;

    void init(SDL_Renderer* renderer) {
        g_renderer = renderer;
        load_atlas("assets/particles_16.xpm");
        load_atlas("assets/particles_32.xpm");
        load_atlas("assets/particles_64.xpm");
        load_atlas("assets/ship_32.xpm");
        load_atlas("assets/tiles_32.xpm");
    }

    void shutdown() {
        for (int i = 0; i < LoadedAtlasCount; i++) {
            free((void*)LoadedAtlasNames[i]);
            LoadedAtlasNames[i] = nullptr;
        }
        LoadedAtlasCount = 0;
    }
}
