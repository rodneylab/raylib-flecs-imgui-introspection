#include "systems.h"

#include "components.h"
#include "constants.h"
#include "physics.h"

#include <flecs/addons/cpp/entity.hpp>
#include <flecs/addons/cpp/flecs.hpp>
#include <flecs/addons/cpp/mixins/query/impl.hpp>
#include <flecs/addons/cpp/world.hpp>
#include <fmt/core.h>
#include <imgui.h>
#include <raylib.h>
#include <spdlog/spdlog.h>

#include <array>
#include <cstddef>
#include <string>

void draw_grid_system(
    const flecs::query<const Position, const GridComponent> &draw_grid_query)
{
    draw_grid_query.each(
        [](const Position & /* position */, const GridComponent &grid) {
            DrawGrid(grid.slices, grid.spacing);
        });
}

void draw_scene_text_system(const Font &font)
{
    const float kDefaultFontSize{10.F};
    DrawTextEx(font,
               "Press F9 for ImGui debug mode",
               Vector2{constants::kTextPositionX, constants::kTextPositionY},
               static_cast<float>(constants::kTextFontSize),
               static_cast<float>(constants::kTextFontSize) / kDefaultFontSize,
               DARKGRAY);
    DrawFPS(constants::kFPSPositionX, constants::kFPSPositionY);
}

void render_simulation_tree_node(DevPanelState &dev_panel_state)
{
    ImGui::SeparatorText("Simulation");

    constexpr float kRegularLightness{0.6F};
    constexpr float kHoveredLightness{0.7F};
    constexpr float kEngagedLightness{0.8F};
    constexpr float kRegularSaturation{0.6F};
    constexpr float kHoveredSaturation{0.7F};
    constexpr float kEngagedSaturation{0.8F};
    constexpr float kPlayHue{0.2857F};
    constexpr float kPauseHue{0.F};
    constexpr float kStepHue{0.5714F};

    // Play button
    ImGui::PushID(0);
    ImGui::PushStyleColor(
        ImGuiCol_Button,
        (ImVec4)ImColor::HSV(kPlayHue, kRegularSaturation, kRegularLightness));
    ImGui::PushStyleColor(
        ImGuiCol_ButtonHovered,
        (ImVec4)ImColor::HSV(kPlayHue, kHoveredSaturation, kHoveredLightness));
    ImGui::PushStyleColor(
        ImGuiCol_Button,
        (ImVec4)ImColor::HSV(kPlayHue, kEngagedSaturation, kEngagedLightness));
    if (ImGui::Button("Play"))
    {
        spdlog::info("Play clicked");
        if (dev_panel_state._paused)
        {
            dev_panel_state._paused = false;
        }
    }
    ImGui::PopStyleColor(3);
    ImGui::PopID();

    // Pause button
    ImGui::SameLine();
    ImGui::PushID(1);
    ImGui::PushStyleColor(
        ImGuiCol_Button,
        (ImVec4)ImColor::HSV(kPauseHue, kRegularSaturation, kRegularLightness));
    ImGui::PushStyleColor(
        ImGuiCol_ButtonHovered,
        (ImVec4)ImColor::HSV(kPauseHue, kHoveredSaturation, kHoveredLightness));
    ImGui::PushStyleColor(
        ImGuiCol_Button,
        (ImVec4)ImColor::HSV(kPauseHue, kEngagedSaturation, kEngagedLightness));
    if (ImGui::Button("Pause"))
    {
        spdlog::info("Pause clicked");
        if (!dev_panel_state._paused)
        {
            dev_panel_state._paused = true;
        }
    }
    ImGui::PopStyleColor(3);
    ImGui::PopID();

    // Step button
    ImGui::SameLine();
    ImGui::PushID(2);
    ImGui::PushStyleColor(
        ImGuiCol_Button,
        (ImVec4)ImColor::HSV(kStepHue, kRegularSaturation, kRegularLightness));
    ImGui::PushStyleColor(
        ImGuiCol_ButtonHovered,
        (ImVec4)ImColor::HSV(kStepHue, kHoveredSaturation, kHoveredLightness));
    ImGui::PushStyleColor(
        ImGuiCol_Button,
        (ImVec4)ImColor::HSV(kStepHue, kEngagedSaturation, kEngagedLightness));
    if (ImGui::Button("Step"))
    {
        spdlog::info("Step clicked");
        if (dev_panel_state._paused)
        {
            dev_panel_state._step = true;
        }
    }
    ImGui::PopStyleColor(3);
    ImGui::PopID();
}

