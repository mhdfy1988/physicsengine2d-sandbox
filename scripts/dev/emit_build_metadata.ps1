param(
    [string]$Target = "editor",
    [string]$Configuration = "Debug",
    [string]$Version = "1.0.0",
    [string]$OutputPath = "artifacts/build/build_metadata.json"
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
Set-Location $root

$outputAbs = if ([System.IO.Path]::IsPathRooted($OutputPath)) { $OutputPath } else { Join-Path $root $OutputPath }
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $outputAbs) | Out-Null

$commit = "unknown"
try {
    $commit = (git rev-parse --short HEAD).Trim()
} catch {
    $commit = "unknown"
}

$compiler = "unknown"
try {
    $compiler = ((g++ --version) | Select-Object -First 1).Trim()
} catch {
    $compiler = "unknown"
}

$meta = [ordered]@{
    version = $Version
    target = $Target
    configuration = $Configuration
    commit = $commit
    build_time_utc = (Get-Date).ToUniversalTime().ToString("yyyy-MM-ddTHH:mm:ssZ")
    platform = "windows"
    compiler = $compiler
}

$meta | ConvertTo-Json -Depth 3 | Set-Content -Path $outputAbs -Encoding UTF8
Write-Host $outputAbs
