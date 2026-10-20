# MP9 audio continuation — 2026-10-19

## User checkpoint request

User requested stopping soon, updating GitHub and AGENTS.md, and handing the task to OpenCode. Audio remains broken: repeated music fragment and missing dice/event SFX. Do not claim repaired audio.

## Verified state

- Source checkpoint before these diagnostic additions: `0575aa3`, branch `codex/mp9-build-metadata-handoff`, remote `myfork` (`RubyPaws39/WiiCompiledParty_VR`), existing draft PR #1.
- All 115 REL modules compiled. Previous complete flat-screen loop: Toad Road -> Skyjinks -> results -> next Mario turn.
- Latest full depth-copy family build passed former Z8/Z8M faults and reached board intro/order dice. Earlier notes saying runtime retest pending are superseded.
- Actual perspective stereo replay and OpenXR projection submission verified in logs. Physical headset depth/head-tracking confirmation still pending. Multi-view minigames retain the original picture; do not claim complete VR coverage.
- Batching improved the same opening from about 16 FPS/60 ms GPU to 48–51 FPS/about 7 ms GPU, with some later sections near 60 FPS. Shader stalls and remaining visual artifacts still exist.
- MP9-specific existing VR panel now controls stereo, scale, recenter, screen geometry, passthrough, resolution, pointer hand/gain and motion gain. ARM64 checks passed; preserve these changes.

## Audio evidence and current experiment

1. User explicitly confirmed repeating audio with worker mixing enabled AND after setting `[audio] mix_worker=false`. Threading toggle alone is not the solution.
2. Device remains configured `mix_worker=false` for diagnosis. Original config backup: `R:/MarioParty9VR/work/quest-config-before-audio-test.toml`; test file: `quest-config-audio-sync-test.toml`. Restore original if appropriate after investigation; do not publish the private config.
3. Saved synchronous-run console: `R:/MarioParty9VR/work/oct19-audio-sync-full.log`, PID 27093. Output queue overflow occurs, but this alone does not establish why PCM repeats.
4. Actual MP9 DSP task and owned DOL IRAM bytes established ucode CRC `0x4CC52064`, `PBLayout::SkipUpdatesOnly`, new command layout/new filter. Do not force a guessed PB layout.
5. New temporary bounded MP9-only diagnostics compiled and packaged successfully:
   - `[mp9-audio-dma]` every 1024 DMA blocks: hash changes, DMA address changes, length.
   - `[mp9-audio-mix]` every 1024 AX outputs: PCM hash changes and destination.
   - `[mp9-audio-pb]` every 8192 PB visits: cursor progress, actual CRC/layout.
   These are probes, not fixes; remove before delivery. No strict checks disabled.

## Next investigation

Installed diagnostic run: `base_1792457762_pid28522`, PID 28522. Automated replay again verified title/main/Solo/characters/setup and sent Start; input override disabled afterward. Full private console saved as `R:/MarioParty9VR/work/oct19-audio-cursors-full.log`.

Initial title silence correctly showed unchanged hashes/zero cursor advancement. After menu navigation, DMA blocks 9216–19456 showed 1024/1024 hash changes AND address changes per window; AX output similarly showed 1024/1024 changes. At output/block 19456 both counts matched, DMA size 384 bytes; approximately 3.07 seconds per 1024 blocks. PB aggregate showed about 170–172 cursor advances per 8192 visits with actual CRC `4cc52064`, layout 3. These facts rule out a globally frozen mixer or a single DMA address stuck throughout this captured interval. They do NOT rule out a longer repeating streaming buffer, periodic source resets, missing game event voice updates, or a later scene-specific fault. Inactive PB examples have stale cursors; do not interpret one inactive example as the active voice.

Next bounded measurement should identify ACTIVE streaming/music voices and compare their source cursor/loop bounds across the audible repetition interval, plus guest refill/voice update activity. Current consecutive-hash counters cannot detect repetition across an entire multi-block ring. No speculative fix has been applied.

Compare DMA output count/hash changes to AX output count/hash changes and PB cursor progression while audible repetition occurs. Changing mixer PCM with unchanging DMA suggests callback/buffer selection; advancing DMA with static PB cursors suggests voice/source state; repeated PB resets need tracing guest producer/update flow. These are discriminators, not established causes.

Inspect `runtime/src/hle/audio/audio.cpp` (`Audio_HLE_Tick`, polling), `ax_mix.cpp` (`ProcessPBList`, `OutputSamples`), and guest SoundThread/AI callbacks. Deferred audio polls during GX waits suppress rescheduling until native ownership unwinds. Verify timing before changing it. Do not mute, inflate buffers, force game flags or skip game logic as a fix.

## Local commands and paths

- ADB `R:/SonicRidersQuest/sdk/platform-tools/adb.exe`; serial `2G97C5ZJ0P00GN`; package `org.marioparty9.quest`, process `:game`.
- Native build from fork: `R:/SonicRidersQuest/sdk/cmake/3.31.4/bin/ninja.exe -C android/app/.cxx/Debug/391a6r4o/arm64-v8a -j8 WiiCompiled`. Gradle alone does not rebuild the game library.
- Deploy: `R:/MarioParty9VR/work/Deploy-PrivateQuestGame.ps1 -LogName quest-oct19-audio-cursors`. No `*>` redirection. Normal importer verifies private game package; do not bypass it.
- Python: `C:/Users/derpy/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe`.
- `R:/MarioParty9VR/work/Replay-Mp9Solo.py`: automated title -> Solo -> Mario -> Toad Road -> Start, with image checks. `Drive-Mp9DebugInput.py --disable` clears overrides. Minus skips optional opening; use fresh capture before further input.
- Runtime logs: `/sdcard/Android/data/org.marioparty9.quest/files/WiiCompiledOpenXRVR/Logs/base_*/console.log`. Pull full console before restarting.
- `debug.wiicompiled.fpslog` reset to 0 after capture; input override explicitly disabled. No debugger or hardware-security bypass in use.

Public user-ROM-to-native generation, online profiles/lobbies, full motion validation and all-mode VR are still unfinished. Do not publish ROM/assets/translated C++/native game library/private `.wcgame`/captures or private runtime logs.
