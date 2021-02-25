#include "minimap.hpp"

#include "graphics/tilemap.hpp"

using namespace blit;

static const blit::Pen minimap_palette[] {
    {  0,   0,   0,   0},
    {  0,   0,   0, 128},
    {255, 255, 255, 128},
    {255,   0,   0, 255}
};

Minimap::Minimap() {
    surface.palette = const_cast<Pen *>(minimap_palette);
}

void Minimap::update(Point center_pos) {
    Point half(size.w / 2, size.h / 2); 

    Point min = center_pos - half;
    Point max = center_pos + half;

    // clamp
    if(min.x < 0) {
        max.x += -min.x;
        min.x = 0;
    }
    if(min.y < 0) {
        max.y += -min.y;
        min.y = 0;
    }

    // assuming map size >= minimap size
    if(max.x > map->bounds.w) {
        min.x -= (max.x - map->bounds.w);
        max.x = map->bounds.w;
    }

    if(max.y > map->bounds.h) {
        min.y -= (max.y - map->bounds.h);
        max.y = map->bounds.h;
    }

    viewport = Rect(min, max);

    int out_i = 0;
    for(int y = min.y; y < max.y; y++) {
        for(int x = min.x; x < max.x; x++) {
            if(center_pos.x == x && center_pos.y == y)
                data[out_i++] = 3;
            else
                data[out_i++] = map->tiles[x + y * map->bounds.w] == 0 ? 1 : 2;
        }
    }
}

void Minimap::render() {
    screen.blit(&surface, Rect(Point(0, 0), size), Point(screen.bounds.w - size.w, screen.bounds.h - size.h));
}

void Minimap::set_map(blit::TileMap *map) {
    this->map = map;
}