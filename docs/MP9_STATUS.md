# Latest — 2026-10-30

Target changed to Mixed Reality. See `MP9_MIXED_REALITY.md`: placed stereo-window MR and reference placement controls built; headset charging, no new physical validation. Full free-standing tabletop parity and audio repetition remain unresolved. Native link, ROM-free APK audit and offline input/presentation tests passed. Preserve existing verified game progression.

# Earlier blocker — 2026-10-19

Audio repeats; dice/event SFX missing per user. Worker-off A/B test did not repair it. See `MP9_AUDIO_HANDOFF.md` for installed bounded diagnostics and exact continuation. Full depth-copy family now passed former faults and reached board intro/order dice. No full-game/audio-complete claim. Older checkpoint entries remain historical.

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

# Current MP9 status � 2026-10-18

Toad Road's 3D flythrough now renders on Quest 3. First turn and minigames remain unverified. Previous installed test stopped at Solo stage wrapper `sme_stage00`, missing native target `0xAD809170`. New full build passed and awaits device installation/testing.

The Start stall was an uncompleted `WPADGetInfoAsync` request, not a missing DVD worker. SSQE01 `0x801C06F0` now binds checked virtual-controller info and the existing deferred callback queue. Production ARM64 regression passes callback arguments, output extent and error behavior. Original game then requests Solo opening and board RELs normally.

All 115 generated REL manifests exist locally, but previous installed graph contained only 17,521 entries. Generation alone was mistaken for installed coverage. Full merge and native build passed: 167,529 total entries; linked ELF contains every one of the 159,140 REL functions across all 115 disc modules. No all-minigame success claim yet. Original REL linking controls active guest state/assets; installed ARM64 text remains file-backed. No player-selected module batches are needed on current evidence.

Prior scheduler queue-count injection was removed: `0x80184000` is OSReceiveMessage, and +28 is genuine queue occupancy. Strict guest, loaded-section and missing-dispatch checks remain enabled. Existing DMA and texture metadata repairs remain.

Presentation remains a world-fixed screen. Immersive cameras, complete physical motion validation, audio stability, online systems and public on-device AOT generation remain unfinished. Private ROM/assets/generated code/libraries/packages stay local.

---

## Historical checkpoint (superseded where corrected above)

# MP9 Quest development status � 2026-10-16

Native ARM64 static recompilation through Wiicompiled_VR AOT + Aurora. **Not playable yet.**

## Verified on Quest 3

- Original title, welcome scene, main menu, Solo information screen, character selection and Toad Road setup render.
- Exploding 3D geometry was caused by missing locked-cache DMA through SPR922/923. Checked DMA now transfers the original matrix palettes into RAM. Fresh captures show castle, landscape, Mario and Toad correctly.
- Blank Solo preview, character portraits and menu background were caused by omitted guest GX texture metadata. The native texture loader now mirrors image0/mode0 into the original SDK shadow. Original coordinate-size code can therefore size secondary texture masks correctly. Fresh captures show all ten portraits and Solo preview.
- Ordinary KPAD input reaches Toad Road setup after selecting Mario. Pressing Start leaves an animated star background; board gameplay has not begun. This remains the current progression frontier.
- Developer input no longer overrides physical controllers indefinitely: button holds expire within two seconds, override expires one second later, and replay disables it on exit. Wii A+B maps to right A + right trigger. New physical-headset verification is still needed.
- Production locked-cache DMA regression fails against the old runtime and passes on ARM64 with store/load/count/wrap/flush/trigger/bounds coverage. Button/motion primitive tests passed previously; physical minigame controls remain unverified.
- Five RELs have private AOT coverage: Introduction, strap, menu_title, menu, mg9409. Module coverage is incomplete. Earlier missing mg9409 target is compiled; this does not prove progression after Start.

## Current limits

Audio still stutters and output-queue overflow was observed. Save.dat writes were observed; banner errors and save reliability remain unverified. Current presentation is a world-fixed virtual screen. Immersive gameplay/cutscene cameras, per-minigame motion validation, online lobbies/friend codes/profiles/statistics and public end-to-end BYO-ROM native generation remain unfinished.

No finished APK is released. Development APK contains a ROM-free launcher/runtime kit; the separate private translated library and game data remain local. Do not distribute ROM files, assets, saves, translated Nintendo C++, private libraries or wcgame packages.

See [continuation handoff](MP9_HANDOFF.md) and [maintainer tools](../projects/mp9/README.md).

## Final checkpoint validation

Clean native target, matching APK build/content audit and verified private import passed. Automated ordinary-KPAD replay reached title, main menu, Solo info, character selection and Toad Road setup. Fresh clean-build captures confirm ten portraits/preview remain visible. Start still leaves animated star background. Debug input file absent after replay. Physical A+B and audible quality still require human verification. Native Ninja graph contains mp9_compile pool depth4. No matrix/portrait debugger or trace hook remains active.

## 2026-10-17 limited follow-up smoke test

Installed diagnostic build launched and ordinary KPAD replay verified title, main menu, Solo info, character selection and Toad Road setup. Fresh capture after Start still shows purple star background; board gameplay is not working. Replay exited successfully because menu traversal completed, not because gameplay succeeded. Developer input file was removed on exit.

OpenCode/Nemotron left uncommitted DVD, REL-context and task-context experiments. Their claim of a fixed module-loading root cause is unverified. Proposed DVD hook calls the REL observer on arbitrary read buffers and swallows errors; it is not accepted into published runtime code. Task logging is unbounded/noisy. `dvd.cpp` is compiled through `mkw_runtime_common` Unity source; absence of a standalone dvd.cpp.o does not mean exclusion. No rebuild or new runtime deployment was performed during this limited check. Runtime experiments remain local pending proper validation.
