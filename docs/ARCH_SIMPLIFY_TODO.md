# Architecture Simplification TODO

1. [x] Remove duplicate pipeline extension entrypoints  
`plugin ABI` is primary; builder setters are kept as deprecated compatibility wrappers.

2. [x] Remove duplicate observability channels  
Event sink is canonical; legacy callbacks are compatibility bridge driven by events.

3. [x] Single configuration write path  
Legacy setters now route through layered config writers.

4. [x] Replace world alias with real world object boundary  
`PhysicsWorld` is now an actual wrapper object in `src/physics_world.c`.

5. [x] Replace weak event payload with typed payload union  
`PhysicsTraceEvent` now carries typed `payload`.

6. [x] Split oversized internal aggregate header  
World state moved into `src/internal/physics_world_state.h`.

7. [x] Move non-stable toggles out of stable config  
`ccd/sleep/threading/worker` moved to `PhysicsExperimentalConfig`.
