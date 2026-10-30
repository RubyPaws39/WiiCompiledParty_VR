// SPDX-License-Identifier: GPL-3.0-or-later
#include "vr/mp9_mixed_reality.h"
#include <cassert>
#include <cstdio>

using namespace mkw::vr::mp9_mr;
static bool near(float a, float b) { return std::abs(a - b) < 0.00001f * (1.0f + std::abs(b)); }

int main() {
    const float rates[]{72, 90, 120};
    assert(PreferredRefreshRate(rates, 3) == 120);
    assert(PreferredRefreshRate(rates, 2) == 0);
    const float sixty[]{60, 90};
    assert(PreferredRefreshRate(sixty, 2) == 60);
    Controls control;
    Input in;
    in.head = {{0, 1.6f, 0}, 0, true};
    in.right = {{0.2f, 0.8f, -0.8f}, 0, true};
    in.left = {{-0.2f, 0.75f, -0.8f}, 0, true};
    in.seconds = 1.0f / 72.0f;
    control.Update(in);
    assert(Usable(control.state.screen));
    assert(near(control.state.screen.position[1], 1.3f));
    assert(near(control.state.screen.position[2], -1.5f));
    const auto fixed = control.state.screen;
    in.head.position[0] += 1;
    control.Update(in);
    assert(control.state.screen.position == fixed.position); // Head movement never drags placement.

    in.rightClick = true;
    assert(control.Update(in).withheld && control.state.placing);
    in.rightClick = false;
    control.Update(in);
    in.leftX = in.leftY = in.rightX = in.rightY = 1;
    const auto initial = control.state;
    for (int i = 0; i < 72; ++i) assert(control.Update(in).withheld);
    assert(near(control.state.units, initial.units * std::exp(-0.8f)));
    assert(near(control.state.width, initial.width * std::exp(0.8f)));
    assert(near(control.state.screen.position[1], initial.screen.position[1] + 0.15f));
    assert(near(control.state.screen.yaw, initial.screen.yaw - 1.5f));
    in.leftX = in.leftY = in.rightX = in.rightY = 0;
    in.rightTrigger = 1;
    control.Update(in);
    assert(control.state.screen.position == in.right.position);
    in.rightTrigger = 0;
    control.Update(in);
    in.rightGrip = 1;
    control.Update(in);
    in.right.position[0] += 0.3f;
    control.Update(in);
    assert(near(control.state.screen.position[0], 0.5f));
    in.right.valid = false;
    const auto beforeLoss = control.state.screen;
    in.right.position[0] = 100;
    control.Update(in);
    assert(control.state.screen.position == beforeLoss.position);
    in.rightGrip = 0;
    in.x = in.y = in.leftClick = true;
    const bool room = control.state.room, stereo = control.state.stereo;
    assert(control.Update(in).qualityChanged);
    assert(control.state.room != room && control.state.stereo != stereo);
    assert(!control.Update(in).qualityChanged); // Held buttons do not repeat toggles.
    in.a = true;
    assert(control.Update(in).save && !control.state.placing);
    assert(control.Update(in).withheld); // Closing A never becomes guest Wii A.
    in.a = in.x = in.y = in.leftClick = false;
    assert(control.Update(in).withheld);
    assert(!control.Update(in).withheld);

    in.rightClick = true;
    in.settings = true;
    control.Update(in);
    assert(!control.state.placing); // Settings retain ownership.
    Controls chord;
    Input both;
    both.leftClick = both.rightClick = true;
    chord.Update(both);
    assert(!chord.state.placing); // Preserve the existing gamepad settings chord.
    control.ResetPose();
    assert(!control.state.screen.valid && !control.state.placing);

    volatile uint32_t nanBits = 0x7fc00001u;
    float nan;
    const uint32_t bits = nanBits;
    std::memcpy(&nan, &bits, sizeof(nan));
    assert(!Finite(nan)); // Must hold with -ffast-math.
    assert(PreferredRefreshRate(&nan, 1) == 0);
    Controls invalid;
    in = {};
    in.head = {{nan, 0, 0}, 0, true};
    invalid.Update(in);
    assert(!invalid.state.screen.valid);
    std::puts("MP9 MR placement, scaling, tracking loss and input ownership passed");
}
