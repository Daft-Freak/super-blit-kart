#pragma once

#include <cstdint>
#include <forward_list>
#include <unordered_map>

#include "camera.hpp"
#include "game-state.hpp"
#include "menu.hpp"
#include "minimap.hpp"
#include "race-state.hpp"
#include "surface-helper.hpp"

namespace blit {
    struct Surface;
};

class Game;
class KartInfo;
struct TimeTrialSaveData;

enum class RaceMode {
    Race = 0,
    TimeTrial
};

class Race final : public GameState {
public:
    Race(Game *game, int player_kart, int track_index, RaceMode mode = RaceMode::Race);
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

    blit::Surface *load_kart_sprite(const KartInfo &info);

    Game *game;

    int player_kart, track_index;
    RaceMode mode;

    RaceState state;
    uint32_t start_time = 0;
    int num_finished = 0;

    // kart index, finish time
    std::tuple<int, uint32_t> kart_finish_times[8];

    Camera cam;

    std::forward_list<Sprite3D *> display_sprites, display_sprites_below;

    std::unordered_map<const uint8_t *, OwnedSurface> kart_sprite_cache;

    OwnedSurface kart_icons;

#ifndef PICO_BUILD
    Minimap minimap;
#endif

    Menu pause_menu, end_menu;
    bool paused = false, show_end_menu = false;
    int num_karts = 8;

    TimeTrialSaveData *time_trial_data = nullptr; // x2
    int worst_time_trial_slot = 0;
    unsigned int worst_time_trial_time = ~0;
    unsigned best_lap_time = 0;
};
