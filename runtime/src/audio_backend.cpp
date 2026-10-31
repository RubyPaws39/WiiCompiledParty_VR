#include "audio_backend.h"

#include "runtime_log.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <array>

#include <SDL3/SDL_init.h>
#if defined(__ANDROID__) && defined(MKW_GAME_MP9)
#include <sys/system_properties.h>
#include <SDL3/SDL_system.h>
#endif

AudioBackend& AudioBackend::Instance() {
    static AudioBackend instance;
    return instance;
}

void AudioBackend::DebugDumpPcm(bool mixed, const void* data, size_t bytes) {
#if defined(__ANDROID__) && defined(MKW_GAME_MP9)
    struct Dump { std::ofstream file; size_t bytes = 0; unsigned polls = 0; bool enabled = false; };
    static std::array<Dump,2> dumps;
    static std::mutex mutex;
    std::lock_guard lock(mutex);
    auto& dump = dumps[mixed ? 1 : 0];
    if ((dump.polls++ & 127u) == 0) {
        char value[PROP_VALUE_MAX]{};
        dump.enabled = __system_property_get("debug.wiicompiled.audio_dump", value) > 0 && value[0] == '1';
        if (!dump.enabled && dump.file.is_open()) dump.file.flush();
    }
    constexpr size_t limit = 32000 * 4 * 30;
    if (!dump.enabled || !data || !bytes || dump.bytes >= limit) return;
    if (!dump.file.is_open()) {
        const auto* internal = SDL_GetAndroidInternalStoragePath();
        if (!internal) return;
        const auto directory = std::filesystem::path(internal) / "debug-tools";
        std::error_code ec;
        std::filesystem::create_directories(directory, ec);
        if (ec) return;
        dump.file.open(directory / (mixed ? "audio-ax-le.pcm" : "audio-dma-be.pcm"), std::ios::binary | std::ios::trunc);
        if (!dump.file) return;
    }
    const size_t count = std::min(bytes, limit - dump.bytes);
    dump.file.write(static_cast<const char*>(data), count);
    dump.bytes += count;
    if (dump.bytes == limit) dump.file.flush();
#endif
}

float AudioBackend::EffectiveGainLocked() const {
    return m_muted ? 0.0f : m_masterVolume;
}

void AudioBackend::ApplyGainLocked() {
    if (m_stream && !SDL_SetAudioStreamGain(m_stream, EffectiveGainLocked())) {
        RT_LOG(RT_TAG_AUDIO) << "SDL_SetAudioStreamGain failed: " << SDL_GetError() << std::endl;
    }
}

void AudioBackend::SetMasterVolume(float volume) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_masterVolume = std::clamp(volume, 0.0f, 1.0f);
    ApplyGainLocked();
}

void AudioBackend::SetMuted(bool muted) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_muted = muted;
    ApplyGainLocked();
}

bool AudioBackend::EnsureInitializedLocked(uint32_t sampleRate, uint32_t channels) {
    if (m_initialized && m_sampleRate == sampleRate && m_channels == channels) {
        return true;
    }

    if (m_stream) {
        SDL_DestroyAudioStream(m_stream);
        m_stream = nullptr;
    }

    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        RT_LOG(RT_TAG_AUDIO) << "SDL_InitSubSystem(SDL_INIT_AUDIO) failed: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_AudioSpec spec{};
    spec.format = SDL_AUDIO_S16LE;
    spec.channels = static_cast<int>(channels);
    spec.freq = static_cast<int>(sampleRate);

    SDL_AudioStream* stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
    if (!stream) {
        RT_LOG(RT_TAG_AUDIO) << "SDL_OpenAudioDeviceStream failed: " << SDL_GetError() << std::endl;
        return false;
    }

    if (!SDL_ResumeAudioStreamDevice(stream)) {
        RT_LOG(RT_TAG_AUDIO) << "SDL_ResumeAudioStreamDevice failed: " << SDL_GetError() << std::endl;
        SDL_DestroyAudioStream(stream);
        return false;
    }

    if (!SDL_SetAudioStreamGain(stream, EffectiveGainLocked())) {
        RT_LOG(RT_TAG_AUDIO) << "SDL_SetAudioStreamGain failed: " << SDL_GetError() << std::endl;
        SDL_DestroyAudioStream(stream);
        return false;
    }

    m_stream = stream;
    m_spec = spec;
    m_sampleRate = sampleRate;
    m_channels = channels;
    m_initialized = true;
    return true;
}

bool AudioBackend::Init(uint32_t sampleRate, uint32_t channels) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return EnsureInitializedLocked(sampleRate, channels);
}

void AudioBackend::Shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_stream) {
        SDL_DestroyAudioStream(m_stream);
        m_stream = nullptr;
    }
    if (m_initialized) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }
    m_initialized = false;
    m_sampleRate = 0;
    m_channels = 0;
    m_reportedDroppedBlock = false;
    m_convertBuffer.clear();
}

