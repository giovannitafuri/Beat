#include "Systems.h"
#include "Components.h"
#include "Input.h"
#include "BeatTime.h"
#include "LSMath.h"
#include "Level.h"
#include "Event.h"
#include <cmath>

void movement_system(World *w) {
    float player_x = 0.0f, player_y = 0.0f;
    for (size_t i = 1; i <= w->entityCount; i++) {
        if (w->componentsMaskList[i] & COMPONENT_PLAYER) {
            player_x = w->positionList[i].data[0];
            player_y = w->positionList[i].data[1];
            break;
        }
    }

    for (size_t id = 1; id <= w->entityCount; id++) {
        uint64_t mask = w->componentsMaskList[id];

        if (mask & COMPONENT_PREV_POSITION) {
            w->prevPositionList[id] = w->positionList[id];
        }

        if (mask & COMPONENT_PLAYER) {
            if (mask & COMPONENT_ROTATION) {
                float turn_speed = 4.5f;
                if (CurrentInput.left)  w->rotationList[id].angle -= turn_speed * Time::Delta;
                if (CurrentInput.right) w->rotationList[id].angle += turn_speed * Time::Delta;

                constexpr float TWO_PI = 6.28318530718f;
                if (w->rotationList[id].angle < 0.0f) w->rotationList[id].angle += TWO_PI;
                else if (w->rotationList[id].angle >= TWO_PI) w->rotationList[id].angle -= TWO_PI;

                float dir_x = sinf(w->rotationList[id].angle);
                float dir_y = -cosf(w->rotationList[id].angle);

                float speed = 350.0f;
                LSMVECTOR vel = LSMVectorZero();
                if (CurrentInput.up) {
                    vel.x = dir_x * speed;
                    vel.y = dir_y * speed;
                } else if (CurrentInput.down) {
                    vel.x = -dir_x * (speed * 0.5f);
                    vel.y = -dir_y * (speed * 0.5f);
                }
                w->velocityList[id] = LSMStoreFloat2(vel);

                static float fire_cooldown = 0.0f;
                if (fire_cooldown > 0.0f) {
                    fire_cooldown -= Time::Delta;
                }

                bool wants_to_shoot = CurrentInput.shoot || (CurrentInput.shoot_held && fire_cooldown <= 0.0f);
                if (wants_to_shoot && fire_cooldown <= 0.0f) {
                    fire_cooldown = 0.15f;
                    CurrentInput.shoot = false;

                    float spawn_x = w->positionList[id].data[0] + 12.0f + dir_x * 16.0f;
                    float spawn_y = w->positionList[id].data[1] + 12.0f + dir_y * 16.0f;

                    Event shoot_evt;
                    shoot_evt.type = EVENT_SHOOT;
                    shoot_evt.data.shoot.x = spawn_x;
                    shoot_evt.data.shoot.y = spawn_y;
                    shoot_evt.data.shoot.dir_x = dir_x;
                    shoot_evt.data.shoot.dir_y = dir_y;
                    add_event(shoot_evt);
                }
            } else {
                float speed = 300.0f;
                LSMVECTOR vel = LSMVectorZero();
                if (CurrentInput.up)    vel.y = -speed;
                if (CurrentInput.down)  vel.y = speed;
                if (CurrentInput.left)  vel.x = -speed;
                if (CurrentInput.right) vel.x = speed;
                w->velocityList[id] = LSMStoreFloat2(vel);
            }
        }

        if ((mask & COMPONENT_POSITION) && (mask & COMPONENT_VELOCITY)) {
            LSMVECTOR pos, vel;
            LSMLoadVector(&w->positionList[id], 2, &pos);
            LSMLoadVector(&w->velocityList[id], 2, &vel);

            pos = pos + vel * Time::Delta;

            float width = 0.0f;
            float height = 0.0f;
            if (mask & COMPONENT_COLLIDER) {
                width = w->colliderList[id].width;
                height = w->colliderList[id].height;
            }

            if (mask & COMPONENT_PLAYER) {
                if (pos.x < 0.0f) pos.x = 0.0f;
                else if (pos.x + width > (float)WORLD_WIDTH) pos.x = (float)WORLD_WIDTH - width;

                if (pos.y < 0.0f) pos.y = 0.0f;
                else if (pos.y + height > (float)WORLD_HEIGHT) pos.y = (float)WORLD_HEIGHT - height;
            } else if (mask & COMPONENT_PROJECTILE) {
                if (pos.x < 0.0f || pos.x + width > (float)WORLD_WIDTH ||
                    pos.y < 0.0f || pos.y + height > (float)WORLD_HEIGHT ||
                    fabsf(pos.x - player_x) > 2000.0f || fabsf(pos.y - player_y) > 2000.0f) {
                    delete_entity(w, {id, w->status[id]});
                    continue;
                }
            } else {
                if (pos.x < 0.0f) {
                    pos.x = 0.0f;
                    vel.x = -vel.x;
                } else if (pos.x + width > (float)WORLD_WIDTH) {
                    pos.x = (float)WORLD_WIDTH - width;
                    vel.x = -vel.x;
                }

                if (pos.y < 0.0f) {
                    pos.y = 0.0f;
                    vel.y = -vel.y;
                } else if (pos.y + height > (float)WORLD_HEIGHT) {
                    pos.y = (float)WORLD_HEIGHT - height;
                    vel.y = -vel.y;
                }
            }

            w->positionList[id] = LSMStoreFloat2(pos);
            w->velocityList[id] = LSMStoreFloat2(vel);
        }
    }
}

