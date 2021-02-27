#pragma once

#include "kart.hpp"

class Track;

class RaceState final {
public:
    Kart karts[8];
    Track *track = nullptr;
    bool started = false;
    int countdown = 3000;
};