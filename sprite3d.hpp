#pragma once

#include "graphics/surface.hpp"
#include "types/point.hpp"
#include "types/size.hpp"
#include "types/vec3.hpp"

class Camera;

class Sprite3D final {
public:
    void render(const Camera &cam);

    blit::Vec3 world_pos;
    blit::Vec3 look_dir;

    blit::Point origin;

    blit::Surface *spritesheet = nullptr;
    blit::Point sheet_base;
    blit::Size size = blit::Size(1, 1);
    int rotation_frames = 0;
};