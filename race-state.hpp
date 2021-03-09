#pragma once

#include "kart.hpp"

class Track;

class RaceState final {
public:
    Kart karts[8];
    Track *track = nullptr;
    int countdown = 3000;
};
