#include "kart.hpp"

#include "engine/api.hpp"
#include "engine/engine.hpp"
#include "math/constants.hpp"
#include "types/mat4.hpp"

#include "race-state.hpp"
#include "save.hpp"
#include "track.hpp"

using namespace blit;

static const float kart_radius = 9.5f;
static const float kart_mass = 30.0f;

static const float kart_accel = 200.0f, kart_brake_rec_accel = -80.0f, kart_drag = 0.005f, kart_friction = 0.85f, kart_turn_speed = 0.5f;

static const float return_to_track_time = 2.0f;

Kart::Kart() {
    sprite.origin_x = 16;
    sprite.origin_y = 26;

    sprite.size_w = sprite.size_h = 4;
    sprite.rotation_frames = 16;
}

void Kart::update() {
    const float dt = 0.01f;

    if(disable_time)
        disable_time--;

    // put back on track
    if(return_to_track_timer > 0.0f) {
        const float half = return_to_track_time / 2.0f;

        if(return_to_track_timer > half) {
            sprite.world_pos.y += return_pos_v.y;
        } else {
            return_pos_v.y = 0.0f;

            sprite.world_pos += return_pos_v;
            sprite.look_dir += return_look_v;
            sprite.look_dir.normalize();
        }

        return_to_track_timer -= dt;
        return;
    }

    if(is_player) {
        // player input
        if(buttons & Button::A)
            acc = Vec3(sprite.look_dir.x, 0.0f, sprite.look_dir.z) * kart_accel;
        else if(buttons & Button::B)
            acc = Vec3(sprite.look_dir.x, 0.0f, sprite.look_dir.z) * kart_brake_rec_accel; // braking/reverse
        else
            acc = Vec3();

        if(buttons & Button::DPAD_LEFT)
            turn_speed = kart_turn_speed;
        else if(buttons & Button::DPAD_RIGHT)
            turn_speed = -kart_turn_speed;
        else
            turn_speed = joystick.x * -kart_turn_speed;

        // use item
        if(current_item != ItemType::None && (buttons.released & Button::X))
            use_item();
    } else
        auto_drive();

    auto pos_2d = get_2d_pos();
    float track_friction = race_state->track->get_friction(pos_2d);
    bool on_track = track_friction != 0.0f;

    current_route_segment = race_state->track->find_closest_route_segment(pos_2d, current_route_frac);

    // trigger boosts if friction is negative
    if(track_friction < 0.0f) {
        track_friction *= -1.0f;
        boost_time = 25;
    }

    auto &track_info = race_state->track->get_info();

    // under track
    if(sprite.world_pos.y < -30.0f) {
        // start putting back on the track
        vel = acc = Vec3();
        return_to_track_timer = return_to_track_time;

        // find a position somewhere on the nearest segment
        auto route_index = current_route_segment;
        auto &info = race_state->track->get_info();

        Vec2 route_vec(info.route[route_index + 1] - info.route[route_index]);
        current_route_frac = std::max(0.0f, std::min(1.0f, current_route_frac));
        Vec2 route_point = Vec2(info.route[route_index]) + route_vec * current_route_frac;

        Vec3 return_pos(route_point.x, 16.0f, route_point.y);

        // return to pointing the right way
        Vec3 return_look;
        return_look.x = route_vec.x;
        return_look.z = route_vec.y;
        return_look.normalize();

        // pre-calc how much we need to add at each step
        return_pos_v = (return_pos - get_pos()) / (return_to_track_time / 2.0f) * dt;
        return_look_v = (return_look - sprite.look_dir) / (return_to_track_time / 2.0f) * dt;

        return;
    } else if(sprite.world_pos.y != 0.0f)
        on_track = false;

    if(!on_track) // uh oh, we're flying
        acc = Vec3(0.0f, -300.0f, 0.0f); // override acceleration (no traction)
    else
        sprite.look_dir.transform(Mat4::rotation(turn_speed * vel.length() * dt, Vec3(0.0f, 1.0f, 0.0f)));

    // boost
    Vec3 boost_acc;
    if(boost_time) {
        boost_acc = Vec3(sprite.look_dir.x, 0.0f, sprite.look_dir.z) * kart_accel * 4.0f;
        boost_time--;
    }

    // update velocity
    auto drag = vel * -kart_drag * vel.length();
    auto friction = vel * -kart_friction * track_friction;

    // ignore accel if the race hasn't started or replaying a ghost (and not falling)
    bool ghost_finished = !is_ghost() || ghost_timer / 10 >= time_trial_data->ghost_data_used;
    if(!race_state->countdown && (acc.y != 0.0f || (ghost_finished && disable_time == 0)))
        vel += (acc + drag + friction + boost_acc) * dt;

    bool was_above = sprite.world_pos.y >= 0.0f;

    // apply velocity
    sprite.world_pos += vel * dt;

    // check if we crossed the finish line
    if(current_route_segment == 0 || current_route_segment == track_info.route_len - 2) {
        // at start/end of route;
        Vec2 finish_vec(track_info.finish_line[1] - track_info.finish_line[0]);

        float finish_side_before = finish_vec.x * (pos_2d.y - track_info.finish_line[0].y) - finish_vec.y * (pos_2d.x - track_info.finish_line[0].x);
        float finish_side_after = finish_vec.x * ((pos_2d.y + vel.y * dt) - track_info.finish_line[0].y) - finish_vec.y * ((pos_2d.x + vel.x * dt) - track_info.finish_line[0].x);
        bool crossed_finish = (finish_side_before < 0.0f) != (finish_side_after < 0.0f);

        if(crossed_finish && !has_finished()) {
            bool forwards = finish_side_after < 0.0f;

            current_lap += forwards ? 1 : -1; // uh, negative laps just so you can't cheat

            if(has_finished()) {
                is_player = false; // take over after race is done
                finish_time = now();
            } else if(forwards && current_lap >= 0 && !lap_start_time[current_lap])
                lap_start_time[current_lap] = now();
        }
    }

    // fell through the track
    if(sprite.world_pos.y < 0.0f && track_friction > 0.0f && was_above)
        sprite.world_pos.y = 0.0f;

    if(!is_ghost()) {
        // collisions - check every kart before this one
        for(auto other_kart = race_state->karts; other_kart != this; other_kart++) {
            auto &kart_a = *this, &kart_b = *other_kart;

            if(other_kart->is_ghost() || std::abs(kart_a.sprite.world_pos.y - kart_b.sprite.world_pos.y) > 1.0f)
                continue;

            auto vec = kart_a.get_2d_pos() - kart_b.get_2d_pos();
            float dist = vec.x * vec.x + vec.y * vec.y; // squared length

            if(dist >= (kart_radius + kart_radius) * (kart_radius + kart_radius))
                continue;

            dist = std::sqrt(dist);

            vec /= dist;

            float penetration = kart_radius * 2.0f - dist;

            // move apart
            kart_a.sprite.world_pos += Vec3(vec.x, 0.0f, vec.y) * penetration * 0.5f;
            kart_b.sprite.world_pos -= Vec3(vec.x, 0.0f, vec.y) * penetration * 0.5f;

            // boing
            float kart_a_mass = kart_mass, kart_b_mass = kart_mass; // maybe in future these will be different

            auto new_a_vel = (kart_a.vel * (kart_a_mass - kart_b_mass) + (kart_b.vel * kart_b_mass * 2.0f)) / (kart_a_mass + kart_b_mass);
            kart_b.vel     = (kart_b.vel * (kart_b_mass - kart_a_mass) + (kart_a.vel * kart_a_mass * 2.0f)) / (kart_a_mass + kart_b_mass);
            kart_a.vel = new_a_vel;
        }

        // collide with track obstacles
        for(size_t i = 0; i < track_info.num_collision_rects; i++) {
            auto &rect = track_info.collision_rects[i];
            if(rect.empty())
                continue;

            Vec2 kart_pos(get_2d_pos());

            Vec2 obstacle_pos(kart_pos);

            if(kart_pos.x < rect.x)
                obstacle_pos.x = rect.x;
            else if(kart_pos.x >= rect.x + rect.w)
                obstacle_pos.x = rect.x + rect.w;

            if(kart_pos.y < rect.y)
                obstacle_pos.y = rect.y;
            else if(kart_pos.y >= rect.y + rect.h)
                obstacle_pos.y = rect.y + rect.h;

            auto vec = kart_pos - obstacle_pos;
            float dist = vec.x * vec.x + vec.y * vec.y; // squared length

            if(dist > kart_radius * kart_radius)
                continue;

            dist = std::sqrt(dist);

            vec /= dist;

            float penetration = kart_radius - dist;

            vel *= dist / kart_radius;
            sprite.world_pos += Vec3(vec.x, 0.0f, vec.y) * penetration;
        }

        // track sprites
        for(auto &track_obj : race_state->track->get_objects())
            track_obj.collide(*this);
    }

    // record
    if(time_trial_data && is_player && ghost_timer++ % 10 == 0 && !has_finished()) {
        auto &ghost_entry = time_trial_data->ghost_data[time_trial_data->ghost_data_used++];
        ghost_entry.pos_x = std::floor(get_2d_pos().x * 8);
        ghost_entry.pos_z = std::floor(get_2d_pos().y * 8);

        ghost_entry.look_ang = std::floor(std::atan2(sprite.look_dir.z, sprite.look_dir.x) * 10430.0f); // ~0x7FFF / pi
    }
}

