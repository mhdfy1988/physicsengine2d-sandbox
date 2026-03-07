param(
    [switch]$Build,
    [string]$BuildDir = "Build_mingw",
    [string]$Version = "1.0.0"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
Set-Location $repoRoot

$buildRoot = Join-Path $repoRoot $BuildDir
$distRoot = Join-Path $repoRoot "dist"
$stageRoot = Join-Path $distRoot "physics_core"
$packageRoot = Join-Path $stageRoot ("physics_core_" + $Version)
$zipPath = Join-Path $stageRoot ("physics_core_" + $Version + ".zip")
$exampleSource = Join-Path $repoRoot "examples\standalone_core_example.cpp"
$kernelLib = Join-Path $buildRoot "lib\libphysics_kernel.a"
$runtimeSupportLib = Join-Path $buildRoot "lib\libphysics_runtime_support.a"
$metadataSource = Join-Path $repoRoot "artifacts\build\build_metadata.json"
$exampleExe = Join-Path $packageRoot "examples\standalone_core_example.exe"

if ($Build) {
    Write-Host "[1/6] Building standalone core targets..."
    $env:CC = "C:\msys64\ucrt64\bin\gcc.exe"
    $env:CXX = "C:\msys64\ucrt64\bin\g++.exe"
    & cmake -S . -B $BuildDir -G "MinGW Makefiles" -DBUILD_TESTS=ON | Out-Null
    & cmake --build $BuildDir --target physics_kernel -- -j4
    & cmake --build $BuildDir --target physics_runtime_support -- -j4
}

if (-not (Test-Path $kernelLib)) {
    throw "Missing kernel library: $kernelLib"
}
if (-not (Test-Path $runtimeSupportLib)) {
    throw "Missing runtime support library: $runtimeSupportLib"
}

Write-Host "[2/6] Preparing package folder..."
if (Test-Path $packageRoot) {
    Remove-Item -Recurse -Force $packageRoot
}
New-Item -ItemType Directory -Force -Path $packageRoot | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $packageRoot "lib") | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $packageRoot "examples") | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $packageRoot "docs") | Out-Null

Copy-Item -Recurse -Force (Join-Path $repoRoot "include") (Join-Path $packageRoot "include")
Copy-Item -Force $kernelLib (Join-Path $packageRoot "lib\libphysics_kernel.a")
Copy-Item -Force $runtimeSupportLib (Join-Path $packageRoot "lib\libphysics_runtime_support.a")
Copy-Item -Force $exampleSource (Join-Path $packageRoot "examples\standalone_core_example.cpp")
Copy-Item -Force (Join-Path $repoRoot "docs\architecture\PHYSICS_CORE_BOUNDARY.md") (Join-Path $packageRoot "docs\PHYSICS_CORE_BOUNDARY.md")
if (Test-Path $metadataSource) {
    Copy-Item -Force $metadataSource (Join-Path $packageRoot "build_metadata.json")
} else {
    & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $repoRoot "scripts\dev\emit_build_metadata.ps1") `
        -Target "physics_core" -Configuration "standalone" -Version $Version -OutputPath (Join-Path $packageRoot "build_metadata.json") | Out-Null
}

$readmeText = @"
Physics Core Standalone Package

Contents:
- lib/libphysics_kernel.a
- lib/libphysics_runtime_support.a
- include/
- examples/standalone_core_example.cpp
- docs/PHYSICS_CORE_BOUNDARY.md
- build_metadata.json

Build example:
  g++ -std=c++17 examples/standalone_core_example.cpp -I include lib/libphysics_runtime_support.a lib/libphysics_kernel.a -o examples/standalone_core_example.exe -lm
"@
Set-Content -Path (Join-Path $packageRoot "README.txt") -Value $readmeText -Encoding UTF8

Write-Host "[3/6] Building packaged example only against packaged files..."
& "C:\msys64\ucrt64\bin\g++.exe" `
    "-std=c++17" `
    (Join-Path $packageRoot "examples\standalone_core_example.cpp") `
    "-I" (Join-Path $packageRoot "include") `
    (Join-Path $packageRoot "lib\libphysics_runtime_support.a") `
    (Join-Path $packageRoot "lib\libphysics_kernel.a") `
    "-o" $exampleExe `
    "-lm"

Write-Host "[4/6] Running packaged example..."
& $exampleExe

Write-Host "[5/6] Creating zip..."
if (-not (Test-Path $stageRoot)) {
    New-Item -ItemType Directory -Force -Path $stageRoot | Out-Null
}
if (Test-Path $zipPath) {
    Remove-Item -Force $zipPath
}
Compress-Archive -Path (Join-Path $packageRoot "*") -DestinationPath $zipPath -CompressionLevel Optimal

Write-Host "[6/6] Done"
Write-Host "Standalone core package: $zipPath"
