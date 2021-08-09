#pragma once

#include "graphics/surface.hpp"
#include "types/vec3.hpp"

class Camera;

class Sprite3D final {
public:
    void update(const Camera &cam);
    void render(const Camera &cam);

    blit::Vec3 world_pos;
    blit::Vec3 look_dir; // y ignored

    float scale = 1.0f;
    float alpha = 1.0f; // uint8?
    uint8_t origin_x = 0, origin_y = 0;

    uint8_t sheet_x = 0, sheet_y = 0;
    uint8_t size_w = 1, size_h = 1;
    uint16_t rotation_frames = 0;
    blit::Surface *spritesheet = nullptr;

    int16_t screen_x = 0, screen_y = 0;
    float screen_scale = 1.0f;
    float z = 0.0f; // used for sorting/culling
};
