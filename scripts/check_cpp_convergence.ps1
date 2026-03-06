param()

$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$allowlistPath = Join-Path $root "docs\phase_e_c_retention_allowlist.txt"

if (!(Test-Path $allowlistPath)) {
    Write-Host "[FAIL] C retention allowlist missing: $allowlistPath"
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

$allowlist = New-Object "System.Collections.Generic.HashSet[string]" ([System.StringComparer]::OrdinalIgnoreCase)
foreach ($rawLine in Get-Content -Path $allowlistPath) {
    $line = $rawLine.Trim()
    if ([string]::IsNullOrWhiteSpace($line)) { continue }
    if ($line.StartsWith("#")) { continue }
    [void]$allowlist.Add($line.Replace('\', '/'))
}

$currentFiles = Get-ChildItem -Path (Join-Path $root "src"), (Join-Path $root "apps"), (Join-Path $root "tools") `
    -File -Recurse -Filter *.c | ForEach-Object { Normalize-RelativePath $_.FullName }

$currentSet = New-Object "System.Collections.Generic.HashSet[string]" ([System.StringComparer]::OrdinalIgnoreCase)
foreach ($path in $currentFiles) {
    [void]$currentSet.Add($path)
}

$failed = $false

foreach ($path in ($currentSet | Sort-Object)) {
    if (-not $allowlist.Contains($path)) {
        Write-Host "[FAIL] New C implementation path outside Phase E retention allowlist: $path"
        $failed = $true
    }
}

foreach ($path in ($allowlist | Sort-Object)) {
    if (-not $currentSet.Contains($path)) {
        Write-Host "[FAIL] Stale allowlist entry not found in repo: $path"
        $failed = $true
    }
}

if ($failed) {
    Write-Host "C++ convergence check failed."
    exit 1
}

Write-Host ("C++ convergence check passed. Allowed retained C implementation paths: {0}" -f $currentSet.Count)
