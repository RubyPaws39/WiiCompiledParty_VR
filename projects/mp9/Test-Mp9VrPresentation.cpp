// SPDX-License-Identifier: GPL-3.0-or-later
#include "vr/mp9_presentation.h"
#include <cassert>
#include <iostream>
#include <limits>

int main() {
    for (uint32_t id : {3, 9, 11, 18, 41, 97, 106, 107, 113}) assert(Mp9IsGameplayModule(id));
    for (uint32_t id : {0, 1, 2, 12, 15, 16, 17, 98, 105, 114, 115, 116}) assert(!Mp9IsGameplayModule(id));
    float matrix[12]{1, 0, 0, 10, 0, 1, 0, 20, 0, 0, 1, -30};
    assert(Mp9VrCanReplay(true, true, false, matrix));
    assert(!Mp9VrCanReplay(false, true, false, matrix)); // Menus remain flat.
    assert(!Mp9VrCanReplay(true, false, false, matrix)); // No fabricated camera.
    assert(!Mp9VrCanReplay(true, true, true, matrix)); // Unverified split layout.
    volatile uint32_t raw = 0x7fc00000u;
    uint32_t bits = raw;
    std::memcpy(&matrix[0], &bits, sizeof(bits));
    assert(!Mp9VrCanReplay(true, true, false, matrix));
    raw = 0x7f800000u;
    bits = raw;
    std::memcpy(&matrix[0], &bits, sizeof(bits));
    assert(!Mp9VrCanReplay(true, true, false, matrix));
    float zero[12]{};
    assert(!Mp9VrCanReplay(true, true, false, zero));
    std::cout << "MP9 VR presentation gates: all checks passed\n";
}
