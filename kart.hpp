#pragma once

#include "sprite3d.hpp"

#include "types/point.hpp"
#include "types/vec2.hpp"
#include "types/vec3.hpp"

class RaceState;

class Kart final {
public:
    Kart();

    void update();

    void set_race_state(RaceState *race_state);

    // position helpers
    const blit::Vec3 &get_pos() const {return sprite.world_pos;}
    blit::Vec2 get_2d_pos() const {return blit::Vec2(sprite.world_pos.x, sprite.world_pos.z);}
    blit::Point get_tile_pos() const {return blit::Point(sprite.world_pos.x / 8.0f, sprite.world_pos.z / 8.0f);}

    Sprite3D sprite;

    bool is_player = false;

private:
    void auto_drive();

    blit::Vec3 vel, acc;
    float turn_speed = 0.0f;

    RaceState *race_state = nullptr;
};