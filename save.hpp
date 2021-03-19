#pragma once

#include <cstdint>

enum class SaveType {
    RaceResult = 0,
    TimeTrial = 1,
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

struct TimeTrialSaveData {
    uint8_t save_version = 1;
    uint8_t pad = 0;
    uint16_t lap_time[3];

    char name[8]{0};

    struct {
        int16_t pos_x, pos_z;
        int16_t look_ang;
        uint16_t pad; // hmm
    } ghost_data[4096];

    uint16_t ghost_data_used = 0;
};

struct UserSaveData {
    uint8_t save_version = 1;
    char name[8]{0};
};