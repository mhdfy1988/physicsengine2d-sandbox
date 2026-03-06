# Phase H Acceptance

Date: 2026-03-06  
Status: Complete

## Checklist

- [x] Build metadata export writes version, commit, build time, platform, and compiler.
- [x] Release matrix script emits `editor_debug`, `editor_release`, `runtime_debug`, `runtime_release`, and `runtime_profile`.
- [x] Runtime product exists as a separate executable (`physics_runtime_cli`).
- [x] Sandbox supports exporting a minimal diagnostic bundle.
- [x] Session state persists clean vs. unclean shutdown markers.
- [x] Startup recovery can restore from the last autosave after unclean shutdown.
- [x] Replay-oriented runtime entry works without the editor UI.
- [x] `mingw32-make test` passes with Phase H regression coverage.
- [x] `mingw32-make sandbox` passes with session/diagnostic integration.
- [x] `mingw32-make runtime-cli` passes.
- [x] `scripts/check_arch_deps.ps1` passes.
- [x] `scripts/check_api_surface.ps1` passes.
- [x] `scripts/check_cpp_convergence.ps1` passes.
- [x] `scripts/run_phase_h_release_matrix.ps1` passes.
- [x] `scripts/run_phase_h_gate_suite.ps1` passes.
- [x] Phase H evidence and final report produced.
