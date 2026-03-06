param(
    [string]$OutputDir = ""
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($OutputDir)) {
    $stamp = Get-Date -Format "yyyyMMdd_HHmmss"
    $OutputDir = Join-Path $root ("artifacts\phase_h_gate_suite_" + $stamp)
}
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

$makeExeCandidate = "C:\msys64\ucrt64\bin\mingw32-make.exe"
$makeExe = if (Test-Path $makeExeCandidate) { $makeExeCandidate } else { "mingw32-make" }

function Invoke-Gate {
    param(
        [string]$Name,
        [string]$FilePath,
        [string[]]$Arguments
    )
    $stdout = Join-Path $OutputDir ($Name + ".stdout.log")
    $stderr = Join-Path $OutputDir ($Name + ".stderr.log")
    $p = Start-Process -FilePath $FilePath -ArgumentList $Arguments -WorkingDirectory $root -NoNewWindow `
        -RedirectStandardOutput $stdout -RedirectStandardError $stderr -PassThru -Wait
    [PSCustomObject]@{
        name = $Name
        exit_code = $p.ExitCode
        ok = ($p.ExitCode -eq 0)
        stdout = $stdout
        stderr = $stderr
    }
}

$results = @()
$results += Invoke-Gate -Name "gate_test" -FilePath $makeExe -Arguments @("test")
$results += Invoke-Gate -Name "gate_sandbox" -FilePath $makeExe -Arguments @("sandbox")
$results += Invoke-Gate -Name "gate_runtime_cli_build" -FilePath $makeExe -Arguments @("runtime-cli")
$results += Invoke-Gate -Name "gate_runtime_cli_run" -FilePath "powershell" -Arguments @("-NoProfile","-ExecutionPolicy","Bypass","-Command","& { & .\bin\physics_runtime_cli.exe --steps 4 }")
$results += Invoke-Gate -Name "gate_arch_deps" -FilePath "powershell" -Arguments @("-NoProfile","-ExecutionPolicy","Bypass","-File","scripts/check_arch_deps.ps1")
$results += Invoke-Gate -Name "gate_api_surface" -FilePath "powershell" -Arguments @("-NoProfile","-ExecutionPolicy","Bypass","-File","scripts/check_api_surface.ps1")
$results += Invoke-Gate -Name "gate_cpp_convergence" -FilePath "powershell" -Arguments @("-NoProfile","-ExecutionPolicy","Bypass","-File","scripts/check_cpp_convergence.ps1")
$results += Invoke-Gate -Name "gate_build_metadata" -FilePath "powershell" -Arguments @("-NoProfile","-ExecutionPolicy","Bypass","-File","scripts/emit_build_metadata.ps1","-Target","editor","-Configuration","gate","-OutputPath","artifacts/build/build_metadata.json")
$results += Invoke-Gate -Name "gate_release_matrix" -FilePath "powershell" -Arguments @("-NoProfile","-ExecutionPolicy","Bypass","-File","scripts/run_phase_h_release_matrix.ps1","-Version","1.0.0","-OutputRoot",(Join-Path $OutputDir "release_matrix"))

$summaryPath = Join-Path $OutputDir "summary.md"
$lines = @("# Phase H Gate Suite Summary")
$lines += ""
$lines += ("Date: " + (Get-Date -Format "yyyy-MM-dd HH:mm:ss"))
$lines += ""
$lines += "| Gate | Exit | Status | Stdout | Stderr |"
$lines += "|---|---:|---|---|---|"
foreach ($r in $results) {
    $status = if ($r.ok) { "PASS" } else { "FAIL" }
    $stdoutRel = Resolve-Path -LiteralPath $r.stdout -Relative
    $stderrRel = Resolve-Path -LiteralPath $r.stderr -Relative
    $lines += "| $($r.name) | $($r.exit_code) | $status | $stdoutRel | $stderrRel |"
}
$lines | Set-Content -Path $summaryPath -Encoding UTF8

if (($results | Where-Object { -not $_.ok }).Count -gt 0) {
    Write-Host ("[FAIL] phase H gate suite failed. Summary: " + $summaryPath)
    exit 1
}
Write-Host ("[PASS] phase H gate suite passed. Summary: " + $summaryPath)
