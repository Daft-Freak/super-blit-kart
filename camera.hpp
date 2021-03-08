#pragma once

#include "engine/engine.hpp"
#include "types/point.hpp"
#include "types/vec3.hpp"

class Camera final {
public:
    void update() {
        forward = look_at - pos;
        forward.normalize();

        right = blit::Vec3(-forward.z, 0.0f, forward.x);
        right.normalize();

        up = right.cross(forward);
    }

    blit::Point world_to_screen(blit::Vec3 world_pos, float &scale, float &z) const {
        blit::Point screen_center(viewport.x + viewport.w / 2, viewport.y + viewport.h / 2);

        auto dist = world_pos - pos;

        float x = dist.dot(right);
        float y = -dist.dot(up);
        z = dist.dot(forward);

        scale = focal_distance / z;

        return blit::Point(blit::Vec2(x, y) * scale) + screen_center;
    }

    blit::Vec3 pos, look_at;

    blit::Vec3 forward, right, up;

    float focal_distance = 320.0f;

    float near = 1.0f, far = 500.0f;

    blit::Rect viewport;
};