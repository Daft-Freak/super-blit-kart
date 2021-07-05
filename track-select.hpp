#pragma once

#include "camera.hpp"
#include "game-state.hpp"
#include "menu.hpp"
#include "race.hpp" // RaceMode
#include "save.hpp"

class Game;
class Track;

class TrackSelect final : public GameState {
public:
    TrackSelect(Game *game, int player_kart, RaceMode mode = RaceMode::Race);
    ~TrackSelect() override;

    void update(uint32_t time) override;
    void render() override;

private:
    void on_track_selected(const Menu::Item &item);

    void load_time_trial_data();

    Game *game;
    int player_kart;
    RaceMode mode;

    int preview_index = -1;
    Menu track_menu;

    Camera cam;
    Track *track = nullptr;

    RaceSaveData race_save; // race
    uint16_t lap_times[3]{0}; // time trial
    bool race_save_loaded = false;
};