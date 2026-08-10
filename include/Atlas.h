#ifndef ATLAS_H
#define ATLAS_H

struct SDL_Texture;

namespace Assets {

    struct Atlas {
        SDL_Texture* texture;
        int sprite_width;
        int sprite_height;
        int num_sprites;
    };

    // Loads an atlas like "assets/atlas/player_32.png"
    /*
     * Atlas constraints:
     * - Format: Only .png files are supported.
     * - Layout: Sprites must be packed in a single horizontal strip (sprite height = image height).
     * - Naming: The filename MUST end with '_WIDTH.png' where WIDTH is the width 
     *   of a single sprite in pixels (e.g. "player_walk_32.png"). This avoids 
     *   needing separate metadata files.
     */
    bool load_atlas(const char* filepath);

}

#endif // ATLAS_H
