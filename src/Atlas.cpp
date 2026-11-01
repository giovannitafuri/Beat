#include "Assets.h"
#include <cstring>
#include <cstdlib>
#include "Atlas.h"
#include <SDL3_image/SDL_image.h>
#include <cstdio>
#include <SDL3/SDL.h>
#include "Assert.h"

namespace Assets {

    extern SDL_Renderer* g_renderer;

    bool load_atlas(const char* filepath) {
        const char* filename = strrchr(filepath, '/');
        if (!filename) filename = strrchr(filepath, '\\');
        filename = filename ? filename + 1 : filepath;

        const char* ext = strrchr(filename, '.');
        ASSERT(ext != nullptr && strcmp(ext, ".png") == 0, "Only .png files are supported");

        const char* underscore = strrchr(filename, '_');
        ASSERT(underscore != nullptr, "Atlas filename must end with _WIDTH");

        int sprite_width = atoi(underscore + 1);
        ASSERT(sprite_width > 0, "Invalid sprite width in filename");

        SDL_Texture* tex = IMG_LoadTexture(g_renderer, filepath);
        ASSERT(tex != nullptr, "IMG_LoadTexture Error");
        
        float fw, fh;
        SDL_GetTextureSize(tex, &fw, &fh);

        Atlas atlas;
        atlas.texture = tex;
        atlas.sprite_width = sprite_width;
        atlas.sprite_height = (int)fh;
        atlas.num_sprites = (int)fw / sprite_width;

        Atlases.insert(filename, atlas);

        if (LoadedAtlasCount < 64) {
            LoadedAtlasNames[LoadedAtlasCount++] = strdup(filename);
        }

        return true;
    }

}