void Kart::set_race_state(RaceState *race_state) {
    this->race_state = race_state;
}

const Vec3 &Kart::get_vel() const {
    return vel;
}

void Kart::set_vel(const Vec3 &vel) {
    this->vel = vel;
}

float Kart::get_radius() const {
    return kart_radius;
}

bool Kart::has_finished() const {
    return current_lap >= 3;
}

int Kart::get_lap_time(int lap) const {
    if(lap < 0 || lap >= 3)
        return 0;

    // not finished yet
    if(lap == current_lap)
        return blit::now() - lap_start_time[lap];

    if(lap < 2)
        return lap_start_time[lap + 1] - lap_start_time[lap];

    // final lap
    return finish_time - lap_start_time[2];
}

int Kart::get_race_time() const {
    int time = 0;

    for(int lap = 0; lap < 3; lap++)
        time += get_lap_time(lap);

    return time;
}

float Kart::get_route_estimate() const {
    return current_route_segment + current_route_frac;
}

void Kart::collect_item() {
    if(current_item != ItemType::None)
        return;

    // pick up a random item
    current_item = static_cast<ItemType>(blit::random() % std::size(item_sprites));
}

void Kart::disable() {
    // still disabled from previous hit, ignore
    if(disable_time)
        return;

    vel = {};

    disable_time = 50;
}

