param(
    [switch]$Update
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$headers = @(
    (Join-Path $root "include\physics.h"),
    (Join-Path $root "include\physics_ext.h"),
    (Join-Path $root "include\physics_world.h")
)
$baseline = Join-Path $root "docs\api_surface_baseline.txt"

function Get-ApiSurface {
    param([string[]]$HeaderPaths)
    $lines = New-Object System.Collections.Generic.List[string]
    foreach ($h in $HeaderPaths) {
        if (!(Test-Path $h)) { continue }
        Get-Content $h | ForEach-Object {
            $line = $_.Trim()
            if ($line -match '^[A-Za-z_].*\(.*\);$' -and
                $line -notmatch '^\s*typedef\s+' -and
                $line -notmatch '\(\*') {
                $norm = ($line -replace '\s+', ' ').Trim()
                $lines.Add($norm)
            }
        }
    }
    return $lines | Sort-Object -Unique
}

$current = Get-ApiSurface -HeaderPaths $headers
if ($Update) {
    $current | Set-Content -Path $baseline -Encoding UTF8
    Write-Host "API surface baseline updated: $baseline"
    exit 0
}

if (!(Test-Path $baseline)) {
    Write-Host "[FAIL] API baseline missing: $baseline"
    Write-Host "Run: powershell -ExecutionPolicy Bypass -File .\\scripts\\check_api_surface.ps1 -Update"
    exit 1
}

$expected = Get-Content $baseline
$diff = Compare-Object -ReferenceObject $expected -DifferenceObject $current
if ($diff) {
    Write-Host "[FAIL] Public API surface changed."
    $diff | ForEach-Object { Write-Host $_ }
    exit 1
}

Write-Host "API surface check passed."
