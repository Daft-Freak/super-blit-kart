#include "engine/save.hpp"
#include "graphics/font.hpp"
#include "graphics/tilemap.hpp"

#include "track-select.hpp"

#include "fonts.hpp"
#include "game.hpp"
#include "race.hpp"
#include "track.hpp"

// duplicated!
extern const TrackInfo track_info[];
extern const int num_tracks;

TrackSelect::TrackSelect(Game *game, RaceMode mode) : game(game), mode(mode), track_menu("", {}, tall_font) {
    const int w = 120;
    track_menu.set_display_rect({blit::screen.bounds.w - w, 0, w, blit::screen.bounds.h});

    int size = blit::screen.bounds.w - w;
    cam.viewport = {0, 0, size, size};
    cam.far = 10000.0f; // basically don't clip

    for(int i = 0; i < num_tracks; i++)
        track_menu.add_item({static_cast<uint16_t>(i), track_info[i].name});

    track_menu.set_on_item_activated(std::bind(&TrackSelect::on_track_selected, this, std::placeholders::_1));
}

TrackSelect::~TrackSelect() {
    delete track;
}

void TrackSelect::update(uint32_t time) {
    track_menu.update(time);

    int menu_item = track_menu.get_current_item();

    if(menu_item != preview_index) {
        preview_index = menu_item;

        delete track;
        track = new Track(track_info[menu_item]);

        race_save_loaded = blit::read_save(race_save, get_save_slot(SaveType::RaceResult, menu_item));

        auto bounds = track->get_map().bounds;
        cam.look_at = {bounds.w * 4.0f, 0.0f, bounds.h * 4.0f};
        cam.pos = cam.look_at + blit::Vec3(4.0f, 1.0f, 4.0f) * bounds.w;

        track->set_fog(50.0f / (bounds.w / 128)); // less fog and even less for desert
    }
}

void TrackSelect::render() {
    using blit::screen;

    if(!track) return;

    screen.pen = track->get_info().background_col;
    screen.rectangle(cam.viewport);

    cam.pos -= cam.look_at;
    cam.pos.transform(blit::Mat4::rotation(-0.5f, {0.0f, 1.0f, 0.0f}));
    cam.pos += cam.look_at;

    cam.update();

    track->render(cam);

    track_menu.render();

    // some info
    screen.pen = {0, 0, 0};
    blit::Rect info_rect(0, cam.viewport.h, cam.viewport.w, screen.bounds.h - cam.viewport.h);
    screen.rectangle(info_rect);

    if(race_save_loaded) {
        info_rect.inflate(-4);

        const char *suffix[]{"st", "nd", "rd", "th"}; // ...

        screen.pen = {255, 255, 255};
        char buf[30];
        snprintf(buf, sizeof(buf), "Best: %i%s place in %02i:%02i.%02i",
            race_save.place + 1, suffix[std::min(uint8_t(3), race_save.place)],
            race_save.time / 6000, (race_save.time % 6000) / 100, race_save.time % 100
        );
        
        screen.text(buf, tall_font, info_rect);
    }
}

void TrackSelect::on_track_selected(const ::Menu::Item &item) {
    game->change_state<Race>(item.id, mode);
}