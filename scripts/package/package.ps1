param(
    [switch]$Build,
    [switch]$WithInstaller,
    [string]$Version = "1.0.0",
    [ValidateSet("editor", "runtime")]
    [string]$Flavor = "editor"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
Set-Location $repoRoot
$sampleProjectRoot = Join-Path $repoRoot "samples\\physics_sandbox_project"
$buildOutputRoot = Join-Path $repoRoot "Build_mingw\\bin"
$mingwBin = Split-Path (Get-Command g++).Source

$exeName = if ($Flavor -eq "runtime") { "physics_runtime_cli.exe" } else { "physics_sandbox.exe" }
$exeCandidates = @(
    (Join-Path $repoRoot ("bin\" + $exeName)),
    (Join-Path $buildOutputRoot $exeName)
)
$exePath = $exeCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
$distRoot = Join-Path $repoRoot "dist"
$portableRoot = Join-Path $distRoot ("PhysicsSandbox_" + $Flavor)
$portableBinRoot = Join-Path $portableRoot "bin"
$zipPath = Join-Path $distRoot ("PhysicsSandbox_" + $Flavor + "_portable_" + $Version + ".zip")

if ($Build) {
    Write-Host "[1/4] Building sandbox..."
    if ($Flavor -eq "runtime") {
        & "C:\msys64\ucrt64\bin\mingw32-make.exe" runtime-cli
    } else {
        & "C:\msys64\ucrt64\bin\mingw32-make.exe" sandbox
    }
}

if (-not $exePath) {
    throw "Missing executable: $exeName. Build first or pass -Build."
}

Write-Host "[2/4] Preparing portable folder..."
if (Test-Path $portableRoot) {
    Remove-Item -Recurse -Force $portableRoot
}
New-Item -ItemType Directory -Force -Path $portableRoot | Out-Null
New-Item -ItemType Directory -Force -Path $portableBinRoot | Out-Null

Copy-Item -Force $exePath (Join-Path $portableBinRoot $exeName)
foreach ($dllName in @("libgcc_s_seh-1.dll", "libstdc++-6.dll")) {
    $dllPath = Join-Path $mingwBin $dllName
    if (Test-Path $dllPath) {
        Copy-Item -Force $dllPath (Join-Path $portableBinRoot $dllName)
    }
}
if (Test-Path "assets") {
    Copy-Item -Recurse -Force "assets" (Join-Path $portableRoot "assets")
}
if (Test-Path $sampleProjectRoot) {
    $portableSampleRoot = Join-Path $portableRoot "samples\\physics_sandbox_project"
    New-Item -ItemType Directory -Force -Path $portableSampleRoot | Out-Null
    & robocopy $sampleProjectRoot $portableSampleRoot /E /XD Build Cache | Out-Null
    if ($LASTEXITCODE -gt 7) {
        throw "robocopy sample project failed: $LASTEXITCODE"
    }
}
if (Test-Path "sandbox_ui.ini") {
    Copy-Item -Force "sandbox_ui.ini" (Join-Path $portableBinRoot "sandbox_ui.ini")
}
& powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $repoRoot "scripts\dev\emit_build_metadata.ps1") `
    -Target $Flavor -Configuration "portable" -Version $Version -OutputPath (Join-Path $portableRoot "build_metadata.json") | Out-Null

$readmeText = @"
Physics Sandbox Portable

Run:
  bin\$exeName

Notes:
- Keep the folder structure unchanged.
- The exe lives under bin, and runtime files are written next to it.
- The bundled sample project content comes from samples/physics_sandbox_project.
- This build targets Windows 10/11.
- Build metadata is written to build_metadata.json.
"@
Set-Content -Path (Join-Path $portableRoot "README.txt") -Value $readmeText -Encoding UTF8

Write-Host "[3/4] Creating zip..."
if (-not (Test-Path $distRoot)) {
    New-Item -ItemType Directory -Force -Path $distRoot | Out-Null
}
if (Test-Path $zipPath) {
    Remove-Item -Force $zipPath
}
Compress-Archive -Path (Join-Path $portableRoot "*") -DestinationPath $zipPath -CompressionLevel Optimal
Write-Host "Portable package: $zipPath"

if ($WithInstaller) {
    Write-Host "[4/4] Building installer..."
    $iss = Join-Path $repoRoot "installer\\physics_sandbox.iss"
    if (-not (Test-Path $iss)) {
        throw "Missing Inno Setup script: $iss"
    }

    $isccCandidates = @(@(
        $env:INNO_SETUP_COMPILER,
        "C:\\Program Files (x86)\\Inno Setup 6\\ISCC.exe",
        "C:\\Program Files\\Inno Setup 6\\ISCC.exe"
    ) | Where-Object { $_ -and (Test-Path $_) })

    if ($isccCandidates.Length -eq 0) {
        throw "ISCC.exe not found. Install Inno Setup or set INNO_SETUP_COMPILER."
    }

    $iscc = $isccCandidates[0]
    & $iscc "/DAppVersion=$Version" $iss
    Write-Host "Installer output folder: dist"
}
