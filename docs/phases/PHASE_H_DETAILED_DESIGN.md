# Phase H Detailed Design

Date: 2026-03-06  
Status: Complete

## Scope

Phase H closes the final master-design gap:

1. build matrix and artifact metadata,
2. minimal diagnostic bundle export,
3. session recovery after unclean shutdown,
4. replay-oriented runtime entry for failure analysis.

## Deliverables

1. Release engineering
   - `scripts/dev/emit_build_metadata.ps1`
   - `scripts/gate/run_phase_h_release_matrix.ps1`
   - `scripts/package/package.ps1`
   - `apps/runtime_cli/main.cpp`
2. Runtime/editor persistence
   - `include/session_recovery.hpp`
   - `src/content/session_recovery.cpp`
   - `ProjectSettings/editor_session.physicssession`
3. Diagnostics
   - `include/diagnostic_bundle.hpp`
   - `src/content/diagnostic_bundle.cpp`
   - sandbox export entry in `apps/sandbox_dwrite/main.cpp`
4. Gate closure
   - `scripts/gate/run_phase_h_gate_suite.ps1`

## Rules

1. Every packaged artifact must carry version, commit hash, build time, platform, and compiler metadata.
2. The release matrix must cover:
   - `editor_debug`
   - `editor_release`
   - `runtime_debug`
   - `runtime_release`
   - `runtime_profile`
3. Session recovery uses an explicit clean/unclean marker file rather than heuristic-only detection.
4. Recovery must be restorable from the last autosave snapshot when the previous editor exit was unclean.
5. Diagnostic export must produce a minimal self-contained folder with metadata, selected files, and recent logs.
6. Replay entry must be available without the editor UI.

## Verification Strategy

1. Regression coverage for session manifest roundtrip and restore gating.
2. Regression coverage for diagnostic bundle export.
3. `runtime_cli` build/run smoke.
4. Release matrix script execution with archived artifact summary.
