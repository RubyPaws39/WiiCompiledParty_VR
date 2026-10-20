// SPDX-License-Identifier: GPL-3.0-or-later
#include "gfx/efb_ram_encoder.hpp"
#include <array>
#include <cassert>
#include <iostream>

int main() {
    using namespace aurora::gfx::efb_ram;
    std::array<uint8_t, 16 * 4 * 4> pixels{};
    for (size_t pixel = 0; pixel < 64; ++pixel) {
        pixels[pixel * 4] = static_cast<uint8_t>(pixel);
        pixels[pixel * 4 + 1] = 0xab;
        pixels[pixel * 4 + 2] = 0xcd;
    }
    std::array<uint8_t, 66> output{};
    output.fill(0xee);
    assert(supports_format(GX_TF_Z8));
    assert(encoded_size(GX_TF_Z8, 16, 4) == 64);
    assert(!encode(output.data(), 63, GX_TF_Z8, 16, 4, pixels.data(), 16, 4, 64, HostPixelOrder::RGBA));
    assert(output[0] == 0xee);
    assert(encode(output.data(), 64, GX_TF_Z8, 16, 4, pixels.data(), 16, 4, 64, HostPixelOrder::RGBA));
    for (size_t block = 0; block < 2; ++block)
        for (size_t y = 0; y < 4; ++y)
            for (size_t x = 0; x < 8; ++x)
                assert(output[block * 32 + y * 8 + x] == y * 16 + block * 8 + x);
    assert(output[64] == 0xee && output[65] == 0xee);
    assert(encode(output.data(), 64, GX_TF_Z8, 16, 4, pixels.data(), 16, 4, 64, HostPixelOrder::BGRA));
    for (size_t i = 0; i < 64; ++i) assert(output[i] == 0xcd);
    for (GXTexFmt format : {GX_TF_Z8, GX_CTF_Z8M, GX_CTF_Z8L}) {
        assert(encoded_size(format, 8, 4) == 32);
        assert(encode(output.data(), 32, format, 8, 4, pixels.data(), 16, 4, 64, HostPixelOrder::RGBA));
    }
    for (size_t i = 0; i < pixels.size(); i += 4) {
        pixels[i] = 0xaa;
        pixels[i + 3] = 0x77;
    }
    assert(encoded_size(GX_CTF_Z4, 8, 8) == 32);
    assert(encode(output.data(), 32, GX_CTF_Z4, 8, 8, pixels.data(), 16, 4, 64, HostPixelOrder::RGBA));
    for (size_t i = 0; i < 32; ++i) assert(output[i] == 0xaa);
    assert(encoded_size(GX_CTF_Z16L, 4, 4) == 32);
    assert(encode(output.data(), 32, GX_CTF_Z16L, 4, 4, pixels.data(), 16, 4, 64, HostPixelOrder::RGBA));
    for (size_t i = 0; i < 32; i += 2) assert(output[i] == 0x77 && output[i + 1] == 0xaa);
    std::cout << "MP9 Z8 depth readback: all checks passed\n";
}
