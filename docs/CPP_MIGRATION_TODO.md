# C to C++ Migration TODO (Plan Only, No Execution Yet)

Status: Planning only. Do not implement until explicitly approved.

## Goals

- Keep existing behavior stable.
- Preserve current C API compatibility.
- Enable incremental migration with rollback points.
- Ensure test and performance baselines do not regress.

## Non-Goals (for now)

- No full rewrite.
- No immediate ECS conversion.
- No large hot-path refactors in one step.

## Phase 0: Baseline Freeze

1. Tag current stable commit.
2. Record baseline:
   - `make test` must pass.
   - CI must pass.
   - Benchmark snapshot (if available).
3. Create migration branch.

Acceptance:
- Baseline artifacts exist and are linked in PR notes.

Rollback:
- Reset to baseline tag.

## Phase 1: Build System Dual-Mode (C + C++)

1. Add C++ target in CMake without changing core logic.
2. Keep C sources compiled as C at first.
3. Verify both build paths:
   - existing C path
   - new C++ wrapper path

Acceptance:
- Both build paths pass CI and local regression tests.

Rollback:
- Disable C++ target and keep C-only pipeline.

## Phase 2: Header Compatibility Layer

1. Add `extern "C"` guards to public headers in `include/`.
2. Ensure headers compile in both C and C++ translation units.
3. Keep ABI stable.

Acceptance:
- C and C++ consumers can include API headers without errors.

Rollback:
- Revert header guard changes only.

## Phase 3: C++ RAII Wrapper (No Core Rewrite)

1. Add wrapper layer (new files under `cpp/` or similar):
   - `PhysicsEngineHandle` using `std::unique_ptr` + custom deleter.
2. Wrap create/free and common API calls.
3. No changes to internal C engine modules.

Acceptance:
- Wrapper tests pass.
- Existing C tests unchanged and still passing.

Rollback:
- Remove wrapper target; C core remains intact.

## Phase 4: Gradual Consumer Migration

1. Migrate top-level app/test call sites to wrapper where useful.
2. Keep C API available for compatibility.
3. Avoid hot-path refactor in this phase.

Acceptance:
- Behavior parity maintained.
- No net increase in memory/resource leaks.

Rollback:
- Switch call sites back to C API only.

## Phase 5: Optional Internal C++ Adoption (Deferred)

1. Evaluate module-by-module conversion (`.c` -> `.cpp`) only if needed.
2. Prioritize low-risk modules first.
3. Keep strict regression/perf gates.

Acceptance:
- No regression in 29/29 regression suite.
- No significant performance drop versus baseline.

Rollback:
- Revert converted module(s) individually.

## Quality Gates (Every Phase)

1. Functional:
   - `make test` must pass.
   - CI workflow must pass.
2. Maintainability:
   - No new unchecked TODO/FIXME.
   - Keep module boundaries clear.
3. Performance:
   - Compare to baseline snapshot before merge.

## Risks and Controls

1. ABI drift risk
   - Control: lock public C API and use `extern "C"`.
2. Hidden behavior drift
   - Control: run full regression on every phase.
3. Over-large migration scope
   - Control: phase gates + mandatory rollback point.

## Immediate Next Step (When Approved)

1. Implement Phase 1 only.
2. Open a PR titled: `build: add cxx compatibility target (no behavior change)`.

