// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <cstring>

inline bool Mp9IsGameplayModule(uint32_t id) noexcept {
    return (id >= 3 && id <= 11) || (id >= 18 && id <= 97) || (id >= 106 && id <= 113);
}

inline bool Mp9VrCanReplay(bool gameplay, bool recorded, bool multipleViews, const float (&matrix)[12]) noexcept {
    if (!gameplay || !recorded || multipleViews) return false;
    bool nonzero = false;
    for (const float& value : matrix) {
        uint32_t bits;
        std::memcpy(&bits, &value, sizeof(bits));
        if ((bits & 0x7f800000u) == 0x7f800000u) return false;
        nonzero |= (bits & 0x7fffffffu) != 0;
    }
    return nonzero;
}

// Producer-thread only, after draining the current frame's GX commands.
void Mp9PublishVrFrame(uint64_t frame);
