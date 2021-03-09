#include "engine/api.hpp"
#include "graphics/color.hpp"
#include "types/mat4.hpp"

#include "race.hpp"

#include "assets.hpp"
#include "fonts.hpp"
#include "game.hpp"
#include "track.hpp"
#include "track-select.hpp"

using namespace blit;

extern const TrackInfo track_info[];
extern const int num_tracks;

Race::Race(Game *game, int track_index) : game(game), pause_menu("Paused", {{Menu_Continue, "Continue"}, {Menu_Restart, "Restart"}, {Menu_Quit, "Quit"}}, tall_font) {
    state.track = new Track(track_info[track_index]);

    minimap.set_track(state.track);

    kart_sprites = Surface::load(asset_kart);

    setup_race();

    Size menu_size(90, 90);
    pause_menu.set_display_rect({{(screen.bounds.w - menu_size.w) / 2, (screen.bounds.h - menu_size.h) / 2}, menu_size});
    pause_menu.set_on_item_activated(std::bind(&Race::on_menu_activated, this, std::placeholders::_1));
}

Race::~Race() {
    delete state.track;

    if(kart_sprites) {
        delete[] kart_sprites->data;
        delete[] kart_sprites->palette;
        delete kart_sprites;
    }
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

    // kart index, finish time
    std::tuple<int, uint32_t> kart_finish_times[8];

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


    const int item_height = 10;
    int leaderboard_height = 8 * item_height;
    int y = (screen.bounds.h - leaderboard_height) / 2;

    char buf[40];

    i = 0;
    for(auto &ft : kart_finish_times) {
        // reached the non-finished players
        if(std::get<1>(ft) == ~0u)
            break;

        int kart_idx = std::get<0>(ft);

        auto &kart = state.karts[kart_idx];

        int time_min, time_sec, time_frac;
        int time = 0;

        for(int lap = 0; lap < 3; lap++)
            time += kart.get_lap_time(lap);

        time_min = time / 60000;
        time_sec = (time / 1000) % 60;
        time_frac = (time % 1000) / 10;

        snprintf(buf, sizeof(buf), "%i - %s - %02i:%02i.%02i", i + 1, kart_idx == 0 ? "You" : "CPU", time_min, time_sec, time_frac);
        screen.text(buf, minimal_font, Point(screen.bounds.w / 2, y), true, TextAlign::top_center);

        y += item_height;
        i++;
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
        // TODO: big obvious numbers so this can be closer to the center
        screen.text(std::to_string(num), minimal_font, Point(screen.bounds.w / 2, screen.bounds.h / 8), true, center_center);
    }

    char buf[20];
    snprintf(buf, 20, "Place: %i\nLap: %i", state.karts[0].current_place, std::max(0, state.karts[0].get_current_lap()) + 1);
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
    if(buttons.pressed & Button::MENU)
        paused = !paused;

    if(paused) {
        pause_menu.update(time);
        return;
    }

    if(!state.started && buttons)
        state.started = true;

    if(state.started && state.countdown)
        state.countdown -= 10;

    // kart index, "progress"
    std::tuple<int, float> kart_progress[8];
    bool all_finished = true; // TODO: don't bother with the last one

    int i = 0;
    for(auto &kart : state.karts) {
        kart.update();

        all_finished = all_finished && kart.has_finished();

        // appox progress through race
        float route_t;
        float progress = kart.get_current_lap() * state.track->get_info().route_len
                       + state.track->find_closest_route_segment(kart.get_2d_pos(), route_t) + route_t;

        kart_progress[i] = std::make_tuple(i, progress);
        i++;
    }

    // sort by progress
    std::sort(std::begin(kart_progress), std::end(kart_progress), [](const std::tuple<int, float> &a, const std::tuple<int, float> &b) {
        return std::get<1>(a) > std::get<1>(b);
    });

    for(int i = 0; i < 8; i++)
        state.karts[std::get<0>(kart_progress[i])].current_place = i + 1;

    if(all_finished && buttons.released & Button::A) {
        // restart race
        setup_race();
        return;
    }

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

    auto sort_func = [](Sprite3D *a, Sprite3D *b) {return a->z > b->z;};
    display_sprites.sort(sort_func);
    display_sprites_below.sort(sort_func);

    // minimap
    // TODO: maybe don't constantly recreate this
    minimap.update(state.karts[0].get_tile_pos());
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

    paused = false;
}