#pragma once
#include <cstddef>
#include <cstdint>
#include <stdexcept>

// Wii GX register IDs and SSQE01 SDK layout; no game executable or asset data.
inline void Mp9InitGxRegisterIds(uint8_t* state, size_t size) {
    if (!state || size < 0x600) throw std::invalid_argument("MP9 GX state requires 0x600 bytes");
    const auto put = [state](size_t offset, uint32_t value) {
        for (size_t byte = 0; byte < 4; ++byte)
            state[offset + byte] = static_cast<uint8_t>(value >> (24 - byte * 8));
    };
    put(0x000, 0x00010000);
    put(0x07C, 0x22000000);
    put(0x148, 0x20000000); put(0x14C, 0x21000000);
    put(0x170, 0x27000000); put(0x174, 0x0F0000FF);
    for (uint32_t i = 0; i < 8; ++i) {
        put(0x108 + i * 4, (0x30 + i * 2) << 24);
        put(0x128 + i * 4, (0x31 + i * 2) << 24);
        put(0x150 + i * 4, (0x28 + i) << 24);
        put(0x200 + i * 4, (0xF6 + i) << 24);
    }
    for (uint32_t i = 0; i < 16; ++i) {
        put(0x180 + i * 4, (0xC0 + i * 2) << 24);
        put(0x1C0 + i * 4, (0xC1 + i * 2) << 24);
        put(0x5A4 + i * 4, 0xFF);
    }
    put(0x220, 0x41000000); put(0x224, 0x42000000);
    put(0x228, 0x40000000); put(0x22C, 0x43000000);
    put(0x560, 0x4B800000); // GX depth scale: 2^24.
}
