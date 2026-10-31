# MP9 tabletop checkpoint — 2026-10-31

User confirmed the board now appears as room geometry. The talking Toad NPC has a reported material/texture defect in MR; its original mono image looks correct. Repeating music remains unresolved. Do not describe either fault as fixed.

## Implemented

- Read-only observers wrap the original private LookAt, Concat and ConcatArray translations. The lowering script pins inspected source hashes, verifies already instrumented bodies and preserves original guest calculations. Regenerate copied build shards after lowering; touching a shard does not update its contents.
- Exact model-view provenance recovers the original pure camera. Signed zero is canonicalized; every other component remains bit-exact. A bounded LRU retains active camera roots. Draw-weighted voting rejects tied camera candidates.
- Valid board/minigame frames cancel the game camera and place world geometry at the MR pose, without the previous screen aperture. Passthrough uses transparent eye clears and separate alpha pipeline siblings. HUD remains on a complete fixed plane with independent depth handling.
- Unproven/multiple cameras retain the original screen. This is a deliberate presentation fallback, not skipped guest logic. Original module loading, checked memory and native dispatch remain intact.
- A requested developer frame capture also saves an actual immersive left-eye BMP when an eye was rendered. The original mono capture remains unchanged. Missing eye capture is not proof of successful rendering.

## Controls

Right stick click enters placement. Left stick right enlarges the world model; left shrinks it. Right stick up/down resizes the UI plane, right stick left/right rotates placement, left stick up/down adjusts height. Right grip carries placement; right trigger places it at the controller. A finishes. Reference controls and the original Wii mapping remain available outside placement.

## Limits

Initial fitting uses the first validated camera target and projected span, not measured complete board bounds. Board-wide framing, sky/backdrop routing, secondary perspective UI cameras, every minigame, persistent room anchors and real-table occlusion are not validated. A talking NPC may use a different camera/material path; investigate paired mono/eye captures before changing textures.

Host camera tests cover signed zero, in-place and bulk concat, bounded-cache retention, ambiguous provenance, draw voting, camera-motion cancellation and unchanged guest RAM. Policy tests verify retained-window/tabletop packet invalidation. Placement tests and full ARM64 links pass. On-device logs proved pure-camera matches and tabletop activation on Toad Road; physical confirmation belongs to the earlier prototype run, not every subsequent diagnostic build.

## Audio evidence and next discriminator

The earlier worker-toggle A/B did not repair repetition. Current timing shows approximately five seconds of PCM produced per five seconds elapsed, usually zero queue-empty arrivals/drops; isolated long gaps and dropped blocks occur. Active music cursors advance, wrap normally and their source-buffer samples change. These measurements rule out a globally frozen mixer; they do not establish the repetition source.

Opt-in Android properties, all default off:

- `debug.wiicompiled.audio_timing=1`: five-second output queue/timing counters.
- `debug.wiicompiled.voice_trace=1`: bounded active-voice cursor, loop and source-change statistics.
- `debug.wiicompiled.audio_dump=1`: at most thirty seconds per process/source of AX native-endian R/L PCM and DMA big-endian R/L PCM in package-private `debug-tools`. Enable after music starts; set back to zero to flush. No microphone recording.

Compare normalized AX and DMA samples and correlate longer repeated segments before changing scheduling, decoder layout or buffers. Preserve the verified DSP CRC/layout. Source references: Dolphin's [AXWii](https://github.com/dolphin-emu/dolphin/blob/master/Source/Core/Core/HW/DSPHLE/UCodes/AXWii.cpp) and [AXVoice](https://github.com/dolphin-emu/dolphin/blob/master/Source/Core/Core/HW/DSPHLE/UCodes/AXVoice.h).

A thirty-second character-selection recording contains 10,000 AX and DMA blocks. After endian normalization, 9,999 DMA blocks match AX with the same one-block delay. Every nonsilent block is distinct; tested short lags (96 ms through 5.376 s) show no strong repetition. This establishes a healthy handoff in that menu sample only. The reported board/NPC music still needs its own recording; do not generalize the menu result.

Night wrap-up: a second thirty-second board-opening/NPC-transition capture also matches **9,999/9,999** overlapping AX/DMA blocks at that same delay. Some silence/repeated blocks occur during scene transitions; correlation at the 2.688-second streaming-ring period is only 0.167 in this sample. The measured handoff is healthy in both recordings, but the user's audible repetition is not explained or repaired. Next compare the actual audible fault interval with captured PCM and inspect downstream host playback/timing. Do not force new decoder layouts or claim a buffer fix from this evidence. Debug properties were reset to zero and the app restarted, so cached diagnostic enable flags are off for the night.

The downloadable APK is a ROM-free experimental launcher/importer. It does not contain translated game code; the user's private native game package is installed separately. Public end-user ROM-to-native generation and online play remain unfinished. Never publish private packages, PCM, captures, logs, generated sources, ROM data or native game libraries.
