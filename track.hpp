#pragma once
#include <cstdint>

#include "types/point.hpp"
#include "types/vec2.hpp"

class TrackInfo final {
public:
    blit::Point finish_line[2];
    const blit::Point *route;
    size_t route_len;

    const uint8_t *map_asset, *tiles_asset;
};