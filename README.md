# WiiCompiledParty_VR

A **standalone native port of Mario Party 9 (SSQE01) for Quest 3**, made with
static recompilation. Fork of
[iChris4/Wiicompiled_VR](https://github.com/iChris4/Wiicompiled_VR), retargeted
from Mario Kart Wii to Mario Party 9: MP9 boot/memory/IOS/video/GX/audio/input
runtime adapters, MP9 SDK bindings, and a ROM-free Android launcher kit.

There's no emulator in the loop, no interpreter, no JIT, no PowerPC
anywhere at runtime. The game is translated ahead of time to ARM64 and runs
natively on the headset.

> [!IMPORTANT]
> There is no Nintendo code, no assets and no game data anywhere in this
> project. You need your own legally dumped copy of Mario Party 9 (USA,
> `SSQE01`). Translation runs locally against your disc image, and nothing
> ever gets uploaded. Never distribute a build containing translated game
> code or game assets.

> [!WARNING]
> **Work in progress — not playable yet.** The APK currently builds, installs,
> launches an OpenXR session, imports a locally built private game package,
> and boots through native SDK initialization, but no game scene runs yet.
> Do not treat any current build as a finished port.

---

## What this fork contains

- **MP9 runtime adapters** (`ports/mp9/`, `runtime/src/hle/`): boot, interrupt,
  EXI/RTC/SRAM, SI discovery, IOS/IPC/NAND services, IOS56 boot profile,
  video (VI), GX/Aurora bring-up, DVD/FST backend, fiber-backed scheduler,
  WPAD/KPAD input encoding, and AI/AX/DSP audio startup — all bound against
  verified SSQE01 symbols, not Mario Kart addresses.
- **Strict guest memory checks** (`runtime/src/guest_flat_memory.cpp`): the MP9
  build stops on the first unmapped guest access with a diagnostic instead of
  fabricating zero-filled RAM. Keep this behavior; never disable it to get
  past a fault.
- **ROM-free launcher kit** (`android/`): the public APK carries only the
  launcher, tools, and runtime kit. Each player's private translated game
  library is built locally from their own dump and imported on-device through
  the normal verifying importer. The private `.wcgame` package must never be
  distributed.
- **Upstream reuse**: Aurora graphics, OpenXR/Vulkan presentation, SDL3,
  and the generic PowerPC translator are used as-is wherever compatible.

## Requirements

- Meta Quest 3 with USB debugging authorized.
- Your own `SSQE01` disc dump (RVZ/ISO accepted by the local tooling).
- Android SDK + NDK 29, JDK 17+, .NET 8 SDK, CMake/Ninja (see
  `android/README.md` and `docs/quest-port.md` for the upstream tooling baseline).

## Building

The documented end-to-end flow (private translation → ROM-free APK audit →
install → verified private-package import → boot-log capture) is orchestrated
by the maintainer scripts; at the CMake/Gradle level the MP9 product is
selected with `-DMKW_GAME_MP9=ON`, the generated translation directory, and
the MP9 disc manifest:

```powershell
.\gradlew.bat assembleModernQuestDebug `
    '-PmkwGeneratedDir=<your-generated-native-boot>' `
    '-Pmp9DiscManifest=<supported-disc.json>'
```

The build audits the APK for bundled game content (`libmain*.so`, DOL/REL/
RVZ/ISO/WBFS, translated objects) and fails if any is found.

## Current status and roadmap

Working: APK build/audit/install, OpenXR session + passthrough, verified
private-package import, native OS interrupt/RTC/SRAM/SI/audio-device/memory/
IPC/IOS56/video/GX/Aurora/DVD-FST/scheduler/drive-inquiry/WPAD/KPAD/AI/AX-DSP
startup, on-device encoder unit tests.

Not working yet: actual board/minigame scenes, immersive MP9 cameras,
complete motion input verification, dynamic REL module loading, world-fixed
menus with recenter/calibration, online lobbies/profiles/stats, and the
public end-to-end BYO-ROM generation flow.

Crashes are deliberately loud instead of quietly swallowed. Bug reports should
include the boot log and the crash snapshot from the app's
`WiiCompiledOpenXRVR/Logs` folder.

## FAQ

**Is this an emulator?**
No. Everything is compiled to native code before you ever press play.

**Do you provide the game?**
No. Don't ask. Nothing in this repo contains Nintendo code or assets.

**Which game version works?**
USA `SSQE01` only. Other regions and modified executables are rejected.

**Is it done?**
No. See the status section above — it boots native code but shows no
playable scene yet.

## Credits

- [iChris4/Wiicompiled_VR](https://github.com/iChris4/Wiicompiled_VR) — the
  upstream project this fork builds on (GPL-3.0).
- [patchzyy/Wiicompiled](https://github.com/patchzyy/Wiicompiled) — the
  translator this work reuses.
- [aurora](https://github.com/encounter/aurora) — the GX rendering/windowing
  backend. MIT licensed.
- [Dawn](https://dawn.googlesource.com/dawn) — Google's WebGPU
  implementation used by aurora's backends.
- [OpenXR](https://www.khronos.org/openxr/) — the cross-platform API used by
  the VR renderer.
- [Dolphin Emulator](https://github.com/dolphin-emu/dolphin) — reference for
  Wii hardware behavior, plus the free DSP coefficient ROM and the unmodified
  default WiiConnect24 bootstrap tree bundled with the runtime.
- [mariopartyrd/marioparty9](https://github.com/mariopartyrd/marioparty9) —
  symbol reference used to verify SSQE01 addresses.

Bundled third-party components and their licenses live in
[`THIRD-PARTY-NOTICES.md`](THIRD-PARTY-NOTICES.md) where present upstream.

## License

Free software under the terms of the
[GNU General Public License, version 3](LICENSE) as published by the Free
Software Foundation, same as upstream.

Distributed WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the license for
details.

Not affiliated with, endorsed by, or associated with Nintendo. Mario Party 9
is a trademark of Nintendo. No Nintendo intellectual property is contained
in, distributed with, or obtainable through this project.
