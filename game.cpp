#include <forward_list>

#include "game.hpp"
#include "assets.hpp"
#include "camera.hpp"
#include "kart.hpp"
#include "minimap.hpp"
#include "sprite3d.hpp"
#include "track.hpp"

using namespace blit;

static Track *track;
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

static Kart kart;

static std::forward_list<Sprite3D *> display_sprites;


void init() {
    set_screen_mode(ScreenMode::hires);

    track = new Track(track_info[0]);

    minimap.set_map(&track->get_map());

    kart_sprites = Surface::load(asset_kart);

    // setup kart

    kart.is_player = true;

    auto &info = track->get_info();

    auto track_start_dir = Vec2(info.route[1] - info.route[0]);
    track_start_dir.normalize();

    kart.sprite.spritesheet = kart_sprites;
    kart.sprite.look_dir = Vec3(track_start_dir.x, 0.0f, track_start_dir.y);
    kart.sprite.world_pos = Vec3(
        (info.finish_line[0].x + info.finish_line[1].x) / 2,
        0.0f,
        (info.finish_line[0].y + info.finish_line[1].y) / 2
    ) - kart.sprite.look_dir * 48.0f;

    cam.look_at = kart.sprite.world_pos;
    cam.pos = cam.look_at - kart.sprite.look_dir * 64.0f + Vec3(0, 16.0f, 0);
    cam.update();
}

void render(uint32_t time) {
    screen.pen = Pen(0,0,0);
    screen.clear();

    track->render(cam);

    for(auto &sprite : display_sprites)
        sprite->render(cam);

    minimap.render();
}

void update(uint32_t time) {
    kart.update();


    // update camera
    Vec3 cam_look_at_target = kart.sprite.world_pos;
    Vec3 cam_pos_target = cam_look_at_target - kart.sprite.look_dir * 64.0f + Vec3(0, 16.0f, 0);

    cam.pos += (cam_pos_target - cam.pos) * 0.03f;
    cam.look_at += (cam_look_at_target - cam.look_at) * 0.1f;
    cam.update();

    // cull/sort
    auto check_sprite = [](Sprite3D &sprite) {
        float near = 1.0f, far = 500.0f; // may need adjusting

        if(sprite.z >= near && sprite.z <= far)
            display_sprites.push_front(&sprite);
    };

    display_sprites.clear();

    kart.sprite.update(cam);
    check_sprite(kart.sprite);

    display_sprites.sort([](Sprite3D *a, Sprite3D *b) {return a->z > b->z;});

    // minimap
    // TODO: maybe don't constantly recreate this
    minimap.update(Point(kart.sprite.world_pos.x, kart.sprite.world_pos.z) / 8);
}