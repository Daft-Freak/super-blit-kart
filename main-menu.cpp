#include "engine/save.hpp"
#include "graphics/font.hpp"
#include "graphics/tilemap.hpp"

#include "main-menu.hpp"

#include "fonts.hpp"
#include "game.hpp"
#include "kart-select.hpp"

MainMenu::MainMenu(Game *game) : game(game), menu("", {{Menu_Race, "Race"}, {Menu_TimeTrial, "Time Trial"}, {Menu_Multiplayer, "Multiplayer"}}, menu_font) {
    int logo_height = 96;

    menu.set_display_rect({0, logo_height, blit::screen.bounds.w, blit::screen.bounds.h - logo_height});

    logo = blit::Surface::load(asset_logo);

    // TODO: nicer menu, maybe some sprites or something

    menu.set_on_item_activated(std::bind(&MainMenu::on_menu_item_selected, this, std::placeholders::_1));
}

MainMenu::~MainMenu() {
    if(logo) {
        delete[] logo->data;
        delete[] logo->palette;
        delete logo;
    }
}

void MainMenu::update(uint32_t time) {
    menu.update(time);

}

void MainMenu::render() {
    using blit::screen;

    screen.pen = {0x63, 0x9b, 0xff}; // "sky" colour
    screen.clear();

    int x = (screen.bounds.w - logo->bounds.w) / 2;
    screen.blit(logo, {{0, 0}, logo->bounds}, {x, 8});

    menu.render();

    if(!message.empty())
        screen.text(message, tall_font, {screen.bounds.w / 2, screen.bounds.h - 20}, true, blit::TextAlign::center_center);
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
