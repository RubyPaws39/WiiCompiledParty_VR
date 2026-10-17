# Muse Spark continuation prompt — 2026-10-16

Continue the native Mario Party 9 Quest 3 port using Wiicompiled_VR AOT + Aurora. Read current status, this handoff, README and local MODLOG before editing. Preserve existing commits and unrelated work. Keep strict guest/MMIO checks enabled. Never fabricate pointers, skip original game logic, use an interpreter, or call the current prototype playable.

Priority: get from the working menus into actual board/minigame gameplay, then validate physical Quest controls. Current world-fixed screen is not immersive gameplay. Online/profile/stat systems and public on-device native generation remain unfinished. Public source/APK must contain no ROM, game assets, saves, translated Nintendo code or private game libraries.

## Established fixes — preserve these

1. runtime/src/ppc_helpers.cpp implements MP9 locked-cache DMA via SPR922/923. The original game computes view/normal matrices in LC; the old runtime only shadowed DMA registers. Missing stores left texture bytes in matrix palettes. New implementation checks entire RAM/LC ranges, handles split line count (zero means128), trigger/flush/direction and16KiB wrap, then notifies GX writes. Production ARM64 regression in projects/mp9/Test-Mp9LcSprDma.cpp fails before and passes after. Original welcome/title geometry now renders correctly. Hardware reference: IBM750CL manual sections2.1.2.10 and9.3, https://fail0verflow.com/media/files/ppc_750cl.pdf . No Dolphin implementation was copied.
2. runtime/src/hle/gx/gx_texture.cpp mirrors guest image0/mode0 on texture load, at GXData +0x564+map*4 / +0x584+map*4. Original SSQE01 instructions80198C1C/80198C20 establish those stores. Original __GXSetSUTexRegs needs them. Before repair, second coordinate scale was1x1 despite a128x128 I4 portrait mask. After repair, Solo preview, all ten portraits and menu background render. Do not revisit null pointers, decompression or missing assets as the same issue.
3. runtime/src/hle/vi.cpp publishes VIGetRetraceCount result after callback polling. Callbacks were observed overwriting r3. This alone did not fix Solo progression.
4. Developer controller in runtime/src/wii_remote_input.cpp is opt-in through app-private debug-tools/wii-input.txt. Holds expire within2s; override expires1s later even with stale file. No file means normal physical controls. Replay finally deletes command file. Earlier indefinite override caused reported A+B failure. Physical mapping: right A + right trigger = Wii A+B.

## Current frontier

Main menu -> Solo information -> character selection -> choose Mario -> OK -> Toad Road setup succeeds. Start removes setup UI and leaves animated purple star background. Guest continues drawing; no successful board gameplay is claimed. Latest pre-cleanup module logs linked1,115,17,16; did not show module46 linking after Start. mg9409 is already compiled and installed; do not add random RELs without a missing-target/module observation.

Before investigating this transition, ensure installed APK and private library match newest build and reproduce using ordinary KPAD input. Trace original menu transition, async loading/completion and task state with bounded diagnostics. Inspect actual wait condition before patching. Do not infer a hang from one idle native fiber stack. Do not bypass callbacks or game tasks to force a scene.

Audio queue repeatedly filled15004..15260 of15360 bytes and dropped samples. gx_thread=true has been restored after an ineffective synchronous-GX experiment. No audible fix has been verified. Matrix-upload diagnostics were removed in cleanup to eliminate unnecessary reads; no audio improvement is claimed from that alone.

## Build and test

- Gradle builds mkw_quest_kit_probe; explicitly build WiiCompiled too. Local Build-Mp9Apk.ps1 does both and audits APK contents.
- Header changes require touching all private generated build_shards/**/*.cpp; custom shard rules do not scan headers. Rebuild PCH only if stale-PCH failure occurs.
- Fresh full-shard rebuild exhausted memory when Gradle launched unconstrained Ninja. MP9 Android CMake now assigns a shared four-job compile pool, including Gradle invocations. For recovery with an older configured graph, use ninja -C <configured arm64 graph> -j2 mkw_quest_kit_probe WiiCompiled first, then normal APK build.
- Always install rebuilt APK before importing new private library when kit identity changes. Import through normal verifier. Never bypass kit/hash checks or accidentally launch stale library.
- Deployment must not use PowerShell *> redirection: adb stderr can become a terminating error.
- Drive-Mp9DebugInput.py sends normal KPAD and requests fresh game-frame BMP. It deletes previous BMP and waits for a complete fresh file, rather than returning stale output. --disable restores physical input immediately.
- Replay-Mp9Solo.py verifies menu headers before progressing and always disables controller override in finally. --characters stops at character selection; --from-main / --from-characters resume there. Private reference BMPs are not distributed. A new workspace must capture its own references. Graphics repairs changed background pixels; updated verified references are required. Header edges provide limited tolerance, not proof of gameplay.
- Startup may take longer than capture's8s timeout; wait for boot before replay. Capture failure must not be reported as a screenshot or successful test.

## Debugger constraints

Read-only LLDB snapshots with ReadMemory and guaranteed Detach worked. Reacquire SBValues after Continue. Never symbolize an old PID using a newly rebuilt ELF. Target Logcat reader is not necessarily guest/GX thread. JavaJIT normal SIGSEGV needs passthrough.

A conditional portrait-breakpoint experiment disconnected and left a SIGTRAP in test process. Force-stop/restart cleared it. Avoid repeating that breakpoint loop. Use bounded native logs or read-only snapshots. threading.Timer cannot reliably interrupt synchronous process.Continue; LLDB may hold Python GIL. If debugger left process stopped, detach and SIGCONT or restart cleanly. Do not leave headset paused.

## Checkpoint/publishing

Existing draft PR: https://github.com/RubyPaws39/WiiCompiledParty_VR/pull/1 , branch codex/mp9-build-metadata-handoff, base openxr-work. Update it; do not create duplicate PR. Audit explicitly staged source/docs for game content, private paths/device IDs and secrets before push. Do not publish generated native outputs or private capture references. Keep local patches/0001-mp9-runtime-disc-gate.patch synced to pinned upstream4dc213cc7e0dfed5f6bb1c4541c2fb6ed5992904 and verify reverse-apply. No finished release or playable APK claim.

## Final checkpoint validation

Clean native target, matching APK build/content audit and verified private import passed. Automated ordinary-KPAD replay reached title, main menu, Solo info, character selection and Toad Road setup. Fresh clean-build captures confirm ten portraits/preview remain visible. Start still leaves animated star background. Debug input file absent after replay. Physical A+B and audible quality still require human verification. Native Ninja graph contains mp9_compile pool depth4. No matrix/portrait debugger or trace hook remains active.

## 2026-10-17 limited follow-up smoke test

Installed diagnostic build launched and ordinary KPAD replay verified title, main menu, Solo info, character selection and Toad Road setup. Fresh capture after Start still shows purple star background; board gameplay is not working. Replay exited successfully because menu traversal completed, not because gameplay succeeded. Developer input file was removed on exit.

OpenCode/Nemotron left uncommitted DVD, REL-context and task-context experiments. Their claim of a fixed module-loading root cause is unverified. Proposed DVD hook calls the REL observer on arbitrary read buffers and swallows errors; it is not accepted into published runtime code. Task logging is unbounded/noisy. `dvd.cpp` is compiled through `mkw_runtime_common` Unity source; absence of a standalone dvd.cpp.o does not mean exclusion. No rebuild or new runtime deployment was performed during this limited check. Runtime experiments remain local pending proper validation.
