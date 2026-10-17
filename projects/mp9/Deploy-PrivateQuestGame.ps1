# Private developer deployment; uses normal package verifier and preserves disc data.
[CmdletBinding()]
param([string]$LogName = 'quest-latest-boot')
$ErrorActionPreference = 'Stop'
if ($LogName -notmatch '^[a-zA-Z0-9_-]+$') { throw 'LogName must be a plain filename stem' }
$root = 'R:\MarioParty9VR'
$adb = 'R:\SonicRidersQuest\sdk\platform-tools\adb.exe'
$serial = $env:ANDROID_SERIAL
if (-not $serial) { throw 'Set ANDROID_SERIAL to your authorized Quest device' }
function Invoke-Adb {
    & $adb -s $serial @args
    if ($LASTEXITCODE -ne 0) { throw "adb failed: $args" }
}
& "$root\work\Prepare-PrivateQuestGame.ps1" -LibraryOnly
Invoke-Adb push "$root\private-game\mp9-native-boot.wcgame" /data/local/tmp/mp9-native-boot.wcgame
Invoke-Adb shell am force-stop org.marioparty9.quest
Invoke-Adb logcat -c
Invoke-Adb shell run-as org.marioparty9.quest cp /data/local/tmp/mp9-native-boot.wcgame files/Import/mp9-native-boot.wcgame
Invoke-Adb shell am start -W -n org.marioparty9.quest/org.wiicompiled.quest.launcher.LauncherActivity
Start-Sleep -Seconds 5
$importLog = Invoke-Adb logcat -d -s WiiCompiledLauncher
$importLog | Set-Content "$root\work\$LogName-import.log"
if (($importLog -join "`n") -notmatch 'Imported') { throw "Check package import: work/$LogName-import.log" }
Invoke-Adb shell am start -W -n org.marioparty9.quest/org.wiicompiled.quest.QuestActivity
Start-Sleep -Seconds 10
Invoke-Adb logcat -d -s WiiCompiled AndroidRuntime SDL | Set-Content "$root\work\$LogName.log"
Write-Output "Boot captured: $root\work\$LogName.log"
