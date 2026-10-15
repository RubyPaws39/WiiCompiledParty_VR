# MuseSpark continuation prompt

Continue Mario Party 9 native Quest 3 work in `R:\MarioParty9VR`. Read local
`MUSE_SPARK_HANDOFF.md`, `MODLOG.md`, and `README.md` first; those contain the full
machine-specific evidence and newest logs. This public document is a concise source-only handoff.

User requirements: standalone native ARM64 APK through Wiicompiled_VR AOT + Aurora;
real Quest buttons, pointing, shake and rotation controls; world-fixed menu with
recenter/control adjustment; immersive gameplay and scenes; later online lobbies,
friend identities and persistent stats. Keep work/cache on R:. Public APK requires
users' own verified ROM and must include no Nintendo executable, translated code,
assets, saves or private package. Existing private local owned dump/import is authorized.

Verified frontier: original save screen and MP9 title display in headset. A+B reaches
menu transition. Native menu.rel coverage fixed missing dispatch key A2015AB0;
next strict failure is original Mobiclip switch target800F51C8 within fn800F4570.
JumpTableDetector followed a stale compare on r0 rather than logical selector r31;
slwi/rlwinm source tracking fixes nine-case recognition. Both new tests fail before
fix and all658 translator tests pass after. Generated code contains missing case.

Before further runtime fixes, finish fresh-metadata rebuild. Earlier Recompile script
omitted --output-metadata, so updated C++ retained stale call declarations and failed
compilation. Recompile now explicitly regenerates base_translation_output.json;
task lowering refreshes modified source hashes and REL merge checks hashes. Do not
hand-edit generated C++ or weaken ABI/guest/MMIO checks. New relevant logs:
work/recompile-fresh-metadata.log and work/build-fresh-metadata.log. Requery actual
build/device state rather than assuming a previous PID or installation is current.

Build: projects/mp9/README.md describes private workspace scripts. Full rebuild needs
explicit WiiCompiled ninja target after Gradle. Frozen shard cuts previously packed
new menu functions into a100MB C++ file; repacking produced balanced72 shards,1.23x
spread. Recompile -RepackShards preserves old map when deliberate repacking is needed.

Accepted runtime changes include texture SDK bindings; copied TLUT descriptors;
MP9 title/home identity; exact custom-task host/FPR context; actual outgoing OSThread
host continuation; OSExitThread on native thread return; copy-side VI pacing disabled
for MP9 so original wait yields; optional Mario Kart volume helper disabled for MP9
because it followed foreign SoundManager addresses. Removing that helper's foreign
memory reads/writes eliminated the prior audio-poll memory fault. Do not re-enable
Mario Kart-specific pointer walks without verified MP9 structures.

Controls: right A=WiiA, right trigger=WiiB, left X=1, right B=2, left stick=D-pad,
left menu/thumbclick=Plus, left trigger=Minus, both grips=Home, left Y=settings,
right thumbclick=recenter. Bare Wii Remote profile; real OpenXR motion path retained.
Startup audio stutters; not declared fixed. No physical minigame validation yet.

Preserve existing work and saves. Debugger must detach even on failure; never leave
headset paused. Temporary IOS/exit/native-unwind tracing removed; bounded display-copy
capture may remain. No fake pointers, skipped game logic, silent missing functions,
zero-backed hardware registers, or launcher-only “playable” claims. Keep local patch
synced against pinned upstream4dc213cc7e0dfed5f6bb1c4541c2fb6ed5992904, not against
an arbitrary later clean fork HEAD. GitHub already has independent MP9 commits;
preserve those commits and inspect changes before pushing. Record exact verified
behavior and limitations, then provide user a current handoff before context runs out.
