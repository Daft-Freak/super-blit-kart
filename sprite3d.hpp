#pragma once

#include "graphics/surface.hpp"
#include "types/point.hpp"
#include "types/size.hpp"
#include "types/vec3.hpp"

class Camera;

class Sprite3D final {
public:
    void update(const Camera &cam);
    void render(const Camera &cam);

    blit::Vec3 world_pos;
    blit::Vec3 look_dir;

    blit::Point origin;
    float scale = 1.0f;

    blit::Surface *spritesheet = nullptr;
    blit::Point sheet_base;
    blit::Size size = blit::Size(1, 1);
    int rotation_frames = 0;

    blit::Point screen_pos;
    float screen_scale = 1.0f;
    float z = 0.0f; // used for sorting/culling
};