uint32_t AudioBackend::QueueLimitBytesLocked() const {
    constexpr uint32_t queueMs = 120;

    const uint64_t bytesPerSecond = static_cast<uint64_t>(m_spec.freq) *
                                    static_cast<uint64_t>(m_spec.channels) *
                                    sizeof(int16_t);
    return static_cast<uint32_t>((bytesPerSecond * queueMs) / 1000u);
}

bool AudioBackend::QueueHasCapacityLocked(int incomingBytes) {
    if (!m_stream) {
        return false;
    }
    const int queued = SDL_GetAudioStreamQueued(m_stream);
    if (queued < 0) {
        return true;
    }
    const uint32_t maxQueued = QueueLimitBytesLocked();
#if defined(__ANDROID__) && defined(MKW_GAME_MP9)
    // Opt-in timing only. No sample hashing, extra mixing or playback changes.
    static const bool timingEnabled = [] {
        char value[PROP_VALUE_MAX]{};
        return __system_property_get("debug.wiicompiled.audio_timing", value) > 0 && value[0] == '1';
    }();
    if (timingEnabled) {
        using Clock = std::chrono::steady_clock;
        static auto start = Clock::now(), previous = start;
        static uint64_t bytes = 0, drops = 0, empty = 0;
        static double maxGapMs = 0;
        const auto now = Clock::now();
        maxGapMs = std::max(maxGapMs, std::chrono::duration<double, std::milli>(now - previous).count());
        previous = now;
        bytes += std::max(incomingBytes, 0);
        empty += queued == 0;
        drops += uint64_t(queued) + std::max(incomingBytes, 0) > maxQueued;
        const double elapsed = std::chrono::duration<double>(now - start).count();
        if (elapsed >= 5) {
            const double bytesPerSecond = double(m_spec.freq) * m_spec.channels * sizeof(int16_t);
            RT_LOG(RT_TAG_AUDIO) << "[mp9-audio-timing] seconds=" << elapsed
                << " input_seconds=" << bytes / bytesPerSecond << " queued_ms=" << queued * 1000 / bytesPerSecond
                << " empty_arrivals=" << empty << " dropped_blocks=" << drops << " max_gap_ms=" << maxGapMs
                << std::endl;
            start = now; bytes = drops = empty = 0; maxGapMs = 0;
        }
    }
#endif
    if (static_cast<uint64_t>(queued) + static_cast<uint64_t>(std::max(incomingBytes, 0)) > maxQueued) {
        // Match Dolphin's FIFO overflow behavior: preserve the continuous audio
        // already queued and discard the new block.  Clearing SDL's entire
        // stream creates an audible discontinuity (the severe crackle seen when
        // VI-batched DMA briefly outran playback).
        // A drop is audible; report the first one so it is not invisible.
        if (!m_reportedDroppedBlock) {
            m_reportedDroppedBlock = true;
            RT_LOG(RT_TAG_AUDIO) << "output queue full (" << queued << "/" << maxQueued
                      << " bytes); dropping blocks to preserve continuity" << std::endl;
        }
        return false;
    }
    return true;
}

bool AudioBackend::PushWiiAiSamplesBE16(const uint8_t* data, size_t bytes) {
    if (!data || bytes == 0) {
        return false;
    }
    DebugDumpPcm(false, data, bytes);

    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_initialized || !m_stream) {
        return false;
    }

    const size_t sampleCount = bytes / sizeof(int16_t);
    if (sampleCount == 0) {
        return false;
    }

    if (m_convertBuffer.size() < sampleCount) {
        m_convertBuffer.resize(sampleCount);
    }

    const size_t frameCount = sampleCount / 2;
    for (size_t frame = 0; frame < frameCount; ++frame) {
        const size_t rightOffset = frame * 4;
        const size_t leftOffset = rightOffset + 2;
        const uint16_t right = static_cast<uint16_t>(data[rightOffset]) << 8 |
                               static_cast<uint16_t>(data[rightOffset + 1]);
        const uint16_t left = static_cast<uint16_t>(data[leftOffset]) << 8 |
                              static_cast<uint16_t>(data[leftOffset + 1]);
        m_convertBuffer[frame * 2] = static_cast<int16_t>(left);
        m_convertBuffer[frame * 2 + 1] = static_cast<int16_t>(right);
    }

    const int lenBytes = static_cast<int>(frameCount * 2 * sizeof(int16_t));
    if (!QueueHasCapacityLocked(lenBytes)) {
        return true;
    }
    if (!SDL_PutAudioStreamData(m_stream, m_convertBuffer.data(), lenBytes)) {
        RT_LOG(RT_TAG_AUDIO) << "SDL_PutAudioStreamData failed: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

bool AudioBackend::PushSamplesLE16(const int16_t* samples, size_t sampleCount) {
    if (!samples || sampleCount == 0) {
        return false;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_initialized || !m_stream) {
        return false;
    }

    const int lenBytes = static_cast<int>(sampleCount * sizeof(int16_t));
    if (!QueueHasCapacityLocked(lenBytes)) {
        return true;
    }
    if (!SDL_PutAudioStreamData(m_stream, samples, lenBytes)) {
        RT_LOG(RT_TAG_AUDIO) << "SDL_PutAudioStreamData failed: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}
