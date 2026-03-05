param(
    [string]$OutputDir = ""
)

$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($OutputDir)) {
    $stamp = Get-Date -Format "yyyyMMdd_HHmmss"
    $OutputDir = Join-Path $root ("artifacts\phase_c_gate_suite_" + $stamp)
}
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

$makeExe = "C:\msys64\ucrt64\bin\mingw32-make.exe"

function Invoke-Gate {
    param(
        [string]$Name,
        [string]$FilePath,
        [string[]]$Arguments
    )

    $stdout = Join-Path $OutputDir ($Name + ".stdout.log")
    $stderr = Join-Path $OutputDir ($Name + ".stderr.log")

    Write-Host ("[RUN] " + $Name)
    $p = Start-Process -FilePath $FilePath `
                       -ArgumentList $Arguments `
                       -WorkingDirectory $root `
                       -NoNewWindow `
                       -RedirectStandardOutput $stdout `
                       -RedirectStandardError $stderr `
                       -PassThru `
                       -Wait
    $ok = ($p.ExitCode -eq 0)
    Write-Host ("[{0}] {1} (exit={2})" -f ($(if ($ok) { "PASS" } else { "FAIL" }), $Name, $p.ExitCode))

    return [PSCustomObject]@{
        name = $Name
        file = $FilePath
        args = ($Arguments -join " ")
        exit_code = $p.ExitCode
        ok = $ok
        stdout = $stdout
        stderr = $stderr
    }
}

$results = @()
$results += Invoke-Gate -Name "gate_test" -FilePath $makeExe -Arguments @("test")
$results += Invoke-Gate -Name "gate_sandbox" -FilePath $makeExe -Arguments @("sandbox")
$results += Invoke-Gate -Name "gate_benchmark" -FilePath $makeExe -Arguments @("benchmark")
$results += Invoke-Gate -Name "gate_arch_deps" -FilePath "powershell" -Arguments @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", "scripts/check_arch_deps.ps1")
$results += Invoke-Gate -Name "gate_api_surface" -FilePath "powershell" -Arguments @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", "scripts/check_api_surface.ps1")
$results += Invoke-Gate -Name "gate_hot_reload_smoke" -FilePath "powershell" -Arguments @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", "scripts/hot_reload_smoke.ps1")

$summaryPath = Join-Path $OutputDir "summary.md"
$lines = @()
$lines += "# Phase C Gate Suite Summary"
$lines += ""
$lines += "Date: " + (Get-Date -Format "yyyy-MM-dd HH:mm:ss")
$lines += "Root: " + $root
$lines += ""
$lines += "| Gate | Exit | Status | Stdout | Stderr |"
$lines += "|---|---:|---|---|---|"
foreach ($r in $results) {
    $status = if ($r.ok) { "PASS" } else { "FAIL" }
    $stdoutRel = Resolve-Path -LiteralPath $r.stdout -Relative
    $stderrRel = Resolve-Path -LiteralPath $r.stderr -Relative
    $lines += "| " + $r.name + " | " + $r.exit_code + " | " + $status + " | " + $stdoutRel + " | " + $stderrRel + " |"
}
$lines | Set-Content -Path $summaryPath -Encoding UTF8

$failed = $results | Where-Object { -not $_.ok }
if ($failed.Count -gt 0) {
    Write-Host ("[FAIL] phase C gate suite failed. Summary: " + $summaryPath)
    exit 1
}

Write-Host ("[PASS] phase C gate suite passed. Summary: " + $summaryPath)
