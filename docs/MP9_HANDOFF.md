# Active checkpoint — Mixed Reality, 2026-10-30

Read [MP9_MIXED_REALITY.md](MP9_MIXED_REALITY.md) first. Target changed to Mixed Reality inspired by MarioParty4. Initial placed stereo-window MR, reference placement gestures and guarded 120 Hz cadence request implemented; native/APK/offline tests passed. Headset discharged/charging: no new device result. Full tabletop world-space rendering, room scan/spatial anchors and audio repair remain unfinished. Temporary audio probes removed; older diagnosis/evidence retained below.

# Earlier checkpoint — audio repetition, 2026-10-19

Read [MP9_AUDIO_HANDOFF.md](MP9_AUDIO_HANDOFF.md) and repository `AGENTS.md` first. User requested stopping and handing off to OpenCode. Synchronous mixing did not fix repeated music/missing event SFX. Bounded audio probes are built and installed; no audio repair claimed. Latest depth-copy family passed prior faults and reached board intro/order dice. Older runtime-pending/flat-only statements below are historical and superseded.

# Latest verification - 2026-10-19

Quest run `base_1792456556_pid23368` confirmed genuine immersive GX replay: 234/234 perspective draws replayed, both eyes prepared successfully and an OpenXR projection layer submitted. Title/main/Solo selection remained a fixed 2D screen. Camera evidence comes from actual recorded perspective model-view data plus active SSQE01 gameplay modules, not guessed camera pointers. Multiple perspective viewports currently retain the original picture until local-player routing is verified.

Immediate and FIFO draws now share state-sensitive batching; per-rectangle split-screen metadata stays separate. Same Solo opening changed from roughly 16 FPS / 60 ms mono GPU / 6,600 draw calls to 48-51 FPS / about 7 ms GPU / 760-970 draw calls. This was an early stereo run, not a guaranteed frame rate across all content. Audio stutter remains unverified/further work.

That run passed the first Z8 readback frontier, then stopped on unsupported Z8M (`0x39`). Latest native build adds the depth-copy family (Z4, Z8, Z8M, Z8L, Z16L) through GPU conversion, tiled CPU encoding, texture decoding and copy-cache compatibility. Existing Z16 behavior preserved. ARM64 depth layout/bounds/channel tests pass; latest package is importing for runtime retest. MP9 now defaults to waiting for unready shaders instead of hiding objects.

MP9 headset panel provides gameplay stereo toggle, world scale, recenter, screen geometry, passthrough, resolution, pointer hand/sensitivity and shake strength. Right A=Wii A, right trigger=Wii B, left X=1, right B=2, left stick=D-pad, left Menu/stick click=+, left trigger=-; settings button reserved. Settings save inside this Android package's Config.toml; [mp9_vr] stores control tuning. Motion strength preserves gravity/tilt. ARM64 controls/panel and fast-math scene-validation tests pass.

Earlier complete flat-screen loop verified: Toad Road dice/movement -> Skyjinks -> results -> next Mario turn. All 115 REL modules and 159,140 REL functions compiled. Do not equate coverage with every minigame tested. Strict checks preserved. Full public user-ROM-to-native generation and online systems remain unfinished; public source only, no private game/translated artifacts.

---

# MP9 Quest checkpoint - 2026-10-19

Native ARM64 developer build verified on Quest 3: Solo -> Mario -> Toad Road, dice roll and movement, Skyjinks minigame, normal results, then return to Mario's next board turn. Evidence: private `work/quest-oct18-board-and-skyjinks.log` and matching captures. This proves one complete gameplay loop, not every mode/minigame.

All 115 disc REL modules / 159,140 REL functions exist in the linked native library; 167,529 total translated entries. Original REL link/unlink controls active guest state/assets. Actual full-build process measurement: about 1.18 GiB PSS; file-backed library text does not require player-selected module batches.

Progression fixes: checked WPADGetInfoAsync completion; canonical REL dispatch for OS worker entry points; blocking DVD reads yield independent worker fibers for 1 ms so the initiating caller can arm its completion flag before a cached read finishes. MP9 custom-task scheduler fiber must not suspend mid-HLE. No fake pointers, queue counts, skipped game logic, interpreter, or disabled strict checks.

