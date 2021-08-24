#pragma once

#include "game-state.hpp"
#include "menu.hpp"
#include "race.hpp" // RaceMode
#include "surface-helper.hpp"

class Game;
class Track;

class KartSelect final : public GameState {
public:
    KartSelect(Game *game, RaceMode mode = RaceMode::Race);
    ~KartSelect() override;

    void update(uint32_t time) override;
    void render() override;

private:
    void on_kart_selected(const Menu::Item &item);

    Game *game;
    RaceMode mode;

    int preview_index = -1;
    Menu kart_menu;

    OwnedSurface kart_sprites;
    blit::Pen *orig_kart_palette = nullptr;
    const uint8_t *cur_sprites_asset = nullptr;
};