param()

$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$legacyAllowlistPath = Join-Path $root "docs\phase_e_c_retention_allowlist.txt"
if (Test-Path $legacyAllowlistPath) {
    Write-Host "[FAIL] Legacy Phase E C retention allowlist still exists: $legacyAllowlistPath"
    exit 1
}

function Normalize-RelativePath {
    param([string]$FullPath)

    $rootPrefix = ($root.TrimEnd('\') + '\')
    if ($FullPath.StartsWith($rootPrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
        $relative = $FullPath.Substring($rootPrefix.Length)
    } else {
        $resolved = Resolve-Path -LiteralPath $FullPath
        $relative = $resolved.Path
    }
    return $relative.Replace('\', '/')
}

$scanRoots = @(
    Join-Path $root "src"
    Join-Path $root "apps"
    Join-Path $root "tools"
    Join-Path $root "tests"
)

$cFiles = Get-ChildItem -Path $scanRoots -File -Recurse -Filter *.c |
    ForEach-Object { Normalize-RelativePath $_.FullName } |
    Sort-Object

if ($cFiles.Count -gt 0) {
    foreach ($path in $cFiles) {
        Write-Host "[FAIL] Project-owned C source remains: $path"
    }
    Write-Host "C++ convergence check failed."
    exit 1
}

Write-Host "C++ convergence check passed. No project-owned .c sources remain under src/, apps/, tools/, or tests/."
