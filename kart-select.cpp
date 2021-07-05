#include "graphics/font.hpp"
#include "graphics/tilemap.hpp"

#include "kart-select.hpp"

#include "fonts.hpp"
#include "game.hpp"
#include "kart-info.hpp"
#include "main-menu.hpp"
#include "track-select.hpp"

KartSelect::KartSelect(Game *game, RaceMode mode) : game(game), mode(mode), kart_menu("", {}, tall_font) {
    const int w = 120;
    kart_menu.set_display_rect({blit::screen.bounds.w - w, 0, w, blit::screen.bounds.h});

    uint16_t i = 0;
    for(auto &kart : kart_info)
        kart_menu.add_item({i++, kart.name});

    kart_menu.set_on_item_activated(std::bind(&KartSelect::on_kart_selected, this, std::placeholders::_1));
}

KartSelect::~KartSelect() {
}

void KartSelect::update(uint32_t time) {
    kart_menu.update(time);

    if(blit::buttons.released & blit::Button::B) {
        game->change_state<MainMenu>();
        return;
    }

    int menu_item = kart_menu.get_current_item();

    if(menu_item != preview_index) {
        preview_index = menu_item;

        auto &info = kart_info[preview_index];

        // load stuff
        if(cur_sprites_asset != info.sprite_asset) {
            if(kart_sprites) {
                delete[] kart_sprites->data;
                delete[] orig_kart_palette;
                delete kart_sprites;
            }

            kart_sprites = blit::Surface::load(info.sprite_asset);
            orig_kart_palette = kart_sprites->palette;
            cur_sprites_asset = info.sprite_asset;
        }

        // maybe replace the palette
        kart_sprites->palette = info.alt_palette ? (blit::Pen *)info.alt_palette : orig_kart_palette;
    }
}

void KartSelect::render() {
    using blit::screen;

    screen.pen = {127, 127, 127};
    screen.clear();

    if(kart_sprites) {
        screen.sprites = kart_sprites;

        int frame = (blit::now() / 100) % 30;
        const int size = 4;
        const int rotation_frames = 16;

        // hmm, copied from sprite3d
        auto transform = blit::SpriteTransform::NONE;
        if(frame >= rotation_frames) {
            // go back through for the other half
            frame = (rotation_frames - 2) - (frame - rotation_frames);
            transform = blit::SpriteTransform::HORIZONTAL;
        }

        screen.sprite({frame * size, 0, size, size}, {screen.bounds.w / 2 - 60, screen.bounds.h / 2}, {16, 16}, 3.0f, transform);
    }

    kart_menu.render();
}

void KartSelect::on_kart_selected(const ::Menu::Item &item) {
    game->change_state<TrackSelect>(item.id, mode);
}
