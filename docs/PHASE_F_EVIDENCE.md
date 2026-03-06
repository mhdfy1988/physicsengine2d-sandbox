# Phase F Evidence Pack

Date: 2026-03-06  
Scope: full project-owned source migration from `.c` to `.cpp`

## Source Migration Evidence

1. Tools migrated to `.cpp`.
2. Content pipeline modules migrated to `.cpp`.
3. Sandbox/editor implementation modules migrated to `.cpp`.
4. Regression, smoke, and benchmark tests migrated to `.cpp`.

## Gate Evidence

1. `scripts/check_cpp_convergence.ps1` now enforces zero project-owned `.c` files.
2. `rg --files -g "*.c"` under `src/`, `apps/`, `tools/`, and `tests/` returns no results.

## Build-System Evidence

1. `Makefile` no longer compiles project-owned `.c` test or tool targets.
2. `CMakeLists.txt` now declares the project as `CXX RC` and builds migrated targets from `.cpp`.
3. Win32 editor and smoke targets use `apps/sandbox_dwrite/infrastructure/com_cpp_compat.hpp` to preserve legacy COM call sites during the migration.
