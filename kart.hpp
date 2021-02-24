#pragma once

#include "sprite3d.hpp"

#include "types/vec2.hpp"

class Kart final {
public:
    Kart();

    void update();

    Sprite3D sprite;

    bool is_player = false;

    blit::Vec2 vel, acc;
    float turn_speed = 0.0f;
};