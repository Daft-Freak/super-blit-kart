#pragma once

#include "sprite3d.hpp"

#include "types/vec2.hpp"
#include "types/vec3.hpp"

class Track;

class Kart final {
public:
    Kart();

    void update();

    void set_track(Track *track);

    Sprite3D sprite;

    bool is_player = false;

    blit::Vec3 vel;

private:
    void auto_drive();

    blit::Vec3 acc;
    float turn_speed = 0.0f;

    Track *track = nullptr;
};