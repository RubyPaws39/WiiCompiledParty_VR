// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <cstdint>

struct Mp9ObservedCamera {
    std::array<float, 12> view{};
    std::array<float, 3> eye{}, target{};
    uint32_t caller = 0;
    bool valid = false;
};

// Read-only observers around the owned SDK's original translated bodies.
// They never replace the matrix calculation or change guest registers/memory.
class Mp9CameraObservationScope {
public:
    enum class Kind { LookAt, Concat, ConcatArray };
    Mp9CameraObservationScope(Kind kind, uint32_t output, uint32_t a, uint32_t b,
                              uint32_t c, uint32_t caller) noexcept;
    ~Mp9CameraObservationScope();
private:
    Kind kind_;
    uint32_t output_;
    uint32_t count_ = 1;
    Mp9ObservedCamera camera_{};
};

bool Mp9CameraForModelView(const float* modelView, Mp9ObservedCamera& camera);
bool Mp9SelectCamera(const float (*models)[12], const uint32_t* draws, unsigned count, Mp9ObservedCamera& camera);
// Diagnostic only. Never used to select a rendering camera.
float Mp9NearestCameraMatrix(const float* modelView, std::array<float,12>& nearest);
void Mp9ResetCameraTracking();
struct Mp9CameraTrackingStats {
    uint64_t lookAt = 0, validLookAt = 0, concat = 0, linkedConcat = 0;
    uint32_t links = 0;
    Mp9ObservedCamera lastCamera{};
};
Mp9CameraTrackingStats Mp9CameraStats();
struct Mp9TabletopAnchor {
    std::array<float, 12> transform{}, view{};
    float units = 0;
    bool valid = false;
};
Mp9TabletopAnchor Mp9GetTabletopAnchor();
Mp9TabletopAnchor Mp9MakeTabletopAnchor(const Mp9ObservedCamera& camera,
    const std::array<float,3>& center, float units, const std::array<float,3>& originOffset);
