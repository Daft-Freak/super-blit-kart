#pragma once

#include <cstdint>

#include "game-state.hpp"

class Game final {
public:
    Game();
    ~Game();

    void update(uint32_t time);
    void render();

    template <class T>
    void change_state();

private:
    // state of the game
    GameState *state = nullptr, *next_state = nullptr;
};

template<class T>
void Game::change_state() {
    // clean up pending state
    if(next_state)
        delete next_state;

    next_state = new T(this);
}