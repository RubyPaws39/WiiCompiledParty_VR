// SDK entry addresses come from the hash-matched SSQE01 symbol map.
// Bodies below reuse inspected OS helpers; this is a partial boot binding set.
#define MP9_VERIFIED_BINDINGS
#include "abi_bridge.h"
#include "memory.h"
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <mutex>
#include <vector>
#include "exi_device.h"
#include "gx_state.h"
#include "audio_backend.h"
#include "../../runtime/src/hle/audio/ax_dsp.h"

namespace OsHleInternal { extern std::atomic<uint32_t> g_interrupt_mask; }

extern "C" int32_t OS__DisableInterrupts_801a65ac();
extern "C" int32_t OS__EnableInterrupts_801a65c0();
extern "C" int32_t OS__RestoreInterrupts_801a65d4(int32_t level);
extern "C" void OS__GetTime_HLE(CpuContext* ctx);
extern "C" uint32_t OS____GetSystemTime_801aad7c(uint32_t, uint32_t, uint32_t, int32_t, uint32_t, uint32_t);
extern "C" void OS__Report_801a25d0(CpuContext* ctx);
extern "C" void OSFatal_HLE_801a4ec4(CpuContext* ctx);
extern "C" void DCInvalidateRange_801a1600(uint32_t address, uint32_t length);
extern "C" void DCFlushRange_801a162c(uint32_t address, uint32_t length);
extern "C" void DCFlushRangeNoSync_801a168c(uint32_t address, uint32_t length);
extern "C" void DCZeroRange_HLE_801a16e4(CpuContext* ctx);
extern "C" uint32_t VIGetHostTvFormat();
extern "C" uint32_t SIProbe(int32_t channel);
extern "C" int32_t NAND_IOS_Open_HLE(uint32_t path, uint32_t mode);
extern "C" int32_t NAND_IOS_Close_HLE(uint32_t fd);
extern "C" int32_t NAND_IOS_Ioctlv_HLE(uint32_t fd, uint32_t command, uint32_t inputs, uint32_t outputs, uint32_t vectors);
extern "C" void IOS_OpenAsync_HLE(CpuContext* ctx);
extern "C" void IOS_IoctlAsync_80194158(CpuContext* ctx);
extern "C" int32_t NAND_IOS_Read_HLE(uint32_t fd, uint32_t buffer, uint32_t length);
extern "C" int32_t NAND_IOS_Write_HLE(uint32_t fd, uint32_t buffer, uint32_t length);
extern "C" int32_t NAND_IOS_Seek_HLE(uint32_t fd, int32_t offset, int32_t whence);
extern "C" int32_t NAND_IOS_Ioctl_HLE(uint32_t fd, uint32_t command, uint32_t input, uint32_t inputLength, uint32_t output, uint32_t outputLength);
extern "C" void IOS_CloseAsync_HLE(CpuContext* ctx);
extern "C" void IOS_ReadAsync_HLE(CpuContext* ctx);
extern "C" void IOS_WriteAsync_HLE(CpuContext* ctx);
extern "C" void IOS_SeekAsync_HLE(CpuContext* ctx);
extern "C" void IOS_IoctlvAsync_HLE(CpuContext* ctx);
extern "C" void VIInit_HLE_801b94a4(CpuContext* ctx);
extern "C" void VISetPreRetraceCallback_HLE_801b90f4(CpuContext* ctx);
extern "C" void VISetPostRetraceCallback_HLE_801b9138(CpuContext* ctx);
extern "C" void VIWaitForRetrace_HLE_801b99ec(CpuContext* ctx);
extern "C" void VIConfigure_HLE_801b9f6c(CpuContext* ctx);
extern "C" void VIFlush_HLE_801ba9a4(CpuContext* ctx);
extern "C" void VISetNextFrameBuffer_HLE_801baab8(CpuContext* ctx);
extern "C" void Mp9VIGetCurrentFrameBuffer(CpuContext* ctx);
extern "C" void VISetBlack_HLE_801bab2c(CpuContext* ctx);
extern "C" void VIGetRetraceCount_HLE_801baba4(CpuContext* ctx);
extern "C" void VIGetNextField_HLE_801babac(CpuContext* ctx);
extern "C" void VIGetCurrentLine_HLE_801bac48(CpuContext* ctx);
extern "C" void Mp9VIGetScanMode(CpuContext* ctx);
extern "C" void VIGetDTVStatus_HLE_801bad38(CpuContext* ctx);
extern "C" void GX__SetCPUFifo_8016c94c(uint32_t fifo);
extern "C" uint32_t GX__Init_8016b850(uint32_t base, uint32_t size);
extern "C" void GX__SetGPFifo_8016cb2c(uint32_t fifo);
extern "C" void __GX__PEInit_8016ee14();
extern "C" void GX__DrawDone_8016eab0();
extern "C" void Mp9GXCopyDisp(uint32_t destination, uint32_t clear);
extern "C" void GX__InitTexObjCI_80170a04(uint32_t object, uint32_t data, uint32_t width, uint32_t height, uint32_t format, uint32_t wrapS, uint32_t wrapT, uint32_t mipmap, uint32_t tlut);
extern "C" void GX__LoadTexObj_80170f2c(uint32_t object, uint32_t map);
extern "C" void GX__InitTlutObj_80170f80(uint32_t object, uint32_t data, uint32_t format, uint32_t entries);
extern "C" void GX__LoadTlut_80170fa8(uint32_t object, uint32_t tlut);
// SSQE01 GXLoadTexObjPreLoaded takes (object, region, map). The native
// texture frontend owns the host cache; the guest region is hardware-only.
extern "C" void Mp9GXLoadTexObjPreLoaded(uint32_t object, uint32_t region, uint32_t map) {
    (void)region;
    GX__LoadTexObj_80170f2c(object, map);
}
extern "C" void GX__InitTexObj_801707f8(uint32_t object, uint32_t data, uint32_t width, uint32_t height, uint32_t format, uint32_t wrapS, uint32_t wrapT, uint32_t mipmap);
extern "C" void GX__InitTexObjLOD_80170a4c(uint32_t object, uint32_t minFilter, uint32_t magFilter, float minLod, float maxLod, float bias, uint32_t biasClamp, uint32_t edgeLod, uint32_t anisotropy);
extern "C" void DVDInit_8015EA1C();
extern "C" int32_t Mp9DVDInquiryAsync(uint32_t block, uint32_t output, uint32_t callback);
extern "C" int32_t DVDReadPrio_8015E834(uint32_t file, uint32_t buffer, int32_t length, int32_t offset, int32_t priority);
extern "C" int32_t DVD__ReadAsyncPrio_HLE_8015e74c(uint32_t file, uint32_t buffer, int32_t length, int32_t offset, uint32_t callback, int32_t priority);
extern "C" uint32_t OSGetResetCode_801a8a50();
extern "C" void SelectThread_801a9c08(CpuContext* ctx);
extern "C" void OSCreateThread_HLE_801a9e84(CpuContext* ctx);
extern "C" void OSExitThread_HLE_801aa0f0(CpuContext* ctx);
extern "C" void OSCancelThread_HLE_801aa1d4(CpuContext* ctx);
extern "C" void OSJoinThread_HLE_801aa3ac(CpuContext* ctx);
extern "C" void OSResumeThread_HLE_801aa58c(CpuContext* ctx);
extern "C" void OSSuspendThread_HLE_801aa6a8(CpuContext* ctx);
extern "C" void OSSleepThread_HLE_801aa9b8(CpuContext* ctx);
extern "C" void OSWakeupThread_HLE_801aaaa4(CpuContext* ctx);
extern "C" int32_t WPADInit_HLE();
extern "C" int32_t WPADGetStatus_HLE();
extern "C" int32_t WPADProbe_HLE(uint32_t channel, uint32_t type);
extern "C" int32_t KPAD__Read_HLE(uint32_t channel, uint32_t output, uint32_t count);
extern "C" void AIInit_801240b0(uint32_t callbackStack);
extern "C" uint32_t AIRegisterDMACallback_80123f88(uint32_t callback);
extern "C" void AIInitDMA_80123fcc(uint32_t start, uint32_t length);
extern "C" void AIStartDMA_80124048();
extern "C" uint32_t AIGetDMABytesLeft_8012405c();
extern "C" uint32_t AIGetDMAStartAddr_8012406c();
extern "C" uint32_t AIGetDMALength_80124084();
extern "C" uint32_t AICheckInit_80124094();
extern "C" void DSPInit_8015d444();
extern "C" uint32_t DSPCheckInit_8015d504();
extern "C" uint32_t DSPAddTask_8015d50c(uint32_t task);
extern "C" uint32_t DSPAssertTask_8015d57c(uint32_t task);
extern "C" uint32_t DSPCheckMailToDSP_8015d3fc();
extern "C" uint32_t DSPCheckMailFromDSP_8015d40c();
extern "C" uint32_t DSPReadMailFromDSP_8015d41c();
extern "C" void DSPSendMailToDSP_8015d430(uint32_t mail);
extern "C" void __AXOutInitDSP_801269bc(CpuContext* ctx);
extern bool NandProcessPendingCallbacks(CpuContext* cpu, int maxToProcess);
void VI_HLE_ProcessRetracesDeferred(int maxToProcess);

