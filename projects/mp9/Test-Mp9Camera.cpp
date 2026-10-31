// SPDX-License-Identifier: GPL-3.0-or-later
// Synthetic RAM only; no game bytes. Compile with mp9_camera.cpp and MKW_GAME_MP9.
#include "mp9_camera.h"
#include "memory.h"
#include <array>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <cmath>

static std::array<uint32_t, 1024> ram{};
bool Memory::Contains(uint32_t address, size_t size) {
    return address <= sizeof(ram) && size <= sizeof(ram) - address;
}
namespace MemoryInline {
uintptr_t g_fullReadablePageBias[kPageCount]{};
uint32_t Read32Slow(uint32_t address) {
    assert(Memory::Contains(address, 4) && address % 4 == 0);
    return ram[address / 4];
}
}
template<size_t N> static void write(uint32_t address, const std::array<float, N>& values) {
    assert(Memory::Contains(address, sizeof(values)));
    std::memcpy(reinterpret_cast<char*>(ram.data()) + address, values.data(), sizeof(values));
}
static void look(float x) {
    write(64, std::array<float, 3>{x, 0, 10});
    write(80, std::array<float, 3>{x, 0, 0});
    Mp9CameraObservationScope scope(Mp9CameraObservationScope::Kind::LookAt, 128, 64, 0, 80, 123);
    write(128, std::array<float, 12>{1,0,0,-x, 0,1,0,0, 0,0,1,-10});
}
int main() {
    Mp9ResetCameraTracking();
    look(0);
    Mp9ObservedCamera camera;
    std::array<float,12> view{1,0,0,0, 0,1,0,0, 0,0,1,-10};
    assert(Mp9CameraForModelView(view.data(), camera) && camera.eye[2] == 10);
    // Original concat may write output in place; the observer must capture its
    // left input before the guest body runs and output only afterward.
    for (unsigned i = 0; i < 20000; ++i) {
        write(256, view);
        Mp9CameraObservationScope scope(Mp9CameraObservationScope::Kind::Concat, 256, 256, 0, 0, 0);
        auto modelView = view;
        modelView[3] = float(i + 1);
        write(256, modelView);
    }
    assert(Mp9CameraForModelView(view.data(), camera)); // Active camera survives capacity.
    assert(Mp9CameraStats().links == 16384);
    write(256, view);
    {
        Mp9CameraObservationScope scope(Mp9CameraObservationScope::Kind::ConcatArray, 512, 256, 0, 2, 0);
        auto first = view, second = view;
        first[7] = 17; second[7] = 23;
        write(512, first); write(560, second);
    }
    auto second = view; second[7] = 23;
    assert(Mp9CameraForModelView(second.data(), camera));
    std::array<float,12> sharedModel{1,0,0,5, 0,1,0,0, 0,0,1,-10};
    write(256, view);
    {
        Mp9CameraObservationScope scope(Mp9CameraObservationScope::Kind::Concat, 512, 256, 0, 0, 0);
        write(512, sharedModel);
    }
    assert(Mp9CameraForModelView(sharedModel.data(), camera));
    look(2);
    {
        Mp9CameraObservationScope scope(Mp9CameraObservationScope::Kind::Concat, 512, 128, 0, 0, 0);
        write(512, sharedModel);
    }
    assert(!Mp9CameraForModelView(sharedModel.data(), camera)); // Ambiguous camera rejected.
    float models[2][12];
    auto otherView = view; otherView[3] = -2;
    std::memcpy(models[0], view.data(), sizeof(view));
    std::memcpy(models[1], otherView.data(), sizeof(view));
    uint32_t votes[]{3,1};
    assert(Mp9SelectCamera(models, votes, 2, camera) && camera.eye[0] == 0);
    votes[1] = 3;
    assert(!Mp9SelectCamera(models, votes, 2, camera)); // Equal cameras fail closed.
    // Moving/rotating the game camera cannot move a fixed world point on table.
    for (const auto& v : {view, std::array<float,12>{0,0,-1,3, 0,1,0,-4, 1,0,0,-2}}) {
        camera.view = v; camera.valid = true;
        const auto table = Mp9MakeTabletopAnchor(camera, {.5f,1,2}, 2, {.2f,.45f,1.35f});
        assert(table.valid);
        const float world[]{1,7,-5}, expected[]{.1f,5.1f,-9.7f};
        float scene[3]{};
        for (unsigned r=0; r<3; ++r) {
            scene[r] = v[r*4+3];
            for (unsigned c=0; c<3; ++c) scene[r] += v[r*4+c]*world[c];
        }
        for (unsigned r=0; r<3; ++r) {
            float placed = table.transform[r*4+3];
            for (unsigned c=0; c<3; ++c) placed += table.transform[r*4+c]*scene[c];
            assert(std::abs(placed-expected[r]) < .0001f);
        }
    }
    assert(!Mp9MakeTabletopAnchor(camera, {}, 0, {}).valid);
    const auto before = ram;
    {
        Mp9CameraObservationScope scope(Mp9CameraObservationScope::Kind::LookAt, UINT32_MAX, UINT32_MAX, 0, 80, 0);
    }
    assert(ram == before); // Observers cannot modify guest RAM.
    Mp9ResetCameraTracking();
    assert(!Mp9CameraForModelView(view.data(), camera));
    puts("MP9 camera provenance: passed");
}
