#include "kart.hpp"

#include "engine/api.hpp"
#include "types/mat4.hpp"

using namespace blit;

static const float kart_accel = 200.0f, kart_drag = 0.005f, kart_friction = 0.75f, kart_turn_speed = 60.0f;

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
    }

    sprite.look_dir.transform(Mat4::rotation(turn_speed * dt, Vec3(0.0f, 1.0f, 0.0f)));

    auto drag = vel * -kart_drag * vel.length();
    auto friction = vel * -kart_friction;

    vel += (acc + drag + friction) * dt;

    sprite.world_pos.x += vel.x * dt;
    sprite.world_pos.z += vel.y * dt;
}
