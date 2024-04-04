#ifndef SRC_COMPONENTS_H
#define SRC_COMPONENTS_H

#include "constants.h"

#include <raylib.h>

struct GridComponent
{
    GridComponent() = default;

    int slices{10};
    float spacing{1.F};
};

struct SphereMesh
{
    SphereMesh() : _colour{BLACK}, _radius{1.F}
    {
    }

    SphereMesh(const Color &colour, const float radius)
        : _colour(colour), _radius(radius)
    {
    }

    Color _colour;
    float _radius;
};

struct Position
{
    Position() = default;
    explicit Position(const Vector3 &centre)
        : _centre{centre.x, centre.y, centre.z}
    {
    }

    Vector3 _centre;
};

struct CameraComponent
{
    CameraComponent() = default;
    explicit CameraComponent(Camera3D *camera) : _camera{camera}
    {
        _camera->position = Vector3{constants::kCameraPositionX,
                                    constants::kCameraPositionY,
                                    constants::kCameraPositionZ};
        _camera->target = Vector3{0.F, 0.F, 0.F};
        _camera->up = Vector3{0.F, 1.F, 0.F};
        _camera->fovy = constants::kCameraFovY;
        _camera->projection = CAMERA_PERSPECTIVE;
    }

    Camera3D *_camera;
};

struct Velocity
{
    Velocity() = default;
    explicit Velocity(const Vector3 &value) : _value{value.x, value.y, value.z}
    {
    }

    Vector3 _value;
};

struct BoxCollider
{
    BoxCollider() = default;
    explicit BoxCollider(const Vector3 &half_extent)
        : _half_extent{half_extent.x, half_extent.y, half_extent.z}
    {
    }

    Vector3 _half_extent;
};

struct SphereCollider
{
    SphereCollider() = default;
    explicit SphereCollider(const float radius) : _radius(radius)
    {
    }

    float _radius;
};

struct DevPanelState
{
    DevPanelState() = default;

    int _selected_sphere_colour{0};
    bool _paused{false};
    bool _step{
        false}; // signal that frame should only advance one frame, then pause
};

#endif
