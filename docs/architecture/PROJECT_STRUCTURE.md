# Project Structure

## Overview

```text
Forge2D/
|-- .github/
|   `-- workflows/
|       `-- regression.yml
|-- apps/
|   |-- runtime_cli/
|   |   `-- main.cpp
|   `-- sandbox_dwrite/
|       |-- app_icon.rc
|       |-- main.cpp
|       |-- application/
|       |-- domain/
|       |-- infrastructure/
|       `-- presentation/
|-- assets/
|   |-- fonts/
|   `-- icons/
|-- docs/
|   |-- api/
|   |-- architecture/
|   |-- archive/
|   |-- phases/
|   |-- reports/
|   `-- todos/
|-- examples/
|   `-- standalone_core_example.cpp
|-- include/
|   |-- physics2d/
|   |-- physics_content/
|   |-- physics_core/
|   |-- physics_runtime/
|   |-- physics.h
|   |-- physics.hpp
|   |-- physics_ext.h
|   |-- physics_ext.hpp
|   |-- physics_world.h
|   `-- physics_world.hpp
|-- installer/
|   `-- physics_sandbox.iss
|-- samples/
|   `-- physics_sandbox_project/
|       |-- Assets/
|       |-- Build/
|       |-- Cache/
|       |-- Packages/
|       |-- Prefabs/
|       |-- ProjectSettings/
|       `-- Scenes/
|-- scripts/
|   |-- check/
|   |-- dev/
|   |-- gate/
|   `-- package/
|-- src/
|   |-- c_api/
|   |-- content/
|   |-- core/
|   |-- internal/
|   |-- physics2d/
|   |-- runtime/
|   `-- physics_internal.hpp
|-- tests/
|   |-- content/
|   |-- data/
|   |-- editor/
|   |-- equivalence/
|   |-- kernel/
|   |-- runtime/
|   |-- smoke/
|   |-- REGRESSION_TEST_TEMPLATE.md
|   |-- benchmark_suite.cpp
|   |-- invariant_tests.cpp
|   |-- regression_tests.cpp
|   `-- regression_tests.hpp
|-- tools/
|   |-- parallel_benchmark_compare.cpp
|   |-- phase_d_profile_capture.cpp
|   |-- scene_migrate_main.cpp
|   `-- subsystem_workflow_demo.cpp
|-- .gitignore
|-- CHANGELOG.md
|-- CMakeLists.txt
|-- CORE_DEV_GUIDE.md
|-- Makefile
|-- QUICKSTART.md
`-- README.md
```

## Notes

- The repository root contains source code, apps, scripts, tests, docs, and packaging assets.
- The built-in sample project lives under `samples/physics_sandbox_project`.
- Public headers are now grouped by domain under `include/physics_core`, `include/physics_content`, and `include/physics_runtime`.
- Top-level `include/` keeps only umbrella entry points and the `physics2d` compatibility family.
- Build outputs such as `Build/`, `Build_mingw/`, `bin/`, `lib/`, `obj/`, and `dist/` are intentionally excluded from this structure summary.
