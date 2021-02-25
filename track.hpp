#pragma once
#include <cstdint>

#include "types/point.hpp"
#include "types/vec2.hpp"

class Camera;

namespace blit {
    class Surface;
    class TileMap;
}

class TrackInfo final {
public:
    blit::Point finish_line[2];
    const blit::Point *route;
    size_t route_len;

    const uint8_t *map_asset, *tiles_asset;
};

class Track final {
public:
    Track(const TrackInfo &info);
    // TODO: cleanup

    void render(const Camera &cam);

    const TrackInfo &get_info() const;

    blit::TileMap &get_map();

private:
    void load_tilemap();

    const TrackInfo &info;

    blit::Surface *tiles;
    blit::TileMap *map;
};