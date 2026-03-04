# Physics2D Engine Architecture

This document defines the current architecture baseline and coding boundaries.

## Layers

1. Public API layer (`include/physics.h`, `src/physics.c`)
2. World state + internal aggregation (`src/physics_internal.h`)
3. Internal subsystem interfaces (`src/internal/*.h`)
4. Pipeline implementations (`src/physics_*`)
5. Math/geometry primitives (`src/math.c`, `src/shape.c`, `src/body.c`, `src/collision.c`, `src/constraint.c`)

## Pipeline Stages

`physics_internal_step` runs the frame in explicit stages:

1. Integrate (`physics_internal_update_velocities(engine, dt)`, `physics_internal_update_positions(engine, dt)`)
2. Broadphase (`build_pairs` op)
3. Narrowphase/contact generation (`build_contacts` op)
4. Solve (`physics_internal_resolve_collisions` with `PhysicsSolverContext`)
5. Clear forces (`physics_internal_clear_forces`)

Each stage is profiled into `PhysicsStepProfile`.

## Configuration

`PhysicsConfig` is the single source of runtime tuning (sanitized by one path in `physics.c`):

- `time_step`, `substeps`, `iterations`, `max_position_iterations_bias`
- `damping`
- `broadphase_type`, `broadphase_cell_size`
- reserved toggles for future systems: `ccd_enabled`, `sleep_enabled`, `threading_enabled`, `worker_count`

Use `physics_engine_set_config/get_config` instead of writing internals directly.

## Memory Model

Per-frame temporary allocations must use scratch arena:

- `physics_internal_scratch_reset`
- `physics_internal_scratch_alloc`

No per-step heap allocations in hot path.

`step` substeps must pass `dt` explicitly to subsystems. Stage code must not mutate shared config (`engine->config.time_step`) during a frame.

## Identity / Handles

Internal identity types are centralized in `src/internal/physics_ids.h`:

- `PhysicsBodyId`
- `PhysicsConstraintId`
- `PhysicsContactId`

Conversion helpers in `src/physics_ids.c` are the only allowed ptr<->id bridge.

## Extension Points

Pipeline replacement is done via ops in world state:

- `PhysicsBroadphaseOps`
- `PhysicsNarrowphaseOps`

Default bindings are installed by `physics_internal_bind_default_pipeline`.
Public registration APIs map to those ops:

- `physics_engine_set_broadphase_builder`
- `physics_engine_set_narrowphase_builder`
- `physics_engine_reset_pipeline`

Solver execution goes through `physics_internal_parallel_for`. The current backend is serial with deterministic chunk dispatch, which keeps call semantics stable for a future worker-thread backend.

Runtime events are unified in `PhysicsCallbacks`:

- frame hooks: `on_pre_step`, `on_post_step`
- stage hooks: `on_post_broadphase`, `on_post_narrowphase`
- contact hook: `on_contact_created`

## Dependency Rules

1. Public headers must not include internal headers.
2. Internal modules must communicate via `src/internal/*.h` interfaces where possible.
3. `src/physics_step.c` orchestrates stages; stage implementations do not call back into step.
4. New subsystems should be added as new modules plus ops/context, not by growing monolithic files.

## Testing Rules

Any change touching pipeline/config/memory must pass:

- `mingw32-make clean`
- `mingw32-make sandbox`
- `mingw32-make test`
