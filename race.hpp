#pragma once

#include <cstdint>
#include <forward_list>

#include "camera.hpp"
#include "game-state.hpp"
#include "menu.hpp"
#include "minimap.hpp"
#include "race-state.hpp"

namespace blit {
    class Surface;
};

class Game;

class Race final : public GameState {
public:
    Race(Game *game, int track_index = 0);
    ~Race() override;

    void update(uint32_t time) override;
    void render() override;

private:
    enum PauseMenuItem {
        Menu_Continue = 0,
        Menu_Restart,
        Menu_Quit
    };

    void setup_race();

    void render_result();

    void on_menu_activated(const Menu::Item &item);

    Game *game;

    RaceState state;
    int num_finished = 0;

    // kart index, finish time
    std::tuple<int, uint32_t> kart_finish_times[8];

    Camera cam;

    std::forward_list<Sprite3D *> display_sprites, display_sprites_below;

    blit::Surface *kart_sprites;

    Minimap minimap;

    Menu pause_menu, end_menu;
    bool paused = false;
};
