# This package contains the user's private game. Never distribute it with the APK.
[CmdletBinding()]
param([switch]$LibraryOnly)
$ErrorActionPreference = 'Stop'
$root = 'R:\MarioParty9VR'
$repo = "$root\work\Wiicompiled_VR"
$kit = "$repo\android\app\build\generated\assets\questGameKit\modernQuestDebug\game_kit"
$descriptor = Get-Content "$root\work\native-probe\supported-disc.json" -Raw | ConvertFrom-Json
$recipe = Get-Content "$kit\kit.json" -Raw | ConvertFrom-Json
if ($recipe.gameId -ne $descriptor.game_id -or -not $recipe.nativeBootOnly) {
    throw 'Expected the MP9 native boot kit, not a Mario Kart kit'
}
$candidates = Get-ChildItem "$repo\android\app\build\intermediates\cxx\Debug" -Recurse -Filter libmain.so |
    Sort-Object LastWriteTime -Descending
$source = $candidates | Select-Object -First 1
if (-not $source) { throw 'No private game library from the launcher CMake build' }
$library = "$root\private-game\libmain-mp9-kit.so"
$strip = 'R:\SonicRidersQuest\sdk\ndk\29.0.14206865\toolchains\llvm\prebuilt\windows-x86_64\bin\llvm-strip.exe'
& $strip --strip-debug -o $library $source.FullName
if ($LASTEXITCODE -ne 0) { throw 'Private library stripping failed' }
Import-Module "$repo\android\QuestGameKit.psm1" -Force
$data = if ($LibraryOnly) { '' } else { "$root\private-game\extracted\DATA" }
$manifest = New-QuestGamePackage -Library $library -DataDir $data `
    -KitDir $kit -GameId $descriptor.game_id -DolSha256 $descriptor.dol_sha256 `
    -RelSha256 $descriptor.code_manifest_sha256 -BuiltBy 'MP9 private native prototype: partial REL coverage, not playable' `
    -OutputPath "$root\private-game\mp9-native-boot.wcgame"
$manifest | ConvertTo-Json | Set-Content "$root\private-game\private-package-report.json" -Encoding utf8
Write-Output 'Private MP9 package created. Importer will verify disc data, kit identity, and library hash.'