// WUDInit is a BOOL transport initializer, unlike WPADInit's status result.
// Keep MP9's original WPAD/KPAD control-block construction and filters above it.
extern "C" int32_t Mp9WUDInit() {
    return WPADInit_HLE() == 0 ? 1 : 0;
}
extern "C" int32_t Mp9WUDGetStatus() {
    return WPADGetStatus_HLE();
}
extern "C" int32_t Mp9KPADReadEx(uint32_t channel, uint32_t output, uint32_t count, uint32_t error) {
    if (error && !Memory::Contains(error, 4)) return 0;
    int32_t result = 0, status = -4;
    if (channel < 4 && count > 0 && Memory::Contains(output, 0xF0)) {
        if (WPADGetStatus_HLE() == 3) {
            result = KPAD__Read_HLE(channel, output, count);
            status = result ? 0 : -2;
        } else status = -3;
    }
    if (error) Memory::Write32(error, static_cast<uint32_t>(status));
    return result;
}
extern "C" void Mp9WPADInitSub(CpuContext* ctx) {
    // Four SSQE01 WPAD blocks live inside the declared 0x2FA0-byte BSS pool.
    // Retain the user's original, pure-memory reset/calibration helper. The
    // host pointer replaces the physical infrared emitter, so no GPIO is used.
    static bool initialized = false;
    if (initialized) return;
    if (WPADInit_HLE() != 0) throw std::runtime_error("MP9 host input initialization failed");
    for (uint32_t i = 0; i < 16; ++i) Memory::Write8(0x802F6260 + i, 0xFF);
    for (uint32_t channel = 0; channel < 4; ++channel) {
        const uint32_t block = 0x802F6280 + channel * 0xBE0;
        Memory::Write32(0x802F5250 + channel * 4, block);
        Memory::Write32(block + 0x8E8, 0);
        CpuContext reset = *ctx;
        reset.gpr[3] = channel;
        InvokeIndirectCpu(0x801BE570, &reset);
        Memory::Write32(block + 0x928, 0);
        Memory::Write32(block + 0x92C, 0);
        Memory::Write8(block + 0xBAE, 0);
        Memory::Write8(0x8026ECC8 + channel, 0);
        Memory::Write8(0x8026EC70 + channel, 0);
        Memory::Write8(0x8026EC74 + channel, 0);
    }
    initialized = true;
}

