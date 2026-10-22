#include "RenderSystem.h"
#include "Assets.h"
#include "Settings.h"
#include "BeatTime.h"
#include "LSMath.h"

void render_system(World* world, SDL_Renderer* renderer, Camera* camera) {
    for (size_t i = 1; i <= world->entityCount; ++i) {
        if (world->componentsMaskList[i] & COMPONENT_SPRITE) {
            float x = 0, y = 0;
            if (world->componentsMaskList[i] & COMPONENT_POSITION) {
                LSMVECTOR pos;
                LSMLoadVector(&world->positionList[i], 2, &pos);
                if (world->componentsMaskList[i] & COMPONENT_PREV_POSITION) {
                    LSMVECTOR prev_pos;
                    LSMLoadVector(&world->prevPositionList[i], 2, &prev_pos);
                    pos = prev_pos * (1.0f - Time::Alpha) + pos * Time::Alpha;
                }
                x = pos.x;
                y = pos.y;
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

        if (world->componentsMaskList[i] & COMPONENT_PLAYER) {
            float x = 0, y = 0;
            float w = 32.0f, h = 32.0f;
            if (world->componentsMaskList[i] & COMPONENT_POSITION) {
                LSMVECTOR pos;
                LSMLoadVector(&world->positionList[i], 2, &pos);
                if (world->componentsMaskList[i] & COMPONENT_PREV_POSITION) {
                    LSMVECTOR prev_pos;
                    LSMLoadVector(&world->prevPositionList[i], 2, &prev_pos);
                    pos = prev_pos * (1.0f - Time::Alpha) + pos * Time::Alpha;
                }
                x = pos.x;
                y = pos.y;
            }
            if (world->componentsMaskList[i] & COMPONENT_COLLIDER) {
                w = world->colliderList[i].width;
                h = world->colliderList[i].height;
            }

            SDL_FRect dst;
            dst.x = x - camera->x + (GlobalSettings.screenWidth / 2.0f);
            dst.y = y - camera->y + (GlobalSettings.screenHeight / 2.0f);
            dst.w = w;
            dst.h = h;

            // Draw player as a cyan/blue rectangle
            SDL_SetRenderDrawColor(renderer, 0, 200, 255, 255);
            SDL_RenderFillRect(renderer, &dst);
        }
    }
}
