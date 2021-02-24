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

    blit::Point world_to_screen(blit::Vec3 world_pos, float &scale) const {
        blit::Point screen_center(blit::screen.bounds.w / 2, blit::screen.bounds.h / 2);

        auto dist = world_pos - pos;

        blit::Vec3 tmp;
        tmp.x = dist.dot(right);
        tmp.y = -dist.dot(up);
        tmp.z = dist.dot(forward);

        scale = focal_distance / tmp.z;

        return blit::Point(blit::Vec2(tmp.x, tmp.y) * scale) + screen_center;
    }

    blit::Vec3 pos, look_at;

    blit::Vec3 forward, right, up;

    float focal_distance = 320.0f;
};