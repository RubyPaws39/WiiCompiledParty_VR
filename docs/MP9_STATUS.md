# MP9 Quest development status

Standalone ARM64 static recompilation through Wiicompiled_VR and Aurora. **Not playable yet.**

Verified on Quest 3:

- Original “Creating save data” screen and MP9 title screen render; user confirmed both in headset.
- Original save worker runs and writes `save.dat` under SSQE01 title home. Save/banner handling still has errors; complete save reliability is unverified.
- Right A + right trigger reaches the menu transition. Previous missing `menu.rel` target is compiled; transition subsequently crashes in Mobiclip switch dispatch.
- MP9 button mapping and motion primitives pass production ARM64 tests. Physical minigame motion/control validation remains pending.
- Four REL modules have native coverage: Introduction, strap, menu_title, menu. This is incomplete game coverage.

Current tested failure: original movie decoder jump `0x800F51C8` was omitted by switch-table analysis. Two regression tests reproduce two cases instead of nine; corrected analysis passes all 658 translator tests. Device rebuild remains pending verification. A rebuild script bug also reused stale direct-call metadata; scripts now regenerate metadata explicitly and validate source hashes.

Startup audio stutters according to headset testing. Copied palette descriptors now upload without the prior invalid-palette warnings. Neither observation proves full graphics/audio correctness.

Remaining: reliable menu/gameplay progression, all required REL entries, immersive scene/camera integration, per-minigame controls, recenter/calibration validation, multiplayer/lobbies/profiles/statistics, and public BYO-ROM native generation. No finished APK is published. Private translated game libraries and packages must never be distributed.
