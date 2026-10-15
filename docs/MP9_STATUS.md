# MP9 Quest development status

Standalone ARM64 static recompilation through Wiicompiled_VR and Aurora. **Not playable yet.**

Verified on Quest 3, 2026-10-15:

- Original save screen, title, and main menu render. User confirmed main menu opens after holding right A + right trigger.
- Movie decoder switch-table analysis fixed; both regressions failed before correction and all 658 translator tests passed after correction.
- Solo mode information preview and all character portraits remain blank. Welcome scene has severely distorted geometry. Repeated graphics experiments have not repaired these symptoms.
- Original save worker writes save.dat in correct SSQE01 title home; banner errors remain and save reliability is unverified.
- Button mapping and motion primitives pass ARM64 tests. Physical minigame controls remain unverified.
- Five REL modules now have private generated coverage: Introduction, strap, menu_title, menu, mg9409. Latest merge: 13,033 entries; mg9409 adds 885. Native build and APK audit passed in work/build-mg9409.log; ELF contains func_A5C01F60. New library has not been deployed: adb currently reports device offline.

Selecting a character reached strict missing native target A5C01F60, module 46 mg9409.rel offset 1F60, caller 8008A314. New module generation addresses that missing coverage; no successful next-stage test is claimed.

Graphics evidence: a perspective indexed draw has valid projection and finite vertex positions, but active PN0 position/normal matrices contain extreme invalid values. Trace matrix production/upload rather than assuming shader compilation is the cause. Diagnostic stack reaches original matrix upload function 8019C430 through paired-single FIFO stores, called from 80170850. Debugger argument/register capture was incomplete; it does not yet establish root cause.

Experimental changes in this checkpoint: live VAT/VCD restoration, CP layout-list scanning, native texture-copy SDK bindings, and bounded graphics diagnostics. The ineffective host-pointer canonicalization experiment was reverted before checkpoint. They did not visibly fix preview, portraits, or geometry. Synchronous GX also did not fix rendering and is not a solution.

Startup audio still stutters. Immersive gameplay camera/scenes, complete module coverage, per-minigame physical validation, online lobbies/profiles/statistics, and public BYO-ROM native generation remain unfinished. Current presentation is a world-fixed virtual screen. No finished APK is released. Private translated libraries, ROM data, saves, and wcgame packages must never be distributed.
