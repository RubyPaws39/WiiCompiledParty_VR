// SPDX-License-Identifier: GPL-3.0-or-later
#if defined(MKW_GAME_MP9)
#include "mp9_camera.h"
#include "memory.h"
#include "vr/mp9_mixed_reality.h"
#include <cmath>
#include <cstring>
#include <list>
#include <limits>
#include <unordered_map>

namespace {
using Key = std::array<uint32_t, 12>;
struct Hash {
    size_t operator()(const Key& key) const noexcept {
        size_t hash = 2166136261u;
        for (auto word : key) hash = (hash ^ word) * 16777619u;
        return hash;
    }
};
using Order = std::list<Key>;
struct Link { Mp9ObservedCamera camera; bool ambiguous = false; Order::iterator position; };
// Guest execution and the producer frame boundary share one host thread.
thread_local std::unordered_map<Key, Link, Hash> links;
thread_local Order order;
thread_local Mp9CameraTrackingStats stats;

bool ReadFloats(uint32_t address, float* out, unsigned count) noexcept {
    if (!Memory::Contains(address, count * sizeof(float))) return false;
    for (unsigned i = 0; i < count; ++i) {
        const uint32_t word = Memory::Read32(address + i * 4);
        std::memcpy(out + i, &word, sizeof(word));
        if (!mkw::vr::mp9_mr::Finite(out[i])) return false;
    }
    return true;
}
Key MatrixKey(const float* matrix) {
    Key key;
    std::memcpy(key.data(), matrix, sizeof(key));
    // GX uploads may canonicalize signed zero. Both encodings denote the same
    // transform; keep every nonzero component bit-exact.
    for (auto& word : key) if ((word & 0x7fffffffu) == 0) word = 0;
    return key;
}
bool SameView(const Mp9ObservedCamera& a, const Mp9ObservedCamera& b) {
    return MatrixKey(a.view.data()) == MatrixKey(b.view.data());
}
void Remember(const float* modelView, const Mp9ObservedCamera& camera) {
    const auto key = MatrixKey(modelView);
    if (auto found = links.find(key); found != links.end()) {
        found->second.ambiguous |= !SameView(found->second.camera, camera);
        order.splice(order.end(), order, found->second.position);
        return;
    }
    // Bounded LRU: retain active camera roots across frames, without periodically
    // losing every provenance chain when animated model matrices fill the cache.
    if (links.size() >= 16384) { links.erase(order.front()); order.pop_front(); }
    order.push_back(key);
    links.emplace(key, Link{camera, false, std::prev(order.end())});
}
bool IsLookAt(const Mp9ObservedCamera& c) {
    for (unsigned r = 0; r < 3; ++r) {
        float norm = 0, eye = c.view[r * 4 + 3], target = eye;
        for (unsigned k = 0; k < 3; ++k) {
            const float v = c.view[r * 4 + k];
            norm += v * v;
            eye += v * c.eye[k];
            target += v * c.target[k];
        }
        if (std::abs(norm - 1.0f) > 0.005f || std::abs(eye) > 0.05f ||
            (r < 2 && std::abs(target) > 0.05f) || (r == 2 && target >= -0.01f)) return false;
        for (unsigned other = r + 1; other < 3; ++other) {
            float dot = 0;
            for (unsigned k = 0; k < 3; ++k) dot += c.view[r * 4 + k] * c.view[other * 4 + k];
            if (std::abs(dot) > 0.005f) return false;
        }
    }
    return true;
}
}

