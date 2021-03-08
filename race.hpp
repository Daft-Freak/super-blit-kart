#pragma once

#include <cstdint>
#include <forward_list>

#include "camera.hpp"
#include "game-state.hpp"
#include "minimap.hpp"
#include "race-state.hpp"

namespace blit {
    class Surface;
};

class Game;

class Race final : public GameState {
public:
    Race(Game *game, int track_index = 0);

    void update(uint32_t time) override;
    void render() override;

private:
    void setup_race();

    void render_result();

    RaceState state;

    Camera cam;

    std::forward_list<Sprite3D *> display_sprites, display_sprites_below;

    blit::Surface *kart_sprites;

    Minimap minimap;
};
