param(
    [switch]$SkipTests
)

$ErrorActionPreference = "Stop"

function Assert-FileExists {
    param([string]$PathToCheck)
    if (-not (Test-Path $PathToCheck)) {
        throw "Missing required file: $PathToCheck"
    }
}

function Assert-ChangelogHasToday {
    param([string]$ChangelogPath)
    $today = (Get-Date).ToString("yyyy-MM-dd")
    $content = Get-Content $ChangelogPath -Raw
    if ($content -notmatch [regex]::Escape($today)) {
        throw "CHANGELOG.md does not contain today's date entry: $today"
    }
}

Write-Host "[check] verifying core maintenance files..."
Assert-FileExists "CHANGELOG.md"
Assert-FileExists "CORE_DEV_GUIDE.md"
Assert-FileExists "tests/REGRESSION_TEST_TEMPLATE.md"

Write-Host "[check] verifying changelog has today's entry..."
Assert-ChangelogHasToday "CHANGELOG.md"

if (-not $SkipTests) {
    Write-Host "[check] running regression tests..."
    & "C:\msys64\ucrt64\bin\mingw32-make.exe" test
    if ($LASTEXITCODE -ne 0) {
        throw "Regression test run failed."
    }
}

Write-Host "[check] core change check passed."
