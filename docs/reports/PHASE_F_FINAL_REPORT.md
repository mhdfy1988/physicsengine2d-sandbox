# Phase F Final Report

Date: 2026-03-06  
Branch: `cpp-migration-baseline`

## Summary

Phase F is complete.

This phase closed the remaining project-owned C source inventory and removed the temporary retained-C policy introduced in Phase E. The codebase now builds its project-owned implementation sources from C++ translation units across `src/`, `apps/`, `tools/`, and `tests/`.

## Delivered

1. Remaining project-owned `.c` files were migrated to `.cpp`.
2. Sandbox/editor modules were kept buildable under C++ by adding `com_cpp_compat.hpp` for legacy COM-style call sites.
3. Makefile targets for tests, benchmarks, and smoke executables now compile and link through `g++`.
4. CMake was updated to a `CXX RC` project baseline and aligned with the migrated source list.
5. The Phase E retained-C allowlist was removed and replaced by a zero-tolerance convergence gate.

## Quality Gates

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

## Notes

1. This closure is about project-owned source language convergence. Third-party C-facing SDK headers may still exist at the integration boundary.
2. Phase G completed later with project/workspace/package persistence, prefab semantics, and editor plugin lifecycle coverage; the next active roadmap item is Phase H.
