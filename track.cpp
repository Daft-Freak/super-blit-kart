#include "track.hpp"

#include "graphics/surface.hpp"
#include "graphics/tilemap.hpp"

#include "camera.hpp"
#include "kart.hpp"

using namespace blit;

static Mat3 mode7_scanline_transform(const Camera &cam, float fog, uint8_t y) {
    float top = cam.viewport.h / 2;
    float left = -cam.viewport.w / 2;

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

    screen.alpha = 255 - std::max(0.0f, std::min(255.0f, (l - 0.2f) * fog));

    return mat;
}

TrackObject::TrackObject(ObjectType type) : type(type) {}

TrackObject::TrackObject(const TrackObjectInfo &info, blit::Surface *spritesheet) : type(info.type) {
    sprite.spritesheet = spritesheet;
    sprite.world_pos.x = info.pos_x;
    sprite.world_pos.z = info.pos_y;
    sprite.scale = info.scale / 16.0f;
    sprite.sheet_x = info.sprite_x;
    sprite.sheet_y = info.sprite_y;
    sprite.size_w = info.sprite_w;
    sprite.size_h = info.sprite_h;
    sprite.origin_x = info.origin_x;
    sprite.origin_y = info.origin_y;
}

void TrackObject::update() {
    if(respawn_timer)
        respawn_timer -= 10;

    if(type == ObjectType::Projectile) {
        sprite.world_pos += vel * 0.01f;
        
        auto &track_info = track->get_info();

        // collide with track obstacles
        // TODO: mostly copy/pasted from kart
        for(size_t i = 0; i < track_info.num_collision_rects; i++) {
            auto &rect = track_info.collision_rects[i];
            if(rect.empty())
                continue;

            Vec2 pos_2d(sprite.world_pos.x, sprite.world_pos.z);
            float radius = sprite.size_w * 4.0f;

            Vec2 obstacle_pos(pos_2d);

            if(pos_2d.x < rect.x)
                obstacle_pos.x = rect.x;
            else if(pos_2d.x >= rect.x + rect.w)
                obstacle_pos.x = rect.x + rect.w;

            if(pos_2d.y < rect.y)
                obstacle_pos.y = rect.y;
            else if(pos_2d.y >= rect.y + rect.h)
                obstacle_pos.y = rect.y + rect.h;

            auto vec = pos_2d - obstacle_pos;
            float dist = vec.length();

            if(dist > radius)
                continue;

            vec /= dist;

            float penetration = radius - dist;
            sprite.world_pos += Vec3(vec.x, 0.0f, vec.y) * penetration;

            // boing
            Vec3 vec3(vec.x, 0.0f, vec.y);
            vel -= vec3 * vec3.dot(vel) * 2.0f;
        }

        // TODO: collide with other objects
    }
}

void TrackObject::collide(Kart &kart) {
    if(!is_active())
        return;

    float sprite_radius = sprite.size_w * 4.0f * sprite.scale;
    float kart_radius = kart.get_radius();

    auto vec = kart.get_2d_pos() - Vec2(sprite.world_pos.x, sprite.world_pos.z);
    float dist = vec.x * vec.x + vec.y * vec.y; // squared length

    if(dist >= (kart_radius + sprite_radius) * (kart_radius + sprite_radius))
        return;

    if(type == ObjectType::Item) {
        kart.collect_item();
        respawn_timer = 10000; // 10s
        return;
    }

    // do collision
    dist = std::sqrt(dist);
    vec /= dist;

    float penetration = kart_radius + sprite_radius - dist;

    kart.sprite.world_pos += Vec3(vec.x, 0.0f, vec.y) * penetration;

    if(type == ObjectType::DroppedItem || type == ObjectType::Projectile) {
        // assume hitting this is bad (okay unless we allow dropped boosts)
        kart.disable();

        type = ObjectType::Removed;
    }
}

bool TrackObject::is_active() const {
    return type != ObjectType::Removed && respawn_timer == 0;
}

