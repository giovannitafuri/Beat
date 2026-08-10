#ifndef ASSETS_H
#define ASSETS_H

#include "BeatMap.h"

// --- Umbrella Includes ---
#include "Atlas.h"
// #include "sounds.h" // in futuro

struct SDL_Renderer;

namespace Assets {

    // --- Global Asset Maps ---
    extern BeatMap<Atlas> atlases;
    extern const char* loaded_atlas_names[64];
    extern int loaded_atlas_count;

    // Call this at startup
    void init(SDL_Renderer* renderer);
    
    // Call this at shutdown
    void shutdown();
}

#endif // ASSETS_H
