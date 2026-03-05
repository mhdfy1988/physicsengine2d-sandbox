# Kernel Next TODO (12 items, in order)

1. [x] P0: Implement CCD/TOI pipeline (continuous collision)
2. [x] P0: Contact persistence and warm-cache across frames
3. [x] P0: Island solver decomposition
4. [x] P0: Sleeping/wakeup system
5. [x] P1: Real multithread backend for job system
6. [x] P1: Constraint/joint framework expansion
7. [x] P1: Physical consistency parameter refinement
8. [x] P1: Broadphase strategy expansion (SAP/BVH)
9. [x] P1: Determinism and reproducibility guarantees
10. [x] P2: Benchmark suite and perf regression gates
11. [x] P2: Testing expansion (fuzz/long-run/invariants)
12. [x] P2: Snapshot/replay and migration tooling

## Post-Review Follow-ups

1. [x] Fix sleep/wakeup robustness:
   - avoid gravity/steady contact force constantly resetting sleep
   - avoid unconditional contact-driven sleep timer reset
2. [x] Fix contact event ordering/identity stability:
   - ensure `PHYSICS_EVENT_CONTACT_CREATED` data remains stable with contact sorting
3. [x] Fix snapshot semantic mismatch:
   - `PhysicsBodySnapshot.mass/inertia` currently captured but not applied
4. [x] Clarify `PHYSICS_BROADPHASE_BVH` behavior:
   - implement BVH path or explicitly signal unsupported mode instead of silent fallback

## Maintainability Pass (Completed)

1. [x] Split monolithic engine implementation into focused modules:
   - `physics_lifecycle.c`, `physics_runtime_api.c`, `physics_config.c`
   - `physics_query.c`, `physics_mutation.c`, `physics_pipeline_api.c`
2. [x] Split collision detection from collision resolution:
   - `collision_detect.c` (SAT/narrow-phase detect path)
   - `collision.c` (impulse/position resolution path)
3. [x] Modularize regression tests by concern and centralize registration:
   - `REGRESSION_TEST_LIST` single source in `tests/regression_tests.h`
4. [x] Centralize internal defaults and tuning constants:
   - `src/internal/physics_defaults.h`
   - `src/internal/physics_tuning.h`
5. [x] CI hardening:
   - GitHub Actions runs both CMake regression and Makefile regression paths