extern "C" void Mp9OSDisableInterrupts(CpuContext* ctx) {
    ctx->gpr[3] = OS__DisableInterrupts_801a65ac();
    ctx->msr &= ~0x8000u;
}
extern "C" void Mp9OSRestoreInterrupts(CpuContext* ctx) {
    const int32_t enabled = static_cast<int32_t>(ctx->gpr[3]);
    const int32_t previous = OS__RestoreInterrupts_801a65d4(enabled);
    ctx->msr = enabled ? (ctx->msr | 0x8000u) : (ctx->msr & ~0x8000u);
    if (enabled) {
        // Runnable busy loops still receive hardware-timed VI interrupts.
        // Existing deferred dispatcher preserves registers and defers switches.
        VI_HLE_ProcessRetracesDeferred(2);
        NandProcessPendingCallbacks(ctx, 8);
    }
    ctx->gpr[3] = static_cast<uint32_t>(previous);
}
extern "C" void Mp9OSEnableInterrupts(CpuContext* ctx) {
    const int32_t previous = OS__EnableInterrupts_801a65c0();
    ctx->msr |= 0x8000u;
    VI_HLE_ProcessRetracesDeferred(2);
    NandProcessPendingCallbacks(ctx, 8);
    ctx->gpr[3] = static_cast<uint32_t>(previous);
}

// Host SDK events replace hardware interrupt delivery. Preserve the guest-visible
// table, software masks, exception callback and caller's interrupt-enable state.
// Addresses and initialization values verified in SSQE01 __OSInterruptInit.
extern "C" void Mp9OSInterruptInit(CpuContext* ctx) {
    const auto enabled = OS__DisableInterrupts_801a65ac();
    constexpr uint32_t handlers = 0x80003040;
    Memory::Write32(0x8026E718, handlers);
    for (uint32_t offset = 0; offset < 0x80; offset += 4) {
        Memory::Write32(handlers + offset, 0);
    }
    Memory::Write32(0x800000C4, 0xFFFFFFF0);
    Memory::Write32(0x800000C8, 0);
    OsHleInternal::g_interrupt_mask.store(0xFFFFFFF0);
    const uint32_t exceptionTable = Memory::Read32(0x8026E6A4);
    if (exceptionTable == 0) {
        throw std::runtime_error("MP9 interrupt initialization requires OSExceptionTable");
    }
    const uint32_t previous = Memory::Read32(exceptionTable + 4 * 4);
    Memory::Write32(exceptionTable + 4 * 4, 0x80183430);
    OS__RestoreInterrupts_801a65d4(enabled);
    ctx->gpr[3] = previous;
}

