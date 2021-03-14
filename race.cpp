#include "engine/api.hpp"
#include "engine/save.hpp"
#include "graphics/color.hpp"
#include "types/mat4.hpp"

#include "race.hpp"

#include "assets.hpp"
#include "fonts.hpp"
#include "game.hpp"
#include "save.hpp"
#include "track.hpp"
#include "track-select.hpp"

using namespace blit;

extern const TrackInfo track_info[];
extern const int num_tracks;

// big text helper
static void stretch_text(std::string_view text, const Font &font, const Point &pos, float scale, TextAlign align) {
    auto bounds = screen.measure_text(text, font);

    auto buf = new uint8_t[bounds.area()]();
    Pen palette[] {
        {0, 0, 0, 0},
        screen.pen
    };

    Surface surf(buf, PixelFormat::P, bounds);
    surf.palette = palette;
    surf.pen = {1};

    Rect surf_rect({0, 0}, bounds);

    surf.text(text, font, surf_rect, true, align);

    Rect dest(pos, bounds * scale);

    // (re-)align
    if(align & TextAlign::center_h)
        dest.x -= bounds.w * scale / 2;
    //...right

    if(align & TextAlign::center_v)
        dest.y -= bounds.h * scale / 2;
    //...bottom

    screen.stretch_blit(&surf, surf_rect, dest);

    delete[] buf;
}

Race::Race(Game *game, int track_index) : game(game), track_index(track_index),
                                          pause_menu("Paused", {{Menu_Continue, "Continue"}, {Menu_Restart, "Restart"}, {Menu_Quit, "Quit"}}, tall_font),
                                          end_menu("", {{Menu_Restart, "Restart"}, {Menu_Quit, "Quit"}}, tall_font) {

    state.track = new Track(track_info[track_index]);

    minimap.set_track(state.track);

    kart_sprites = Surface::load(asset_kart);

    setup_race();

    Size menu_size(90, 90);
    pause_menu.set_display_rect({{(screen.bounds.w - menu_size.w) / 2, (screen.bounds.h - menu_size.h) / 2}, menu_size});
    pause_menu.set_on_item_activated(std::bind(&Race::on_menu_activated, this, std::placeholders::_1));

    menu_size.h = 44;
    end_menu.set_display_rect({{(screen.bounds.w - menu_size.w) / 2, (screen.bounds.h - (menu_size.h + 16))}, menu_size});
    end_menu.set_on_item_activated(std::bind(&Race::on_menu_activated, this, std::placeholders::_1));
}

Race::~Race() {
    delete state.track;

    if(kart_sprites) {
        delete[] kart_sprites->data;
        delete[] kart_sprites->palette;
        delete kart_sprites;
    }
}

void Race::render() {
    screen.pen = state.track->get_info().background_col;
    screen.clear();

    for(auto &sprite : display_sprites_below)
        sprite->render(cam);

    state.track->render(cam);

    for(auto &sprite : display_sprites)
        sprite->render(cam);

    screen.pen = Pen(255, 255, 255);

    if(state.countdown) {
        int num = std::ceil(state.countdown / 1000.0f);

        float scale = 3.0f + (1.0f - (state.countdown % 1000) / 1000.0f) * 3.0f;
        stretch_text(std::to_string(num), tall_font, Point(screen.bounds.w / 2, screen.bounds.h / 4), scale, center_center);
    }

    char buf[20];

    // uint8 to silence truncation wawning
    uint8_t place = state.karts[0].current_place;
    uint8_t lap = std::max(0, state.karts[0].get_current_lap()) + 1;
    snprintf(buf, 20, "Place: %u\nLap: %u", place, lap);
    screen.text(buf, minimal_font, Point(8, 8));

    minimap.render();

    // kart locations on minimap
    auto viewport = minimap.get_viewport();
    Point minimap_pos(screen.bounds.w - viewport.w, screen.bounds.h  - viewport.h);
    auto old_clip = screen.clip;

    screen.clip = Rect(minimap_pos, viewport.size());

    auto off = minimap_pos - viewport.tl();

    int i = 0;
    for(auto &kart : state.karts) {
        // this will probably be a sprite eventually
        screen.pen = hsv_to_rgba(i++ / 8.0f, 1.0f, 1.0f);
        screen.rectangle(Rect(off + kart.get_tile_pos() - Point(4, 4), Size(8, 8)));
    }

    screen.clip = old_clip;

    // we've finished - display the result
    if(state.karts[0].has_finished())
        render_result();
    else if(paused) {
        screen.pen = Pen(0, 0, 0, 150);
        screen.clear();
        pause_menu.render();
        return;
    }
}

