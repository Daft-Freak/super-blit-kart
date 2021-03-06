#pragma once

#include "camera.hpp"
#include "game-state.hpp"
#include "menu.hpp"
#include "surface-helper.hpp"

class Game;

class MainMenu final : public GameState {
public:
    MainMenu(Game *game, bool initial_state = false);
    ~MainMenu() override;

    void update(uint32_t time) override;
    void render() override;

private:
    enum MainMenuItem {
        Menu_Race = 0,
        Menu_TimeTrial,
        Menu_Multiplayer
    };

    void on_menu_item_selected(const Menu::Item &item);

    Game *game;

    bool display_menu = false, intro_done = false;

    OwnedSurface logo;
    int logo_y;
    int fade = 0xFF;

    // background karts driving past
    OwnedSurface sprites;

    struct {
        blit::Point pos{-64, 0};
        int kart = 0;
        int speed = 0;
    } karts[4];

    Menu menu;
    std::string message;
};