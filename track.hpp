#pragma once
#include <cstdint>

#include "graphics/surface.hpp"
#include "types/point.hpp"
#include "types/rect.hpp"
#include "types/vec2.hpp"

class Camera;

namespace blit {
    class TileMap;
}

class TrackInfo final {
public:
    const char *name;
    blit::Point finish_line[2];

    const blit::Point *route;
    size_t route_len;

    const blit::Rect *collision_rects;
    size_t num_collision_rects;

    const float *tile_friction;
    size_t tile_friction_len;

    const uint8_t *map_asset, *tiles_asset;

    blit::Pen background_col;
};

class Track final {
public:
    Track(const TrackInfo &info);
    ~Track();

    void render(const Camera &cam);

    const TrackInfo &get_info() const;

    blit::TileMap &get_map();

    unsigned int find_closest_route_segment(blit::Vec2 pos, float &segment_t) const;

    float get_friction(blit::Vec2 pos);

    blit::Vec2 get_starting_dir() const;

    void set_fog(float fog);

private:
    void load_tilemap();

    const TrackInfo &info;

    blit::Surface *tiles;
    blit::TileMap *map;

    float fog = 170.0f;
};