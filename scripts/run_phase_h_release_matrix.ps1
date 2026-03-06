param(
    [string]$Version = "1.0.0",
    [string]$OutputRoot = ""
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($OutputRoot)) {
    $stamp = Get-Date -Format "yyyyMMdd_HHmmss"
    $OutputRoot = Join-Path $root ("artifacts\phase_h_release_matrix_" + $stamp)
}
New-Item -ItemType Directory -Force -Path $OutputRoot | Out-Null

$cmake = "C:\Program Files\CMake\bin\cmake.exe"
if (!(Test-Path $cmake)) {
    throw "Missing cmake: $cmake"
}

$matrix = @(
    @{ name = "editor_debug";  type = "editor";  build_type = "Debug";         target = "physics_sandbox";     build_editor = "ON"  },
    @{ name = "editor_release"; type = "editor"; build_type = "Release";       target = "physics_sandbox";     build_editor = "ON"  },
    @{ name = "runtime_debug"; type = "runtime"; build_type = "Debug";         target = "physics_runtime_cli"; build_editor = "OFF" },
    @{ name = "runtime_release"; type = "runtime"; build_type = "Release";     target = "physics_runtime_cli"; build_editor = "OFF" },
    @{ name = "runtime_profile"; type = "runtime"; build_type = "RelWithDebInfo"; target = "physics_runtime_cli"; build_editor = "OFF" }
)

$summary = @()
foreach ($entry in $matrix) {
    $buildDir = Join-Path $root ("build_" + $entry.name)
    $artifactDir = Join-Path $OutputRoot $entry.name
    New-Item -ItemType Directory -Force -Path $artifactDir | Out-Null
    & $cmake -S $root -B $buildDir -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=$($entry.build_type) -DBUILD_TESTS=OFF -DBUILD_EDITOR=$($entry.build_editor) | Out-Null
    & $cmake --build $buildDir --target $entry.target | Out-Null
    $binName = if ($entry.type -eq "runtime") { "physics_runtime_cli.exe" } else { "physics_sandbox.exe" }
    Copy-Item -Force (Join-Path $buildDir ("bin\" + $binName)) (Join-Path $artifactDir $binName)
    & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $root "scripts\emit_build_metadata.ps1") `
        -Target $entry.type -Configuration $entry.build_type -Version $Version -OutputPath (Join-Path $artifactDir "build_metadata.json") | Out-Null
    $summary += "| $($entry.name) | PASS | $binName | .\$(Resolve-Path -LiteralPath (Join-Path $artifactDir 'build_metadata.json') -Relative) |"
}

$summaryPath = Join-Path $OutputRoot "summary.md"
$lines = @("# Phase H Release Matrix Summary")
$lines += ""
$lines += ("Date: " + (Get-Date -Format "yyyy-MM-dd HH:mm:ss"))
$lines += ""
$lines += "| Artifact | Status | Binary | Metadata |"
$lines += "|---|---|---|---|"
$lines += $summary
$lines | Set-Content -Path $summaryPath -Encoding UTF8
Write-Host $summaryPath
