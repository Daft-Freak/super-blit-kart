#pragma once

#include <cstdint>

class GameState {
public:
    virtual ~GameState(){}

    virtual void update(uint32_t time) = 0;
    virtual void render() = 0;
};