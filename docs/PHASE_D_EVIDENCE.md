# Phase D Evidence Pack

Date: 2026-03-06  
Scope: D1 closure evidence (hot-reload taxonomy + headless gate)

## 1. Gate Commands

1. `mingw32-make test`
   - Result: PASS
   - Key line: `Result: PASS (49/49)` + `app_runtime_smoke PASS`
2. `mingw32-make sandbox`
   - Result: PASS
3. `mingw32-make benchmark`
   - Result: PASS
   - Key line: `[INFO] benchmark avg_step_ms=0.2283 gate=8.0000`
4. `powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check_arch_deps.ps1`
   - Result: PASS
   - Key line: `Architecture dependency checks passed.`
5. `powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check_api_surface.ps1`
   - Result: PASS
   - Key line: `API surface check passed.`
6. `powershell -NoProfile -ExecutionPolicy Bypass -File scripts/hot_reload_smoke.ps1`
   - Result: PASS
   - Key line: `[PASS] hot-reload smoke passed`
7. `powershell -NoProfile -ExecutionPolicy Bypass -File scripts/hot_reload_smoke_headless.ps1`
   - Result: PASS
   - Key line: `[PASS] hot-reload headless smoke passed`
8. `powershell -NoProfile -ExecutionPolicy Bypass -File scripts/run_phase_d_gate_suite.ps1`
   - Result: PASS
   - Key line: `[PASS] phase D gate suite passed.`

## 2. Hot-Reload Taxonomy Evidence

1. App runtime error code extension:
   - `apps/sandbox_dwrite/infrastructure/app_event_bus.h`
   - Added:
     - `APP_RUNTIME_ERROR_CODE_HOT_RELOAD_SCAN_FAILED`
     - `APP_RUNTIME_ERROR_CODE_HOT_RELOAD_IMPORT_FAILED`
     - `APP_RUNTIME_ERROR_CODE_HOT_RELOAD_BATCH_FAILED`
2. Sandbox error emission path:
   - `apps/sandbox_dwrite/main.c`
   - Scan failure -> `hot_reload_scan_failed`
   - Import failure count -> `hot_reload_import_failed`
   - Batch pipeline failure -> `hot_reload_batch_failed`
3. Runtime semantics doc update:
   - `docs/RUNTIME_ERROR_SEMANTICS.md`

## 3. Headless Smoke Evidence

1. Headless smoke script:
   - `scripts/hot_reload_smoke_headless.ps1`
2. One-click D gate runner:
   - `scripts/run_phase_d_gate_suite.ps1`
3. Archived summary snapshot:
   - `artifacts/phase_d_gate_suite_20260306_010737/summary.md`
4. Log archive pattern:
   - `artifacts/phase_d_gate_suite_20260306_010737/gate_*.stdout.log`
   - `artifacts/phase_d_gate_suite_20260306_010737/gate_*.stderr.log`

## 4. Runtime Regression Evidence

1. Added taxonomy propagation assertion:
   - `tests/app_runtime_tick_smoke.c`
2. Existing hot-reload regressions remain green:
   - `tests/regression_asset_hot_reload_tests.c`
   - Includes:
     - missing-source failure contract
     - failed reimport keeps previous artifact
     - rewatch does not swallow change

## 5. Commit Evidence

1. `e5104bf`
   - Win32 file-watch trigger + poll fallback integration.
2. `19a55fc`
   - Hot-reload taxonomy + headless smoke + D gate runner.