MP9-specific headset panel replaces kart camera/steering options. Left Y toggles; tracked-controller trigger selects; thumbsticks scroll; right stick click recenters. Screen geometry, passthrough and resolution use existing rendering controls. Pointer sensitivity, shake strength and left-hand pointing/motion save under [mp9_vr]. Gravity/tilt are preserved when scaling shake. Android package storage already separates MP9 configuration from the Mario Kart app.

Current performance investigation: original Solo opening measured roughly 16-18 FPS, 54-61 ms mono GPU work/frame and 5,500-6,600 draw calls; menus reach about 60 FPS. Audio output overflow observed. Do not claim the stutter fixed. Renderer orthographic batching is under investigation; temporary diagnostic must be removed before delivery.

Current presentation remains a world-fixed screen. Immersive scene cameras, complete physical motion validation, remaining graphics artifacts, smooth audio/cutscenes, online profiles/lobbies and end-user on-device native generation remain unfinished. Public APK is a ROM-free developer importer; private translated library/ROM/assets/packages must not be published.

---

## Earlier notes (superseded where contradicted above)

# Current continuation � 2026-10-18

Prioritize finishing full native module coverage and verifying an actual board turn. Read current status before historical notes below. Preserve strict checks and existing verified graphics repairs. No interpreter fallback or fabricated guest state.

Verified progression: WPAD info completion fix -> Solo opening `sme_op` id107 -> normal Minus skip -> `bd01` id3 -> Toad Road terrain/paths/windmill render -> `sme_stage00` id108 -> missing target `0xAD809170`. First turn not reached yet. Use fresh game captures and ordinary KPAD developer input; remove override afterward. If Quest loses focus, wake/resume activity before blaming game logic.

Controller fix: `ports/mp9/sdk_bindings.cpp` binds SSQE01 `0x801C06F0` to `WPADGetInfoAsync_HLE`; `runtime/src/hle/input/wpad.cpp` fills bounded 24-byte peripheral status and queues `(channel,result)` through existing completion pump. `projects/mp9/Test-Mp9WpadInfo.cpp` passed on ARM64 hardware.

Corrected old diagnosis: `0x80184000` is OSReceiveMessage; descriptor +28 claim was queue occupancy. `0x8026E880` is VI retrace wait queue. Workers exist and DVD audio reads continue. Never restore queue count=1 injection or observe arbitrary DVD buffers as REL headers. Prior unverified experiments are backed up locally.

Coverage: all 115 REL manifests total159,140 functions. Full merge must be followed by shard emission, native WiiCompiled build and verified private import. Installed pre-expansion graph remains18,386 entries until new import. Full build passed:167,529 total entries,167,418 shared translated functions plus111 SDK overrides. ELF symbol audit verifies159,140/159,140 REL functions across115/115 modules. Private package prepared; device install/test pending. Original helper interior entries0x8020BF14/0x8020BF60 were missing; prepare-runtime-probe now adds REL-required DOL map roots and validates WPAD info binding. Regenerated DOL has8,389 entries; task lowering passed. MP9 rebuild selects512 shards; generated Android code uses line-table debug records to control disk use. Original guest REL link/unlink controls active module state, while all ARM64 code can remain installed/file-backed. No speculative batching loader.

`Merge-Mp9RelAot.py` validates retained source hashes and direct dependencies, stages checked flat hard links to private module sources, and supports incremental `--add`. Regenerate dispatch after SDK binding edits; stale-dispatch checks must remain ON. Gradle alone builds launcher kit, so explicitly build WiiCompiled. Default four-job cap remains safe. Local balanced full build completed with eight jobs after measuring17GiB free RAM and0.4�0.6GiB per compiler; this changes scheduling only. Keep all large files on maintainer's R drive and all translated code/data private.

---

## Historical handoff (superseded where corrected above)

# Muse Spark continuation prompt � 2026-10-16

Continue the native Mario Party 9 Quest 3 port using Wiicompiled_VR AOT + Aurora. Read current status, this handoff, README and local MODLOG before editing. Preserve existing commits and unrelated work. Keep strict guest/MMIO checks enabled. Never fabricate pointers, skip original game logic, use an interpreter, or call the current prototype playable.

Priority: get from the working menus into actual board/minigame gameplay, then validate physical Quest controls. Current world-fixed screen is not immersive gameplay. Online/profile/stat systems and public on-device native generation remain unfinished. Public source/APK must contain no ROM, game assets, saves, translated Nintendo code or private game libraries.

## Established fixes � preserve these

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
