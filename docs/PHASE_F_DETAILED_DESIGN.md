# Phase F Detailed Design

Date: 2026-03-06  
Branch: `cpp-migration-baseline`

## Goal

Phase F closes the temporary migration window left by Phase E and moves the project-owned implementation tree to a pure C++ source baseline.

## In Scope

1. Migrate all project-owned `.c` files under `src/`, `apps/`, `tools/`, and `tests/` to `.cpp`.
2. Remove the Phase E retained-C allowlist mechanism.
3. Make both Makefile and CMake build the migrated tree through C++ compiler/link paths.
4. Keep sandbox, regression, smoke, benchmark, and migration tools green after the rename wave.

## Out of Scope

1. Third-party SDK headers and COM interface naming conventions.
2. Project / Workspace / Package system work.
3. Prefab conflict UI and plugin extension work.
4. Release engineering and crash recovery work.

## Work Breakdown

### F1 Tools

1. Rename remaining tool entry points to `.cpp`.
2. Build them with `g++` / C++ linker language only.

### F2 Content

1. Rename remaining `src/content/*.c` files to `.cpp`.
2. Keep asset pipeline, watcher, and schema regressions passing.

### F3 Editor / App

1. Rename sandbox/editor implementation files to `.cpp`.
2. Add a C++ compatibility shim for legacy Win32/D2D/DWrite COM call sites so migration does not require a full UI rewrite in the same batch.

### F4 Tests and Gates

1. Rename remaining test sources to `.cpp`.
2. Replace the retained-C allowlist with a zero-tolerance convergence check.
3. Re-run Makefile and CMake verification after the final rename wave.

## Completion Criteria

Phase F is complete when all of the following are true:

1. No project-owned `.c` source remains under `src/`, `apps/`, `tools/`, or `tests/`.
2. `scripts/check_cpp_convergence.ps1` passes by enforcing zero `.c` files instead of checking an allowlist.
3. Makefile targets and CMake targets compile and link through C++ paths for migrated project code.
4. Core quality gates remain green.
