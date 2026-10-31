#pragma once
#include <cstdint>
namespace Memory {
struct AccessViolation {};
inline unsigned reads = 0;
inline unsigned writes = 0;
inline bool TryRead32(uint32_t, uint32_t&) { ++reads; return false; }
inline void Write32(uint32_t, uint32_t) { ++writes; }
}