Track::Track(const TrackInfo &info) : info(info) {
    tiles = Surface::load(info.tiles_asset);
    load_tilemap();

    // some extra for dynamic objects
    objects.reserve(info.num_sprites + 16);
    reset_objects(); // hmm, this will always be called by setup_race
}

Track::~Track() {
    delete map;
}

void Track::render(const Camera &cam) {
    using namespace std::placeholders;

    int horizon = (screen.bounds.h / 2) - (((cam.far * -cam.forward.y) - cam.pos.y) * cam.focal_distance) /  (cam.far * cam.up.y);

    map->draw(&screen, Rect(cam.viewport.x, cam.viewport.y + horizon, cam.viewport.w, cam.viewport.h - horizon), std::bind(mode7_scanline_transform, cam, fog, _1));

    //map->transform = /*Mat3::translation(Vec2(cam.look_at.x - screen.bounds.w / 2, cam.look_at.z - screen.bounds.h / 2)) */ Mat3::scale(Vec2(0.5f, 0.5f));
    //map->draw(&screen, Rect({0, 0}, screen.bounds));

    screen.alpha = 255;
}

unsigned int Track::find_closest_route_segment(Vec2 pos, float &segment_t) const {
    unsigned int ret = 0;
    float min_dist = INFINITY;

    for(size_t i = 0; i < info.route_len - 1; i++) {
        Vec2 a(info.route[i]), b(info.route[i + 1]);

        // intersection
        Vec2 r = b - a;

        Vec2 dir(r.y, -r.x);

        float rxs = (r.x * dir.y) - (r.y * dir.x);
        float t = ((pos.x - a.x) * dir.y - (pos.y - a.y) * dir.x) / rxs; // distance along route segment

        // clamp to segment
        Vec2 route_point;

        if(t < 0.0f)
            route_point = a;
        else if(t > 1.0f)
            route_point = b;
        else
            route_point = a + r * t;

        float dist = (pos - route_point).length();

        if(dist < min_dist) {
            ret = i;
            segment_t = t;
            min_dist = dist;
        }
    }

    return ret;
}

float Track::get_friction(blit::Vec2 pos) {
    Point tile_coord(pos / Vec2(8.0f, 8.0f));

    if(tile_coord.x < 0 || tile_coord.y < 0 || tile_coord.x >= map->bounds.w || tile_coord.y >= map->bounds.h)
        return 0.0f;

    // we know that the coord is in bounds so skip the offset calcs
    //auto tile_id = map->tile_at(tile_coord);
    auto tile_id = map->tiles[tile_coord.x + tile_coord.y * map->bounds.w];

    // default
    if(tile_id >= info.tile_friction_len)
        return 1.0f;

    return info.tile_friction[tile_id];
}

Vec2 Track::get_starting_dir() const {
    Vec2 dir(info.route[1] - info.route[0]);
    dir.normalize();
    return dir;
}

void Track::set_fog(float fog) {
    this->fog = fog;
}

const TrackInfo &Track::get_info() const {
    return info;
}

blit::Surface *Track::get_tiles() {
    return tiles;
}

TileMap &Track::get_map() {
    return *map;
}

void Track::reset_objects() {
    objects.clear();

    for(size_t i = 0; i < info.num_sprites; i++) {
        objects.emplace_back(info.sprites[i], tiles);
        objects.back().track = this;
    }
}

std::vector<TrackObject> &Track::get_objects() {
    return objects;
}

void Track::add_object(TrackObject object) {

    if(!object.sprite.spritesheet)
        object.sprite.spritesheet = tiles;

    object.track = this;

    // attempt reuse
    for(auto it = objects.begin() + info.num_sprites; it != objects.end(); ++it) {
        if(it->type == ObjectType::Removed) {
            *it = object;
            return;
        }
    }

    objects.emplace_back(std::move(object));
}

void Track::load_tilemap() {
    map = TileMap::load_tmx(info.map_asset, tiles, 0, 0);
    map->repeat_mode = TileMap::CLAMP_TO_EDGE;
}