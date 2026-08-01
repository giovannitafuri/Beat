#ifndef ASSETS_H
#define ASSETS_H

#include "beatMap.h"

// --- Umbrella Includes ---
#include "atlas.h"
// #include "sounds.h" // in futuro

struct SDL_Renderer;

namespace Assets {

    // --- Global Asset Maps ---
    extern BeatMap<Atlas> atlases;

    // Call this at startup
    void init(SDL_Renderer* renderer);
    
    // Call this at shutdown
    void shutdown();
}

#endif // ASSETS_H
