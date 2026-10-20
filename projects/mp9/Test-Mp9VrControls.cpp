// SPDX-License-Identifier: GPL-3.0-or-later
// Compile with MKW_GAME_MP9 and runtime/src/vr/openxr_wii_remote.cpp.
#include "vr/openxr_wii_remote.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

int main() {
    using namespace mkw::vr;
    using namespace mkw::vr::wii_remote;
    HandInputs left{}, right{};
    assert(RemoteButtons(left, right) == 0);
    right.primary = true;
    right.trigger = 1;
    assert(RemoteButtons(left, right) == (kButtonA | kButtonB));
    left = {}; right = {};
    left.primary = true; right.secondary = true;
    assert(RemoteButtons(left, right) == (kButtonOne | kButtonTwo));
    left = {}; right = {};
    left.secondary = true;
    assert(RemoteButtons(left, right) == 0); // Y belongs to settings.
    left = {}; left.stick_x = -1; left.stick_y = 1;
    assert(RemoteButtons(left, right) == (kButtonLeft | kButtonUp));
    left = {}; right = {}; left.menu = true;
    assert(RemoteButtons(left, right) == kButtonPlus);
    left = {}; left.thumbstick_click = true;
    assert(RemoteButtons(left, right) == kButtonPlus);
    left = {}; left.squeeze = .8f;
    assert(RemoteButtons(left, right) == 0);
    right.squeeze = .8f;
    assert(RemoteButtons(left, right) == kButtonHome);
    left = {}; right = {}; left.secondary = true; right.thumbstick_click = true;
    assert(RemoteButtons(left, right) == 0);
    left = {}; right = {}; left.stick_x = .9f;
    assert(RemoteButtons(left, right) == kButtonRight);
    left.stick_x = 0; left.stick_y = -.9f;
    assert(RemoteButtons(left, right) == kButtonDown);
    left.stick_x = -.9f;
    assert(RemoteButtons(left, right) == (kButtonLeft | kButtonDown));
    left = {}; right = {}; left.trigger = .8f;
    assert(RemoteButtons(left, right) == kButtonMinus);
    left = {}; left.stick_x = .3f; left.stick_y = .3f; right.stick_y = .9f;
    assert(RemoteButtons(left, right) == 0);
    OpenXRSetWiiRemoteTuning({-1, 10, true});
    auto tuning = OpenXRGetWiiRemoteTuning();
    assert(tuning.pointer_gain == .5f && tuning.motion_gain == 2 && tuning.left_remote);
    OpenXRWiiRemoteTuning invalid{};
    volatile uint32_t raw = 0x7fc00000u;
    uint32_t bits = raw;
    std::memcpy(&invalid.pointer_gain, &bits, sizeof(bits));
    raw = 0x7f800000u; bits = raw;
    std::memcpy(&invalid.motion_gain, &bits, sizeof(bits));
    OpenXRSetWiiRemoteTuning(invalid);
    tuning = OpenXRGetWiiRemoteTuning();
    assert(tuning.pointer_gain == 1 && tuning.motion_gain == 1 && !tuning.left_remote);
    const Quat identity{0, 0, 0, 1};
    const auto rest = KpadAcceleration(identity, {});
    const auto still = ScaleKpadMotion(rest, identity, 2);
    assert(still == rest); // Strength must not amplify gravity/tilt.
    const auto moving = ScaleKpadMotion({1, -1, 0}, identity, 2);
    assert(moving[0] == 2 && moving[1] == -1 && moving[2] == 0);
    const auto clamped = ScaleKpadMotion({100, -100, 100}, identity, 2);
    for (float axis : clamped) assert(std::abs(axis) <= kAccelRangeG);
    std::cout << "MP9 VR controls: all checks passed\n";
}
