#pragma once
#include <cstdint>

#include "assets.hpp"
#include "assets/palettes.h"

class KartInfo final {
public:
    const char *name;
    const uint8_t *sprite_asset, *alt_palette;
    // possibly other properties
};

const inline KartInfo kart_info[]{
    {"Race-A-Tron 3000", asset_kart, kart_robot_default_palette},
    {"Race-A-Tron 2000", asset_kart, nullptr}
};