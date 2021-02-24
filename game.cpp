#include "game.hpp"
#include "assets.hpp"

using namespace blit;

class Camera final {
public:
    void update() {
        forward = look_at - pos;
        forward.normalize();

        right = Vec3(-forward.z, 0.0f, forward.x);
        right.normalize();

        up = right.cross(forward);
    }

    Point world_to_screen(Vec3 world_pos, float &scale) {
        Point screen_center(screen.bounds.w / 2, screen.bounds.h / 2);

        auto dist = world_pos - pos;

        Vec3 tmp;
        tmp.x = dist.dot(right);
        tmp.y = -dist.dot(up);
        tmp.z = dist.dot(forward);

        scale = focal_distance / tmp.z;

        return Point(Vec2(tmp.x, tmp.y) * scale) + screen_center;
    }

    Vec3 pos, look_at;

    Vec3 forward, right, up;

    float focal_distance = 320.0f;
};

static Surface *map_tiles;
static TileMap *map;

static Vec3 cam_base_pos(-32.0f, 32.0f, -32.0f);
static Camera cam;

static Surface *cart_sprites;

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

    cart_sprites = Surface::load(asset_cart);

    cam.pos = cam_base_pos;
    cam.look_at = Vec3(64.0f, 0.0f, 64.0f);
    cam.update();
}

float r = 0.0f;// + 90;

Mat3 mode7_scanline_transform(uint8_t y) {
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

void render(uint32_t time) {
    screen.pen = Pen(0,0,0);
    screen.clear();

    int horizon = (screen.bounds.h / 2) - ((-cam.forward.y * cam.focal_distance) / cam.up.y);
    map->draw(&screen, Rect(0, horizon, screen.bounds.w, screen.bounds.h - horizon), mode7_scanline_transform);

    screen.sprites = cart_sprites;

    for(int y = 0; y < 2; y++) {
        for(int x = 0; x < 2; x++) {
            Vec3 world_pos(x * 64 + 32, 0.0f, y * 64 + 32);

            Point origin(16, 26);
            float scale;
            Point pos = cam.world_to_screen(world_pos, scale);

            // rotation
            const int rotation_frames = 30; // 16 + 14 mirrored
            Vec3 cam_to_sprite = world_pos - cam.pos;
            Vec3 sprite_look = Vec3(64, 0, 64) - world_pos;

            // ignore y
            cam_to_sprite.y = 0;
            sprite_look.y = 0;

            cam_to_sprite.normalize();
            sprite_look.normalize();

            float ang = std::atan2(cam_to_sprite.x * sprite_look.z - cam_to_sprite.z * sprite_look.x, cam_to_sprite.dot(sprite_look));
            ang = (blit::pi * 2.0f) - ang;

            int rot = std::round((ang / (blit::pi * 2.0f)) * rotation_frames);

            int frame = (rot + 15) % rotation_frames;
            int transform = 0;

            if(frame >= 16) {
                // frames 16-29 are 14-1 mirrored
                frame = 14 - (frame - 16);
                transform = SpriteTransform::HORIZONTAL;
            }

            screen.sprite(Rect(frame * 4, 0, 4, 4), pos, origin, scale, transform);
        }
    }
}

void update(uint32_t time) {
    r += 0.5f;

    // move camera
    cam.pos = cam_base_pos - cam.look_at;
    cam.pos.transform(Mat4::rotation(r, Vec3(0,1,0)));
    cam.pos += cam.look_at;

    cam.pos.y += std::sin(r / 100.0f) * 16.0f;
    cam.update();
}