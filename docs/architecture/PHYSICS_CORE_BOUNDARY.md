# Physics Core Boundary

Date: 2026-03-06

## Reusable Core

The standalone physics-core boundary is:

- `src/physics2d/*`
- `src/core/*`
- `src/c_api/*`
- `src/runtime_support/snapshot_repo.cpp`
- build targets:
  - `physics_kernel`
  - `physics_runtime_support`

These targets are intended for reuse outside the sandbox editor.

## Public Headers

Core-only public entry points:

- `include/physics.h`
- `include/physics_core/physics.h`
- `include/physics_core/foundation.hpp`
- `include/physics_core/raii.hpp`
- `include/physics2d/physics_foundation.hpp`
- `include/physics2d/physics_raii.hpp`
- `include/physics2d/physics_world_raii.hpp`
- `include/physics2d/physics_body_raii.hpp`
- `include/physics2d/physics_snapshot.hpp`
- `include/physics2d/physics_status.hpp`
- `include/runtime_snapshot_repo.hpp`

Broader engine/runtime headers that are not part of the minimal standalone surface:

- `include/physics2d/physics2d.hpp`
- `include/physics2d/physics_ecs.hpp`
- `include/physics2d/physics_runtime_facade.hpp`
- `include/physics2d/physics_script_bridge.hpp`
- content/project/prefab/asset/editor/session headers under `include/`

## Layer Split

- `physics_kernel`: low-level physics engine, C API, core runtime primitives
- `physics_runtime_support`: reusable non-UI helpers built on top of the kernel
- `physics_content`: project, prefab, asset, diagnostic, session, and editor-support systems
- `physics_sandbox`: editor/UI application layer

## Downstream Guidance

For standalone integration:

1. link `physics_kernel`
2. optionally link `physics_runtime_support` if snapshot text save/load is needed
3. include headers from `include/physics_core/*` or the listed core-safe `include/physics2d/*` headers
4. avoid `physics2d` umbrella target if you do not want content systems pulled in
