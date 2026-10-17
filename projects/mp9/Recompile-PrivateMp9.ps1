# Rebuild translated code from the privately extracted supported disc. Never distribute its output.
[CmdletBinding()]
param([switch]$RepackShards)
$ErrorActionPreference = 'Stop'
$root = 'R:\MarioParty9VR'
$dotnet = "$root\work\dotnet\dotnet.exe"
$translator = "$root\work\Wiicompiled_VR\translator\src\Translator.Cli\bin\Release\net8.0\Translator.Cli.dll"
$python = 'R:\Python\python.exe'
$manifest = "$root\work\native-probe\mp9-native-boot.yml"
$generated = "$root\private-game\generated-native-boot"
& $python "$root\work\prepare-runtime-probe.py"
if ($LASTEXITCODE -ne 0) { throw 'SDK symbol validation failed' }
& $dotnet $translator translate-recursive 0x80004050 --project $manifest --threads 4 --output-metadata "$generated\base_translation_output.json"
if ($LASTEXITCODE -ne 0) { throw 'Private DOL translation failed' }
& $dotnet $translator generate-data-init --project $manifest --target-os android
if ($LASTEXITCODE -ne 0) { throw 'Private DOL data initialization failed' }
& $python "$root\work\Lower-Mp9TaskContexts.py" $root
if ($LASTEXITCODE -ne 0) { throw 'Private MP9 task continuation lowering failed' }
$relManifests = @(Get-ChildItem "$root\private-game\generated-rel-aot\*\module.json" -ErrorAction SilentlyContinue)
if ($relManifests.Count) {
    $env:DOTNET_CLI_HOME = "$root\work\dotnet-home"
    $env:DOTNET_CLI_TELEMETRY_OPTOUT = '1'
    $env:DOTNET_SKIP_FIRST_TIME_EXPERIENCE = '1'
    foreach ($relManifest in $relManifests) {
        $moduleName = (Get-Content $relManifest.FullName -Raw | ConvertFrom-Json).name
        & $dotnet run --project "$root\work\Mp9RelAot\Mp9RelAot.csproj" --configuration Release -- $root $moduleName
        if ($LASTEXITCODE -ne 0) { throw "Private REL AOT failed: $moduleName" }
    }
    & $python "$root\work\Merge-Mp9RelAot.py"
    if ($LASTEXITCODE -ne 0) { throw 'Private REL AOT merge failed' }
}
# New RELs fall above the frozen address cuts and can otherwise collect in
# one enormous shard. Repack explicitly when adding a module; keep the old map.
$shardMap = "$generated\build_shards\base_common_shard_map.json"
if ($RepackShards -and (Test-Path -LiteralPath $shardMap)) {
    $shardBackup = "$root\work\base_common_shard_map.$(Get-Date -Format yyyyMMdd-HHmmss).json"
    Move-Item -LiteralPath $shardMap -Destination $shardBackup
}
& $dotnet $translator emit-build-shards --base-metadata "$generated\base_translation_output.json" `
    --base-functions-dir "$generated\functions" --out "$generated\build_shards" `
    --native-source-dir "$root\work\Wiicompiled_VR\ports\mp9"
if ($LASTEXITCODE -ne 0) { throw 'Private DOL shard generation failed' }
# Shard custom rules do not scan the generated native-call headers. A changed
# SDK binding must rebuild every caller, even when its C++ text is unchanged.
$shardStamp = Get-Date
Get-ChildItem "$generated\build_shards" -Recurse -Filter '*.cpp' |
    ForEach-Object { $_.LastWriteTime = $shardStamp }
