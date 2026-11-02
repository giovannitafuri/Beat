#include "RenderSystem.h"
#include "BatchRenderer.h"
#include "Assets.h"
#include "Settings.h"
#include "BeatTime.h"
#include "LSMath.h"

void render_system(World* world, SDL_Renderer* renderer, Camera* camera, BatchRenderer* batch) {
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
            Assets::Atlas* a = spr.atlas;
            if (a && a->texture) {
                float dst_x = x - camera->x + (GlobalSettings.screenWidth / 2.0f);
                float dst_y = y - camera->y + (GlobalSettings.screenHeight / 2.0f);
                float spr_w = (float)a->sprite_width;
                float spr_h = (float)a->sprite_height;

                // Frustum culling: skip sprites completely outside the screen
                if (dst_x + spr_w < 0.0f || dst_x > (float)GlobalSettings.screenWidth ||
                    dst_y + spr_h < 0.0f || dst_y > (float)GlobalSettings.screenHeight) {
                    continue;
                }

                float total_w = (float)(a->num_sprites * a->sprite_width);
                float u0 = (float)(spr.frame_index * a->sprite_width) / total_w;
                float u1 = (float)((spr.frame_index + 1) * a->sprite_width) / total_w;

                LSMVECTOR pos_size = { dst_x, dst_y, spr_w, spr_h };
                LSMVECTOR uv = { u0, 0.0f, u1, 1.0f };
                SDL_FColor white = { 1.0f, 1.0f, 1.0f, 1.0f };
                if (world->componentsMaskList[i] & COMPONENT_ROTATION) {
                    batch_draw_sprite_rotated(batch, renderer, pos_size, a->texture, uv, white, world->rotationList[i].angle);
                } else {
                    batch_draw_sprite(batch, renderer, pos_size, a->texture, uv, white);
                }
            }
        }
    }

    batch_flush(batch, renderer);
}