namespace {
struct ExiChannel {
    bool locked = false, selected = false;
    uint32_t owner = 0;
    std::vector<std::pair<uint32_t, uint32_t>> waiters;
};
std::array<ExiChannel, 3> exiChannels;
std::mutex exiMutex;
Mp9RtcSram& rtcSram() {
    static Mp9RtcSram device([] {
        const char* root = std::getenv("MKW_ANDROID_DATA_DIR");
        if (!root) throw std::runtime_error("MP9 EXI requires configured private user-data directory");
        return std::filesystem::path(root) / "WiiCompiledOpenXRVR" / "MP9" / "sram.bin";
    }());
    return device;
}
void exiCallback(CpuContext* ctx, uint32_t callback, uint32_t channel) {
    if (!callback) return;
    CpuContext event = *ctx;
    event.gpr[3] = channel;
    event.gpr[4] = Memory::Read32(0x800000D4);
    InvokeIndirectCpu(callback, &event);
}
void exiTransfer(CpuContext* ctx, bool dma) {
    const uint32_t channel = ctx->gpr[3], buffer = ctx->gpr[4], length = ctx->gpr[5];
    const uint32_t type = ctx->gpr[6], callback = ctx->gpr[7];
    if (channel != 0 || type > 2 || !length || (!dma && length > 4) ||
        (dma && ((buffer | length) & 31)) || length > 0x01000000 || buffer > UINT32_MAX - length) {
        ctx->gpr[3] = 0; return;
    }
    {
        std::lock_guard<std::mutex> lock(exiMutex);
        if (!exiChannels[channel].selected) { ctx->gpr[3] = 0; return; }
        auto& device = rtcSram();
        for (uint32_t i = 0; i < length; ++i) {
            const uint8_t outgoing = type == 0 ? 0 : Memory::Read8(buffer + i);
            const uint8_t incoming = device.transfer(outgoing);
            if (type != 1) Memory::Write8(buffer + i, incoming);
        }
    }
    // Native transfer completes synchronously; publish completion before callback.
    ctx->gpr[3] = 1;
    exiCallback(ctx, callback, channel);
}
}
extern "C" void Mp9EXIInit() {
    std::lock_guard<std::mutex> lock(exiMutex);
    rtcSram(); // Validate/load persistent data before reporting initialization.
    exiChannels = {};
    for (uint32_t offset = 0; offset < 0xC0; offset += 4) Memory::Write32(0x802CD8B0 + offset, 0);
    const uint32_t table = Memory::Read32(0x8026E718);
    if (!table) throw std::runtime_error("MP9 EXI requires initialized interrupt handlers");
    for (uint32_t irq = 9; irq <= 16; ++irq) {
        const uint32_t handler = (irq - 9) % 3 == 0 ? 0x8018B0E0 :
                                 (irq - 9) % 3 == 1 ? 0x8018B1A0 : 0x8018B3C0;
        Memory::Write32(table + irq * 4, handler);
    }
    Memory::Write32(0x8026E7C8, 0); // No physical serial-port adapter.
    Memory::Write32(0x800030C0, 0);
    Memory::Write32(0x800030C4, 0);
}
extern "C" uint32_t Mp9EXISelect(uint32_t channel, uint32_t device, uint32_t frequency) {
    if (channel != 0 || device != 1 || frequency > 5) return 0;
    std::lock_guard<std::mutex> lock(exiMutex);
    auto& state = exiChannels[channel];
    if (state.selected || !state.locked || state.owner != device) return 0;
    const auto now = std::chrono::system_clock::now().time_since_epoch();
    rtcSram().select(static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(now).count() - 946684800));
    state.selected = true;
    return 1;
}
extern "C" uint32_t Mp9EXIDeselect(uint32_t channel) {
    if (channel != 0) return 0;
    std::lock_guard<std::mutex> lock(exiMutex);
    if (!exiChannels[channel].selected) return 0;
    rtcSram().deselect();
    exiChannels[channel].selected = false;
    return 1;
}
extern "C" uint32_t Mp9EXILock(uint32_t channel, uint32_t device, uint32_t callback) {
    if (channel >= 3 || device >= 3) return 0;
    std::lock_guard<std::mutex> lock(exiMutex);
    auto& state = exiChannels[channel];
    if (state.locked) {
        bool pending = false;
        for (const auto& waiter : state.waiters) pending |= waiter.first == device;
        if (callback && !pending) state.waiters.emplace_back(device, callback);
        return 0;
    }
    state.locked = true; state.owner = device;
    return 1;
}
extern "C" void Mp9EXIUnlock(CpuContext* ctx) {
    const uint32_t channel = ctx->gpr[3];
    std::vector<std::pair<uint32_t, uint32_t>> callbacks;
    if (channel >= 3) { ctx->gpr[3] = 0; return; }
    {
        std::lock_guard<std::mutex> lock(exiMutex);
        auto& state = exiChannels[channel];
        if (!state.locked || state.selected) { ctx->gpr[3] = 0; return; }
        state.locked = false; callbacks.swap(state.waiters);
    }
    ctx->gpr[3] = 1;
    for (const auto& waiter : callbacks) exiCallback(ctx, waiter.second, channel);
}
extern "C" void Mp9EXIImm(CpuContext* ctx) { exiTransfer(ctx, false); }
extern "C" void Mp9EXIDma(CpuContext* ctx) { exiTransfer(ctx, true); }
extern "C" uint32_t Mp9EXISync(uint32_t channel) {
    std::lock_guard<std::mutex> lock(exiMutex);
    return channel < 3 && exiChannels[channel].selected;
}
extern "C" uint32_t Mp9EXIGetID(uint32_t channel, uint32_t device, uint32_t buffer) {
    if (channel >= 3 || device >= 3 || !buffer) return 0;
    if (channel == 0 && device == 1) throw std::runtime_error("MP9 IPL ID probe needs a supported ROM provider");
    Memory::Write32(buffer, 0); // Device absent; failure status is returned.
    return 0;
}

