# Phase F Progress

Date: 2026-03-06  
Branch: `cpp-migration-baseline`  
Status: Complete

## Completed

1. Migrated remaining tool sources to C++:
   - `tools/parallel_benchmark_compare.cpp`
   - `tools/phase_d_profile_capture.cpp`
   - `tools/subsystem_workflow_demo.cpp`
2. Migrated remaining content sources to C++:
   - `src/content/*.cpp`
3. Migrated sandbox/editor implementation sources to C++:
   - `apps/sandbox_dwrite/**/*.cpp`
   - `apps/sandbox_dwrite/infrastructure/com_cpp_compat.hpp`
4. Migrated remaining regression, smoke, and benchmark test sources to C++:
   - `tests/**/*.cpp`
5. Removed the Phase E retained-C allowlist and tightened the convergence gate.
6. Updated Makefile and CMakeLists to use C++ compile/link paths for the migrated sources.

## Current Source-State Snapshot

1. `src/`, `apps/`, `tools/`, and `tests/` contain zero project-owned `.c` files.
2. The convergence check now fails if any new project-owned `.c` file is introduced.

## Verification Summary

1. `mingw32-make test`
2. `mingw32-make sandbox`
3. `mingw32-make benchmark`
4. `mingw32-make parallel-benchmark-compare`
5. `mingw32-make subsystem-workflow-demo`
6. `mingw32-make phase-d-profile`
7. `mingw32-make scene-migrate`
8. `powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check/check_arch_deps.ps1`
9. `powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check/check_api_surface.ps1`
10. `powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check/check_cpp_convergence.ps1`
11. `cmake -S . -B build_phase_f -G "MinGW Makefiles" -DBUILD_TESTS=ON`
12. `cmake --build build_phase_f --target physics_tests physics_sandbox physics_app_runtime_smoke physics_editor_undo_redo_smoke physics_editor_pie_lifecycle_smoke physics_subsystem_render_audio_animation_smoke parallel_benchmark_compare subsystem_workflow_demo phase_d_profile_capture scene_migrate`
