#pragma once
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>

// Native RTC/SRAM protocol. Register layout verified against Dolphin EXI_DeviceIPL
// and the user's SDK call sequence; no Nintendo ROM or executable bytes here.
class Mp9RtcSram {
    std::array<uint8_t, 0x44> bytes{};
    std::filesystem::path path;
    uint32_t command = 0, commandBytes = 0, cursor = 0;
    uint8_t rtcFlags = 0;
    bool dirty = false;
public:
    explicit Mp9RtcSram(std::filesystem::path storage) : path(std::move(storage)) {
        if (std::filesystem::exists(path)) {
            std::ifstream file(path, std::ios::binary);
            file.read(reinterpret_cast<char*>(bytes.data()), bytes.size());
            if (!file || file.peek() != EOF) throw std::runtime_error("MP9 SRAM file must contain 68 bytes");
        } else {
            bytes[0x17] = 0x2C; // Stereo, configured, default English.
            uint16_t sum = 0, inverse = 0;
            for (size_t i = 0x10; i < 0x18; i += 2) {
                const uint16_t word = (bytes[i] << 8) | bytes[i + 1];
                sum += word;
                inverse += static_cast<uint16_t>(~word);
            }
            bytes[4] = sum >> 8; bytes[5] = sum;
            bytes[6] = inverse >> 8; bytes[7] = inverse;
        }
    }
    void select(uint32_t secondsSince2000) {
        command = commandBytes = cursor = 0;
        for (unsigned i = 0; i < 4; ++i) bytes[i] = secondsSince2000 >> (24 - i * 8);
    }
    uint8_t transfer(uint8_t outgoing) {
        if (commandBytes < 4) {
            command = (command << 8) | outgoing;
            ++commandBytes;
            return 0xFF;
        }
        const uint32_t address = (command >> 6) & 0x1FFFFFF;
        const bool write = (command & 0x80000000) != 0;
        if (address >= 0x800000 && address < 0x800044) {
            const uint32_t index = (address - 0x800000 + cursor++) % bytes.size();
            if (write) { bytes[index] = outgoing; dirty = true; }
            return bytes[index];
        }
        if (address == 0x840020) {
            if (write) rtcFlags = outgoing;
            return rtcFlags;
        }
        throw std::runtime_error("MP9 EXI RTC/SRAM command unsupported: " + std::to_string(command));
    }
    void deselect() {
        if (!dirty) return;
        std::filesystem::create_directories(path.parent_path());
        const auto staging = std::filesystem::path(path.string() + ".tmp");
        {
            std::ofstream file(staging, std::ios::binary | std::ios::trunc);
            file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
            file.close();
            if (!file) throw std::runtime_error("MP9 SRAM persistence failed");
        }
        std::filesystem::rename(staging, path);
        dirty = false;
    }
};
