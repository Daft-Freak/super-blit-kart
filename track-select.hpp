#pragma once

#include "camera.hpp"
#include "game-state.hpp"
#include "menu.hpp"
#include "save.hpp"

class Game;
class Track;

class TrackSelect final : public GameState {
public:
    TrackSelect(Game *game);
    ~TrackSelect() override;

    void update(uint32_t time) override;
    void render() override;

private:
    void on_track_selected(const Menu::Item &item);

    Game *game;

    int preview_index = -1;
    Menu track_menu;

    Camera cam;
    Track *track = nullptr;

    RaceSaveData race_save;
    bool race_save_loaded = false;
};