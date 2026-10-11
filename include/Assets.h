#ifndef ASSETS_H
#define ASSETS_H

#include "BeatMap.h"

// --- Umbrella Includes ---
#include "Atlas.h"
// #include "sounds.h" // TODO

struct SDL_Renderer;

namespace Assets {

    // --- Global Asset Maps ---
    extern BeatMap<Atlas> Atlases;
    extern const char* LoadedAtlasNames[64];
    extern int LoadedAtlasCount;

    // Call this at startup
    void init(SDL_Renderer* renderer);
    
    // Call this at shutdown
    void shutdown();
}

#endif // ASSETS_H
