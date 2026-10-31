# ROM-free development launcher. Private translated game libraries stay separate.
[CmdletBinding()]
param([switch]$Install)
$ErrorActionPreference = 'Stop'
$root = 'R:\MarioParty9VR'
$repo = "$root\work\Wiicompiled_VR"
$env:JAVA_HOME = 'R:\Android\jbr'
$env:ANDROID_HOME = 'R:\SonicRidersQuest\sdk'
$env:ANDROID_SDK_ROOT = $env:ANDROID_HOME
$env:GRADLE_USER_HOME = 'R:\SonicRidersQuest\cache\gradle'
$env:CARGO_HOME = 'R:\Rust\cargo'
$env:RUSTUP_HOME = 'R:\Rust\rustup'
$env:TEMP = "$root\work\temp"
$env:TMP = $env:TEMP
$env:PATH = "$env:ANDROID_HOME\cmake\3.22.1\bin;R:\Rust\cargo\bin;$env:PATH"
@('sdk.dir=R:/SonicRidersQuest/sdk', 'cmake.dir=R:/SonicRidersQuest/sdk/cmake/3.31.4') |
    Set-Content "$repo\android\local.properties" -Encoding ascii
$deps = "$root\work\android-dependencies"
New-Item -ItemType Directory -Force $deps | Out-Null
Get-ChildItem "$root\work\mp9-runtime-build\_deps" -Directory -Filter '*-src' | ForEach-Object {
    $name = $_.Name -replace '-src$', ''
    if (-not (Test-Path "$deps\$name")) {
        New-Item -ItemType Junction -Path "$deps\$name" -Target $_.FullName | Out-Null
    }
}
Push-Location "$repo\android"
try {
    & .\gradlew.bat assembleModernQuestDebug `
        '-PmkwGeneratedDir=R:/MarioParty9VR/private-game/generated-native-boot' `
        '-PmkwDependenciesDir=R:/MarioParty9VR/work/android-dependencies' `
        '-Pmp9DiscManifest=R:/MarioParty9VR/work/native-probe/supported-disc.json' `
        --max-workers=4 --console=plain
    if ($LASTEXITCODE -ne 0) { throw 'MP9 launcher build failed' }
} finally { Pop-Location }
# Gradle builds the ROM-free kit probe only. Explicitly build the private
# game target too, so deployment cannot silently pick a stale libmain.so.
$nativeGraph = Get-ChildItem "$repo\android\app\.cxx\Debug\*\arm64-v8a\build.ninja" |
    Sort-Object LastWriteTime -Descending | Select-Object -First 1
if (-not $nativeGraph) { throw 'No configured Quest native build graph' }
& "$env:ANDROID_HOME\cmake\3.31.4\bin\ninja.exe" -C $nativeGraph.DirectoryName -j 4 WiiCompiled
if ($LASTEXITCODE -ne 0) { throw 'Private MP9 native game library build failed' }
$apk = "$repo\android\app\build\outputs\apk\modernQuest\debug\app-modernQuest-debug.apk"
Add-Type -AssemblyName System.IO.Compression.FileSystem
$zip = [IO.Compression.ZipFile]::OpenRead($apk)
try {
    $forbidden = @($zip.Entries | Where-Object {
        $_.FullName -match '(^|/)libmain[^/]*\.so$|\.(dol|rel|rvz|iso|wbfs)$|data_sections_init|guest_symbol_table|base_common|base_registration'
    })
    if ($forbidden.Count) { throw "Game content found in APK: $($forbidden.FullName -join ', ')" }
    if (-not $zip.GetEntry('assets/game_kit/kit.json')) { throw 'Missing runtime kit identity' }
    if ($zip.GetEntry('assets/quest_toolchain/files.zip')) { throw 'Unsupported Mario Kart toolchain found in MP9 APK' }
} finally { $zip.Dispose() }
Write-Output "ROM-free MP9 prototype APK verified: $apk"
if ($Install) {
    if (-not $env:ANDROID_SERIAL) { throw 'Set ANDROID_SERIAL to your authorized Quest device' }
    & "$env:ANDROID_HOME\platform-tools\adb.exe" -s $env:ANDROID_SERIAL install -r $apk
    if ($LASTEXITCODE -ne 0) { throw 'APK installation failed' }
}
