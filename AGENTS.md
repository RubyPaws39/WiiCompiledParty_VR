# MP9 continuation instructions

Latest (2026-10-31): read `docs/MP9_TABLETOP_STATUS.md` first. User confirmed tabletop board geometry; talking Toad NPC textures and repeating music remain broken. Native pure-camera provenance/cancellation and transparent-world/HUD paths now exist. Subsequent diagnostic builds add draw-weighted camera selection, paired mono/eye capture and opt-in bounded AX/DMA PCM recording. Preserve working private tabletop checkpoint. Do not claim audio repaired or every scene converted. Older charging/window-only notes below are superseded.

Read `docs/MP9_MIXED_REALITY.md` and `docs/MP9_HANDOFF.md` first. Newest checkpoints supersede older entries.

Current user target (2026-10-30): Mixed Reality inspired by zeranemesis/Marioparty4, preserving MP9 progression. Initial placed stereo-window MR and reference placement gestures implemented; full tabletop world/camera separation is unfinished. Headset charging: build/offline checks only so far, no device verification. Do not claim identical tabletop behavior. Source reference and concrete validation steps are in the MR checkpoint.

- Keep responses terse; technical details exact. Code and commits use normal prose.
- Audio remains a blocker: repeated music and missing event sound effects. A synchronous-mixer A/B test did not fix repetition. Do not call it fixed or repeat that experiment without new evidence.
- Preserve verified board/minigame progression, full 115-module coverage, graphics fixes, actual perspective replay and MP9-specific settings. All-module coverage does not mean all modes were tested.
- Keep strict memory, loaded-section, stale-dispatch and source checks enabled. No fake pointers, forced queue counts, skipped game logic or interpreter fallback.
- Use existing automated KPAD replay/capture; minimize requests for the user to navigate the headset. Clear input overrides after testing.
- Build the native `WiiCompiled` target explicitly; Gradle builds only the launcher kit. Never run concurrent Ninja builds in the same graph. While compilation runs, wait instead of repeatedly reasoning or polling.
- Keep large/private outputs on R:. Never publish ROMs, assets, saves, translated game sources, native game libraries, `.wcgame` packages or private captures/logs.
- Public source branch: `codex/mp9-build-metadata-handoff`, remote `myfork`; existing draft PR #1. User authorized source updates. Audit staged files before pushing.
- Synchronize the outer project's `patches/0001-mp9-runtime-disc-gate.patch` against upstream `4dc213cc7e0dfed5f6bb1c4541c2fb6ed5992904`; verify reverse application.
- Temporary `[mp9-audio-*]` probes removed from the MR build. October 19 evidence remains saved; reintroduce bounded tracing only for a specific discriminator, then remove before delivery.
- Report measured results and unresolved limitations. Do not label the ROM-free developer importer a complete public game: end-user native generation, online play and full VR coverage remain unfinished.
