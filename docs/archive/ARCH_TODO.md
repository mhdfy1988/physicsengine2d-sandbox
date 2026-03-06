# Physics Kernel Architecture TODO

Execution policy: strict priority order, architecture only, backward-compatible migration first.

1. [x] P1: Introduce `World/Scene` boundary with world-first API migration layer
2. [x] P2: Split public API into stable (`physics.hpp`) and extension (`physics_ext.hpp`) layers
3. [x] P3: Add versioned plugin ABI contract (`api_version`, capabilities, init/shutdown, fallback)
4. [x] P4: Add unified error model (error code + diagnostics channel)
5. [x] P5: Add job-system abstraction (submit/wait/fence) above `parallel_for`
6. [x] P6: Split config into runtime/solver/pipeline profiles + frozen step snapshot
7. [x] P7: Complete handle/id model with generation-safe references
8. [x] P8: Enforce dependency boundaries in CI (include graph + forbidden deps)
9. [x] P9: Add unified trace/event stream with compile-time cost control
10. [x] P10: Add compatibility policy (semver, deprecation windows, API diff gate)

## Current migration notes

- `include/physics_world.hpp` provides world-first naming (`PhysicsWorld`, `PhysicsScene`) as a compatibility layer.
- `include/physics_ext.hpp` owns advanced/experimental APIs and is included by `physics.hpp` unless `PHYSICS_NO_EXTENSIONS` is set.
- Existing `physics_engine_*` API remains intact to avoid integration breakage.