// Legacy wired-controller service uses Aurora's actual host controller discovery.
// Wii Remote input is a separate WPAD service, not provided by SI.
extern "C" uint32_t Mp9SIGetType(int32_t channel) {
    if (channel < 0 || channel >= 4) return 8; // SI_ERROR_NO_RESPONSE
    const uint32_t type = SIProbe(channel);
    Memory::Write32(0x8025B6E0 + channel * 4, type);
    return type;
}
extern "C" void Mp9SISetSamplingRate(uint32_t rate) {
    // Host controller snapshots are polled by Aurora; retain SDK's requested
    // cadence in its guest global instead of touching Broadway's poll register.
    Memory::Write32(0x8026E7F0, rate > 11 ? 11 : rate);
}
extern "C" void Mp9SIInit() {
    for (uint32_t offset = 0; offset < 0x80; offset += 4) Memory::Write32(0x802CD970 + offset, 0);
    for (uint32_t channel = 0; channel < 4; ++channel) {
        Memory::Write32(0x802CD970 + channel * 0x20, 0xFFFFFFFF);
        Mp9SIGetType(channel);
    }
    Memory::Write32(0x8025B6C8, 0xFFFFFFFF); // No pending SI transfer.
    Memory::Write32(0x8025B6CC, 0);
    Mp9SISetSamplingRate(0);
    const uint32_t table = Memory::Read32(0x8026E718);
    if (!table) throw std::runtime_error("MP9 SI requires initialized interrupt handlers");
    Memory::Write32(table + 20 * 4, 0x8018C3F0);
    const uint32_t mask = Memory::Read32(0x800000C4) & ~0x800u;
    Memory::Write32(0x800000C4, mask);
    OsHleInternal::g_interrupt_mask.store(mask);
}
extern "C" void Mp9OSInitAudioSystem() {
    // Initialize native output and ARAM views. DSP task globals belong to the
    // game's audio module; the existing Mario Kart addresses must not be seeded.
    AxDspHle::InitAram();
    if (!AudioBackend::Instance().Init(32000, 2)) {
        throw std::runtime_error("MP9 native 32 kHz stereo output initialization failed");
    }
    RT_LOGF(RT_TAG_AUDIO, "MP9 native audio output initialized: 32000 Hz stereo; AX module bindings pending\n");
}
namespace {
uint32_t changeInterruptMask(uint32_t mask, bool unmask) {
    const int32_t enabled = OS__DisableInterrupts_801a65ac();
    const uint32_t previous = Memory::Read32(0x800000C4);
    const uint32_t updated = unmask ? previous & ~mask : previous | mask;
    Memory::Write32(0x800000C4, updated);
    OsHleInternal::g_interrupt_mask.store(updated);
    OS__RestoreInterrupts_801a65d4(enabled);
    return previous;
}
}
extern "C" uint32_t Mp9OSMaskInterrupts(uint32_t mask) { return changeInterruptMask(mask, false); }
extern "C" uint32_t Mp9OSUnmaskInterrupts(uint32_t mask) { return changeInterruptMask(mask, true); }
extern "C" void Mp9OSInitMemoryProtection(CpuContext* ctx) {
    if (Memory::Read32(0x8026E720)) return;
    if (!Memory::Contains(Memory::kMem1CachedBase, Memory::kMem1Size) ||
        !Memory::Contains(Memory::kMem2CachedBase, Memory::kMem2Size)) {
        throw std::runtime_error("MP9 native RAM mappings unavailable");
    }
    const int32_t enabled = OS__DisableInterrupts_801a65ac();
    const uint32_t table = Memory::Read32(0x8026E718);
    if (!table) throw std::runtime_error("MP9 memory protection requires interrupt table");
    // Native checked/flat memory supplies bounds and executable-write guards.
    // Keep those guards; the Wii MI's initial state permits all data accesses.
    Memory::RefreshWritableFastPathsForExecutableRanges();
    Mp9OSMaskInterrupts(0xF0000000);
    for (uint32_t irq = 0; irq <= 4; ++irq) Memory::Write32(table + irq * 4, 0x80184220);
    ctx->gpr[3] = 0x8024F220; // Original MP9 shutdown-info node, not MKW metadata.
    InvokeDirectCpu<0x80184D00>(ctx);
    Mp9OSUnmaskInterrupts(0x08000000);
    Memory::Write32(0x8026E720, 1);
    OS__RestoreInterrupts_801a65d4(enabled);
}
extern "C" void Mp9IPCCltInit(CpuContext* ctx) {
    if (Memory::Read32(0x8026EC3C)) { ctx->gpr[3] = 0; return; }
    InvokeDirectCpu<0x801B8950>(ctx); // Original buffer setup; pure guest-memory operations.
    const uint32_t lo = Memory::Read32(0x8026EC1C);
    const uint32_t hi = Memory::Read32(0x8026EC20);
    if (lo > UINT32_MAX - 0x2000 || lo + 0x2000 > hi || !Memory::Contains(lo, 0x2000)) {
        ctx->gpr[3] = static_cast<uint32_t>(-22); return;
    }
    ctx->gpr[3] = lo; ctx->gpr[4] = 0x2000;
    InvokeDirectCpu<0x801BA4A0>(ctx); // Original IOS heap constructor.
    if (static_cast<int32_t>(ctx->gpr[3]) < 0) return;
    Memory::Write32(0x8026E0CC, ctx->gpr[3]);
    Memory::Write32(0x8026EC1C, lo + 0x2000);
    const uint32_t table = Memory::Read32(0x8026E718);
    if (!table) throw std::runtime_error("MP9 IPC requires initialized interrupt handlers");
    Memory::Write32(table + 27 * 4, 0x801B8CD0);
    Mp9OSUnmaskInterrupts(0x10);
    InvokeDirectCpu<0x801BA9D0>(ctx); // Original request/profiling state.
    Memory::Write32(0x802F16B0, 0);
    Memory::Write32(0x802F16B4, 0);
    Memory::Write32(0x8026EC3C, 1);
    ctx->gpr[3] = 0;
}

