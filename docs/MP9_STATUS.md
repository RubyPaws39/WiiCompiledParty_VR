# MP9 Quest development status — 2026-10-16

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
