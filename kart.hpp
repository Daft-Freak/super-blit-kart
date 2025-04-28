#pragma once

#include "items.hpp"
#include "sprite3d.hpp"

#include "types/point.hpp"
#include "types/vec2.hpp"
#include "types/vec3.hpp"

class RaceState;
struct TimeTrialSaveData;

class Kart final {
public:
    Kart();

    void update();

    void set_race_state(RaceState *race_state);

    // position helpers
    const blit::Vec3 &get_pos() const {return sprite.world_pos;}
    blit::Vec2 get_2d_pos() const {return blit::Vec2(sprite.world_pos.x, sprite.world_pos.z);}
    blit::Point get_tile_pos() const {return blit::Point(sprite.world_pos.x / 8.0f, sprite.world_pos.z / 8.0f);}

    const blit::Vec3 &get_vel() const;
    void set_vel(const blit::Vec3 &vel);

    float get_radius() const;

    int get_current_lap() const {return current_lap;}

    bool has_finished() const;
    uint32_t get_finish_time() const {return finish_time;}
    int get_lap_time(int lap) const;
    int get_race_time() const;

    float get_route_estimate() const;

    void collect_item();
    ItemType get_current_item() const {return current_item;}

    void disable();

    void set_time_trial_data(TimeTrialSaveData *data, int offset = 0, int length = 0);
    bool is_ghost() const {return time_trial_data && !is_player;}

    Sprite3D sprite;

    bool is_player = false;
    int current_place = 0;
    int kart_index = 0; // index into kart_info

private:
    void auto_drive();

    void use_item();

    blit::Vec3 vel, acc;
    float turn_speed = 0.0f;

    int boost_time = 0, disable_time = 0;

    float return_to_track_timer = 0.0f;
    blit::Vec3 return_pos_v, return_look_v;

    int current_lap = -1; // -1 because we start behind the finish line
    uint32_t lap_start_time[3]{0};
    uint32_t finish_time = 0;

    unsigned int current_route_segment = 0;
    float current_route_frac = 0.0f;

    ItemType current_item = ItemType::None;

    TimeTrialSaveData *time_trial_data = nullptr; // if this is set we're either recording a time trial (is_player), or a ghost
    int ghost_offset = 0, ghost_len = 0; // used for replay
    int ghost_timer = 0; // read/write every 10 updates

    RaceState *race_state = nullptr;
};