param(
    [int]$StartupWaitMs = 2500,
    [int]$StepTimeoutSec = 12,
    [int]$SecondStageAttempts = 6,
    [switch]$SkipLaunch,
    [switch]$KeepArtifacts
)

$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$sandboxExe = Join-Path $root "bin\physics_sandbox.exe"
$smokeDir = Join-Path $root "assets\hot_reload_smoke"
$smokeSource = Join-Path $smokeDir "smoke_texture.png"
$script:SmokeWriteSeq = 0

function Get-AssetGuidFromPath {
    param([string]$SourcePath)
    [UInt64]$hash = 1469598103934665603
    $prime = [System.Numerics.BigInteger]1099511628211
    $mod64 = [System.Numerics.BigInteger]::Pow(2, 64)
    $bytes = [System.Text.Encoding]::UTF8.GetBytes($SourcePath)
    foreach ($b in $bytes) {
        $hash = [UInt64]($hash -bxor [UInt64]$b)
        $hashBig = [System.Numerics.BigInteger]$hash
        $hashBig = ($hashBig * $prime) % $mod64
        $hash = [UInt64]$hashBig
    }
    return ("asset://{0:x16}" -f $hash)
}

function Read-FileText {
    param([string]$Path)
    if (!(Test-Path $Path)) {
        return $null
    }
    return [System.IO.File]::ReadAllText($Path)
}

function Read-FileBytes {
    param([string]$Path)
    if (!(Test-Path $Path)) {
        return $null
    }
    return [System.IO.File]::ReadAllBytes($Path)
}

function Write-FileBytes {
    param(
        [string]$Path,
        [byte[]]$Bytes
    )
    if ($null -eq $Bytes) {
        return
    }
    [System.IO.File]::WriteAllBytes($Path, $Bytes)
}

function Delete-FileIfExists {
    param([string]$Path)
    if ([string]::IsNullOrWhiteSpace($Path)) {
        return
    }
    if ([System.IO.File]::Exists($Path)) {
        [System.IO.File]::Delete($Path)
    }
}

function Wait-ArtifactContentChange {
    param(
        [string]$ArtifactPath,
        [string]$PreviousContent,
        [int]$TimeoutSec
    )
    $deadline = (Get-Date).AddSeconds($TimeoutSec)
    while ((Get-Date) -lt $deadline) {
        $content = Read-FileText -Path $ArtifactPath
        if ($null -ne $content -and $content -ne $PreviousContent) {
            return $content
        }
        Start-Sleep -Milliseconds 200
    }
    return $null
}

function Write-SmokeContent {
    param(
        [string]$Path,
        [string]$Tag
    )
    $script:SmokeWriteSeq++
    $padding = ("#" * $script:SmokeWriteSeq)
    $payload = "smoke-$Tag-" + [DateTimeOffset]::UtcNow.ToUnixTimeMilliseconds().ToString() + "-" + $padding
    [System.IO.File]::WriteAllBytes($Path, [System.Text.Encoding]::UTF8.GetBytes($payload))
}

$relSourcePath = "assets/hot_reload_smoke/smoke_texture.png"
$guid = Get-AssetGuidFromPath -SourcePath $relSourcePath
$guidSuffix = $guid.Substring(8)
$artifactPath = Join-Path $root ("Cache\imported\{0}.asset" -f $guidSuffix)
$metaPath = "$smokeSource.meta"
$historyPath = Join-Path $root "history_00.txt"
$tracePath = Join-Path $root "spawn_trace.log"
$sandboxUiPath = Join-Path $root "sandbox_ui.ini"

$smokeDirExisted = Test-Path $smokeDir
$smokeSourceBefore = Read-FileBytes -Path $smokeSource
$metaBefore = Read-FileBytes -Path $metaPath
$artifactBeforeSnapshot = Read-FileBytes -Path $artifactPath
$historyExisted = Test-Path $historyPath
$traceExisted = Test-Path $tracePath
$sandboxUiBefore = Read-FileBytes -Path $sandboxUiPath

Write-Host "[INFO] source:   $relSourcePath"
Write-Host "[INFO] guid:     $guid"
Write-Host "[INFO] artifact: $artifactPath"
Write-Host "[INFO] meta:     $metaPath"

    $proc = $null