REGISTER_NATIVE_FUNCTION(0x80182CB0, Mp9OSDisableInterrupts);
REGISTER_NATIVE_FUNCTION(0x80182CD0, Mp9OSEnableInterrupts);
REGISTER_NATIVE_FUNCTION(0x80182CF0, Mp9OSRestoreInterrupts);
REGISTER_NATIVE_FUNCTION(0x801877C0, OS__GetTime_HLE);
REGISTER_NATIVE_FUNCTION(0x801877F0, OS____GetSystemTime_801aad7c);
REGISTER_NATIVE_FUNCTION(0x8017F500, OS__Report_801a25d0);
REGISTER_NATIVE_FUNCTION(0x801817E0, OSFatal_HLE_801a4ec4);
REGISTER_NATIVE_FUNCTION(0x8017E5F0, DCInvalidateRange_801a1600);
REGISTER_NATIVE_FUNCTION(0x8017E620, DCFlushRange_801a162c);
REGISTER_NATIVE_FUNCTION(0x8017E680, DCFlushRangeNoSync_801a168c);
REGISTER_NATIVE_FUNCTION(0x8017E6E0, DCZeroRange_HLE_801a16e4);
REGISTER_NATIVE_FUNCTION(0x80182D50, Mp9OSInterruptInit);
REGISTER_NATIVE_FUNCTION(0x8018B490, Mp9EXIInit);
REGISTER_NATIVE_FUNCTION(0x8018AEB0, Mp9EXISelect);
REGISTER_NATIVE_FUNCTION(0x8018AFE0, Mp9EXIDeselect);
REGISTER_NATIVE_FUNCTION(0x8018B660, Mp9EXILock);
REGISTER_NATIVE_FUNCTION(0x8018B760, Mp9EXIUnlock);
REGISTER_NATIVE_FUNCTION(0x8018A410, Mp9EXIImm);
REGISTER_NATIVE_FUNCTION(0x8018A740, Mp9EXIDma);
REGISTER_NATIVE_FUNCTION(0x8018A840, Mp9EXISync);
REGISTER_NATIVE_FUNCTION(0x8018B870, Mp9EXIGetID);
REGISTER_NATIVE_FUNCTION(0x8018F800, VIGetHostTvFormat);
REGISTER_NATIVE_FUNCTION(0x8018C7E0, Mp9SIInit);
REGISTER_NATIVE_FUNCTION(0x8018D130, Mp9SISetSamplingRate);
REGISTER_NATIVE_FUNCTION(0x8018CF70, Mp9SIGetType);
REGISTER_NATIVE_FUNCTION(0x8017E330, Mp9OSInitAudioSystem);
REGISTER_NATIVE_FUNCTION(0x80183080, Mp9OSMaskInterrupts);
REGISTER_NATIVE_FUNCTION(0x80183100, Mp9OSUnmaskInterrupts);
REGISTER_NATIVE_FUNCTION(0x80184880, Mp9OSInitMemoryProtection);
REGISTER_NATIVE_FUNCTION(0x801B8E90, Mp9IPCCltInit);
REGISTER_NATIVE_FUNCTION(0x801B9360, NAND_IOS_Open_HLE);
REGISTER_NATIVE_FUNCTION(0x801B9240, IOS_OpenAsync_HLE);
REGISTER_NATIVE_FUNCTION(0x801B9BF0, IOS_IoctlAsync_80194158);
REGISTER_NATIVE_FUNCTION(0x801BA090, NAND_IOS_Ioctlv_HLE);
REGISTER_NATIVE_FUNCTION(0x801B9550, NAND_IOS_Close_HLE);

