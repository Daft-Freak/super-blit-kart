#include "kart.hpp"

#include "engine/api.hpp"
#include "math/constants.hpp"
#include "types/mat4.hpp"

#include "race-state.hpp"
#include "track.hpp"

using namespace blit;

static const float kart_accel = 200.0f, kart_drag = 0.005f, kart_friction = 0.85f, kart_turn_speed = 60.0f;

static const float return_to_track_time = 2.0f;

Kart::Kart() {
    sprite.origin = Point(16, 26);

    sprite.size = Size(4, 4);
    sprite.rotation_frames = 16;
}

void Kart::update() {
    const float dt = 0.01f;

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
        if(buttons & Button::A)
            acc = Vec3(sprite.look_dir.x, 0.0f, sprite.look_dir.z) * kart_accel;
        else
            acc = Vec3();

        if(buttons & Button::DPAD_LEFT)
            turn_speed = kart_turn_speed;
        else if(buttons & Button::DPAD_RIGHT)
            turn_speed = -kart_turn_speed;
        else
            turn_speed = joystick.x * -kart_turn_speed;
    } else
        auto_drive();

    auto pos_2d = get_2d_pos();
    float track_friction = race_state->track->get_friction(pos_2d);
    bool on_track = track_friction != 0.0f;

    // under track
    if(sprite.world_pos.y < -30.0f) {
        // start putting back on the track
        vel = acc = Vec3();
        return_to_track_timer = return_to_track_time;

        // find a position somewhere on the nearest segment
        float route_t;
        auto route_index = race_state->track->find_closest_route_segment(pos_2d, route_t);
        auto &info = race_state->track->get_info();

        Vec2 route_vec(info.route[route_index + 1] - info.route[route_index]);
        route_t = std::max(0.0f, std::min(1.0f, route_t));
        Vec2 route_point = Vec2(info.route[route_index]) + route_vec * route_t;

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
        sprite.look_dir.transform(Mat4::rotation(turn_speed * dt, Vec3(0.0f, 1.0f, 0.0f)));

    auto drag = vel * -kart_drag * vel.length();
    auto friction = vel * -kart_friction * track_friction;

    if(race_state->started && !race_state->countdown)
        vel += (acc + drag + friction) * dt;

    bool was_above = sprite.world_pos.y >= 0.0f;

    sprite.world_pos += vel * dt;

    // fell through the track
    if(sprite.world_pos.y < 0.0f && track_friction > 0.0f && was_above)
        sprite.world_pos.y = 0.0f;

    // collisions - check every kart before this one
    for(auto other_kart = race_state->karts; other_kart != this; other_kart++) {
        auto &kart_a = *this, &kart_b = *other_kart;

        if(std::abs(kart_a.sprite.world_pos.y - kart_b.sprite.world_pos.y) > 1.0f)
            continue;

        const float kart_radius = 10.0f;

        auto vec = kart_a.get_2d_pos() - kart_b.get_2d_pos();
        float dist = vec.length();

        if(dist >= kart_radius * 2.0f)
            continue;

        vec /= dist;

        float penetration = kart_radius * 2.0f - dist;

        kart_a.vel += Vec3(vec.x, 0.0f, vec.y) * penetration;// * 0.5f;
        kart_b.vel -= Vec3(vec.x, 0.0f, vec.y) * penetration;// * 0.5f;
    }
}

void Kart::set_race_state(RaceState *race_state) {
    this->race_state = race_state;
}

void Kart::auto_drive() {
    // CPU control
    acc = Vec3(sprite.look_dir.x, 0.0f, sprite.look_dir.z) * kart_accel;

    // try to face the right way
    Vec2 look_2d(sprite.look_dir.x, sprite.look_dir.z);
    auto pos_2d = get_2d_pos();
    float route_t;
    auto route_index = race_state->track->find_closest_route_segment(pos_2d, route_t);
    auto &info = race_state->track->get_info();

    Vec2 route_vec(info.route[route_index + 1] - info.route[route_index]);
    Vec2 route_point = Vec2(info.route[route_index]) + route_vec * route_t;
    route_vec.normalize();

    // try to stay on track
    auto to_track_center = route_point - pos_2d;
    float dist = to_track_center.length();

    float recenter_turn_speed = 0.0f;

    if(dist > 40.0f) { // ~third of the track width
        to_track_center.normalize();

        float ang = to_track_center.angle(look_2d);

        if(std::abs(ang) < pi / 4.0f) // don't want to fully align with this vector or we'll be going sideways
            recenter_turn_speed = 0.0f;
        else
            recenter_turn_speed = ang < 0.0f ? -kart_turn_speed * 0.5f : kart_turn_speed * 0.5f;
    }

    // turn into corners
    auto next_segment = route_index + 1;

    if(next_segment + 1 == info.route_len)
        next_segment = 0; // wrap (the first and last points are the same)

    Vec2 next_route_vec(info.route[next_segment + 1] - info.route[next_segment]);
    next_route_vec.normalize();

    float ang = next_route_vec.angle(look_2d);

    turn_speed = std::min(kart_turn_speed, std::max(-kart_turn_speed, ang * 100.0f));

    // whatever is telling us to turn the hardest
    if(std::abs(recenter_turn_speed) > std::abs(turn_speed))
        turn_speed = recenter_turn_speed;
}
