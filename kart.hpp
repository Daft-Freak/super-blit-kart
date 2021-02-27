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

    int get_current_lap() const {return current_lap;}

    bool has_finished() const;
    uint32_t get_finish_time() const {return finish_time;}

    Sprite3D sprite;

    bool is_player = false;
    int current_place = 0;

private:
    void auto_drive();

    blit::Vec3 vel, acc;
    float turn_speed = 0.0f;

    float return_to_track_timer = 0.0f;
    blit::Vec3 return_pos_v, return_look_v;

    int current_lap = -1; // -1 because we start behind the finish line
    uint32_t finish_time = 0; // TODO: time from start of race?

    RaceState *race_state = nullptr;
};