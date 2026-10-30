// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>

namespace mkw::vr::mp9_mr {

inline bool Finite(const float& value) noexcept {
    uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    return (bits & 0x7f800000u) != 0x7f800000u;
}

struct Pose {
    std::array<float, 3> position{};
    float yaw = 0;
    bool valid = false;
};

inline bool Usable(const Pose& pose) noexcept {
    return pose.valid && Finite(pose.yaw) && Finite(pose.position[0]) &&
           Finite(pose.position[1]) && Finite(pose.position[2]);
}

inline std::array<float, 3> Rotate(float yaw, const std::array<float, 3>& p) noexcept {
    const float c = std::cos(yaw), s = std::sin(yaw);
    return {c * p[0] + s * p[2], p[1], -s * p[0] + c * p[2]};
}

struct Snapshot {
    bool enabled = true;
    bool placing = false;
    bool room = true;
    bool stereo = true;
    Pose screen{}; // Session-local OpenXR space. Never persist as a spatial anchor.
    float width = 1.2f;
    float units = 500.0f;
    unsigned quality = 1; // 50%, 75%, 100% of runtime-recommended eye resolution.
};

struct Input {
    Pose head{}, right{}, left{};
    float leftX = 0, leftY = 0, rightX = 0, rightY = 0;
    float rightGrip = 0, leftGrip = 0, rightTrigger = 0, leftTrigger = 0;
    bool rightClick = false, leftClick = false, a = false, b = false;
    bool x = false, y = false, menu = false;
    bool settings = false;
    float seconds = 0;
};

struct Result {
    bool withheld = false;
    bool save = false;
    bool qualityChanged = false;
};

// A 60 Hz game has even compositor cadence at 120 or 60 Hz. Otherwise retain
// the runtime's choice; never request a rate the headset did not enumerate.
inline float PreferredRefreshRate(const float* rates, unsigned count) noexcept {
    for (float target : {120.0f, 60.0f})
        for (unsigned i = 0; i < count; ++i)
            if (Finite(rates[i]) && std::abs(rates[i] - target) < 0.1f) return rates[i];
    return 0;
}

// Independent implementation of the reference project's placement gestures.
// Placement consumes both controllers; a release barrier prevents A/B/Plus leakage.
class Controls {
public:
    Snapshot state{};

    void ResetPose() noexcept {
        state.screen = {};
        state.placing = false;
        grabbing = false;
        release = true;
    }

    Result Update(const Input& in) noexcept {
        Result out{};
        const bool entered = in.rightClick && !in.leftClick && !previous.rightClick && !state.placing && !in.settings;
        if (state.enabled && !Usable(state.screen) && Usable(in.head)) {
            state.screen = in.head;
            const auto ahead = Rotate(in.head.yaw, {0, -0.3f, -1.5f});
            for (unsigned i = 0; i < 3; ++i) state.screen.position[i] += ahead[i];
        }
        if (!state.enabled) {
            state.placing = false;
            grabbing = false;
        } else if (entered) {
            state.placing = true;
        }
        if (state.placing) {
            out.withheld = true;
            const bool done = !entered && ((in.a && !previous.a) || (in.b && !previous.b) ||
                (in.menu && !previous.menu) || (in.rightClick && !previous.rightClick));
            if (done) {
                state.placing = false;
                out.save = true;
                release = true;
                grabbing = false;
            } else {
                const float dt = Finite(in.seconds) ? std::clamp(in.seconds, 0.0f, 0.05f) : 0.0f;
                const auto axis = [](const float& v) {
                    return Finite(v) && std::abs(v) > 0.2f ? std::clamp(v, -1.0f, 1.0f) : 0.0f;
                };
                const bool carry = in.rightGrip > (grabbing ? 0.4f : 0.6f) && Usable(in.right);
                if (carry && Usable(state.screen)) {
                    if (!grabbing) {
                        std::array<float, 3> delta{};
                        for (unsigned i = 0; i < 3; ++i) delta[i] = state.screen.position[i] - in.right.position[i];
                        grabOffset = Rotate(-in.right.yaw, delta);
                        grabYaw = state.screen.yaw - in.right.yaw;
                    }
                    const auto delta = Rotate(in.right.yaw, grabOffset);
                    for (unsigned i = 0; i < 3; ++i) state.screen.position[i] = in.right.position[i] + delta[i];
                    state.screen.yaw = in.right.yaw + grabYaw;
                }
                grabbing = carry;
                if (in.rightTrigger > 0.6f && previous.rightTrigger <= 0.6f &&
                    Usable(in.right) && Usable(in.head)) {
                    state.screen = in.right;
                    state.screen.yaw = std::atan2(in.head.position[0] - in.right.position[0],
                                                  in.head.position[2] - in.right.position[2]);
                }
                if (in.leftTrigger > 0.6f && previous.leftTrigger <= 0.6f && Usable(in.left))
                    state.screen.position[1] = in.left.position[1];
                state.width = std::clamp(state.width * std::exp(axis(in.rightY) * dt * 0.8f), 0.35f, 5.0f);
                state.units = std::clamp(state.units * std::exp(-axis(in.leftX) * dt * 0.8f), 10.0f, 10000.0f);
                state.screen.yaw = std::remainder(state.screen.yaw - axis(in.rightX) * dt * 1.5f, 6.283185307f);
                state.screen.position[1] = std::clamp(state.screen.position[1] + axis(in.leftY) * dt * 0.15f, -5.0f, 5.0f);
                if (in.leftClick && !previous.leftClick) state.stereo = !state.stereo;
                if (in.x && !previous.x) state.room = !state.room;
                if (in.y && !previous.y) {
                    state.quality = (state.quality + 1) % 3;
                    out.qualityChanged = true;
                }
            }
        }
        // Continue withholding until every placement control is released, including sticks.
        if (release) {
            out.withheld = true;
            const bool clean = !in.a && !in.b && !in.x && !in.y && !in.menu && !in.rightClick && !in.leftClick &&
                in.rightTrigger < 0.2f && in.leftTrigger < 0.2f && in.rightGrip < 0.2f && in.leftGrip < 0.2f &&
                std::abs(in.leftX) < 0.2f && std::abs(in.leftY) < 0.2f &&
                std::abs(in.rightX) < 0.2f && std::abs(in.rightY) < 0.2f;
            if (clean) release = false;
        }
        previous = in;
        return out;
    }

private:
    Input previous{};
    bool grabbing = false, release = false;
    std::array<float, 3> grabOffset{};
    float grabYaw = 0;
};

Snapshot Read() noexcept;
void SetEnabled(bool enabled) noexcept;
void SetPresentation(bool stereo, bool room) noexcept;
void ResetPose() noexcept;
// Game thread only: flush a completed placement's small settings write.
void PersistPendingSettings();

} // namespace mkw::vr::mp9_mr
