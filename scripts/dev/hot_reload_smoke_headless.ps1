param(
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"

$root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$testsExe = Join-Path $root "bin\physics_tests.exe"
$makeExeCandidate = "C:\msys64\ucrt64\bin\mingw32-make.exe"
$makeExe = if (Test-Path $makeExeCandidate) { $makeExeCandidate } else { "mingw32-make" }

function Invoke-Cmd {
    param(
        [string]$FilePath,
        [string[]]$Arguments
    )
    & $FilePath @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw ("command failed: {0} {1} (exit={2})" -f $FilePath, ($Arguments -join " "), $LASTEXITCODE)
    }
}

if (-not $SkipBuild) {
    Write-Host "[STEP] building test binaries (headless)..."
    Invoke-Cmd -FilePath $makeExe -Arguments @("test")
}

if (-not (Test-Path $testsExe)) {
    throw "missing test executable: $testsExe"
}

Write-Host "[STEP] running physics_tests for hot-reload headless assertions..."
$outputLines = & $testsExe 2>&1
if ($LASTEXITCODE -ne 0) {
    $outputLines | ForEach-Object { Write-Host $_ }
    throw "physics_tests failed in headless smoke"
}

$requiredMarkers = @(
    "[PASS] asset hot-reload service pipeline bridge",
    "[PASS] asset fs poll source bridge",
    "[PASS] asset hot-reload missing-source failure",
    "[PASS] asset hot-reload failure keeps previous artifact",
    "[PASS] asset fs poll rewatch does not swallow change",
    "Result: PASS"
)

$missing = @()
$allText = ($outputLines -join "`n")
foreach ($marker in $requiredMarkers) {
    if ($allText.IndexOf($marker, [System.StringComparison]::Ordinal) -lt 0) {
        $missing += $marker
    }
}

if ($missing.Count -gt 0) {
    $outputLines | ForEach-Object { Write-Host $_ }
    throw ("hot-reload headless smoke missing markers: " + ($missing -join ", "))
}

Write-Host "[PASS] hot-reload headless smoke passed"
