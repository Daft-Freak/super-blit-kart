#pragma once
#include "types/rect.hpp"

enum class ItemType {
    None = -1,
    Drop = 0,
    Projectile,
    Boost
};

// in the track spritesheet for now
inline const blit::Rect item_sprites[] {
    {2, 15, 1, 1},
    {3, 15, 1, 1},
    {4, 15, 1, 1}
};