// Standard IPC wrappers verified by packet command, arguments and callback fields.
REGISTER_NATIVE_FUNCTION(0x801B9490, IOS_CloseAsync_HLE);
REGISTER_NATIVE_FUNCTION(0x801B9600, IOS_ReadAsync_HLE);
REGISTER_NATIVE_FUNCTION(0x801B9700, NAND_IOS_Read_HLE);
REGISTER_NATIVE_FUNCTION(0x801B9810, IOS_WriteAsync_HLE);
REGISTER_NATIVE_FUNCTION(0x801B9910, NAND_IOS_Write_HLE);
REGISTER_NATIVE_FUNCTION(0x801B9A20, IOS_SeekAsync_HLE);
REGISTER_NATIVE_FUNCTION(0x801B9B00, NAND_IOS_Seek_HLE);
REGISTER_NATIVE_FUNCTION(0x801B9D30, NAND_IOS_Ioctl_HLE);
REGISTER_NATIVE_FUNCTION(0x801B9FA0, IOS_IoctlvAsync_HLE);

// Native VI backend: staged configuration, frame buffers and timed retraces.
REGISTER_NATIVE_FUNCTION(0x8018DF90, VIInit_HLE_801b94a4);
REGISTER_NATIVE_FUNCTION(0x8018DBD0, VISetPreRetraceCallback_HLE_801b90f4);
REGISTER_NATIVE_FUNCTION(0x8018DC20, VISetPostRetraceCallback_HLE_801b9138);
REGISTER_NATIVE_FUNCTION(0x8018E4E0, VIWaitForRetrace_HLE_801b99ec);
REGISTER_NATIVE_FUNCTION(0x8018EA70, VIConfigure_HLE_801b9f6c);
REGISTER_NATIVE_FUNCTION(0x8018F4A0, VIFlush_HLE_801ba9a4);
REGISTER_NATIVE_FUNCTION(0x8018F5C0, VISetNextFrameBuffer_HLE_801baab8);
REGISTER_NATIVE_FUNCTION(0x8018F630, Mp9VIGetCurrentFrameBuffer);
REGISTER_NATIVE_FUNCTION(0x8018F640, VISetBlack_HLE_801bab2c);
REGISTER_NATIVE_FUNCTION(0x8018F6C0, VIGetRetraceCount_HLE_801baba4);
REGISTER_NATIVE_FUNCTION(0x8018F6D0, VIGetNextField_HLE_801babac);
REGISTER_NATIVE_FUNCTION(0x8018F770, VIGetCurrentLine_HLE_801bac48);
REGISTER_NATIVE_FUNCTION(0x8018F860, Mp9VIGetScanMode);
REGISTER_NATIVE_FUNCTION(0x8018F8C0, VIGetDTVStatus_HLE_801bad38);

// Aurora owns native graphics initialization; original FIFO descriptors remain.
REGISTER_NATIVE_FUNCTION(0x80194830, GX__SetCPUFifo_8016c94c);
REGISTER_NATIVE_FUNCTION(0x80194A10, GX__SetGPFifo_8016cb2c);
REGISTER_NATIVE_FUNCTION(0x80196980, __GX__PEInit_8016ee14);
REGISTER_NATIVE_FUNCTION(0x80196560, GX__DrawDone_8016eab0);
REGISTER_NATIVE_FUNCTION(0x80197830, Mp9GXCopyDisp);

extern "C" void Mp9GXInit(CpuContext* ctx) {
    const uint32_t fifo = GX__Init_8016b850(ctx->gpr[3], ctx->gpr[4]);
    constexpr uint32_t state = 0x802CDDA0;
    Mp9InitGxRegisterIds(static_cast<uint8_t*>(Memory::GetPointer(state, 0x600)), 0x600);
    // Original pure-memory region constructors read the user's private address table.
    for (uint32_t bank = 0; bank < 3; ++bank) {
        for (uint32_t i = 0; i < 8; ++i) {
            ctx->gpr[3] = state + 0x258 + bank * 0x80 + i * 0x10;
            ctx->gpr[4] = bank == 2;
            ctx->gpr[5] = Memory::Read32(0x8025C470 + bank * 0x40 + i * 4);
            ctx->gpr[6] = 0;
            ctx->gpr[7] = Memory::Read32(0x8025C490 + bank * 0x40 + i * 4);
            ctx->gpr[8] = 0;
            InvokeDirectCpu<0x80198D90>(ctx);
        }
    }
    for (uint32_t i = 0; i < 20; ++i) {
        ctx->gpr[3] = state + 0x3D8 + i * 0x10;
        ctx->gpr[4] = i < 16 ? 0xC0000 + i * 0x2000 : 0xE0000 + (i - 16) * 0x8000;
        ctx->gpr[5] = i < 16 ? 16 : 64;
        InvokeDirectCpu<0x80198E40>(ctx);
    }
    // Keep the game's platform-independent revision and default GX setup.
    InvokeDirectCpu<0x80193640>(ctx);
    InvokeDirectCpu<0x80193C80>(ctx);
    ctx->gpr[3] = fifo;
}
REGISTER_NATIVE_FUNCTION(0x80193770, Mp9GXInit);
REGISTER_NATIVE_FUNCTION(0x80198480, GX__InitTexObj_801707f8);
REGISTER_NATIVE_FUNCTION(0x80198690, GX__InitTexObjCI_80170a04);
REGISTER_NATIVE_FUNCTION(0x80198AF0, Mp9GXLoadTexObjPreLoaded);
REGISTER_NATIVE_FUNCTION(0x80198C60, GX__LoadTexObj_80170f2c);
REGISTER_NATIVE_FUNCTION(0x80198CC0, GX__InitTlutObj_80170f80);
REGISTER_NATIVE_FUNCTION(0x80198CF0, GX__LoadTlut_80170fa8);
REGISTER_NATIVE_FUNCTION(0x801986E0, GX__InitTexObjLOD_80170a4c);
REGISTER_NATIVE_FUNCTION(0x8019DA20, DVDInit_8015EA1C);
REGISTER_NATIVE_FUNCTION(0x801A1AA0, Mp9DVDInquiryAsync);
REGISTER_NATIVE_FUNCTION(0x8019D810, DVDReadPrio_8015E834);
REGISTER_NATIVE_FUNCTION(0x8019D720, DVD__ReadAsyncPrio_HLE_8015e74c);

