#include "kart.hpp"

#include "engine/api.hpp"
#include "math/constants.hpp"
#include "types/mat4.hpp"

#include "track.hpp"

using namespace blit;

static const float kart_accel = 200.0f, kart_drag = 0.005f, kart_friction = 0.85f, kart_turn_speed = 60.0f;

Kart::Kart() {
    sprite.origin = Point(16, 26);

    sprite.size = Size(4, 4);
    sprite.rotation_frames = 16;
}

void Kart::update() {
    const float dt = 0.01f;

    if(is_player) {
        if(buttons & Button::A)
            acc = Vec2(sprite.look_dir.x, sprite.look_dir.z) * kart_accel;
        else
            acc = Vec2();

        if(buttons & Button::DPAD_LEFT)
            turn_speed = kart_turn_speed;
        else if(buttons & Button::DPAD_RIGHT)
            turn_speed = -kart_turn_speed;
        else
            turn_speed = joystick.x * -kart_turn_speed;
    } else
        auto_drive();

    sprite.look_dir.transform(Mat4::rotation(turn_speed * dt, Vec3(0.0f, 1.0f, 0.0f)));

    auto drag = vel * -kart_drag * vel.length();
    auto friction = vel * -kart_friction;

    vel += (acc + drag + friction) * dt;

    sprite.world_pos.x += vel.x * dt;
    sprite.world_pos.z += vel.y * dt;
}

void Kart::set_track(Track *track) {
    this->track = track;
}

void Kart::auto_drive() {
    // CPU control
    acc = Vec2(sprite.look_dir.x, sprite.look_dir.z) * kart_accel;

    // try to face the right way
    Vec2 look_2d(sprite.look_dir.x, sprite.look_dir.z);
    Vec2 pos_2d(sprite.world_pos.x, sprite.world_pos.z);
    float route_t;
    auto route_index = track->find_closest_route_segment(pos_2d, route_t);
    auto &info = track->get_info();

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