void Kart::set_time_trial_data(TimeTrialSaveData *data) {
    time_trial_data = data;
}

void Kart::auto_drive() {
    // replay ghost
    if(is_ghost() && ghost_timer++ % 10 == 0) {
        int index = ghost_timer / 10;

        if(index < time_trial_data->ghost_data_used) {
            auto &ghost_entry = time_trial_data->ghost_data[index];
            auto &next_entry = time_trial_data->ghost_data[std::min(time_trial_data->ghost_data_used - 1, index + 1)];

            sprite.world_pos.x = ghost_entry.pos_x / 8.0f;
            sprite.world_pos.z = ghost_entry.pos_z / 8.0f;

            // calculate a velocity
            vel.x = (next_entry.pos_x - ghost_entry.pos_x) / 8.0f * 10.0f;
            vel.z = (next_entry.pos_z - ghost_entry.pos_z) / 8.0f * 10.0f;

            sprite.look_dir.x = std::cos(ghost_entry.look_ang / 10430.0f);
            sprite.look_dir.z = std::sin(ghost_entry.look_ang / 10430.0f);

            acc = Vec3();
            return;
        }
    }

    // maybe use item if we have one
    if(current_item != ItemType::None && (blit::random() & 0x3FF) < 1)
        use_item();

    // CPU control
    acc = Vec3(sprite.look_dir.x, 0.0f, sprite.look_dir.z) * kart_accel;

    // try to face the right way
    Vec2 look_2d(sprite.look_dir.x, sprite.look_dir.z);
    auto pos_2d = get_2d_pos();
    auto route_index = current_route_segment;
    auto &info = race_state->track->get_info();

    Vec2 route_vec(info.route[route_index + 1] - info.route[route_index]);
    Vec2 route_point = Vec2(info.route[route_index]) + route_vec * current_route_frac;

    // try to stay on track
    auto to_track_center = route_point - pos_2d;
    float dist = to_track_center.length();

    float recenter_turn_speed = 0.0f;

    // high friction - almost definitely off the track
    bool off_track = race_state->track->get_friction(pos_2d) > 1.0f;

    // ~third of the track width for rainbow
    if(dist > 40.0f || off_track) {
        to_track_center.normalize();

        float ang = to_track_center.angle(look_2d);

        float scale = off_track ? 1.0f : 0.5f; // turn harder if we're being slowed down

        if(std::abs(ang) < pi / 4.0f) // don't want to fully align with this vector or we'll be going sideways
            recenter_turn_speed = 0.0f;
        else
            recenter_turn_speed = ang < 0.0f ? -kart_turn_speed * scale : kart_turn_speed * scale;
    }

    // turn into corners
    auto next_segment = route_index + 1;

    if(next_segment + 1 == info.route_len)
        next_segment = 0; // wrap (the first and last points are the same)

    Vec2 next_route_vec(info.route[next_segment + 1] - info.route[next_segment]);
    next_route_vec.normalize();

    float ang = next_route_vec.angle(look_2d);

    turn_speed = std::min(kart_turn_speed, std::max(-kart_turn_speed, ang * 0.8f));

    // whatever is telling us to turn the hardest, unless we're off the track
    if(off_track || std::abs(recenter_turn_speed) > std::abs(turn_speed))
        turn_speed = recenter_turn_speed;
}

void Kart::use_item() {
    // doesn't create an object
    if(current_item == ItemType::Boost) {
        boost_time = 50;
        current_item = ItemType::None;
        return;
    }

    auto &item_sprite = item_sprites[static_cast<int>(current_item)];

    Vec3 pos;
    auto type = ObjectType::DroppedItem;
    if(current_item == ItemType::Drop) {
        pos = sprite.world_pos - sprite.look_dir * (kart_radius + item_sprite.w * 4.0f);
    } else { // projectile
        pos = sprite.world_pos + sprite.look_dir * (kart_radius + item_sprite.w * 4.0f + 2.0f);
        type = ObjectType::Projectile;
    }

    TrackObject obj(type);

    obj.sprite.world_pos = pos;
    // center bottom
    obj.sprite.origin_x = item_sprite.w * 4;
    obj.sprite.origin_y = item_sprite.h * 8;

    obj.sprite.sheet_x = item_sprite.x;
    obj.sprite.sheet_y = item_sprite.y;
    obj.sprite.size_w = item_sprite.w;
    obj.sprite.size_h = item_sprite.h;

    obj.vel = sprite.look_dir * 200.0f; // only used for projectiles

    race_state->track->add_object(obj);

    current_item = ItemType::None;
}
