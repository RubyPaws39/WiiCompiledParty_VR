#if defined(MKW_GAME_MP9)
#include "mp9_rel_context.h"
#include "mp9_camera.h"
#include "memory.h"
#include "runtime_log.h"
#include "vr/mp9_presentation.h"
#include "vr/mkw_vr_policy.h"
#include "vr/mp9_mixed_reality.h"
#include "settings_overlay.h"
#include <aurora/aurora.h>
#include <array>
#include <algorithm>
#include <cstring>
#include <iomanip>
#include <stdexcept>
#include <vector>

namespace {
struct Section { uint32_t fileOffset, size; bool code; };
struct Module { uint32_t header = 0, table = 0; std::vector<Section> sections; };
// Guest linking and execution share the guest scheduler's host thread.
thread_local std::array<Module, 116> modules;
struct Calibration { bool valid = false; std::array<float, 3> center{}; float fit = 1; };
thread_local std::array<Calibration, 116> calibrations;
thread_local Mp9TabletopAnchor tabletopAnchor;
}

Mp9TabletopAnchor Mp9GetTabletopAnchor() { return tabletopAnchor; }

void Mp9ObserveRelLink(uint32_t header) {
    const uint32_t id = Memory::Read32(header);
    const uint32_t count = Memory::Read32(header + 0xC);
    const uint32_t offset = Memory::Read32(header + 0x10);
    RT_LOG(RT_TAG_MOD) << "[mp9-rel-link] id=" << id << " header=0x" << std::hex
                       << header << std::dec << " count=" << count << " off=" << offset
                       << std::endl;
    if (id == 0 || id >= modules.size() || count == 0 || count > 64 || offset > 0x200000 ||
        !Memory::Contains(header, 0x40) || !Memory::Contains(header + offset, count * 8))
        throw std::runtime_error("MP9 invalid REL link header");
    Module module{header, header + offset, {}};
    for (uint32_t index = 0; index < count; ++index) {
        const auto encoded = Memory::Read32(module.table + index * 8);
        const auto size = Memory::Read32(module.table + index * 8 + 4);
        const auto fileOffset = encoded & ~3u;
        if (fileOffset >= 0x200000 || size >= 0x200000 ||
            static_cast<uint64_t>(fileOffset) + size > 0x200000)
            throw std::runtime_error("MP9 REL section exceeds native dispatch slot");
        module.sections.push_back({fileOffset, size, (encoded & 1) != 0});
    }
    modules[id] = std::move(module);
}

void Mp9ObserveRelUnlink(uint32_t header) {
    const uint32_t id = Memory::Read32(header);
    if (id < modules.size() && modules[id].header == header) {
        modules[id] = {};
        calibrations[id] = {};
    }
}

