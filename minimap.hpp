#pragma once
#include <cstdint>

#include "graphics/surface.hpp"
#include "types/size.hpp"

namespace blit {
    class TileMap;
}

class Minimap final {
public:
    Minimap();

    void update(blit::Point center_pos);
    void render();

    void set_map(blit::TileMap *map);

private:
    static constexpr blit::Size size{96, 96};
    uint8_t data[size.area()];

    blit::Surface surface{data, blit::PixelFormat::P, size};

    blit::TileMap *map = nullptr;
};