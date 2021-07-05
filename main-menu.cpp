#include "engine/save.hpp"
#include "graphics/font.hpp"
#include "graphics/tilemap.hpp"

#include "main-menu.hpp"

#include "fonts.hpp"
#include "game.hpp"
#include "kart-select.hpp"

MainMenu::MainMenu(Game *game) : game(game), menu("Super Blit Kart (TODO: Improved Menu)", {{Menu_Race, "Race"}, {Menu_TimeTrial, "Time Trial"}, {Menu_Multiplayer, "Multiplayer"}}, tall_font) {
    menu.set_display_rect({{0, 0}, blit::screen.bounds});

    // TODO: nicer menu, maybe some sprites or something

    menu.set_on_item_activated(std::bind(&MainMenu::on_menu_item_selected, this, std::placeholders::_1));
}

void MainMenu::update(uint32_t time) {
    menu.update(time);

}

void MainMenu::render() {
    using blit::screen;

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