void resolve_collision_system(World *w) {
    for (uint32_t i = 0; i < CollisionsCount; i++) {
        Collision current_coll = CollisionsList[i];
        uint32_t id1 = current_coll.e1;
        uint32_t id2 = current_coll.e2;

        bool e1_proj = (w->componentsMaskList[id1] & COMPONENT_PROJECTILE) != 0;
        bool e2_proj = (w->componentsMaskList[id2] & COMPONENT_PROJECTILE) != 0;

        if (e1_proj || e2_proj) {
            uint32_t proj_id = e1_proj ? id1 : id2;
            uint32_t other_id = e1_proj ? id2 : id1;

            if (!(w->componentsMaskList[other_id] & COMPONENT_PLAYER) &&
                (w->componentsMaskList[other_id] & COMPONENT_COLLIDER)) {
                Event hit_evt;
                hit_evt.type = EVENT_ASTEROID_HIT;
                hit_evt.data.asteroid_hit.asteroid_id = other_id;
                hit_evt.data.asteroid_hit.projectile_id = proj_id;
                add_event(hit_evt);
            }
            continue;
        }

        LSMVECTOR e1pos, e2pos;
        LSMLoadVector(&w->positionList[id1], 2, &e1pos);
        LSMLoadVector(&w->positionList[id2], 2, &e2pos);

        Collider e1collider = w->colliderList[id1];
        Collider e2collider = w->colliderList[id2];

        float overlap_x = 0.0f;
        float direction_x = 0.0f;
        if (e1pos.x < e2pos.x) {
            overlap_x = (e1pos.x + e1collider.width) - e2pos.x;
            direction_x = -1.0f;
        } else {
            overlap_x = (e2pos.x + e2collider.width) - e1pos.x;
            direction_x = 1.0f;
        }

        float overlap_y = 0.0f;
        float direction_y = 0.0f;
        if (e1pos.y < e2pos.y) {
            overlap_y = (e1pos.y + e1collider.height) - e2pos.y;
            direction_y = -1.0f;
        } else {
            overlap_y = (e2pos.y + e2collider.height) - e1pos.y;
            direction_y = 1.0f;
        }

        bool e1_dynamic = (w->componentsMaskList[id1] & COMPONENT_VELOCITY) != 0;
        bool e2_dynamic = (w->componentsMaskList[id2] & COMPONENT_VELOCITY) != 0;
        bool e1_player = (w->componentsMaskList[id1] & COMPONENT_PLAYER) != 0;
        bool e2_player = (w->componentsMaskList[id2] & COMPONENT_PLAYER) != 0;

        if (e1_dynamic && !e2_dynamic) {
            LSMVECTOR e1vel;
            LSMLoadVector(&w->velocityList[id1], 2, &e1vel);
            if (overlap_x <= overlap_y) {
                e1pos.x += overlap_x * direction_x;
                e1vel.x = e1_player ? 0.0f : -e1vel.x;
            } else {
                e1pos.y += overlap_y * direction_y;
                e1vel.y = e1_player ? 0.0f : -e1vel.y;
            }
            w->positionList[id1] = LSMStoreFloat2(e1pos);
            w->velocityList[id1] = LSMStoreFloat2(e1vel);
        } else if (!e1_dynamic && e2_dynamic) {
            LSMVECTOR e2vel;
            LSMLoadVector(&w->velocityList[id2], 2, &e2vel);
            if (overlap_x <= overlap_y) {
                e2pos.x -= overlap_x * direction_x;
                e2vel.x = e2_player ? 0.0f : -e2vel.x;
            } else {
                e2pos.y -= overlap_y * direction_y;
                e2vel.y = e2_player ? 0.0f : -e2vel.y;
            }
            w->positionList[id2] = LSMStoreFloat2(e2pos);
            w->velocityList[id2] = LSMStoreFloat2(e2vel);
        } else if (e1_player && e2_dynamic) {
            LSMVECTOR e2vel;
            LSMLoadVector(&w->velocityList[id2], 2, &e2vel);
            if (overlap_x <= overlap_y) {
                e2pos.x -= overlap_x * direction_x;
                float speed2 = fabsf(e2vel.x);
                if (speed2 < 40.0f) speed2 = 80.0f;
                e2vel.x = speed2 * (-direction_x);
            } else {
                e2pos.y -= overlap_y * direction_y;
                float speed2 = fabsf(e2vel.y);
                if (speed2 < 40.0f) speed2 = 80.0f;
                e2vel.y = speed2 * (-direction_y);
            }
            w->positionList[id2] = LSMStoreFloat2(e2pos);
            w->velocityList[id2] = LSMStoreFloat2(e2vel);
        } else if (e2_player && e1_dynamic) {
            LSMVECTOR e1vel;
            LSMLoadVector(&w->velocityList[id1], 2, &e1vel);
            if (overlap_x <= overlap_y) {
                e1pos.x += overlap_x * direction_x;
                float speed1 = fabsf(e1vel.x);
                if (speed1 < 40.0f) speed1 = 80.0f;
                e1vel.x = speed1 * direction_x;
            } else {
                e1pos.y += overlap_y * direction_y;
                float speed1 = fabsf(e1vel.y);
                if (speed1 < 40.0f) speed1 = 80.0f;
                e1vel.y = speed1 * direction_y;
            }
            w->positionList[id1] = LSMStoreFloat2(e1pos);
            w->velocityList[id1] = LSMStoreFloat2(e1vel);
        } else if (e1_dynamic && e2_dynamic) {
            LSMVECTOR e1vel, e2vel;
            LSMLoadVector(&w->velocityList[id1], 2, &e1vel);
            LSMLoadVector(&w->velocityList[id2], 2, &e2vel);

            if (overlap_x <= overlap_y) {
                float half_ox = overlap_x * 0.5f;
                e1pos.x += half_ox * direction_x;
                e2pos.x -= half_ox * direction_x;

                float speed1 = fabsf(e1vel.x);
                float speed2 = fabsf(e2vel.x);
                if (speed1 < 40.0f) speed1 = 80.0f;
                if (speed2 < 40.0f) speed2 = 80.0f;

                e1vel.x = speed1 * direction_x;
                e2vel.x = speed2 * (-direction_x);
            } else {
                float half_oy = overlap_y * 0.5f;
                e1pos.y += half_oy * direction_y;
                e2pos.y -= half_oy * direction_y;

                float speed1 = fabsf(e1vel.y);
                float speed2 = fabsf(e2vel.y);
                if (speed1 < 40.0f) speed1 = 80.0f;
                if (speed2 < 40.0f) speed2 = 80.0f;

                e1vel.y = speed1 * direction_y;
                e2vel.y = speed2 * (-direction_y);
            }

            w->positionList[id1] = LSMStoreFloat2(e1pos);
            w->velocityList[id1] = LSMStoreFloat2(e1vel);
            w->positionList[id2] = LSMStoreFloat2(e2pos);
            w->velocityList[id2] = LSMStoreFloat2(e2vel);
        }
    }
}