try {
    if ($SkipLaunch) {
        Write-Host "[STEP] SkipLaunch enabled; smoke execution is skipped (preflight only)"
        Write-Host "[PASS] hot-reload smoke preflight passed"
        return
    }
    if (!(Test-Path $sandboxExe)) {
        throw "sandbox executable missing: $sandboxExe"
    }
    if (!(Test-Path $smokeDir)) {
        New-Item -ItemType Directory -Path $smokeDir | Out-Null
    }
    Write-SmokeContent -Path $smokeSource -Tag "bootstrap"
    $proc = Start-Process -FilePath $sandboxExe -WorkingDirectory $root -PassThru
    Write-Host "[STEP] launched sandbox pid=$($proc.Id), waiting $StartupWaitMs ms"
    Start-Sleep -Milliseconds $StartupWaitMs

    $artifactBefore = Read-FileText -Path $artifactPath
    Write-SmokeContent -Path $smokeSource -Tag "v1"
    Write-Host "[STEP] wrote v1 content, waiting for artifact content change..."
    $firstArtifact = Wait-ArtifactContentChange -ArtifactPath $artifactPath -PreviousContent $artifactBefore -TimeoutSec $StepTimeoutSec
    if ($null -eq $firstArtifact) {
        throw "artifact content did not change after v1 update within ${StepTimeoutSec}s"
    }
    Write-Host "[OK] artifact content changed after v1"

    $secondArtifact = $null
    Write-Host "[STEP] attempting v2 updates (up to $SecondStageAttempts attempts)..."
    for ($i = 1; $i -le $SecondStageAttempts; $i++) {
        Write-SmokeContent -Path $smokeSource -Tag ("v2-" + $i.ToString())
        $secondArtifact = Wait-ArtifactContentChange -ArtifactPath $artifactPath -PreviousContent $firstArtifact -TimeoutSec ([Math]::Max(2, [int]($StepTimeoutSec / [Math]::Max(1, $SecondStageAttempts))))
        if ($null -ne $secondArtifact) {
            Write-Host "[OK] artifact content changed on attempt $i"
            break
        }
        Write-Host "[WARN] no change observed on attempt $i"
    }
    if ($null -eq $secondArtifact) {
        throw "artifact content did not change after v2 attempts"
    }
    Write-Host "[OK] artifact content changed again after v2 stage"

    Write-Host "[PASS] hot-reload smoke passed"
}
catch {
    Write-Host "[FAIL] $($_.Exception.Message)"
    exit 1
}
finally {
    if ($null -ne $proc -and !$proc.HasExited) {
        try {
            $null = $proc.CloseMainWindow()
            if (!$proc.WaitForExit(2000)) {
                $proc.Kill()
                $proc.WaitForExit()
            }
        }
        catch {
        }
    }
    if (!$KeepArtifacts) {
        try {
            if ($null -ne $smokeSourceBefore) {
                Write-FileBytes -Path $smokeSource -Bytes $smokeSourceBefore
            } else {
                Delete-FileIfExists -Path $smokeSource
            }
            if ($null -ne $metaBefore) {
                Write-FileBytes -Path $metaPath -Bytes $metaBefore
            } else {
                Delete-FileIfExists -Path $metaPath
            }
            if ($null -ne $artifactBeforeSnapshot) {
                Write-FileBytes -Path $artifactPath -Bytes $artifactBeforeSnapshot
            } else {
                Delete-FileIfExists -Path $artifactPath
            }
            if (-not $historyExisted) {
                Delete-FileIfExists -Path $historyPath
            }
            if (-not $traceExisted) {
                Delete-FileIfExists -Path $tracePath
            }
            if ($null -ne $sandboxUiBefore) {
                Write-FileBytes -Path $sandboxUiPath -Bytes $sandboxUiBefore
            }
            if (-not $smokeDirExisted -and [System.IO.Directory]::Exists($smokeDir)) {
                $entries = [System.IO.Directory]::GetFileSystemEntries($smokeDir)
                if ($entries.Length -eq 0) {
                    [System.IO.Directory]::Delete($smokeDir, $false)
                }
            }
        }
        catch {
        }
    }
}
