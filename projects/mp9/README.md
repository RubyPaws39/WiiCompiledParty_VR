# MP9 maintainer tools

Source-only scripts used by the private native Quest development build. These are
maintainer workspace tools, not an end-user ROM importer or a complete setup wizard.
No ROM, generated Nintendo C++, game library, save, or private package belongs here.

## Workspace layout

Scripts currently target `R:\MarioParty9VR`. Stage this directory's files into
`R:\MarioParty9VR\work`, preserving subdirectories, and keep this repository at
`R:\MarioParty9VR\work\Wiicompiled_VR`. Adjust tool paths for your SDK, NDK,
JDK, Python, .NET, and caches. Public copies use `R:\Python\python.exe` and require
`ANDROID_SERIAL` for installation/deployment; no maintainer device identifier is included.

The existing private workspace must already contain a verified owned SSQE01 dump,
extracted DATA, decoded modules, symbol reference, function map, and native-probe
project/disc manifests. Those private inputs and setup artifacts are not supplied.
The public APK's end-to-end on-device native code generation remains unfinished.

## Rebuild sequence

1. `Recompile-PrivateMp9.ps1` regenerates both C++ **and matching build metadata**,
   lowers custom task continuations, recompiles registered RELs, validates source
   hashes, merges metadata, and emits native shards. Source and call metadata must
   never come from different translation runs.
2. Add `-RepackShards` after substantial new module coverage if frozen address cuts
   produce an oversized final shard. The previous map is backed up before repacking.
3. `Build-Mp9Apk.ps1 -Install` builds/audits the ROM-free prototype and explicitly
   builds the separate private native library. A launcher-only build is insufficient.
4. `Deploy-PrivateQuestGame.ps1 -LogName quest-NAME-boot` imports the private package
   using the normal verifier, preserves installed disc data, and captures boot logs.
   Do not redirect all PowerShell streams with `*>` around this deployment script;
   adb progress on stderr can be treated as an error by PowerShell.

All translated outputs and `.wcgame` packages stay private. These scripts do not make
the current game playable; see [status](../../docs/MP9_STATUS.md) and [handoff](../../docs/MP9_HANDOFF.md).

## Locked-cache DMA regression

`Test-Mp9LcSprDma.cpp` links the production `runtime/src/ppc_helpers.cpp` with checked synthetic memory. Compile with the Android NDK using `-std=c++20 -O2 -ffunction-sections -fdata-sections -DMKW_GAME_MP9 -Iruntime/include -Wl,--gc-sections -static-libstdc++`, then run the ARM64 executable through adb. Old runtime fails the first store assertion; repaired runtime passes load/store, count encoding, zero-length encoding, cache wrap, flush/trigger behavior and invalid-range rejection.

## Developer controller and fresh captures

`Drive-Mp9DebugInput.py` is an opt-in local debugging tool for the debuggable MP9 APK. It writes app-private `debug-tools/wii-input.txt` using adb run-as; normal KPAD handles buttons and pointing. Button holds expire after at most two seconds; the input override expires one second later, even if its command file remains. `--disable` deletes the command file immediately. Replay always disables the override on exit. `--capture` requests the next rendered game frame, deletes the old device image first, and waits for a complete new BMP; a stalled guest raises an error instead of returning an old image.

Example: `python Drive-Mp9DebugInput.py --buttons 0xc00 --press-ms 300` presses Wii A+B. Pointer coordinates range from -1 to 1; right/down are positive. `--capture --output R:/MarioParty9VR/work/frame.bmp` captures game output. These tools are temporary developer support, not gameplay controls or an end-user importer.

`Replay-Mp9Solo.py` sends ordinary inputs and verifies each menu header against local private reference captures before continuing. `--characters` stops at character selection; `--from-main` and `--from-characters` resume verified menu routes. Header edges tolerate restored background graphics; references remain private captures. Reference BMPs listed in the script remain local and are not distributed; a fresh maintainer workspace must capture its own references. Matching headers verifies menus reached, not successful gameplay after Start.