void Mp9PublishVrFrame(uint64_t frame) {
    using namespace mkw::vr;
    AuroraPerspectiveEvidence evidence{};
    const bool recorded = aurora_take_perspective_evidence(&evidence);
    Mp9ObservedCamera observedCamera;
    const bool cameraLinked = recorded && Mp9SelectCamera(evidence.models, evidence.modelDraws,
                                                         evidence.modelCount, observedCamera);
    static unsigned cameraLogFrames = 0;
    if (recorded && (++cameraLogFrames % 300 == 1)) {
        RT_LOG(RT_TAG_RUNTIME) << "[mp9-camera] linked=" << cameraLinked;
        const auto stats = Mp9CameraStats();
        RT_LOG(RT_TAG_RUNTIME) << " lookAt=" << stats.lookAt << "/" << stats.validLookAt
            << " concat=" << stats.concat << "/" << stats.linkedConcat << " links=" << stats.links
            << " models=" << evidence.modelCount;
        if (cameraLinked) {
            RT_LOG(RT_TAG_RUNTIME) << " caller=0x" << std::hex << observedCamera.caller << std::dec
                << " eye=" << observedCamera.eye[0] << "/" << observedCamera.eye[1] << "/" << observedCamera.eye[2]
                << " target=" << observedCamera.target[0] << "/" << observedCamera.target[1] << "/" << observedCamera.target[2];
        }
        if (!cameraLinked && stats.lastCamera.valid) {
            const auto& c = stats.lastCamera;
            RT_LOG(RT_TAG_RUNTIME) << " lastCaller=0x" << std::hex << c.caller << std::dec
                << " lastEye=" << c.eye[0] << "/" << c.eye[1] << "/" << c.eye[2]
                << " lastTarget=" << c.target[0] << "/" << c.target[1] << "/" << c.target[2];
            if (evidence.modelCount) {
                std::array<float,12> nearest{};
                const float error = Mp9NearestCameraMatrix(evidence.models[0], nearest);
                RT_LOG(RT_TAG_RUNTIME) << " nearestError=" << error << " model=";
                for (float v : evidence.models[0]) RT_LOG(RT_TAG_RUNTIME) << v << ",";
                RT_LOG(RT_TAG_RUNTIME) << " nearest=";
                for (float v : nearest) RT_LOG(RT_TAG_RUNTIME) << v << ",";
            }
        }
        RT_LOG(RT_TAG_RUNTIME) << std::endl;
    }
    // Verified SSQE01 module identities: boards/tutorial, minigames and rendered
    // Solo scenes. Title, selection and instructions remain on the fixed screen.
    bool gameplay = false;
    for (uint32_t id = 3; id < modules.size(); ++id) {
        if (modules[id].header && Mp9IsGameplayModule(id)) gameplay = true;
    }
    if (modules[16].header || modules[17].header) gameplay = false;
    // Multiple perspective viewports need explicit local-player camera routing;
    // keep their original picture until that mapping is verified.
    const auto mr = mp9_mr::Read();
    uint32_t tabletopModule = 0;
    for (uint32_t id = 3; id <= 11; ++id) if (modules[id].header) tabletopModule = id;
    for (uint32_t id = 18; id <= 97; ++id) if (modules[id].header) tabletopModule = id;
    const bool tabletop = mr.enabled && tabletopModule && cameraLinked && !evidence.multipleViews && gameplay;
    tabletopAnchor = {};
    if (tabletop) {
        auto& calibration = calibrations[tabletopModule];
        if (!calibration.valid && std::abs(evidence.projectionX) > 0.01f) {
            float distanceSquared = 0;
            for (unsigned axis = 0; axis < 3; ++axis) {
                const float d = observedCamera.eye[axis] - observedCamera.target[axis];
                distanceSquared += d * d;
            }
            const float span = 2.0f * std::sqrt(distanceSquared) / std::abs(evidence.projectionX);
            calibration.fit = std::clamp(span / mr.width, 10.0f, 10000.0f);
            calibration.center = observedCamera.target;
            calibration.valid = distanceSquared > 0.01f && mp9_mr::Finite(calibration.fit);
        }
        if (calibration.valid) {
            const float units = calibration.fit * mr.units / 500.0f;
            auto policy = MkwVRPolicyGetSnapshot().config;
            if (policy.world_units_per_meter != units) {
                policy.world_units_per_meter = units;
                MkwVRPolicyConfigure(policy);
                settings_overlay::RefreshVrHudVirtualScreen();
            }
            // The proven view is rigid: inverse rotation is its transpose.
            // world -> placed model is fixed; inverse(gameView) cancels camera motion.
            auto originOffset = mp9_mr::kTabletopHudOffset;
            originOffset[2] += policy.hud_distance_meters;
            tabletopAnchor = Mp9MakeTabletopAnchor(observedCamera, calibration.center, units, originOffset);
        }
    }
    const bool immersive = mr.enabled ? tabletopAnchor.valid :
        Mp9VrCanReplay(gameplay, recorded, evidence.multipleViews, evidence.modelView);
    MkwVRSceneObservation scene{};
    scene.mode = immersive ? VRSceneMode::Race : VRSceneMode::Other;
    scene.local_player_count = immersive ? 1 : 0;
    scene.guest_frame_index = frame;
    MkwVRPolicyPublishScene(scene);
    if (immersive) {
        MkwVRCameraObservation camera{};
        std::copy(std::begin(evidence.modelView), std::end(evidence.modelView), camera.view_from_world.begin());
        camera.guest_frame_index = frame;
        camera.valid = true;
        camera.tabletop = tabletopAnchor.valid;
        if (camera.tabletop) camera.view_from_world = observedCamera.view;
        // Actual recorded model-view evidence; replay retains every original
        // draw's own matrices. No guessed guest camera pointer or identity stub.
        MkwVRPolicyPublishRaceCamera(camera);
    } else {
        MkwVRPolicyInvalidateRaceCamera();
    }
    static int last = -1;
    if (last != int(immersive)) {
        last = int(immersive);
        RT_LOG(RT_TAG_RUNTIME) << "[mp9-vr] " << (immersive ? "stereo gameplay" : "fixed screen")
            << " frame=" << frame << " perspective=" << evidence.draws
            << " multiview=" << evidence.multipleViews << std::endl;
    }
}

uint32_t Mp9RelAddress(uint32_t id, uint32_t section, uint32_t offset) {
    if (id == 0 || id >= modules.size() || !modules[id].header ||
        section >= modules[id].sections.size() || offset >= modules[id].sections[section].size)
        throw std::runtime_error("MP9 native REL address outside loaded section");
    const uint32_t base = Memory::Read32(modules[id].table + section * 8) & ~3u;
    const uint64_t result = static_cast<uint64_t>(base) + offset;
    if (!base || result > UINT32_MAX || !Memory::Contains(static_cast<uint32_t>(result), 1))
        throw std::runtime_error("MP9 native REL section not linked in guest RAM");
    return static_cast<uint32_t>(result);
}

uint32_t Mp9CanonicalRelTarget(uint32_t target) {
    if (target < 0x803343B8u || (target & 3) != 0) return target;
    for (uint32_t id = 1; id < modules.size(); ++id) {
        const auto& module = modules[id];
        if (!module.header) continue;
        for (uint32_t section = 0; section < module.sections.size(); ++section) {
            const auto& info = module.sections[section];
            if (!info.code || !info.size) continue;
            const auto base = Memory::Read32(module.table + section * 8) & ~3u;
            if (target >= base && static_cast<uint64_t>(target) < static_cast<uint64_t>(base) + info.size)
                return 0xA0000000u + (id << 21) + info.fileOffset + target - base;
        }
    }
    return target;
}

uint32_t Mp9RelHighImmediate(uint32_t id, uint32_t section, uint32_t offset) {
    if (offset > UINT32_MAX - 3) throw std::runtime_error("MP9 REL immediate offset overflow");
    (void)Mp9RelAddress(id, section, offset + 3);
    return static_cast<uint32_t>(Memory::Read16(Mp9RelAddress(id, section, offset + 2))) << 16;
}
uint32_t Mp9RelSignedImmediate(uint32_t id, uint32_t section, uint32_t offset) {
    if (offset > UINT32_MAX - 3) throw std::runtime_error("MP9 REL immediate offset overflow");
    (void)Mp9RelAddress(id, section, offset + 3);
    return static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(
        Memory::Read16(Mp9RelAddress(id, section, offset + 2)))));
}
#endif