bool Mp9CameraForModelView(const float* modelView, Mp9ObservedCamera& camera) {
    const auto it = links.find(MatrixKey(modelView));
    if (it == links.end() || it->second.ambiguous) return false;
    order.splice(order.end(), order, it->second.position);
    camera = it->second.camera;
    return camera.valid;
}
void Mp9ResetCameraTracking() { links.clear(); order.clear(); stats = {}; }
bool Mp9SelectCamera(const float (*models)[12], const uint32_t* draws, unsigned count, Mp9ObservedCamera& camera) {
    if (count > 24) return false;
    std::array<Mp9ObservedCamera,24> candidates{};
    std::array<uint64_t,24> votes{};
    unsigned used = 0;
    for (unsigned i = 0; i < count; ++i) {
        Mp9ObservedCamera candidate;
        if (!draws[i] || !Mp9CameraForModelView(models[i], candidate)) continue;
        unsigned slot = 0;
        while (slot < used && !SameView(candidates[slot], candidate)) ++slot;
        if (slot == used) candidates[used++] = candidate;
        votes[slot] += draws[i];
    }
    uint64_t best = 0;
    bool tied = false;
    for (unsigned i = 0; i < used; ++i) {
        if (votes[i] > best) { best = votes[i]; camera = candidates[i]; tied = false; }
        else if (votes[i] == best) tied = true;
    }
    return best != 0 && !tied;
}
Mp9TabletopAnchor Mp9MakeTabletopAnchor(const Mp9ObservedCamera& camera,
    const std::array<float,3>& center, float units, const std::array<float,3>& originOffset) {
    Mp9TabletopAnchor result;
    if (!camera.valid || !mkw::vr::mp9_mr::Finite(units) || units <= 0) return result;
    result.view = camera.view;
    result.units = units;
    for (unsigned r = 0; r < 3; ++r) {
        if (!mkw::vr::mp9_mr::Finite(center[r]) || !mkw::vr::mp9_mr::Finite(originOffset[r])) return {};
        float translation = -center[r] - originOffset[r] * units;
        for (unsigned c = 0; c < 3; ++c) {
            const float value = camera.view[c * 4 + r];
            result.transform[r * 4 + c] = value;
            translation -= value * camera.view[c * 4 + 3];
        }
        result.transform[r * 4 + 3] = translation;
    }
    result.valid = true;
    return result;
}
float Mp9NearestCameraMatrix(const float* modelView, std::array<float,12>& nearest) {
    float best = std::numeric_limits<float>::max();
    for (const auto& [key, link] : links) {
        if (link.ambiguous) continue;
        std::array<float,12> candidate;
        std::memcpy(candidate.data(), key.data(), sizeof(candidate));
        float error = 0;
        for (unsigned i = 0; i < 12; ++i)
            error = std::max(error, std::abs(modelView[i] - candidate[i]) / (1.0f + std::abs(modelView[i])));
        if (error < best) { best = error; nearest = candidate; }
    }
    return best;
}
Mp9CameraTrackingStats Mp9CameraStats() { stats.links = static_cast<uint32_t>(links.size()); return stats; }

Mp9CameraObservationScope::Mp9CameraObservationScope(Kind kind, uint32_t output,
    uint32_t a, uint32_t b, uint32_t c, uint32_t caller) noexcept : kind_(kind), output_(output) {
    if (kind == Kind::LookAt) {
        ++stats.lookAt;
        camera_.caller = caller;
        camera_.valid = ReadFloats(a, camera_.eye.data(), 3) && ReadFloats(c, camera_.target.data(), 3);
    } else {
        ++stats.concat;
        float left[12];
        camera_.valid = ReadFloats(a, left, 12) && Mp9CameraForModelView(left, camera_);
        if (kind == Kind::ConcatArray) {
            count_ = c;
            camera_.valid &= count_ > 0 && count_ <= 65535 && Memory::Contains(output_, size_t(count_) * 48);
        }
        stats.linkedConcat += camera_.valid;
    }
    (void)b;
}
Mp9CameraObservationScope::~Mp9CameraObservationScope() {
    if (!camera_.valid) return;
    float output[12];
    if (!ReadFloats(output_, output, 12)) return;
    if (kind_ == Kind::LookAt) {
        std::memcpy(camera_.view.data(), output, sizeof(output));
        if (!IsLookAt(camera_)) return;
        ++stats.validLookAt;
        stats.lastCamera = camera_;
    }
    Remember(output, camera_);
    for (uint32_t i = 1; i < count_; ++i)
        if (ReadFloats(output_ + i * 48, output, 12)) Remember(output, camera_);
}
#endif