void render_introspection_tree_node(const Position &position,
                                    const Velocity &velocity)
{
    if (ImGui::TreeNode("Sphere components"))
    {
        if (ImGui::TreeNode("Position"))
        {
            ImGui::Text( // NOLINT [cppcoreguidelines-pro-type-vararg]
                "%s",
                fmt::format("{:.{}f}", position._centre.x, 2).c_str());
            ImGui::SameLine();
            ImGui::Text( // NOLINT [cppcoreguidelines-pro-type-vararg]
                "%s",
                fmt::format("{:.{}f}", position._centre.y, 2).c_str());
            ImGui::SameLine();
            ImGui::Text( // NOLINT [cppcoreguidelines-pro-type-vararg]
                "%s",
                fmt::format("{:.{}f}", position._centre.z, 2).c_str());
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Velocity"))
        {
            ImGui::Text( // NOLINT [cppcoreguidelines-pro-type-vararg]
                "%s",
                fmt::format("{:.{}f}", velocity._value.x, 2).c_str());
            ImGui::SameLine();
            ImGui::Text( // NOLINT [cppcoreguidelines-pro-type-vararg]
                "%s",
                fmt::format("{:.{}f}", velocity._value.y, 2).c_str());
            ImGui::SameLine();
            ImGui::Text( // NOLINT [cppcoreguidelines-pro-type-vararg]
                "%s",
                fmt::format("{:.{}f}", velocity._value.z, 2).c_str());
            ImGui::TreePop();
        }
    }
}

void draw_dev_panel_system(
    const flecs::
        query<const Position, const Velocity, const SphereMesh, DevPanelState>
            &draw_dev_panel_query)
{
    draw_dev_panel_query.each([](const Position &position,
                                 const Velocity &velocity,
                                 const SphereMesh & /* sphere_mesh */,
                                 DevPanelState &dev_panel_state) {
        ImGui::Begin("Dev Panel");

        ImGui::Text("%s", // NOLINT [cppcoreguidelines-pro-type-vararg]
                    fmt::format("FPS: {}", GetFPS()).c_str());

        render_simulation_tree_node(dev_panel_state);
        render_introspection_tree_node(position, velocity);

        ImGui::TreePop();
        if (ImGui::TreeNode("Sphere colour"))
        {
            int index{0};
            for (const std::string &colour : constants::kSphereColourLabels)
            {
                ImGui::RadioButton(colour.c_str(),
                                   &dev_panel_state._selected_sphere_colour,
                                   index);
                ++index;
            }
        }
        ImGui::End();
    });
}

void draw_sphere_system(
    const flecs::query<const Position,
                       const SphereMesh,
                       const DevPanelState //const CameraComponent
                       > &draw_sphere_query)
{
    draw_sphere_query.each([](const Position &position,
                              const SphereMesh &sphere_mesh,
                              const DevPanelState &dev_panel_state) {
        const size_t selected_colour_index{
            static_cast<size_t>(dev_panel_state._selected_sphere_colour)};
        const Color sphere_colour{
            constants::kSphereColours[selected_colour_index]};
        DrawSphere(position._centre, sphere_mesh._radius, sphere_colour);
    });
}

void spawn_camera_system(const flecs::world &world, Camera3D *camera)
{
    world.entity().set<CameraComponent>(CameraComponent{camera});
}

void spawn_sphere_system(const flecs::world &world)
{
    constexpr float kSphereInitialPositionY{10.F};
    world.entity()
        .set<Position>(Position{Vector3{0.F, kSphereInitialPositionY, 0.F}})
        .set<SphereMesh>({constants::kSphereColours[0], 0.5F})
        .set<SphereCollider>(SphereCollider{0.5F})
        .set<Velocity>(Velocity{Vector3{0.5F, 0.F, 0.F}});
}

void spawn_floor_system(const flecs::world &world)
{
    world.entity()
        .set<Position>(Position{Vector3{0.F, -1.F, 0.F}})
        .add<GridComponent>()
        .set<BoxCollider>(BoxCollider{Vector3{5.F, 1.F, 5.F}});
}

void update_sphere_system(
    const flecs::query<const SphereCollider, Position, Velocity, DevPanelState>
        &update_sphere_query,
    const float frame_time,
    PhysicsEngine &physics_engine)
{
    update_sphere_query.each([frame_time, &physics_engine](
                                 const SphereCollider & /* sphere_collider */,
                                 Position &position,
                                 Velocity &velocity,
                                 DevPanelState &dev_panel_state) {
        const bool do_not_update_physics =
            dev_panel_state._paused && !dev_panel_state._step;
        physics_engine.update(frame_time,
                              position._centre,
                              velocity._value,
                              do_not_update_physics);

        // this needs to be a little more sophisticated - if there are multiple spheres, we should only flip step once all the spheres have been updated
        if (dev_panel_state._paused && dev_panel_state._step)
        {
            dev_panel_state._step = false;
        }
    });
}

void create_entity_colliders_system(const flecs::world &world,
                                    PhysicsEngine &physics_engine)
{
    world.each([&physics_engine](const BoxCollider &box_collider,
                                 const Position &position) {
        physics_engine.create_floor(box_collider._half_extent,
                                    position._centre);
    });

    world.each([&physics_engine](const SphereCollider &sphere_collider,
                                 const Position &position,
                                 const Velocity &velocity) {
        physics_engine.create_ball(sphere_collider._radius,
                                   position._centre,
                                   velocity._value);
    });
}
