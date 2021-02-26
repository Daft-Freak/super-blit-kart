#include "track.hpp"

#include "graphics/surface.hpp"
#include "graphics/tilemap.hpp"

#include "camera.hpp"

using namespace blit;

static Mat3 mode7_scanline_transform(const Camera &cam, uint8_t y) {
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

Track::Track(const TrackInfo &info) : info(info) {
    tiles = Surface::load(info.tiles_asset);
    load_tilemap();
}

void Track::render(const Camera &cam) {
    using namespace std::placeholders;

    int horizon = (screen.bounds.h / 2) - ((-cam.forward.y * cam.focal_distance) / cam.up.y);
    map->draw(&screen, Rect(0, horizon, screen.bounds.w, screen.bounds.h - horizon), std::bind(mode7_scanline_transform, cam, _1));
}

unsigned int Track::find_closest_route_segment(Vec2 pos, float &segment_t) const {
    unsigned int ret = 0;
    float min_dist = INFINITY;

    for(size_t i = 0; i < info.route_len - 1; i++) {
        Vec2 a(info.route[i]), b(info.route[i + 1]);

        // intersection
        Vec2 r = b - a;

        Vec2 dir(r.y, -r.x);

        float rxs = (r.x * dir.y) - (r.y * dir.x);
        float t = ((pos.x - a.x) * dir.y - (pos.y - a.y) * dir.x) / rxs; // distance along route segment

        // clamp to segment
        Vec2 route_point;

        if(t < 0.0f)
            route_point = a;
        else if(t > 1.0f)
            route_point = b;
        else
            route_point = a + r * t;

        float dist = (pos - route_point).length();

        if(dist < min_dist) {
            ret = i;
            segment_t = t;
            min_dist = dist;
        }
    }

    return ret;
}

float Track::get_friction(blit::Vec2 pos) {
    Point tile_coord(pos / Vec2(8.0f, 8.0f));

    if(tile_coord.x < 0 || tile_coord.y < 0 || tile_coord.x >= map->bounds.w || tile_coord.y >= map->bounds.h)
        return 0.0f;

    // "empty" tile
    if(map->tile_at(tile_coord) == 0)
        return 0.0f;

    // TODO: tiles with more friction

    return 1.0f;
}

const TrackInfo &Track::get_info() const {
    return info;
}

TileMap &Track::get_map() {
    return *map;
}

void Track::load_tilemap() {
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

    auto map_struct = reinterpret_cast<const TMX *>(info.map_asset);
    auto layer_size = map_struct->width * map_struct->height;

    // const_cast the tile data (not going to modify it)
    map = new TileMap(
        const_cast<uint8_t *>(map_struct->data),
        const_cast<uint8_t *>(map_struct->data + layer_size),
        Size(map_struct->width, map_struct->height),
        tiles
    );
}