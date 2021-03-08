#pragma once

#include <cstdint>

#include "game-state.hpp"

class Game final {
public:
    Game();
    ~Game();

    void update(uint32_t time);
    void render();

    template <class T, typename ...Args>
    void change_state(Args ...args);

private:
    // state of the game
    GameState *state = nullptr, *next_state = nullptr;
};

template<class T, typename ...Args>
void Game::change_state(Args ...args) {
    // clean up pending state
    if(next_state)
        delete next_state;

    next_state = new T(this, args...);
}