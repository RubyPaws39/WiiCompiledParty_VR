#pragma once
#include <cstdint>

void Mp9ObserveRelLink(uint32_t module);
void Mp9ObserveRelUnlink(uint32_t module);
uint32_t Mp9CanonicalRelTarget(uint32_t address);
uint32_t Mp9RelAddress(uint32_t id, uint32_t section, uint32_t offset);
uint32_t Mp9RelHighImmediate(uint32_t id, uint32_t section, uint32_t instructionOffset);
uint32_t Mp9RelSignedImmediate(uint32_t id, uint32_t section, uint32_t instructionOffset);
