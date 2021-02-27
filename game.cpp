#include <forward_list>

#include "graphics/color.hpp"

#include "game.hpp"
#include "assets.hpp"
#include "camera.hpp"
#include "kart.hpp"
#include "minimap.hpp"
#include "race-state.hpp"
#include "sprite3d.hpp"
#include "track.hpp"

using namespace blit;

static Minimap minimap;

static const Point track0_route[]{{512, 96}, {704, 96}, {928, 96}, {928, 704}, {928, 928}, {824, 928}, {624, 928}, {624, 672}, {624, 512}, {584, 512}, {360, 512}, {360, 736}, {360, 936}, {320, 936}, {96, 936}, {96, 320}, {96, 96}, {512, 96}};
static const TrackInfo track_info[] {
    {
        {{512, 32}, {512, 160}}, // finish line
        track0_route, std::size(track0_route), // route
        asset_map, asset_tiles // assets
    }
};

static Camera cam;

static Surface *kart_sprites;

static std::forward_list<Sprite3D *> display_sprites, display_sprites_below;

static RaceState state;

void init() {
    set_screen_mode(ScreenMode::hires);

    state.track = new Track(track_info[0]);

    minimap.set_map(&state.track->get_map());

    kart_sprites = Surface::load(asset_kart);

    // setup karts
    auto &info = state.track->get_info();

    auto track_start_dir = state.track->get_starting_dir();

    Vec2 finish_line(info.finish_line[1] - info.finish_line[0]);
    float w = finish_line.length();
    finish_line /= w;

    Vec2 start_pos = Vec2(info.finish_line[0]) + finish_line * w * (1.0f / 5.0f);

    float inc = (w / 5.0f) * 3.0f / 3.0f;

    int i = 0;
    for(auto &kart : state.karts) {
        kart.set_race_state(&state);
        kart.sprite.scale = 0.75f;

        kart.sprite.spritesheet = kart_sprites;
        kart.sprite.look_dir = Vec3(track_start_dir.x, 0.0f, track_start_dir.y);
        kart.sprite.world_pos = Vec3(
            start_pos.x,
            0.0f,
            start_pos.y
        ) - kart.sprite.look_dir * 48.0f;

        i++;
        if(i == 4) {
            start_pos -= finish_line * inc * 3.0f;
        } else
            start_pos += finish_line * inc;

        start_pos -= track_start_dir * 8.0f;
    }

    // player kart
    state.karts[0].is_player = true;
    cam.look_at = state.karts[0].get_pos();
    cam.pos = cam.look_at - state.karts[0].sprite.look_dir * 64.0f + Vec3(0, 16.0f, 0);
    cam.update();
}

static Mat3 scroll_callback(uint8_t y) {
    return Mat3::translation(Vec2(cam.look_at.x - screen.bounds.w / 2, cam.look_at.z - screen.bounds.h / 2));
}

void render(uint32_t time) {
    screen.pen = Pen(0,0,0);
    screen.clear();

    for(auto &sprite : display_sprites_below)
        sprite->render(cam);

    state.track->render(cam);

    for(auto &sprite : display_sprites)
        sprite->render(cam);

    if(state.countdown) {
        int num = std::ceil(state.countdown / 1000.0f);
        screen.pen = Pen(255, 255, 255);
        // TODO: big obvious numbers so this can be closer to the center
        screen.text(std::to_string(num), minimal_font, Point(screen.bounds.w / 2, screen.bounds.h / 8), true, center_center);
    }

    minimap.render();

    // kart locations on minimap
    auto viewport = minimap.get_viewport();
    Point minimap_pos(screen.bounds.w - viewport.w, screen.bounds.h  - viewport.h);
    auto old_clip = screen.clip;

    screen.clip = Rect(minimap_pos, viewport.size());

    auto off = minimap_pos - viewport.tl();

    int i = 0;
    for(auto &kart : state.karts) {
        // this will probably be a sprite eventually
        screen.pen = hsv_to_rgba(i++ / 8.0f, 1.0f, 1.0f);
        screen.rectangle(Rect(off + kart.get_tile_pos() - Point(4, 4), Size(8, 8)));
    }

    screen.clip = old_clip;
}

void update(uint32_t time) {

    if(!state.started && buttons)
        state.started = true;

    if(state.started && state.countdown)
        state.countdown -= 10;

    for(auto &kart : state.karts)
        kart.update();

    // update camera
    Vec3 cam_look_at_target = state.karts[0].get_pos();
    Vec3 cam_pos_target = cam_look_at_target - state.karts[0].sprite.look_dir * 64.0f + Vec3(0, 16.0f, 0);

    cam.pos += (cam_pos_target - cam.pos) * 0.03f;
    cam.look_at += (cam_look_at_target - cam.look_at) * 0.1f;

    cam.pos.y = std::max(1.0f, cam.pos.y); // prevent the camera going below the track
    cam.update();

    // cull/sort
    auto check_sprite = [](Sprite3D &sprite) {
        float near = 1.0f, far = 500.0f; // may need adjusting

        if(sprite.z >= near && sprite.z <= far) {
            if(sprite.world_pos.y < 0.0f)
                display_sprites_below.push_front(&sprite);
            else
                display_sprites.push_front(&sprite);
        }
    };

    display_sprites.clear();
    display_sprites_below.clear();

    for(auto &kart : state.karts) {
        kart.sprite.update(cam);
        check_sprite(kart.sprite);
    }

    auto sort_func = [](Sprite3D *a, Sprite3D *b) {return a->z > b->z;};
    display_sprites.sort(sort_func);
    display_sprites_below.sort(sort_func);

    // minimap
    // TODO: maybe don't constantly recreate this
    minimap.update(state.karts[0].get_tile_pos());
}