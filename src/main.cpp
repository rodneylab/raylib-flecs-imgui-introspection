#include "components.h"
#include "constants.h"
#include "game/game.h"
#include "physics.h"
#include "systems.h"

#include <flecs/addons/cpp/entity.hpp>
#include <flecs/addons/cpp/flecs.hpp>
#include <flecs/addons/cpp/mixins/query/impl.hpp>
#include <flecs/addons/cpp/world.hpp>
#include <imgui.h>
#include <raylib.h>
#include <rlImGui.h>
#include <spdlog/spdlog.h>

#include <cstdint>
#include <queue>
#include <string>

void setup_camera_system(Camera3D &camera)
{
    camera.position = Vector3{constants::kCameraPositionX,
                              constants::kCameraPositionY,
                              constants::kCameraPositionZ};
    constexpr float kCameraTargetX{0.F};
    camera.target = Vector3{kCameraTargetX, 0.F, 0.F};
    camera.up = Vector3{0.F, 1.F, 0.F};
    camera.fovy = constants::kCameraFovY;
    camera.projection = CAMERA_PERSPECTIVE;
}

int main(int /* argc */, char ** /* argv */)
{
    const flecs::world world;
    spawn_floor_system(world);
    spawn_sphere_system(world);
    flecs::entity dev_panel_state_entity = world.entity<DevPanelState>();
    dev_panel_state_entity.set<DevPanelState>({0});

    double tickTimer{0.0};
    std::queue<int> keyQueue{std::queue<int>()};
    bool debugMenu = false;
    const Vector2 windowSize{
        Vector2{constants::kWindowWidth, constants::kWindowHeight}};
    RenderTexture gameTexture;
    RenderTexture debugTexture;

    SetWindowState(FLAG_MSAA_4X_HINT);
    InitWindow(static_cast<int>(windowSize.x),
               static_cast<int>(windowSize.y),
               constants::kTitle.c_str());
    rlImGuiSetup(true);
    ImGui::GetStyle().ScaleAllSizes(constants::kUIScaleFactor);
    ImGui::GetIO().FontGlobalScale = constants::kUIScaleFactor;

    gameTexture = LoadRenderTexture((int)windowSize.x, (int)windowSize.y);
    constexpr float kDebugScaleUp{1.5F};
    debugTexture =
        LoadRenderTexture(static_cast<int>(windowSize.x / kDebugScaleUp),
                          static_cast<int>(windowSize.y / kDebugScaleUp));

    const Rectangle source_rectangle{0,
                                     -windowSize.y,
                                     windowSize.x,
                                     -windowSize.y};
    const Rectangle destination_rectangle{0,
                                          0,
                                          windowSize.x / kDebugScaleUp,
                                          windowSize.y / kDebugScaleUp};
    Camera3D camera{};
    setup_camera_system(camera);

    constexpr int kMillisecondsPerSecond{1000};
    const Font font{LoadFont(ASSETS_PATH "ibm-plex-mono-v19-latin-500.ttf")};

    spdlog::info("Creating Physics Engine");
    PhysicsEngine physics_engine{};

    spdlog::info("Initialising Physics Engine");
    physics_engine.initialise();

    spdlog::info("Creating colliders");
    create_entity_colliders_system(world, physics_engine);

    spdlog::info("Initiating Pre-simulation Optimisation");
    physics_engine.start_simulation();

    const flecs::
        query<const Position, const Velocity, const SphereMesh, DevPanelState>
            draw_dev_panel_query{world
                                     .query_builder<const Position,
                                                    const Velocity,
                                                    const SphereMesh,
                                                    DevPanelState>()
                                     .term_at(4)
                                     .singleton()
                                     .build()};
    const flecs::query<const Position, const GridComponent> draw_grid_query{
        world.query_builder<const Position, const GridComponent>().build()};
    const flecs::query<const Position, const SphereMesh, const DevPanelState>
        draw_sphere_query{world
                              .query_builder<const Position,
                                             const SphereMesh,
                                             const DevPanelState>()
                              .term_at(3)
                              .singleton()
                              .build()};

    const flecs::query<const SphereCollider, Position, Velocity, DevPanelState>
        update_sphere_query{world
                                .query_builder<const SphereCollider,
                                               Position,
                                               Velocity,
                                               DevPanelState>()
                                .term_at(4)
                                .singleton()
                                .build()};

    // We simulate the physics world in discrete time steps. 60 Hz is a good rate
    // to update the physics system.
    SetTargetFPS(constants::kTargetFramerate);

    spdlog::info("Starting Simulation");

    while (!WindowShouldClose())
    {
        const float frame_time{GetFrameTime()};
        if (GetTime() - tickTimer >
            static_cast<float>(kMillisecondsPerSecond) /
                static_cast<float>(constants::kTickrate) /
                static_cast<float>(kMillisecondsPerSecond))
        {
            tickTimer = GetTime();
            Game_Update(&keyQueue, &debugMenu);
        }

        keyQueue.push(GetKeyPressed());

        BeginDrawing();
        rlImGuiBegin();
        ClearBackground(DARKGRAY);

        if (debugMenu)
        {
            BeginTextureMode(gameTexture);
            ClearBackground(RAYWHITE);
            BeginMode3D(camera);
            draw_grid_system(draw_grid_query);
            draw_sphere_system(draw_sphere_query);
            EndMode3D();
            draw_scene_text_system(font);
            EndTextureMode();

            BeginTextureMode(debugTexture);
            DrawTexturePro(gameTexture.texture,
                           source_rectangle,
                           destination_rectangle,
                           {0, 0},
                           0.F,
                           RAYWHITE);
            EndTextureMode();

            draw_dev_panel_system(draw_dev_panel_query);

            ImGui::Begin(
                "Jolt raylib Hello World!",
                &debugMenu,
                static_cast<uint8_t>( // NOLINT [hicpp-signed-bitwise]
                    ImGuiWindowFlags_AlwaysAutoResize) |
                    static_cast<uint8_t>(ImGuiWindowFlags_NoResize) |
                    static_cast<uint8_t>(ImGuiWindowFlags_NoBackground));
            rlImGuiImageRenderTexture(&debugTexture);
            ImGui::End();
        }
        else
        {
            ClearBackground(RAYWHITE);
            BeginMode3D(camera);
            draw_grid_system(draw_grid_query);
            draw_sphere_system(draw_sphere_query);
            EndMode3D();
            draw_scene_text_system(font);
        }
        rlImGuiEnd();
        EndDrawing();

        // advance the physics engine one step and get the updated sphere_position
        update_sphere_system(update_sphere_query, frame_time, physics_engine);
    }

    spdlog::info("Preparing Physics Engine for Shutdown");
    physics_engine.cleanup();

    return 0;
}
