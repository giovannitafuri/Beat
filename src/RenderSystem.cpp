#include "RenderSystem.h"
#include "Assets.h"
#include "Settings.h"

void render_system(World* world, SDL_Renderer* renderer, Camera* camera) {
    for (size_t i = 1; i <= world->entityCount; ++i) {
        if (world->componentsMaskList[i] & COMPONENT_SPRITE) {
            float x = 0, y = 0;
            if (world->componentsMaskList[i] & COMPONENT_POSITION) {
                x = world->positionList[i].x;
                y = world->positionList[i].y;
            }
            
            Sprite& spr = world->spriteList[i];
            Assets::Atlas* a = Assets::Atlases.find(spr.atlas_name);
            if (a && a->texture) {
                SDL_FRect dst;
                dst.x = x - camera->x + (GlobalSettings.screenWidth / 2.0f);
                dst.y = y - camera->y + (GlobalSettings.screenHeight / 2.0f);
                dst.w = a->sprite_width;
                dst.h = a->sprite_height;
                
                SDL_FRect src;
                src.x = spr.frame_index * a->sprite_width;
                src.y = 0;
                src.w = a->sprite_width;
                src.h = a->sprite_height;
                
                SDL_RenderTexture(renderer, a->texture, &src, &dst);
            }
        }
    }
}
