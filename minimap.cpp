#include "minimap.hpp"

#include "graphics/tilemap.hpp"

#include "track.hpp"

using namespace blit;

static const blit::Pen minimap_palette[] {
    {  0,   0,   0,   0},
    {  0,   0,   0, 128},
    {128, 128, 128, 128},
    {255, 255, 255, 128}
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

    auto &map = track->get_map();

    // assuming map size >= minimap size
    if(max.x > map.bounds.w) {
        min.x -= (max.x - map.bounds.w);
        max.x = map.bounds.w;
    }

    if(max.y > map.bounds.h) {
        min.y -= (max.y - map.bounds.h);
        max.y = map.bounds.h;
    }

    viewport = Rect(min, max);

    int out_i = 0;
    for(int y = min.y; y < max.y; y++) {
        for(int x = min.x; x < max.x; x++) {

            float friction = track->get_friction(Vec2(x, y) * 8.0f);

            if(friction == 0.0f)
                data[out_i++] = 1;
            else if(friction > 1.0f)
                data[out_i++] = 2;
            else
                data[out_i++] = 3;
        }
    }
}

void Minimap::render() {
    screen.blit(&surface, Rect(Point(0, 0), size), Point(screen.bounds.w - size.w, screen.bounds.h - size.h));
}

void Minimap::set_track(Track *track) {
    this->track = track;
}