void Race::update(uint32_t time) {
    // toggle pause menu, but not after the race ends
    if((buttons.pressed & Button::MENU) && !state.karts[0].has_finished())
        paused = !paused;

    if(paused) {
        pause_menu.update(time);
        return;
    }

    if(state.countdown)
        state.countdown -= 10;

    // kart index, "progress"
    std::tuple<int, float> kart_progress[8];
    // TODO: don't bother with the last one
    int new_num_finished = 0;

    int i = 0;
    for(auto &kart : state.karts) {
        kart.update();

        if(kart.has_finished())
            new_num_finished++;

        // appox progress through race
        float route_t;
        float progress = static_cast<int>(kart.get_current_lap() * state.track->get_info().route_len
                       + state.track->find_closest_route_segment(kart.get_2d_pos(), route_t)) + route_t;

        kart_progress[i] = std::make_tuple(i, progress);
        i++;
    }

    // sort by progress
    std::sort(std::begin(kart_progress), std::end(kart_progress), [](const std::tuple<int, float> &a, const std::tuple<int, float> &b) {
        return std::get<1>(a) > std::get<1>(b);
    });

    for(int i = 0; i < 8; i++)
        state.karts[std::get<0>(kart_progress[i])].current_place = i + 1;

    // update the results when someone finishes
    if(new_num_finished != num_finished) {
        int i = 0;
        for(auto &kart : state.karts) {
            if(!kart.has_finished())
                kart_finish_times[i] = std::make_tuple(i, ~0u);
            else
                kart_finish_times[i] = std::make_tuple(i, kart.get_finish_time());

            i++;
        }

        std::sort(std::begin(kart_finish_times), std::end(kart_finish_times), [](const std::tuple<int, uint32_t> &a, const std::tuple<int, uint32_t> &b) {
            return std::get<1>(a) < std::get<1>(b);
        });
    }

    num_finished = new_num_finished;

    // end of race restart/quit menu
    if(num_finished == 8)
        end_menu.update(time);

    // update camera
    Vec3 cam_look_at_target = state.karts[0].get_pos();
    Vec3 cam_pos_target = cam_look_at_target - state.karts[0].sprite.look_dir * 64.0f + Vec3(0, 16.0f, 0);

    cam.look_at += (cam_look_at_target - cam.look_at) * 0.1f;

    if(state.karts[0].has_finished()) {
        // spin camera around after race finished
        cam.pos += (cam_look_at_target - cam.look_at) * 0.1f; // follow

        cam.pos -= cam.look_at;
        cam.pos.transform(Mat4::rotation(0.2f, Vec3(0.0f, 1.0f, 0.0f)));
        cam.pos += cam.look_at;
    } else
        cam.pos += (cam_pos_target - cam.pos) * 0.03f;

    cam.pos.y = std::max(1.0f, cam.pos.y); // prevent the camera going below the track
    cam.update();

    // cull/sort
    auto check_sprite = [this](Sprite3D &sprite) {
        if(sprite.z >= cam.near && sprite.z <= cam.far) {
            if(sprite.world_pos.y < 0.0f)
                display_sprites_below.push_front(&sprite);
            else
                display_sprites.push_front(&sprite);
        }
    };

    display_sprites.clear();
    display_sprites_below.clear();

    for(auto &kart : state.karts) {
        kart.sprite.update(cam);
        check_sprite(kart.sprite);
    }

    for(auto &sprite: state.track->get_sprites()) {
        sprite.update(cam);
        check_sprite(sprite);
    }

    auto sort_func = [](Sprite3D *a, Sprite3D *b) {return a->z > b->z;};
    display_sprites.sort(sort_func);
    display_sprites_below.sort(sort_func);

    // minimap
    // TODO: maybe don't constantly recreate this
    minimap.update(state.karts[0].get_tile_pos());
}

