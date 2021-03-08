#include "graphics/font.hpp"
#include "graphics/tilemap.hpp"

#include "track-select.hpp"

#include "assets.hpp"
#include "game.hpp"
#include "race.hpp"
#include "track.hpp"

// duplicated!
extern const TrackInfo track_info[];
extern const int num_tracks;

static blit::Font tall_font(asset_tall_font);

TrackSelect::TrackSelect(Game *game) : game(game), track_menu("", {}, tall_font) {
    const int w = 120;
    track_menu.set_display_rect({blit::screen.bounds.w - w, 0, w, blit::screen.bounds.h});

    int size = blit::screen.bounds.w - w;
    cam.viewport = {0, 0, size, size};
    cam.far = 10000.0f; // basically don't clip

    for(int i = 0; i < num_tracks; i++)
        track_menu.add_item({static_cast<uint16_t>(i), track_info[i].name});

    track_menu.set_on_item_activated(std::bind(&TrackSelect::on_track_selected, this, std::placeholders::_1));
}

void TrackSelect::update(uint32_t time) {
    track_menu.update(time);

    int menu_item = track_menu.get_current_item();

    if(menu_item != preview_index) {
        preview_index = menu_item;

        delete track;
        track = new Track(track_info[menu_item]);

        auto bounds = track->get_map().bounds;
        cam.look_at = {bounds.w * 4.0f, 0.0f, bounds.h * 4.0f};
        cam.pos = cam.look_at + blit::Vec3(4.0f, 1.0f, 4.0f) * bounds.w;

        track->set_fog(50.0f / (bounds.w / 128)); // less fog and even less for desert
    }
}

void TrackSelect::render() {
    using blit::screen;

    screen.pen = track->get_info().background_col;
    screen.rectangle(cam.viewport);

    cam.pos -= cam.look_at;
    cam.pos.transform(blit::Mat4::rotation(-0.5f, {0.0f, 1.0f, 0.0f}));
    cam.pos += cam.look_at;

    cam.update();

    track->render(cam);

    track_menu.render();

    // display some info here?
    screen.pen = {0, 0, 0};
    screen.rectangle({0, cam.viewport.h, cam.viewport.w, screen.bounds.h - cam.viewport.h});
}

void TrackSelect::on_track_selected(const ::Menu::Item &item) {
    game->change_state<Race>(item.id);
}