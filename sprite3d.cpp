#include "sprite3d.hpp"

#include "graphics/surface.hpp"
#include "math/constants.hpp"

#include "camera.hpp"

using namespace blit;

void Sprite3D::update(const Camera &cam) {
    auto screen_pos = cam.world_to_screen(world_pos, screen_scale, z);

    screen_x = screen_pos.x;
    screen_y = screen_pos.y;
}

void Sprite3D::render(const Camera &cam) {
    int frame = 0;
    int transform = 0;

    if(rotation_frames) {
        // we have sprites for half the rotation, fliped for the other half (-2 for the front/back views)
        const int total_frames = rotation_frames + rotation_frames - 2; 
        Vec3 cam_to_sprite = world_pos - cam.pos;
        Vec3 sprite_look = look_dir;

        // ignore y
        cam_to_sprite.y = 0;
        sprite_look.y = 0;

        cam_to_sprite.normalize();
        sprite_look.normalize();

        float ang = std::atan2(cam_to_sprite.x * sprite_look.z - cam_to_sprite.z * sprite_look.x, cam_to_sprite.dot(sprite_look));
        ang = (pi * 2.0f) - ang;

        int rot = std::round((ang / (pi * 2.0f)) * total_frames);

        frame = (rot + 15) % total_frames;

        if(frame >= rotation_frames) {
            // go back through for the other half
            frame = (rotation_frames - 2) - (frame - rotation_frames);
            transform = SpriteTransform::HORIZONTAL;
        }
    }

    float fade_start = cam.far * 0.6f;

    if(z > fade_start)
        screen.alpha = std::max(0, static_cast<int>((1.0f - (z - fade_start) / (cam.far - fade_start)) * 255.0f * alpha));
    else
        screen.alpha = alpha * 255.0f;

    screen.sprites = spritesheet;
    screen.sprite(Rect(sheet_x + frame * size_w, sheet_y, size_w, size_h), {screen_x, screen_y}, {origin_x, origin_y}, screen_scale * scale, transform);

    screen.alpha = 255;
}
