#ifndef SRC_SYSTEMS_H
#define SRC_SYSTEMS_H

#include "components.h"
#include "physics.h"

#include <flecs.h> // NOLINT [misc-include-cleaner]
#include <flecs/addons/cpp/flecs.hpp>
#include <flecs/addons/cpp/mixins/query/impl.hpp>
#include <flecs/addons/cpp/world.hpp>
#include <raylib.h>

void draw_grid_system(
    const flecs::query<const Position, const GridComponent> &draw_grid_query);
void draw_scene_text_system(const Font &font);
void draw_dev_panel_system(
    const flecs::
        query<const Position, const Velocity, const SphereMesh, DevPanelState>
            &draw_dev_panel_query);
void draw_sphere_system(
    const flecs::query<const Position, const SphereMesh, const DevPanelState>
        &draw_sphere_query);

void spawn_sphere_system(const flecs::world &world);
void spawn_floor_system(const flecs::world &world);

void update_sphere_system(
    const flecs::query<const SphereCollider, Position, Velocity, DevPanelState>
        &update_sphere_query,
    float frame_time,
    PhysicsEngine &physics_engine);
void create_entity_colliders_system(const flecs::world &world,
                                    PhysicsEngine &physics_engine);

#endif