extern "C" uint32_t Mp9OSGetResetCode() {
    if (Memory::Read32(0x802CC6C0)) return Memory::Read32(0x802CC6C4) | 0x80000000u;
    return OSGetResetCode_801a8a50();
}
REGISTER_NATIVE_FUNCTION(0x801855E0, Mp9OSGetResetCode);

// Reuse fiber-backed SDK threading; guest context resumes keep their native stacks.
REGISTER_NATIVE_FUNCTION(0x80186750, SelectThread_801a9c08);
REGISTER_NATIVE_FUNCTION(0x801869E0, OSCreateThread_HLE_801a9e84);
REGISTER_NATIVE_FUNCTION(0x80186C50, OSExitThread_HLE_801aa0f0);
REGISTER_NATIVE_FUNCTION(0x80186D40, OSCancelThread_HLE_801aa1d4);
REGISTER_NATIVE_FUNCTION(0x80186F20, OSJoinThread_HLE_801aa3ac);
REGISTER_NATIVE_FUNCTION(0x80187070, OSResumeThread_HLE_801aa58c);
REGISTER_NATIVE_FUNCTION(0x80187310, OSSuspendThread_HLE_801aa6a8);
REGISTER_NATIVE_FUNCTION(0x801874A0, OSSleepThread_HLE_801aa9b8);
REGISTER_NATIVE_FUNCTION(0x80187590, OSWakeupThread_HLE_801aaaa4);
REGISTER_NATIVE_FUNCTION(0x801D0CC0, Mp9WUDInit);
REGISTER_NATIVE_FUNCTION(0x801D1030, Mp9WUDGetStatus);
REGISTER_NATIVE_FUNCTION(0x801BE8B0, Mp9WPADInitSub);
REGISTER_NATIVE_FUNCTION(0x801D6F80, Mp9KPADReadEx);
REGISTER_NATIVE_FUNCTION(0x801C0370, WPADProbe_HLE);
REGISTER_NATIVE_FUNCTION(0x801A5FC0, AIInit_801240b0);
REGISTER_NATIVE_FUNCTION(0x801A5E80, AIRegisterDMACallback_80123f88);
REGISTER_NATIVE_FUNCTION(0x801A5ED0, AIInitDMA_80123fcc);
REGISTER_NATIVE_FUNCTION(0x801A5F50, AIStartDMA_80124048);
REGISTER_NATIVE_FUNCTION(0x801A5F70, AIGetDMABytesLeft_8012405c);
REGISTER_NATIVE_FUNCTION(0x801A5F80, AIGetDMAStartAddr_8012406c);
REGISTER_NATIVE_FUNCTION(0x801A5FA0, AIGetDMALength_80124084);
REGISTER_NATIVE_FUNCTION(0x801A5FB0, AICheckInit_80124094);
REGISTER_NATIVE_FUNCTION(0x801B1810, DSPInit_8015d444);
REGISTER_NATIVE_FUNCTION(0x801B18D0, DSPCheckInit_8015d504);
REGISTER_NATIVE_FUNCTION(0x801B18E0, DSPAddTask_8015d50c);
REGISTER_NATIVE_FUNCTION(0x801B1950, DSPAssertTask_8015d57c);
REGISTER_NATIVE_FUNCTION(0x801B17B0, DSPCheckMailToDSP_8015d3fc);
REGISTER_NATIVE_FUNCTION(0x801B17C0, DSPCheckMailFromDSP_8015d40c);
REGISTER_NATIVE_FUNCTION(0x801B17D0, DSPReadMailFromDSP_8015d41c);
REGISTER_NATIVE_FUNCTION(0x801B17F0, DSPSendMailToDSP_8015d430);
REGISTER_NATIVE_FUNCTION(0x801A80C0, __AXOutInitDSP_801269bc);
