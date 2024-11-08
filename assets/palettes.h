#pragma once
#include <cstdint>

inline uint8_t kart_robot_default_palette[]{
    0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x20, 0xb4, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x4a,
    0x18, 0x18, 0x18, 0xb4, 0x08, 0x08, 0x08, 0xde, 0x08, 0x08, 0x08, 0x94, 0x10, 0x10, 0x10, 0xb4,
    0x10, 0x10, 0x10, 0xde, 0x08, 0x08, 0x08, 0xb4, 0x08, 0x08, 0x08, 0xff, 0x10, 0x10, 0x10, 0xff,
    0x00, 0x00, 0x00, 0x31, 0x20, 0x20, 0x20, 0x6a, 0x10, 0x10, 0x10, 0x94, 0x00, 0x00, 0x00, 0x6a,
    0x18, 0x18, 0x18, 0x94, 0x8c, 0x8c, 0x8c, 0xff, 0x8a, 0x8a, 0x8a, 0xb4, 0x61, 0x62, 0x61, 0x4a,
    0x8a, 0x8a, 0x8a, 0x94, 0x82, 0x82, 0x82, 0x4a, 0x51, 0x51, 0x51, 0xff, 0x38, 0x39, 0x38, 0xff,
    0x92, 0x93, 0x92, 0xff, 0x40, 0x41, 0x40, 0xff, 0x4b, 0x4b, 0x4b, 0xff, 0x43, 0x42, 0x43, 0xff,
    0x3b, 0x3a, 0x3b, 0xff, 0x87, 0x87, 0x87, 0xff, 0x7f, 0x7f, 0x7f, 0xff, 0x7a, 0x7b, 0x7c, 0xde,
    0x82, 0x83, 0x84, 0x94, 0x7a, 0x7b, 0x7c, 0xff, 0x72, 0x72, 0x73, 0xff, 0x1a, 0x1a, 0x1a, 0xff,
    0x1a, 0x1a, 0x1a, 0xb4, 0x1a, 0x1a, 0x1a, 0x94, 0x6d, 0x6e, 0x70, 0xff, 0x24, 0x24, 0x24, 0xff,
    0x12, 0x12, 0x12, 0xff, 0x12, 0x12, 0x12, 0x94, 0x65, 0x66, 0x68, 0xff, 0x65, 0x66, 0x68, 0x94,
    0x24, 0x24, 0x25, 0xde, 0x1c, 0x1c, 0x1c, 0xff, 0x69, 0x69, 0x6c, 0xb4, 0x61, 0x61, 0x64, 0xff,
    0x14, 0x14, 0x14, 0xde, 0x51, 0x50, 0x53, 0x6a, 0x28, 0x28, 0x29, 0xff, 0x28, 0x28, 0x29, 0xb4,
    0x5d, 0x5d, 0x60, 0x94, 0x20, 0x20, 0x21, 0x94, 0x2b, 0x2c, 0x2e, 0xff, 0x4f, 0x50, 0x54, 0xff,
    0x53, 0x55, 0x59, 0xde, 0x53, 0x55, 0x59, 0xff, 0x3c, 0x3c, 0x3f, 0xde, 0x17, 0x17, 0x19, 0xde,
    0x17, 0x17, 0x19, 0xff, 0x17, 0x17, 0x19, 0x94, 0x23, 0x24, 0x26, 0x94, 0x2f, 0x2f, 0x32, 0xde,
    0x2f, 0x2f, 0x32, 0xff, 0x30, 0x2f, 0x32, 0x20, 0x30, 0x2f, 0x32, 0x6a, 0x33, 0x34, 0x37, 0xff,
    0x33, 0x34, 0x37, 0xb4, 0x33, 0x34, 0x37, 0xff, 0x33, 0x33, 0x37, 0xff, 0x27, 0x27, 0x2a, 0xde,
    0x37, 0x37, 0x3b, 0xb4, 0x1b, 0x1b, 0x1e, 0xde, 0x37, 0x37, 0x3b, 0xde, 0x37, 0x37, 0x3b, 0xff,
    0x1c, 0x1b, 0x1e, 0xff, 0x2b, 0x2b, 0x2e, 0xde, 0x2b, 0x2b, 0x2e, 0xb4, 0x1f, 0x20, 0x22, 0xff,
    0x4e, 0x4f, 0x56, 0xb4, 0x3e, 0x3f, 0x44, 0x4a, 0x3e, 0x3e, 0x44, 0xff, 0x1f, 0x1f, 0x22, 0xde,
    0x2f, 0x31, 0x33, 0xff, 0x2f, 0x2f, 0x33, 0xde, 0x2f, 0x2f, 0x33, 0x94, 0x32, 0x33, 0x38, 0xde,
    0x32, 0x33, 0x38, 0xb4, 0x56, 0x56, 0x5e, 0xff, 0x56, 0x56, 0x5e, 0xb4, 0x32, 0x32, 0x38, 0x94,
    0x23, 0x23, 0x26, 0x94, 0x23, 0x23, 0x26, 0xff, 0x23, 0x23, 0x26, 0xb4, 0x46, 0x46, 0x4d, 0xb4,
    0x36, 0x36, 0x3c, 0xff, 0x49, 0x4a, 0x52, 0xff, 0x49, 0x4a, 0x52, 0x94, 0x26, 0x29, 0x2b, 0xff,
    0x13, 0x14, 0x15, 0xff, 0x26, 0x27, 0x2b, 0xff, 0x26, 0x27, 0x2b, 0x94, 0x13, 0x13, 0x15, 0xff,
    0x13, 0x13, 0x15, 0xde, 0x26, 0x26, 0x2b, 0xb4, 0x4d, 0x50, 0x56, 0x6a, 0x4d, 0x50, 0x56, 0x20,
    0x3a, 0x3a, 0x40, 0xff, 0x4d, 0x4e, 0x56, 0x94, 0x3a, 0x3a, 0x40, 0xb4, 0x4d, 0x4e, 0x56, 0xde,
    0x4d, 0x4e, 0x56, 0xff, 0x3a, 0x3a, 0x40, 0xde, 0x4d, 0x4e, 0x56, 0xb4, 0x4d, 0x4e, 0x56, 0xde,
    0x4d, 0x4d, 0x56, 0x94, 0x4d, 0x4d, 0x56, 0xff, 0x4d, 0x4d, 0x56, 0xb4, 0x4d, 0x4d, 0x56, 0xde,
    0x4e, 0x4d, 0x56, 0x6a, 0x51, 0x52, 0x5a, 0xff, 0x3d, 0x3e, 0x45, 0xb4, 0x3d, 0x3e, 0x45, 0x94,
    0x3d, 0x3e, 0x45, 0x94, 0x3d, 0x3e, 0x45, 0xb4, 0x3d, 0x3e, 0x45, 0xff, 0x2a, 0x2c, 0x30, 0xb4,
    0x2a, 0x2b, 0x30, 0xff, 0x2a, 0x2b, 0x30, 0xde, 0x2a, 0x2b, 0x30, 0x94, 0x2a, 0x2b, 0x30, 0x94,
    0x2a, 0x2b, 0x30, 0x94, 0x41, 0x42, 0x4a, 0xde, 0x41, 0x42, 0x4a, 0xff, 0x41, 0x42, 0x4a, 0xde,
    0x41, 0x42, 0x4a, 0xff, 0x2e, 0x32, 0x34, 0xb4, 0x17, 0x19, 0x1a, 0xff, 0x2e, 0x31, 0x34, 0xff,
    0x45, 0x47, 0x4e, 0xff, 0x17, 0x17, 0x1a, 0x94, 0x2e, 0x2f, 0x34, 0xff, 0x45, 0x46, 0x4e, 0xb4,
    0x45, 0x46, 0x4e, 0x94, 0x45, 0x46, 0x4e, 0xff, 0x2e, 0x2e, 0x34, 0xff, 0x32, 0x34, 0x38, 0xff,
    0x32, 0x32, 0x38, 0xff, 0x32, 0x32, 0x38, 0xb4, 0x32, 0x32, 0x38, 0xde, 0x1b, 0x1d, 0x1e, 0xb4,
    0x1b, 0x1d, 0x1e, 0xde, 0x1b, 0x1d, 0x1e, 0xff, 0x1b, 0x1c, 0x1e, 0xff, 0x35, 0x37, 0x3d, 0xff,
    0x1b, 0x1b, 0x1e, 0xff, 0x35, 0x37, 0x3d, 0xde, 0x35, 0x36, 0x3d, 0xb4, 0x35, 0x36, 0x3d, 0xff,
    0x35, 0x36, 0x3d, 0xff, 0x35, 0x36, 0x3d, 0x94, 0x35, 0x36, 0x3d, 0xb4, 0x39, 0x3d, 0x41, 0xff,
    0x39, 0x3c, 0x41, 0xff, 0x39, 0x3a, 0x41, 0xff, 0x39, 0x3a, 0x41, 0xde, 0x3d, 0x3f, 0x46, 0xde,
    0x1e, 0x1f, 0x23, 0xff, 0x3d, 0x3f, 0x46, 0xff, 0x3d, 0x3f, 0x46, 0xff, 0x41, 0x46, 0x4a, 0x94,
    0x41, 0x44, 0x4a, 0xde, 0x41, 0x44, 0x4a, 0xde, 0x41, 0x43, 0x4a, 0xff, 0x22, 0x25, 0x27, 0xff,
    0x48, 0x4f, 0x53, 0xff, 0x44, 0x49, 0x4f, 0x94, 0x44, 0x48, 0x4f, 0xff, 0x48, 0x4c, 0x53, 0xff,
    0x26, 0x29, 0x2c, 0xff, 0x4c, 0x50, 0x58, 0xff, 0x2a, 0x2d, 0x30, 0xff, 0x2a, 0x2d, 0x30, 0xb4,
    0x2a, 0x2b, 0x30, 0x94, 0x2a, 0x2b, 0x30, 0xde, 0x2a, 0x2b, 0x30, 0xff, 0x2a, 0x2b, 0x30, 0xb4,
    0x2a, 0x2a, 0x30, 0xde, 0x2a, 0x2a, 0x30, 0xb4, 0x2a, 0x2a, 0x30, 0xff, 0x2d, 0x31, 0x35, 0x94,
    0x2d, 0x31, 0x35, 0xff, 0x2d, 0x31, 0x35, 0xff, 0x2d, 0x2f, 0x35, 0xff, 0x2d, 0x2e, 0x35, 0x94,
    0x2d, 0x2e, 0x35, 0xde, 0x2d, 0x2e, 0x35, 0xb4, 0x35, 0x3a, 0x3d, 0xde, 0x35, 0x3a, 0x3d, 0xff,
    0x31, 0x35, 0x39, 0xb4, 0x3c, 0x43, 0x47, 0xff, 0x3c, 0x42, 0x47, 0xff, 0x40, 0x46, 0x4b, 0xb4,
    0x44, 0x4a, 0x4f, 0x94, 0x44, 0x4a, 0x4f, 0xde, 0x40, 0x46, 0x4b, 0xff, 0x40, 0x45, 0x4b, 0xff,
    0x44, 0x48, 0x4f, 0xff, 0x48, 0x4f, 0x54, 0xff, 0x4b, 0x53, 0x59, 0xde, 0x4b, 0x52, 0x59, 0xff,
    0x48, 0x4e, 0x54, 0xff, 0x4b, 0x51, 0x59, 0xff, 0x4f, 0x55, 0x5d, 0xde, 0x1e, 0x21, 0x23, 0x4a,
    0x1e, 0x21, 0x23, 0x6a, 0x4b, 0x54, 0x59, 0x20, 0x43, 0x4b, 0x51, 0xff, 0x2d, 0x32, 0x35, 0x6a,
    0x2d, 0x32, 0x35, 0x29, 0x43, 0x4b, 0x51, 0x4a, 0x38, 0x3f, 0x43, 0xb4, 0x4b, 0x54, 0x59, 0x6a,
    0x38, 0x3f, 0x43, 0xff, 0x4b, 0x53, 0x59, 0xb4, 0x4e, 0x57, 0x5e, 0xde, 0x4b, 0x53, 0x59, 0xde,
    0x4e, 0x57, 0x5e, 0xff, 0x2d, 0x32, 0x35, 0xde, 0x2d, 0x32, 0x35, 0x94, 0x3c, 0x42, 0x47, 0xde,
    0x34, 0x3a, 0x3f, 0xde, 0x34, 0x3a, 0x3f, 0x94, 0x43, 0x4b, 0x51, 0xde, 0x47, 0x4f, 0x55, 0xde,
    0x38, 0x3e, 0x43, 0x94, 0x34, 0x3a, 0x3f, 0xff, 0x38, 0x3e, 0x43, 0xff, 0x47, 0x4f, 0x55, 0x94,
    0x30, 0x36, 0x3a, 0xb4, 0x29, 0x2d, 0x31, 0xff, 0x43, 0x4b, 0x51, 0xff, 0x4b, 0x53, 0x59, 0xff,
    0x29, 0x2d, 0x31, 0xb4, 0x3c, 0x42, 0x47, 0xff, 0x47, 0x4e, 0x55, 0xff, 0x25, 0x29, 0x2d, 0xde,
    0x25, 0x29, 0x2d, 0xff, 0x4b, 0x51, 0x59, 0xff, 0x2d, 0x2f, 0x35, 0x4a, 0x1e, 0x1e, 0x23, 0x20,
    0x1e, 0x1e, 0x23, 0x4a, 0x1e, 0x1e, 0x23, 0x6a, 0x2d, 0x2d, 0x35, 0x20, 0x2d, 0x2d, 0x35, 0x6a,
};