#include "game.hpp"
#include "assets.hpp"

using namespace blit;

static Surface *map_tiles;
static TileMap *map;

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
}

void render(uint32_t time) {
    screen.clear();

    map->draw(&screen, Rect(Point(0, 0), screen.bounds));
}

void update(uint32_t time) {
}