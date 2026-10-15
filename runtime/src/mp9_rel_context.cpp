#if defined(MKW_GAME_MP9)
#include "mp9_rel_context.h"
#include "memory.h"
#include <array>
#include <stdexcept>
#include <vector>

namespace {
struct Section { uint32_t fileOffset, size; bool code; };
struct Module { uint32_t header = 0, table = 0; std::vector<Section> sections; };
// Guest linking and execution share the guest scheduler's host thread.
thread_local std::array<Module, 116> modules;
}

void Mp9ObserveRelLink(uint32_t header) {
    const uint32_t id = Memory::Read32(header);
    const uint32_t count = Memory::Read32(header + 0xC);
    const uint32_t offset = Memory::Read32(header + 0x10);
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
    if (id < modules.size() && modules[id].header == header) modules[id] = {};
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
