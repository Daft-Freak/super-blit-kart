#include "engine/save.hpp"
#include "graphics/font.hpp"
#include "graphics/tilemap.hpp"

#include "main-menu.hpp"

#include "fonts.hpp"
#include "game.hpp"
#include "kart-select.hpp"

static const int menu_target_y = 96;
static const int logo_target_y = 8; // where the logo is when the menu is up

MainMenu::MainMenu(Game *game, bool initial_state) : game(game), menu("", {{Menu_Race, "Race"}, {Menu_TimeTrial, "Time Trial"}, {Menu_Multiplayer, "Multiplayer"}}, menu_font) {

    int menu_y;

    logo = blit::Surface::load(asset_logo);

    // nice animations on initial load
    if(initial_state) {
        logo_y = ((blit::screen.bounds.h - 16) - logo->bounds.h) / 2;
        menu_y = blit::screen.bounds.h;

        sprites = blit::Surface::load(asset_menu_sprites);
    } else {
        logo_y = logo_target_y;
        menu_y = menu_target_y;
        display_menu = intro_done = true;
        fade = 0;
    }

    menu.set_display_rect({0, menu_y, blit::screen.bounds.w, blit::screen.bounds.h - menu_target_y});

    // TODO: nicer menu, maybe some sprites or something

    menu.set_on_item_activated(std::bind(&MainMenu::on_menu_item_selected, this, std::placeholders::_1));
}

MainMenu::~MainMenu() {
}

void MainMenu::update(uint32_t time) {

    if(fade)
        fade--;

    if(!intro_done) {
        for(auto &kart : karts) {
            if(kart.pos.x < -32 || kart.pos.x > blit::screen.bounds.w) {
                // maybe reset
                auto rand = blit::random();
                if(rand & 0x1FF)
                    continue;

                if(((rand >> 10) & 0x3F) == 0) {
                    // go backwards for fun
                    kart.pos.x = blit::screen.bounds.w;
                    kart.speed = -1;
                } else {
                    kart.pos.x = -32;
                    kart.speed = ((rand >> 16) & 3) + 1;
                }

                kart.pos.y = blit::screen.bounds.h - 44 + ((rand >> 18) & 0xF);
                kart.kart = (rand >> 22) & 1;
            }

            kart.pos.x += kart.speed;
        }

        // sort by y
        std::sort(std::begin(karts), std::end(karts), [](auto &a, auto &b){return a.pos.y < b.pos.y;});
    }

    if(!display_menu) {
        if(blit::buttons.released) {
            display_menu = true;
        }
        return;
    }

    // transition
    if(logo_y > logo_target_y) {
        logo_y--;
        return;
    }

    auto &menu_rect = menu.get_display_rect();
    if(menu_rect.y > menu_target_y) {
        menu.set_display_rect({menu_rect.x, menu_rect.y - 1, menu_rect.w, menu_rect.h});
    } else
        intro_done = true;

    menu.update(time);
}

void MainMenu::render() {
    using blit::screen;

    screen.pen = {0x63, 0x9b, 0xff}; // "sky" colour
    screen.clear();

    int x = (screen.bounds.w - logo->bounds.w) / 2;
    screen.blit(logo, {{0, 0}, logo->bounds}, {x, logo_y});

    // road
    screen.pen = {0x59, 0x56, 0x52};
    screen.rectangle({0, screen.bounds.h - 24, screen.bounds.w, 24});

    if(!display_menu && blit::now() % 1000 < 500) {
        screen.pen = {255, 255, 255};
        screen.text("Press a button!", tall_font, {screen.bounds.w / 2, screen.bounds.h - 40}, true, blit::TextAlign::center_center);
    }

    // karts
    // display until menu has finished scrolling
    if(!intro_done) {
        screen.sprites = sprites;
        for(auto &kart : karts)
            screen.sprite({kart.kart * 4, 0, 4, 4}, kart.pos);
    }

    menu.render();

    if(!message.empty())
        screen.text(message, tall_font, {screen.bounds.w / 2, screen.bounds.h - 20}, true, blit::TextAlign::center_center);

    if(fade) {
        screen.pen = {0,0,0, fade};
        screen.clear();
    }
}

void MainMenu::on_menu_item_selected(const Menu::Item &item) {
    switch(item.id) {
        case Menu_Race:
            game->change_state<KartSelect>(RaceMode::Race);
            break;
        case Menu_TimeTrial:
            game->change_state<KartSelect>(RaceMode::TimeTrial);
            break;
        case Menu_Multiplayer:
            message = "Maybe later : )";
            break;
    }
}
