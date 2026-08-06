#include "assets.h"
#include <cstring>
#include <cstdlib>
#include "atlas.h"
#include <SDL3_image/SDL_image.h>
#include <cstring>
#include <cstdio>
#include <SDL3/SDL.h>

namespace Assets {

    extern SDL_Renderer* g_renderer;

    bool load_atlas(const char* filepath) {
        const char* filename = strrchr(filepath, '/');
        if (!filename) filename = strrchr(filepath, '\\');
        filename = filename ? filename + 1 : filepath;

        const char* ext = strrchr(filename, '.');
        if (!ext || strcmp(ext, ".png") != 0) {
            fprintf(stderr, "Error: Only .png files are supported: %s\n", filepath);
            return false;
        }

        const char* underscore = strrchr(filename, '_');
        if (!underscore) {
            fprintf(stderr, "Error: Atlas filename must end with _WIDTH (e.g. _32.png): %s\n", filepath);
            return false;
        }

        int sprite_width = atoi(underscore + 1);
        if (sprite_width <= 0) {
            fprintf(stderr, "Error: Invalid sprite width in filename: %s\n", filepath);
            return false;
        }

        SDL_Texture* tex = IMG_LoadTexture(g_renderer, filepath);
        if (!tex) {
            fprintf(stderr, "IMG_LoadTexture Error: %s\n", SDL_GetError());
            return false;
        }
        
        float fw, fh;
        SDL_GetTextureSize(tex, &fw, &fh);

        Atlas atlas;
        atlas.texture = tex;
        atlas.sprite_width = sprite_width;
        atlas.sprite_height = (int)fh;
        atlas.num_sprites = (int)fw / sprite_width;

        atlases.insert(filename, atlas);

        if (loaded_atlas_count < 64) {
            loaded_atlas_names[loaded_atlas_count++] = strdup(filename);
        }

        return true;
    }

}