void Race::setup_race() {
    state.countdown = 3000;

    // setup karts
    auto &info = state.track->get_info();

    auto track_start_dir = state.track->get_starting_dir();

    Vec2 finish_line(info.finish_line[1] - info.finish_line[0]);
    float w = finish_line.length();
    finish_line /= w;

    Vec2 start_pos = Vec2(info.finish_line[0]) + finish_line * w * (1.0f / 5.0f);

    float inc = (w / 5.0f) * 3.0f / 3.0f;

    int i = 0;
    for(auto &kart : state.karts) {
        kart = Kart();
        kart.set_race_state(&state);
        kart.sprite.scale = 0.75f;

        kart.sprite.spritesheet = kart_sprites;
        kart.sprite.look_dir = Vec3(track_start_dir.x, 0.0f, track_start_dir.y);
        kart.sprite.world_pos = Vec3(
            start_pos.x,
            0.0f,
            start_pos.y
        ) - kart.sprite.look_dir * 48.0f;

        i++;
        if(i == 4) {
            start_pos -= finish_line * inc * 3.0f;
        } else
            start_pos += finish_line * inc;

        start_pos -= track_start_dir * 8.0f;
    }

    // player kart
    state.karts[0].is_player = true;
    cam.look_at = state.karts[0].get_pos();
    cam.pos = cam.look_at - state.karts[0].sprite.look_dir * 64.0f + Vec3(0, 16.0f, 0);
    cam.viewport = {{0, 0}, screen.bounds};
    cam.update();
}

void Race::render_result() {
    screen.pen = Pen(0, 0, 0, 150);
    screen.clear();

    screen.pen = Pen(255, 255, 255);

    const int item_height = 10;
    int leaderboard_height = 8 * item_height;
    int y = (screen.bounds.h - leaderboard_height) / 2;

    char buf[40];

    int i = 0;
    for(auto &ft : kart_finish_times) {
        // reached the non-finished players
        if(std::get<1>(ft) == ~0u)
            break;

        int kart_idx = std::get<0>(ft);

        auto &kart = state.karts[kart_idx];

        int time_min, time_sec, time_frac;
        int time = kart.get_race_time();

        time_min = time / 60000;
        time_sec = (time / 1000) % 60;
        time_frac = (time % 1000) / 10;

        snprintf(buf, sizeof(buf), "%i - %s - %02i:%02i.%02i", i + 1, kart_idx == 0 ? "You" : "CPU", time_min, time_sec, time_frac);
        screen.text(buf, minimal_font, Point(screen.bounds.w / 2, y), true, TextAlign::top_center);

        y += item_height;
        i++;
    }

    if(num_finished == 8)
        end_menu.render();
}

void Race::on_menu_activated(const ::Menu::Item &item) {
    switch(item.id) {
        case Menu_Continue:
            break; // do nothing

        case Menu_Restart:
            setup_race();
            break;

        case Menu_Quit:
            game->change_state<TrackSelect>(); // main menu?
            break;
    }

    // write save on continue/exit
    if(num_finished == 8) {
        RaceSaveData save;
        for(save.place = 0; save.place < 8; save.place++) {
            if(std::get<0>(kart_finish_times[save.place]) == 0)
                break;
        }

        save.time = state.karts[0].get_race_time() / 10;

        RaceSaveData old_save;
        int slot = get_save_slot(SaveType::RaceResult, track_index);

        // overwrite if improved
        bool have_old_save = read_save(old_save, slot) && old_save.save_version == 1 && old_save.time > 0;
        if(!have_old_save || save.place < old_save.place || (save.place == old_save.place && save.time < old_save.time))
            write_save(save, slot);
    }

    paused = false;
}