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
