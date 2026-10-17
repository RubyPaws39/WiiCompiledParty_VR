#include "music_attenuation.h"
#include "memory.h"
#include <cassert>
#include <cstdio>
int main() {
    MusicAttenuation::SetEnabled(true);
    MusicAttenuation::SetMusicVolume(0.25f);
    MusicAttenuation::SetSoundEffectsVolume(0.5f);
    for (unsigned i = 0; i < 64; ++i) MusicAttenuation::TickGuest();
    assert(Memory::reads == 0 && Memory::writes == 0);
    puts("MP9 audio isolation: scheduler volume tick never accesses Mario Kart guest objects");
}
