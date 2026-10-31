# MP9 Mixed Reality checkpoint — 2026-10-30

**Superseded in part:** read [the October 31 tabletop status](MP9_TABLETOP_STATUS.md) first. User confirmed free-standing board geometry after pure-camera tracking was added. NPC rendering and audio remain broken. The original stereo-window implementation below is historical.

User changed the target from immersive VR to Mixed Reality, using [Party Board / MarioParty4](https://github.com/zeranemesis/Marioparty4) as the interaction reference. Existing MP9 progression, 115-module coverage, graphics repairs, input mappings and checked native dispatch are preserved.

## Implemented now

MP9 defaults to a **placed stereo game window with room passthrough**, using Aurora's existing immersive-window replay and alpha mask. Menus and the game window remain fixed in the current OpenXR reference space when the head moves. The frame's world transform can scale about the screen plane without scaling the HUD. Placement resets on recenter/reference-space changes; no session-local coordinates are misrepresented as persistent spatial anchors.

This is **not yet the reference project's free-standing tabletop diorama**. Current content remains camera-framed inside a stereo aperture. It does not remove the original game camera's motion, recover off-camera game geometry, identify the physical table, or provide real-world occlusion. MP4 camera addresses and game-state hooks are not valid for MP9. Do not advertise identical behavior or a finished conversion.

### Placement controls

| Control | MP9 MR behavior |
| --- | --- |
| Right stick click | Enter placement; click again to finish |
| Right trigger | Put screen center at right controller aim origin, facing the head |
| Right grip, held | Carry placement with the controller, preserving the initial offset |
| Right stick up/down | Screen size |
| Right stick left/right | Turn placement |
| Left stick up/down | Height |
| Left stick left/right | Model scale; right makes geometry larger |
| Left trigger | Match left controller height |
| Left stick click | Stereo gameplay on/off |
| X | Room passthrough on/off |
| Y | Eye resolution: 50%, 75%, 100% of the runtime recommendation |
| A, B, Menu | Finish placement |

The placement help uses the existing WiiCompiled compositor panel. Both VR controllers are withheld from guest input during placement and until all controls are released afterward. Existing both-stick settings chord is preserved. Outside placement, original Wii mappings and left Y settings access remain unchanged. Placement does not pause guest simulation.

The reference's left-grip table/camera mode, automatic room scan and persistent spatial anchors are **not implemented**. Resolution gestures match the reference's intent, but its 1080p/1440p/4K surface sizes do not map directly to this renderer's per-eye swapchains.

`[mp9_mr] enabled`, `screen_width`, and `units_per_meter` persist separately from the legacy VR settings. Room/stereo settings reuse the existing package-private config. Writes happen on the game thread after placement, not on every XR input sample. Arbitrary custom eye resolution is preserved unless Y explicitly cycles quality.

### Smoothness work

- Preserve the previously measured immediate/FIFO batching repair; no new performance result is claimed without a headset run.
- Request 120 Hz when enumerated, or 60 Hz when enumerated without 120 Hz, so the original 60 Hz game can have even display cadence. Otherwise retain the runtime's rate. Request refusal is logged, never treated as success.
- Use sustained-high performance as MP9's fallback when no explicit performance-level setting exists. Existing user settings win.
- Remove temporary per-DMA/per-PB diagnostic logging and hashing. Audio semantics are unchanged: repeated music/missing event sound remains unresolved. Saved October 19 evidence remains in `MP9_AUDIO_HANDOFF.md` and private work logs.

## Verification

- Full ARM64 `WiiCompiled` native link passed.
- ROM-free `assembleModernQuestDebug` APK built and audited: no game library, DOL, REL or disc content; no unsupported MKW on-device toolchain.
- Windows host tests run under `-O3 -ffast-math -UNDEBUG`: MR placement/scaling/tracking-loss/input-ownership/refresh selection; original MP9 Wii controls; existing presentation gates. All passed.
- New MR test also cross-compiled and linked for Android ARM64. **Not executed on headset yet.**
- User reports headset discharged/charging. No new build was installed or visually verified during this checkpoint. Prior complete board/minigame/results loop remains prior evidence, not a test of this MR build.

## Next device pass

1. Install updated ROM-free APK and import the separately prepared private native package through normal verifier. Preserve the user's disc and saves.
2. Confirm title A + right trigger, Solo, characters and Toad Road still work using automated KPAD replay.
3. Wear headset once for placement: right stick click, move/scale/turn, finish with A; verify no guest A/Plus/Home leakage. Move head to verify placement remains fixed and stereo depth works.
4. Verify settings pointer matches the visible panel after placement/yaw/scale changes. Check both room toggle and stereo-off fallback.
5. Record actual headset rate, GPU/game FPS, frame stalls and audio behavior. A request for 120 Hz is not a verified 120 Hz result.

## Next implementation work

To match the reference's actual tabletop model, identify and validate MP9's **camera-only view matrix**, separate it from per-object model transforms, and replay board world coordinates at an anchored physical pose. Current perspective evidence contains model-view matrices; do not blindly invert it as a pure camera. Add validated sky/background routing and per-minigame framing before claiming free-standing models. Room-scene queries/spatial anchors require their own OpenXR capability and lifecycle integration. Preserve strict checks and original guest logic throughout.

Reference inspected: `zeranemesis/Marioparty4`, branch `audio-local`, commit `2714c86bc7066c5efbcc73355e603f5e3010c6d5`; especially `platforms/android/README.md` and `platforms/android/app/src/main/cpp/quest_xr.cpp`. Interaction behavior was adapted independently; no MP4 game source/assets or incompatible camera hooks were transplanted. Local reference checkout stays outside the public fork.

Build with the existing explicit native Ninja target; Gradle alone builds only the ROM-free kit probe. Tests: `projects/mp9/Test-Mp9MixedReality.cpp`, `Test-Mp9VrControls.cpp`, `Test-Mp9VrPresentation.cpp`. End-user ROM-to-native generation, online play and full minigame/physical-motion coverage remain unfinished.
