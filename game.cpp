#include <forward_list>

#include "game.hpp"
#include "assets.hpp"
#include "camera.hpp"
#include "kart.hpp"
#include "minimap.hpp"
#include "sprite3d.hpp"
#include "track.hpp"

using namespace blit;

static Surface *map_tiles;
static TileMap *map;
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

void load_tilemap() {
    // some of this should really be in the API...
    #pragma pack(push,1)
    struct TMX {
        char head[4];
        uint8_t empty_tile;
        uint16_t width;
        uint16_t height;
        uint16_t layers;
        uint8_t data[];
    };
    #pragma pack(pop)

    auto map_struct = reinterpret_cast<const TMX *>(asset_map);
    auto layer_size = map_struct->width * map_struct->height;

    // const_cast the tile data (not going to modify it)
    map = new TileMap(
        const_cast<uint8_t *>(map_struct->data),
        const_cast<uint8_t *>(map_struct->data + layer_size),
        Size(map_struct->width, map_struct->height),
        map_tiles
    );
}

void init() {
    set_screen_mode(ScreenMode::hires);

    map_tiles = Surface::load(asset_tiles);
    load_tilemap();
    minimap.set_map(map);

    kart_sprites = Surface::load(asset_kart);

    // setup kart

    kart.is_player = true;

    auto &track = track_info[0];

    auto track_start_dir = Vec2(track.route[1] - track.route[0]);
    track_start_dir.normalize();

    kart.sprite.spritesheet = kart_sprites;
    kart.sprite.look_dir = Vec3(track_start_dir.x, 0.0f, track_start_dir.y);
    kart.sprite.world_pos = Vec3(
        (track.finish_line[0].x + track.finish_line[1].x) / 2,
        0.0f,
        (track.finish_line[0].y + track.finish_line[1].y) / 2
    ) - kart.sprite.look_dir * 48.0f;

    cam.look_at = kart.sprite.world_pos;
    cam.pos = cam.look_at - kart.sprite.look_dir * 64.0f + Vec3(0, 16.0f, 0);
    cam.update();
}

Mat3 mode7_scanline_transform(uint8_t y) {
    float top = screen.bounds.h / 2;
    float left = -screen.bounds.w / 2;

    // pitch
    float pitch_c = cam.up.y;
    float pitch_s = -cam.forward.y;

    float yb = (y - top) * pitch_c + cam.focal_distance * pitch_s;
    float zb = (y - top) * pitch_s - cam.focal_distance * pitch_c;

    float l = cam.pos.y / yb;

    // yaw
    float scaled_yaw_c = l * cam.right.x;
    float scaled_yaw_s = l * cam.right.z;

    // final translation
    float tx = cam.pos.x + scaled_yaw_c * left - scaled_yaw_s * zb;
    float ty = cam.pos.z + scaled_yaw_s * left + scaled_yaw_c * zb;

    Mat3 mat = Mat3::identity();

    mat.v00 = scaled_yaw_c; mat.v10 = scaled_yaw_s;

    mat.v11 = 0.0f;

    mat.v02 = tx; mat.v12 = ty;

    return mat;
}

void render(uint32_t time) {
    screen.pen = Pen(0,0,0);
    screen.clear();

    int horizon = (screen.bounds.h / 2) - ((-cam.forward.y * cam.focal_distance) / cam.up.y);
    map->draw(&screen, Rect(0, horizon, screen.bounds.w, screen.bounds.h - horizon), mode7_scanline_transform);

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