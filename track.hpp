#pragma once
#include <cstdint>
#include <vector>

#include "graphics/surface.hpp"
#include "types/point.hpp"
#include "types/rect.hpp"
#include "types/vec2.hpp"

#include "sprite3d.hpp"

class Camera;
class Kart;

namespace blit {
    struct TileMap;
}

enum class ObjectType : uint8_t {
    Removed,
    Static, // obstacle
    Item,
    DroppedItem, // usualy an obstacale, removed when hit
};

class TrackObjectInfo final {
public:
    ObjectType type;
    uint8_t scale; //*16
    uint16_t pos_x, pos_y;
    uint8_t sprite_x, sprite_y, sprite_w, sprite_h; // spritesheet coords
    uint8_t origin_x, origin_y;
};

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

    const TrackObjectInfo *sprites;
    size_t num_sprites;

    const uint8_t *map_asset, *tiles_asset;

    blit::Pen background_col;
};

class TrackObject final {
public:
    TrackObject(ObjectType type);
    TrackObject(const TrackObjectInfo &info, blit::Surface *spritesheet);

    void update();
    void collide(Kart &kart);

    bool is_active() const;

    ObjectType type;

    Sprite3D sprite;

    uint32_t respawn_timer = 0;
};

class Track final {
public:
    Track(const TrackInfo &info);
    ~Track();

    void render(const Camera &cam);

    const TrackInfo &get_info() const;

    blit::Surface *get_tiles();
    blit::TileMap &get_map();

    void reset_objects();
    std::vector<TrackObject> &get_objects();
    void add_object(TrackObject object);

    unsigned int find_closest_route_segment(blit::Vec2 pos, float &segment_t) const;

    float get_friction(blit::Vec2 pos);

    blit::Vec2 get_starting_dir() const;

    void set_fog(float fog);

private:
    void load_tilemap();

    const TrackInfo &info;

    blit::Surface *tiles;
    blit::TileMap *map;

    std::vector<TrackObject> objects;

    float fog = 170.0f;
};