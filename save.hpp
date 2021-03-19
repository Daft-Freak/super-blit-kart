#pragma once

#include <cstdint>

enum class SaveType {
    RaceResult = 0,
    UserData = 0xF
};

inline int get_save_slot(SaveType type, int track, int slot = 0) {
    return (static_cast<int>(type) << 12) | (track << 4) | slot;
}

struct RaceSaveData {
    uint8_t save_version = 1;
    uint8_t place;
    uint16_t time; // ms/10
};

struct UserSaveData {
    uint8_t save_version = 1;
    char name[8]{0};
};