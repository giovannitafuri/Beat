#include "Event.h"
#include "Assets.h"

EventsMachine GlobalEventsMachine;

void init_event_machine() {
    GlobalEventsMachine.currentList = 1; // A
    GlobalEventsMachine.countA = 0;
    GlobalEventsMachine.countB = 0;
}

void add_event(Event e) {
    if (GlobalEventsMachine.currentList == 1) { // A
        if (GlobalEventsMachine.countB < MAX_EVENTS) {
            GlobalEventsMachine.eventsListB[GlobalEventsMachine.countB++] = e;
        }
    } else {
        if (GlobalEventsMachine.countA < MAX_EVENTS) {
            GlobalEventsMachine.eventsListA[GlobalEventsMachine.countA++] = e;
        }
    }
}

Event* get_events(uint32_t *out_count) {
    if (GlobalEventsMachine.currentList == 1) {
        *out_count = GlobalEventsMachine.countA;
        return GlobalEventsMachine.eventsListA;
    } else {
        *out_count = GlobalEventsMachine.countB;
        return GlobalEventsMachine.eventsListB;
    }
}

void events_machine_clear() {
    if (GlobalEventsMachine.currentList == 1) {
        GlobalEventsMachine.currentList = 2; // Swap to B
        GlobalEventsMachine.countA = 0;
    } else {
        GlobalEventsMachine.currentList = 1; // Swap to A
        GlobalEventsMachine.countB = 0;
    }
}

void event_system(World* world, uint32_t* score) {
    uint32_t count = 0;
    Event* events = get_events(&count);
    if (!events || count == 0) return;

    for (uint32_t i = 0; i < count; i++) {
        Event& e = events[i];
        if (e.type == EVENT_SHOOT) {
            Entity p = create_entity(world);
            if (p.id != 0) {
                add_position_component(world, p.id, e.data.shoot.x, e.data.shoot.y);
                add_prev_position_component(world, p.id, e.data.shoot.x, e.data.shoot.y);
                float speed = 800.0f;
                add_velocity_component(world, p.id, e.data.shoot.dir_x * speed, e.data.shoot.dir_y * speed);
                add_collider_component(world, p.id, 8.0f, 8.0f);
                world->componentsMaskList[p.id] |= COMPONENT_PROJECTILE;

                Assets::Atlas* laser_atlas = Assets::Atlases.find("particles_16.xpm");
                if (!laser_atlas) laser_atlas = Assets::Atlases.find("assets/particles_16.xpm");
                if (laser_atlas) {
                    add_sprite_component(world, p.id, laser_atlas, 2);
                }
            }
        } else if (e.type == EVENT_ASTEROID_HIT) {
            uint32_t ast_id = e.data.asteroid_hit.asteroid_id;
            uint32_t proj_id = e.data.asteroid_hit.projectile_id;

            // Invariant: verify both entities are still active before mutating
            if ((world->componentsMaskList[ast_id] & COMPONENT_COLLIDER) &&
                (world->componentsMaskList[proj_id] & COMPONENT_PROJECTILE)) {

                float ast_w = world->colliderList[ast_id].width;
                float pos_x = world->positionList[ast_id].data[0];
                float pos_y = world->positionList[ast_id].data[1];
                float vx = (world->componentsMaskList[ast_id] & COMPONENT_VELOCITY) ? world->velocityList[ast_id].data[0] : 0.0f;
                float vy = (world->componentsMaskList[ast_id] & COMPONENT_VELOCITY) ? world->velocityList[ast_id].data[1] : 0.0f;

                delete_entity(world, {proj_id, world->status[proj_id]});
                delete_entity(world, {ast_id, world->status[ast_id]});

                if (score) (*score)++;

                if (ast_w > 32.5f) {
                    Assets::Atlas* atlas32 = Assets::Atlases.find("particles_32.xpm");
                    if (!atlas32) atlas32 = Assets::Atlases.find("assets/particles_32.xpm");
                    for (int k = 0; k < 2; k++) {
                        Entity child = create_entity(world);
                        if (child.id != 0) {
                            float sign = (k == 0) ? -1.0f : 1.0f;
                            float nvx = vy * sign + (k == 0 ? -40.0f : 40.0f);
                            float nvy = -vx * sign + (k == 0 ? 40.0f : -40.0f);
                            add_position_component(world, child.id, pos_x + 16.0f * k, pos_y + 16.0f * k);
                            add_prev_position_component(world, child.id, pos_x + 16.0f * k, pos_y + 16.0f * k);
                            add_velocity_component(world, child.id, nvx, nvy);
                            add_collider_component(world, child.id, 32.0f, 32.0f);
                            if (atlas32) add_sprite_component(world, child.id, atlas32, (k * 3) % 8);
                        }
                    }
                } else if (ast_w > 16.5f) {
                    Assets::Atlas* atlas16 = Assets::Atlases.find("particles_16.xpm");
                    if (!atlas16) atlas16 = Assets::Atlases.find("assets/particles_16.xpm");
                    for (int k = 0; k < 2; k++) {
                        Entity child = create_entity(world);
                        if (child.id != 0) {
                            float sign = (k == 0) ? -1.0f : 1.0f;
                            float nvx = vy * sign + (k == 0 ? -60.0f : 60.0f);
                            float nvy = -vx * sign + (k == 0 ? 60.0f : -60.0f);
                            add_position_component(world, child.id, pos_x + 8.0f * k, pos_y + 8.0f * k);
                            add_prev_position_component(world, child.id, pos_x + 8.0f * k, pos_y + 8.0f * k);
                            add_velocity_component(world, child.id, nvx, nvy);
                            add_collider_component(world, child.id, 16.0f, 16.0f);
                            if (atlas16) add_sprite_component(world, child.id, atlas16, (k * 5) % 8);
                        }
                    }
                }
            }
        }
    }
}
