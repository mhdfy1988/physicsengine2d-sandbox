# Project Restructure Plan

## Purpose

This document proposes a repository-wide structure cleanup for `Forge2D`.

The goal is not to redesign the engine architecture from scratch. The goal is to make the current architecture easier to read, maintain, and extend by aligning directory layout with actual module boundaries that already exist in the codebase.

Primary problems today:

- `include/` mixes multiple kinds of public headers at the top level.
- `docs/` is too flat and mixes API docs, architecture docs, phase reports, and todo tracking.
- `tests/` is too flat and mixes kernel/content/editor/runtime/smoke/equivalence coverage.
- repository root mixes engine source and sample project content.
- script files are all under one directory with different purposes mixed together.

## Repository Model Decision

This repository should be treated as:

- a single repository containing:
  - engine code
  - editor/application code
  - at least one sample project

This is not a pure engine-only repository.

That means the repository root should primarily express repository structure:

- source code
- apps
- tests
- tools
- scripts
- docs

And sample-project content should live under an explicit project root rather than directly at repository root.

Recommended model:

```text
Forge2D/
鈹溾攢 apps/
鈹溾攢 src/
鈹溾攢 include/
鈹溾攢 tests/
鈹溾攢 tools/
鈹溾攢 scripts/
鈹溾攢 docs/
鈹溾攢 assets/
鈹斺攢 samples/
   鈹斺攢 physics_sandbox_project/
      鈹溾攢 Packages/
      鈹溾攢 Prefabs/
      鈹溾攢 ProjectSettings/
      鈹斺攢 Scenes/
```

In other words:

- repository root is not project root
- `samples/physics_sandbox_project/` becomes the canonical sample-project root
- runtime/editor startup should resolve content paths from project root, not repository root

## Restructure Goals

The restructure should achieve these outcomes:

- make directory names reflect architecture boundaries
- reduce 鈥渨here does this file belong鈥?ambiguity
- make public API surfaces easier to understand
- separate reusable engine code from sample/editor/project content
- keep migration risk controlled through staged changes
- preserve backward compatibility where needed during transition

## Design Principles

1. Prefer structural clarity over cosmetic renaming.
2. Move one concern at a time.
3. Do not combine directory cleanup with behavior changes unless required.
4. Keep compatibility headers or forwarding includes when include-path churn would be too disruptive.
5. Treat build-system updates and include-path updates as first-class migration work, not follow-up cleanup.

## Current Pain Points

### 1. Public headers are split across three styles

Current `include/` has:

- top-level public headers such as `asset_database.hpp`, `prefab_schema.hpp`, `physics_world.hpp`
- compatibility-oriented headers under `include/physics2d/`
- newer core API headers under `include/physics_core/`

This creates ambiguity:

- some headers are domain modules
- some are low-level physics API
- some are facade or compatibility API
- their placement does not consistently express that difference

### 2. `docs/` is overloaded

The current `docs/` directory contains:

- API documentation
- architecture notes
- phase acceptance/design/evidence/final reports
- todo lists
- archived plans

This is workable for a short period, but as history grows it becomes difficult to navigate.

### 3. `tests/` is too flat

Current tests mix:

- kernel regression tests
- content regression tests
- editor smokes
- runtime smokes
- API smokes
- equivalence tests
- data files

The result is a naming-based organization instead of a structure-based organization.

### 4. Repository root mixes code and sample project content

These directories are not source code in the same sense as `src/` or `apps/`:

- `Packages/`
- `Prefabs/`
- `ProjectSettings/`
- `Scenes/`

They represent project content or sample workspace state, but they currently sit at the repository root next to build/config/source directories.

### 5. `scripts/` is functionally mixed

Current scripts cover:

- architecture checks
- API checks
- packaging
- gate suite execution
- icon generation
- build metadata emission

These should be grouped by purpose.

## Target Structure

The following target structure is recommended.

```text
Forge2D/
鈹溾攢 .github/
鈹溾攢 apps/
鈹? 鈹溾攢 runtime_cli/
鈹? 鈹斺攢 sandbox_dwrite/
鈹溾攢 assets/
鈹溾攢 docs/
鈹? 鈹溾攢 api/
鈹? 鈹溾攢 architecture/
鈹? 鈹溾攢 phases/
鈹? 鈹溾攢 reports/
鈹? 鈹溾攢 todos/
鈹? 鈹斺攢 archive/
鈹溾攢 examples/
鈹溾攢 include/
鈹? 鈹溾攢 physics_core/
鈹? 鈹溾攢 physics_content/
鈹? 鈹溾攢 physics_runtime/
鈹? 鈹斺攢 physics2d/
鈹溾攢 installer/
鈹溾攢 samples/
鈹? 鈹斺攢 physics_sandbox_project/
鈹?    鈹溾攢 Packages/
鈹?    鈹溾攢 Prefabs/
鈹?    鈹溾攢 ProjectSettings/
鈹?    鈹斺攢 Scenes/
鈹溾攢 scripts/
鈹? 鈹溾攢 check/
鈹? 鈹溾攢 gate/
鈹? 鈹溾攢 package/
鈹? 鈹斺攢 dev/
鈹溾攢 src/
鈹? 鈹溾攢 core/
鈹? 鈹溾攢 content/
鈹? 鈹溾攢 runtime/
鈹? 鈹溾攢 c_api/
鈹? 鈹溾攢 internal/
鈹? 鈹斺攢 physics2d/
鈹溾攢 tests/
鈹? 鈹溾攢 kernel/
鈹? 鈹溾攢 content/
鈹? 鈹溾攢 runtime/
鈹? 鈹溾攢 editor/
鈹? 鈹溾攢 smoke/
鈹? 鈹溾攢 equivalence/
鈹? 鈹斺攢 data/
鈹溾攢 tools/
鈹溾攢 CMakeLists.txt
鈹溾攢 Makefile
鈹斺攢 README.md
```

## Recommended Target Layout By Area

### `include/`

Recommended split:

- `include/physics_core/`
  - pure physics engine public surface
  - low-level world/body/constraint/shape/core C and C++ entry points
- `include/physics_content/`
  - asset database, importer, asset pipeline, prefab, scene, workspace, plugin-facing content APIs
- `include/physics_runtime/`
  - runtime support such as snapshots, session recovery, runtime integration helpers
- `include/physics2d/`
  - compatibility facade and umbrella headers preserved for downstream users

Suggested mapping examples:

- `body.hpp`, `collision.hpp`, `constraint.hpp`, `shape.hpp`, `physics_world.*`, `physics*.h/.hpp`
  - move to `include/physics_core/`
- `asset_*.hpp`, `prefab_*.hpp`, `scene_schema.hpp`, `project_workspace.hpp`, `editor_plugin.hpp`, `diagnostic_bundle.hpp`
  - move to `include/physics_content/`
- `runtime_snapshot_repo.hpp`, `session_recovery.hpp`
  - move to `include/physics_runtime/`

Compatibility strategy:

- keep temporary forwarding headers in top-level `include/` during migration
- update internal includes first
- update tests/examples next
- remove forwarding headers only after a dedicated compatibility pass

### `src/`

Current `src/` is already closer to the desired shape than `include/`.

Recommended final internal grouping:

- `src/core/`
- `src/content/`
- `src/runtime/`
- `src/c_api/`
- `src/internal/`
- `src/physics2d/`

Concrete change:

- rename `src/runtime_support/` to `src/runtime/` for consistency with `include/physics_runtime/`

### `tests/`

Recommended split:

- `tests/kernel/`
  - kernel regression tests and kernel runner
- `tests/content/`
  - content regression tests and content runner
- `tests/runtime/`
  - runtime-related smokes and integration tests
- `tests/editor/`
  - editor-specific smokes and workflow tests
- `tests/smoke/`
  - broad API smoke tests and compatibility smoke tests
- `tests/equivalence/`
  - keep as-is but under a clear dedicated bucket
- `tests/data/`
  - keep as shared test fixtures

This improves discoverability and makes target definitions easier to maintain.

### `docs/`

Recommended split:

- `docs/api/`
  - API_CN, API_EN, compatibility, schema docs
- `docs/architecture/`
  - engine architecture and boundary docs
- `docs/phases/`
  - phase progress and acceptance docs
- `docs/reports/`
  - final reports, evidence bundles, profile reports
- `docs/todos/`
  - active task lists
- `docs/archive/`
  - obsolete or superseded planning docs

Suggested moves:

- `PHYSICS_CORE_BOUNDARY.md` -> `docs/architecture/`
- `ENGINE_ARCHITECTURE.md` -> `docs/architecture/`
- `PHYSICS_CORE_EXTRACTION_TODO.md` -> `docs/todos/`
- `POST_PHASE_H_REVIEW_TODO.md` -> `docs/todos/`
- `PREFAB_SCHEMA_V1.md`, `SCENE_SCHEMA_V1.md`, `API_*.md` -> `docs/api/`
- phase acceptance/progress files -> `docs/phases/`
- evidence/final report/profile files -> `docs/reports/`

### Sample project content

Recommended move:

- move `Packages/`, `Prefabs/`, `ProjectSettings/`, `Scenes/` under one explicit sample project root
- for this repository, the recommended canonical location is:
  - `samples/physics_sandbox_project/`

Example:

```text
samples/physics_sandbox_project/
鈹溾攢 Packages/
鈹溾攢 Prefabs/
鈹溾攢 ProjectSettings/
鈹斺攢 Scenes/
```

This makes it explicit that these are not repository infrastructure folders. They are project content.

This also establishes an important architectural rule:

- repository root is where source code and repository infrastructure live
- sample project root is where editor/runtime content lives

After this move, bootstrap and startup code should stop assuming:

- `./Scenes`
- `./Prefabs`
- `./Packages`
- `./ProjectSettings`

And should instead resolve:

- `<project_root>/Scenes`
- `<project_root>/Prefabs`
- `<project_root>/Packages`
- `<project_root>/ProjectSettings`

### `scripts/`

Recommended split:

- `scripts/check/`
  - architecture/API/convergence checks
- `scripts/gate/`
  - phase gate suite execution
- `scripts/package/`
  - package creation and release matrix logic
- `scripts/dev/`
  - icon generation, metadata emission, ad hoc helper scripts

Suggested mapping:

- `check_api_surface.ps1`, `check_arch_deps.ps1`, `check_core_change.ps1`, `check_cpp_convergence.ps1`
  - `scripts/check/`
- `run_phase_*`
  - `scripts/gate/`
- `package.ps1`, `package_physics_core.ps1`
  - `scripts/package/`
- `generate_app_icon.py`, `emit_build_metadata.ps1`, `hot_reload_smoke*.ps1`
  - `scripts/dev/`

## Migration Plan

The restructure should be done in stages.

### Phase 1: Low-risk organization cleanup

Scope:

- reorganize `docs/`
- reorganize `tests/`
- reorganize `scripts/`
- update documentation links and CMake paths accordingly

Why first:

- low semantic risk
- high readability payoff
- limited downstream compatibility impact

Deliverables:

- no behavior changes
- green test/build after path updates

### Phase 2: Public header reorganization

Scope:

- create `physics_content/` and `physics_runtime/` public header namespaces
- move top-level headers into domain directories
- keep forwarding headers for compatibility
- update internal includes and examples

Why second:

- this is the most visible structural improvement
- it affects many include paths, so it should happen after docs/tests layout is stabilized

Deliverables:

- new public header tree
- compatibility include layer
- updated header smoke tests

### Phase 3: Runtime/content/sample-project root cleanup

Scope:

- move `Packages/`, `Prefabs/`, `ProjectSettings/`, `Scenes/` under `samples/physics_sandbox_project/`
- align app bootstrap paths and packaging scripts
- rename `src/runtime_support/` to `src/runtime/`
- make `project_root` an explicit concept in startup/bootstrap code

Why third:

- path sensitivity is higher
- packaging/bootstrap code depends on these locations
- easier after header/test/doc restructuring is complete

Deliverables:

- explicit sample project root at `samples/physics_sandbox_project/`
- updated startup/bootstrap path config
- updated package/build scripts
- startup path resolution based on project root instead of repository root

### Phase 4: Optional composition cleanup

Scope:

- further reduce `apps/sandbox_dwrite/main.cpp`
- move app composition/bootstrap wiring into a dedicated composition module

This is useful, but it is architectural cleanup rather than directory cleanup, so it should be treated separately.

## Compatibility Strategy

To avoid breaking downstream code too aggressively:

1. introduce new directories first
2. move internal includes to new paths
3. add forwarding headers at old paths
4. keep smoke tests covering both canonical and compatibility includes
5. remove deprecated paths only in a later cleanup window

Forwarding headers are especially important for:

- top-level `include/*.hpp`
- compatibility-style `physics2d` entry points

## Build System Impact

Expected files to update during migration:

- `CMakeLists.txt`
- `Makefile`
- packaging scripts
- architecture check scripts
- README and quickstart docs

Checks to preserve after each phase:

- all main test targets still compile
- header smoke tests still compile
- packaging scripts still run
- architecture dependency checks still pass

## Risks

Main risks:

- include path churn causing broad compile breaks
- stale documentation links after file moves
- scripts hardcoding old content paths
- editor/runtime startup code depending on repository-root sample project paths

Mitigations:

- phase the work
- keep forwarding includes
- update scripts in the same commit as directory moves
- rerun packaging and architecture checks after each phase

## Recommended Commit Plan

Recommended breakdown:

1. `Reorganize docs, tests, and script directories`
2. `Introduce namespaced public header layout with compatibility forwards`
3. `Move sample project content under dedicated content root`
4. `Rename runtime support paths and update packaging/bootstrap references`
5. `Remove deprecated compatibility paths` (optional, later)

## Immediate Recommendation

If this plan is executed, the best next real implementation step is:

1. reorganize `docs/`
2. reorganize `tests/`
3. reorganize `scripts/`

That sequence gives the cleanest payoff with the lowest risk before touching include paths or startup content paths.

## Detailed Execution Plan

This section turns the restructure direction into a concrete implementation plan.

The plan is intentionally staged so that:

- structure changes remain reviewable
- build breakages stay localized
- runtime path changes do not get mixed with header-path cleanup

### Phase 0: Freeze Baseline

Purpose:

- establish a clean before-state
- identify all hardcoded path assumptions before moving directories

Tasks:

- record all current executable entry points:
  - `physics_sandbox`
  - `physics_runtime_cli`
- identify all code and scripts that assume repository root is project root
- search for hardcoded references to:
  - `Packages`
  - `Prefabs`
  - `ProjectSettings`
  - `Scenes`
- record current document links and script paths before moving files
- run the current verification set and capture the result

Recommended commands:

- `rg "Packages|Prefabs|ProjectSettings|Scenes" apps src scripts tests docs`
- `powershell -ExecutionPolicy Bypass -File .\scripts\check_arch_deps.ps1`

Validation:

- architecture check passes
- primary tests pass
- key smoke targets compile

Commit:

- no commit required if this is only analysis
- if desired, add a baseline note to docs

### Phase 1: Reorganize `docs/`

Purpose:

- reduce documentation sprawl
- make active documents easier to find

Target layout:

```text
docs/
鈹溾攢 api/
鈹溾攢 architecture/
鈹溾攢 phases/
鈹溾攢 reports/
鈹溾攢 todos/
鈹斺攢 archive/
```

Moves:

- move to `docs/api/`
  - `API_CN.md`
  - `API_EN.md`
  - `API_COMPATIBILITY.md`
  - `PREFAB_SCHEMA_V1.md`
  - `SCENE_SCHEMA_V1.md`
- move to `docs/architecture/`
  - `ENGINE_ARCHITECTURE.md`
  - `PHYSICS_CORE_BOUNDARY.md`
  - `RUNTIME_ERROR_SEMANTICS.md`
- move to `docs/todos/`
  - `PHYSICS_CORE_EXTRACTION_TODO.md`
  - `POST_PHASE_H_REVIEW_TODO.md`
- move to `docs/phases/`
  - `PHASE_*_ACCEPTANCE.md`
  - `PHASE_*_DETAILED_DESIGN.md`
  - `PHASE_*_PROGRESS.md`
  - `RUNTIME_PHASE_B_ACCEPTANCE*.md`
  - `RUNTIME_PHASE_B_DETAILED_DESIGN.md`
  - `RUNTIME_PHASE_B_PROGRESS.md`
- move to `docs/reports/`
  - `PHASE_*_EVIDENCE.md`
  - `PHASE_*_FINAL_REPORT.md`
  - `PHASE_D_PROFILE_REPORT.md`
  - `RUNTIME_PHASE_B_FINAL_REPORT.md`
  - gate suite output docs
- keep in `docs/archive/`
  - historical or superseded todo/planning docs

Tasks:

- move files
- update relative links inside moved docs
- update README and QUICKSTART references
- update scripts or CI if any doc path is referenced

Validation:

- `rg "docs/" README.md QUICKSTART.md docs scripts .github`
- spot-check links in major docs

Commit:

- `Reorganize documentation layout`

### Phase 2: Reorganize `tests/`

Purpose:

- align test structure with system boundaries
- make runners and coverage easier to understand

Target layout:

```text
tests/
鈹溾攢 kernel/
鈹溾攢 content/
鈹溾攢 runtime/
鈹溾攢 editor/
鈹溾攢 smoke/
鈹溾攢 equivalence/
鈹斺攢 data/
```

Suggested file mapping:

- `tests/kernel/`
  - `kernel_regression_runner.cpp`
  - `regression_collision_core_tests.cpp`
  - `regression_engine_feature_tests.cpp`
  - `regression_event_snapshot_tests.cpp`
  - `regression_pipeline_error_tests.cpp`
  - `regression_sleep_broadphase_tests.cpp`
  - `regression_stress_constraint_tests.cpp`
- `tests/content/`
  - `content_regression_runner.cpp`
  - `regression_asset_database_tests.cpp`
  - `regression_asset_hot_reload_tests.cpp`
  - `regression_asset_importer_tests.cpp`
  - `regression_asset_pipeline_tests.cpp`
  - `regression_editor_plugin_tests.cpp`
  - `regression_prefab_schema_tests.cpp`
  - `regression_prefab_semantics_tests.cpp`
  - `regression_project_workspace_tests.cpp`
  - `regression_scene_schema_tests.cpp`
  - `regression_session_recovery_tests.cpp`
- `tests/runtime/`
  - `app_runtime_tick_smoke.cpp`
  - `cpp_runtime_app_bridge_smoke.cpp`
  - `cpp_runtime_error_channel_smoke.cpp`
  - `cpp_runtime_error_recovery_smoke.cpp`
  - `cpp_runtime_events_smoke.cpp`
  - `cpp_runtime_facade_smoke.cpp`
  - `cpp_runtime_multi_error_smoke.cpp`
  - `cpp_runtime_pause_step_smoke.cpp`
  - `cpp_snapshot_smoke.cpp`
- `tests/editor/`
  - `editor_extension_state_service_smoke.cpp`
  - `editor_pie_lifecycle_smoke.cpp`
  - `editor_undo_redo_smoke.cpp`
- `tests/smoke/`
  - `cpp_api_smoke.cpp`
  - `cpp_body_raii_smoke.cpp`
  - `cpp_constraint_raii_smoke.cpp`
  - `cpp_ecs_bridge_smoke.cpp`
  - `cpp_ecs_dirty_sync_smoke.cpp`
  - `cpp_ecs_pipeline_config_smoke.cpp`
  - `cpp_ecs_pipeline_smoke.cpp`
  - `cpp_ecs_query_smoke.cpp`
  - `cpp_ecs_smoke.cpp`
  - `cpp_engine_view_smoke.cpp`
  - `cpp_foundation_smoke.cpp`
  - `cpp_raii_smoke.cpp`
  - `cpp_script_bridge_smoke.cpp`
  - `cpp_single_include_smoke.cpp`
  - `cpp_status_smoke.cpp`
  - `cpp_world_raii_smoke.cpp`
  - `physics_core_header_smoke.cpp`
  - `subsystem_render_audio_animation_smoke.cpp`
- `tests/equivalence/`
  - keep existing equivalence tests
- `tests/data/`
  - keep fixtures

Keep in test root for now if needed:

- `benchmark_suite.cpp`
- `invariant_tests.cpp`
- `regression_tests.cpp`
- `regression_tests.hpp`
- `REGRESSION_TEST_TEMPLATE.md`

Tasks:

- move files
- update `CMakeLists.txt`
- update include paths if any test uses relative includes
- keep target names stable at first

Validation:

- `physics_tests`
- `physics_kernel_tests`
- `physics_content_tests`
- smoke targets still compile

Commit:

- `Reorganize test layout by layer`

### Phase 3: Reorganize `scripts/`

Purpose:

- separate checks, gates, packaging, and dev helpers

Target layout:

```text
scripts/
鈹溾攢 check/
鈹溾攢 gate/
鈹溾攢 package/
鈹斺攢 dev/
```

Suggested file mapping:

- `scripts/check/`
  - `check_api_surface.ps1`
  - `check_arch_deps.ps1`
  - `check_core_change.ps1`
  - `check_cpp_convergence.ps1`
- `scripts/gate/`
  - `run_phase_c_gate_suite.ps1`
  - `run_phase_d_gate_suite.ps1`
  - `run_phase_e_gate_suite.ps1`
  - `run_phase_g_gate_suite.ps1`
  - `run_phase_h_gate_suite.ps1`
  - `run_phase_h_release_matrix.ps1`
- `scripts/package/`
  - `package.ps1`
  - `package_physics_core.ps1`
- `scripts/dev/`
  - `emit_build_metadata.ps1`
  - `generate_app_icon.py`
  - `hot_reload_smoke.ps1`
  - `hot_reload_smoke_headless.ps1`

Tasks:

- move scripts
- update internal script references
- update `.github/workflows/regression.yml`
- update README, QUICKSTART, and docs

Validation:

- run one script from each category
- confirm CI workflow paths are correct

Commit:

- `Reorganize scripts by purpose`

### Phase 4: Reorganize `include/`

Purpose:

- make public API layout reflect module boundaries

Target layout:

```text
include/
鈹溾攢 physics_core/
鈹溾攢 physics_content/
鈹溾攢 physics_runtime/
鈹斺攢 physics2d/
```

Suggested mapping:

- move to `include/physics_core/`
  - `body.hpp`
  - `collision.hpp`
  - `constraint.hpp`
  - `shape.hpp`
  - `physics.h`
  - `physics.hpp`
  - `physics_ext.h`
  - `physics_ext.hpp`
  - `physics_math.hpp`
  - `physics_world.h`
  - `physics_world.hpp`
- move to `include/physics_content/`
  - `asset_database.hpp`
  - `asset_fs_poll.hpp`
  - `asset_fs_watch.hpp`
  - `asset_hot_reload.hpp`
  - `asset_importer.hpp`
  - `asset_invalidation.hpp`
  - `asset_pipeline.hpp`
  - `asset_watch.hpp`
  - `diagnostic_bundle.hpp`
  - `editor_plugin.hpp`
  - `prefab_schema.hpp`
  - `prefab_semantics.hpp`
  - `project_workspace.hpp`
  - `scene_schema.hpp`
  - `subsystem_render_audio_animation.hpp`
- move to `include/physics_runtime/`
  - `runtime_snapshot_repo.hpp`
  - `session_recovery.hpp`
- keep in `include/physics2d/`
  - compatibility and facade headers

Compatibility strategy:

- keep temporary forwarding headers at old top-level paths
- switch internal includes to new canonical paths first
- keep top-level compatibility until all tests/examples/docs are updated

Tasks:

- create new header directories
- move headers
- add forwarding headers
- update internal includes
- update tests and examples

Validation:

- all public-header smoke tests compile
- compatibility include paths still work

Commit:

- `Introduce namespaced public header layout with compatibility forwards`

### Phase 5: Align `src/` With Public Module Layout

Purpose:

- make implementation directories mirror module boundaries more clearly

Target layout:

```text
src/
鈹溾攢 core/
鈹溾攢 content/
鈹溾攢 runtime/
鈹溾攢 c_api/
鈹溾攢 internal/
鈹斺攢 physics2d/
```

Tasks:

- rename `src/runtime_support/` to `src/runtime/`
- update all references in:
  - `CMakeLists.txt`
  - `Makefile`
  - scripts
  - tests
- review whether any `src/content/` files should be further grouped later

Validation:

- build succeeds after path rename
- runtime-support targets still link correctly

Commit:

- `Align source layout with public module boundaries`

### Phase 6: Move Sample Project Content Under `samples/physics_sandbox_project/`

Purpose:

- separate repository infrastructure from sample project content
- establish a canonical sample project root

Target layout:

```text
samples/
鈹斺攢 physics_sandbox_project/
   鈹溾攢 Packages/
   鈹溾攢 Prefabs/
   鈹溾攢 ProjectSettings/
   鈹斺攢 Scenes/
```

Moves:

- `Packages/` -> `samples/physics_sandbox_project/Packages/`
- `Prefabs/` -> `samples/physics_sandbox_project/Prefabs/`
- `ProjectSettings/` -> `samples/physics_sandbox_project/ProjectSettings/`
- `Scenes/` -> `samples/physics_sandbox_project/Scenes/`

Tasks:

- move sample project directories
- update any docs and scripts that refer to old root paths
- define the new canonical sample-project root in startup configuration

Important rule introduced in this phase:

- repository root is not project root
- `samples/physics_sandbox_project/` is the project root for the built-in sample project

Validation:

- sample content still loads correctly
- package/import/session paths still resolve

Commit:

- `Move sample project content under samples/physics_sandbox_project`

### Phase 7: Make `project_root` Explicit in Startup and Bootstrap

Purpose:

- stop deriving content paths from repository root
- formalize project-root path handling

Tasks:

- add explicit `project_root` to startup config/service
- derive all content paths from `project_root`
- update bootstrap/default-generation code
- update session recovery path resolution
- add log output indicating active `project_root`
- optionally add CLI support such as:
  - `--project-root <path>`

Affected areas:

- sandbox startup
- editor extension startup service
- package/prefab/scene/settings/session load
- packaging or installer flows if they assume root-relative paths

Validation:

- default startup works with sample project root
- explicit custom project-root startup works
- missing-path diagnostics report the correct root

Commit:

- `Make project_root explicit in startup and bootstrap flow`

### Phase 8: Update Repository Documentation and Developer Onboarding

Purpose:

- make the new structure self-explanatory

Tasks:

- update `README.md`
- update `QUICKSTART.md`
- update `PROJECT_STRUCTURE.md`
- document:
  - repository root vs project root
  - sample project location
  - canonical include paths
  - canonical script paths

Validation:

- follow README/QUICKSTART from scratch and ensure instructions still make sense

Commit:

- `Update docs for repository and sample-project layout`

### Phase 9: Remove Compatibility Paths

Purpose:

- finish the migration after downstream code is updated

Tasks:

- remove temporary forwarding headers if no longer needed
- remove old script path references
- remove any transitional path aliases
- tighten guardrails to enforce the new structure

Do not do this early.

This should only happen after all previous stages are stable and validated.

Commit:

- `Remove deprecated compatibility paths`

## Verification Matrix Per Phase

These checks should be rerun after any phase that affects code, paths, includes, or scripts:

- `powershell -ExecutionPolicy Bypass -File .\scripts\check\check_arch_deps.ps1`
- main regression target
- `physics_kernel_tests`
- `physics_content_tests`
- header smoke targets
- key runtime/editor smokes
- `physics_sandbox` compile
- packaging script where relevant

For earlier phases before scripts are moved, the current script paths should be used.

## Recommended Commit Sequence

1. `Reorganize documentation layout`
2. `Reorganize test layout by layer`
3. `Reorganize scripts by purpose`
4. `Introduce namespaced public header layout with compatibility forwards`
5. `Align source layout with public module boundaries`
6. `Move sample project content under samples/physics_sandbox_project`
7. `Make project_root explicit in startup and bootstrap flow`
8. `Update docs for repository and sample-project layout`
9. `Remove deprecated compatibility paths`

## Implementation Priority

Recommended execution order:

1. `docs`
2. `tests`
3. `scripts`
4. `include`
5. `src`
6. `samples/physics_sandbox_project`
7. `project_root` startup path model
8. compatibility cleanup

Why:

- the first three phases are low-risk and improve navigation immediately
- the include/source phases are more invasive but still mostly compile-time refactors
- the sample-project and startup-path phases are the most behavior-sensitive and should happen after structure is already cleaner

## Migration Mapping Table

This table maps current locations to recommended target locations.

## Root-Level Directories

| Current | Target | Notes |
|---|---|---|
| `apps/` | `apps/` | Keep in place |
| `assets/` | `assets/` | Keep in place |
| `docs/` | `docs/` | Reorganize internally |
| `examples/` | `examples/` | Keep in place |
| `include/` | `include/` | Reorganize internally |
| `installer/` | `installer/` | Keep in place |
| `scripts/` | `scripts/` | Reorganize internally |
| `src/` | `src/` | Reorganize internally |
| `tests/` | `tests/` | Reorganize internally |
| `tools/` | `tools/` | Keep in place |
| `Packages/` | `samples/physics_sandbox_project/Packages/` | Move under sample project root |
| `Prefabs/` | `samples/physics_sandbox_project/Prefabs/` | Move under sample project root |
| `ProjectSettings/` | `samples/physics_sandbox_project/ProjectSettings/` | Move under sample project root |
| `Scenes/` | `samples/physics_sandbox_project/Scenes/` | Move under sample project root |
| `Cache/` | `Cache/` | Keep for now; review later if it should move under sample project or generated data |

## Documentation Mapping

| Current | Target |
|---|---|
| `docs/api/API_CN.md` | `docs/api/API_CN.md` |
| `docs/api/API_EN.md` | `docs/api/API_EN.md` |
| `docs/api/API_COMPATIBILITY.md` | `docs/api/API_COMPATIBILITY.md` |
| `docs/api/PREFAB_SCHEMA_V1.md` | `docs/api/PREFAB_SCHEMA_V1.md` |
| `docs/api/SCENE_SCHEMA_V1.md` | `docs/api/SCENE_SCHEMA_V1.md` |
| `docs/architecture/ENGINE_ARCHITECTURE.md` | `docs/architecture/ENGINE_ARCHITECTURE.md` |
| `docs/architecture/PHYSICS_CORE_BOUNDARY.md` | `docs/architecture/PHYSICS_CORE_BOUNDARY.md` |
| `docs/RUNTIME_ERROR_SEMANTICS.md` | `docs/architecture/RUNTIME_ERROR_SEMANTICS.md` |
| `docs/todos/PHYSICS_CORE_EXTRACTION_TODO.md` | `docs/todos/PHYSICS_CORE_EXTRACTION_TODO.md` |
| `docs/todos/POST_PHASE_H_REVIEW_TODO.md` | `docs/todos/POST_PHASE_H_REVIEW_TODO.md` |
| `docs/PHASE_*_ACCEPTANCE.md` | `docs/phases/` |
| `docs/PHASE_*_DETAILED_DESIGN.md` | `docs/phases/` |
| `docs/PHASE_*_PROGRESS.md` | `docs/phases/` |
| `docs/RUNTIME_PHASE_B_ACCEPTANCE*.md` | `docs/phases/` |
| `docs/phases/RUNTIME_PHASE_B_DETAILED_DESIGN.md` | `docs/phases/` |
| `docs/phases/RUNTIME_PHASE_B_PROGRESS.md` | `docs/phases/` |
| `docs/PHASE_*_EVIDENCE.md` | `docs/reports/` |
| `docs/PHASE_*_FINAL_REPORT.md` | `docs/reports/` |
| `docs/phases/PHASE_D_PROFILE_REPORT.md` | `docs/reports/PHASE_D_PROFILE_REPORT.md` |
| `docs/reports/RUNTIME_PHASE_B_FINAL_REPORT.md` | `docs/reports/RUNTIME_PHASE_B_FINAL_REPORT.md` |
| `docs/PHASE_C_GATE_SUITE_*.md` | `docs/reports/` |
| `docs/archive/*` | `docs/archive/*` |

## Script Mapping

| Current | Target |
|---|---|
| `scripts/check/check_api_surface.ps1` | `scripts/check/check_api_surface.ps1` |
| `scripts/check/check_arch_deps.ps1` | `scripts/check/check_arch_deps.ps1` |
| `scripts/check/check_core_change.ps1` | `scripts/check/check_core_change.ps1` |
| `scripts/check/check_cpp_convergence.ps1` | `scripts/check/check_cpp_convergence.ps1` |
| `scripts/gate/run_phase_c_gate_suite.ps1` | `scripts/gate/run_phase_c_gate_suite.ps1` |
| `scripts/gate/run_phase_d_gate_suite.ps1` | `scripts/gate/run_phase_d_gate_suite.ps1` |
| `scripts/gate/run_phase_e_gate_suite.ps1` | `scripts/gate/run_phase_e_gate_suite.ps1` |
| `scripts/gate/run_phase_g_gate_suite.ps1` | `scripts/gate/run_phase_g_gate_suite.ps1` |
| `scripts/gate/run_phase_h_gate_suite.ps1` | `scripts/gate/run_phase_h_gate_suite.ps1` |
| `scripts/gate/run_phase_h_release_matrix.ps1` | `scripts/gate/run_phase_h_release_matrix.ps1` |
| `scripts/package/package.ps1` | `scripts/package/package.ps1` |
| `scripts/package/package_physics_core.ps1` | `scripts/package/package_physics_core.ps1` |
| `scripts/dev/emit_build_metadata.ps1` | `scripts/dev/emit_build_metadata.ps1` |
| `scripts/dev/generate_app_icon.py` | `scripts/dev/generate_app_icon.py` |
| `scripts/dev/hot_reload_smoke.ps1` | `scripts/dev/hot_reload_smoke.ps1` |
| `scripts/dev/hot_reload_smoke_headless.ps1` | `scripts/dev/hot_reload_smoke_headless.ps1` |

## Public Header Mapping

### Move to `include/physics_core/`

| Current | Target |
|---|---|
| `include/body.hpp` | `include/physics_core/body.hpp` |
| `include/collision.hpp` | `include/physics_core/collision.hpp` |
| `include/constraint.hpp` | `include/physics_core/constraint.hpp` |
| `include/shape.hpp` | `include/physics_core/shape.hpp` |
| `include/physics.h` | `include/physics_core/physics.h` or keep as compatibility forward |
| `include/physics.hpp` | `include/physics_core/physics.hpp` |
| `include/physics_ext.h` | `include/physics_core/physics_ext.h` |
| `include/physics_ext.hpp` | `include/physics_core/physics_ext.hpp` |
| `include/physics_math.hpp` | `include/physics_core/physics_math.hpp` |
| `include/physics_world.h` | `include/physics_core/physics_world.h` |
| `include/physics_world.hpp` | `include/physics_core/physics_world.hpp` |

### Move to `include/physics_content/`

| Current | Target |
|---|---|
| `include/asset_database.hpp` | `include/physics_content/asset_database.hpp` |
| `include/asset_fs_poll.hpp` | `include/physics_content/asset_fs_poll.hpp` |
| `include/asset_fs_watch.hpp` | `include/physics_content/asset_fs_watch.hpp` |
| `include/asset_hot_reload.hpp` | `include/physics_content/asset_hot_reload.hpp` |
| `include/asset_importer.hpp` | `include/physics_content/asset_importer.hpp` |
| `include/asset_invalidation.hpp` | `include/physics_content/asset_invalidation.hpp` |
| `include/asset_pipeline.hpp` | `include/physics_content/asset_pipeline.hpp` |
| `include/asset_watch.hpp` | `include/physics_content/asset_watch.hpp` |
| `include/diagnostic_bundle.hpp` | `include/physics_content/diagnostic_bundle.hpp` |
| `include/editor_plugin.hpp` | `include/physics_content/editor_plugin.hpp` |
| `include/prefab_schema.hpp` | `include/physics_content/prefab_schema.hpp` |
| `include/prefab_semantics.hpp` | `include/physics_content/prefab_semantics.hpp` |
| `include/project_workspace.hpp` | `include/physics_content/project_workspace.hpp` |
| `include/scene_schema.hpp` | `include/physics_content/scene_schema.hpp` |
| `include/subsystem_render_audio_animation.hpp` | `include/physics_content/subsystem_render_audio_animation.hpp` |

### Move to `include/physics_runtime/`

| Current | Target |
|---|---|
| `include/runtime_snapshot_repo.hpp` | `include/physics_runtime/runtime_snapshot_repo.hpp` |
| `include/session_recovery.hpp` | `include/physics_runtime/session_recovery.hpp` |

### Keep under `include/physics2d/`

| Current | Target |
|---|---|
| `include/physics2d/*` | `include/physics2d/*` |
| top-level compatibility forwards | optional temporary forwards to new canonical paths |

## Source Mapping

| Current | Target |
|---|---|
| `src/core/*` | `src/core/*` |
| `src/content/*` | `src/content/*` |
| `src/c_api/*` | `src/c_api/*` |
| `src/internal/*` | `src/internal/*` |
| `src/physics2d/*` | `src/physics2d/*` |
| `src/runtime_support/snapshot_repo.cpp` | `src/runtime/snapshot_repo.cpp` |

## Test Mapping

### Move to `tests/kernel/`

| Current | Target |
|---|---|
| `tests/kernel_regression_runner.cpp` | `tests/kernel/kernel_regression_runner.cpp` |
| `tests/regression_collision_core_tests.cpp` | `tests/kernel/regression_collision_core_tests.cpp` |
| `tests/regression_engine_feature_tests.cpp` | `tests/kernel/regression_engine_feature_tests.cpp` |
| `tests/regression_event_snapshot_tests.cpp` | `tests/kernel/regression_event_snapshot_tests.cpp` |
| `tests/regression_pipeline_error_tests.cpp` | `tests/kernel/regression_pipeline_error_tests.cpp` |
| `tests/regression_sleep_broadphase_tests.cpp` | `tests/kernel/regression_sleep_broadphase_tests.cpp` |
| `tests/regression_stress_constraint_tests.cpp` | `tests/kernel/regression_stress_constraint_tests.cpp` |

### Move to `tests/content/`

| Current | Target |
|---|---|
| `tests/content_regression_runner.cpp` | `tests/content/content_regression_runner.cpp` |
| `tests/regression_asset_database_tests.cpp` | `tests/content/regression_asset_database_tests.cpp` |
| `tests/regression_asset_hot_reload_tests.cpp` | `tests/content/regression_asset_hot_reload_tests.cpp` |
| `tests/regression_asset_importer_tests.cpp` | `tests/content/regression_asset_importer_tests.cpp` |
| `tests/regression_asset_pipeline_tests.cpp` | `tests/content/regression_asset_pipeline_tests.cpp` |
| `tests/regression_editor_plugin_tests.cpp` | `tests/content/regression_editor_plugin_tests.cpp` |
| `tests/regression_prefab_schema_tests.cpp` | `tests/content/regression_prefab_schema_tests.cpp` |
| `tests/regression_prefab_semantics_tests.cpp` | `tests/content/regression_prefab_semantics_tests.cpp` |
| `tests/regression_project_workspace_tests.cpp` | `tests/content/regression_project_workspace_tests.cpp` |
| `tests/regression_scene_schema_tests.cpp` | `tests/content/regression_scene_schema_tests.cpp` |
| `tests/regression_session_recovery_tests.cpp` | `tests/content/regression_session_recovery_tests.cpp` |

### Move to `tests/runtime/`

| Current | Target |
|---|---|
| `tests/app_runtime_tick_smoke.cpp` | `tests/runtime/app_runtime_tick_smoke.cpp` |
| `tests/cpp_runtime_app_bridge_smoke.cpp` | `tests/runtime/cpp_runtime_app_bridge_smoke.cpp` |
| `tests/cpp_runtime_error_channel_smoke.cpp` | `tests/runtime/cpp_runtime_error_channel_smoke.cpp` |
| `tests/cpp_runtime_error_recovery_smoke.cpp` | `tests/runtime/cpp_runtime_error_recovery_smoke.cpp` |
| `tests/cpp_runtime_events_smoke.cpp` | `tests/runtime/cpp_runtime_events_smoke.cpp` |
| `tests/cpp_runtime_facade_smoke.cpp` | `tests/runtime/cpp_runtime_facade_smoke.cpp` |
| `tests/cpp_runtime_multi_error_smoke.cpp` | `tests/runtime/cpp_runtime_multi_error_smoke.cpp` |
| `tests/cpp_runtime_pause_step_smoke.cpp` | `tests/runtime/cpp_runtime_pause_step_smoke.cpp` |
| `tests/cpp_snapshot_smoke.cpp` | `tests/runtime/cpp_snapshot_smoke.cpp` |

### Move to `tests/editor/`

| Current | Target |
|---|---|
| `tests/editor_extension_state_service_smoke.cpp` | `tests/editor/editor_extension_state_service_smoke.cpp` |
| `tests/editor_pie_lifecycle_smoke.cpp` | `tests/editor/editor_pie_lifecycle_smoke.cpp` |
| `tests/editor_undo_redo_smoke.cpp` | `tests/editor/editor_undo_redo_smoke.cpp` |

### Move to `tests/smoke/`

| Current | Target |
|---|---|
| `tests/cpp_api_smoke.cpp` | `tests/smoke/cpp_api_smoke.cpp` |
| `tests/cpp_body_raii_smoke.cpp` | `tests/smoke/cpp_body_raii_smoke.cpp` |
| `tests/cpp_constraint_raii_smoke.cpp` | `tests/smoke/cpp_constraint_raii_smoke.cpp` |
| `tests/cpp_ecs_bridge_smoke.cpp` | `tests/smoke/cpp_ecs_bridge_smoke.cpp` |
| `tests/cpp_ecs_dirty_sync_smoke.cpp` | `tests/smoke/cpp_ecs_dirty_sync_smoke.cpp` |
| `tests/cpp_ecs_pipeline_config_smoke.cpp` | `tests/smoke/cpp_ecs_pipeline_config_smoke.cpp` |
| `tests/cpp_ecs_pipeline_smoke.cpp` | `tests/smoke/cpp_ecs_pipeline_smoke.cpp` |
| `tests/cpp_ecs_query_smoke.cpp` | `tests/smoke/cpp_ecs_query_smoke.cpp` |
| `tests/cpp_ecs_smoke.cpp` | `tests/smoke/cpp_ecs_smoke.cpp` |
| `tests/cpp_engine_view_smoke.cpp` | `tests/smoke/cpp_engine_view_smoke.cpp` |
| `tests/cpp_foundation_smoke.cpp` | `tests/smoke/cpp_foundation_smoke.cpp` |
| `tests/cpp_raii_smoke.cpp` | `tests/smoke/cpp_raii_smoke.cpp` |
| `tests/cpp_script_bridge_smoke.cpp` | `tests/smoke/cpp_script_bridge_smoke.cpp` |
| `tests/cpp_single_include_smoke.cpp` | `tests/smoke/cpp_single_include_smoke.cpp` |
| `tests/cpp_status_smoke.cpp` | `tests/smoke/cpp_status_smoke.cpp` |
| `tests/cpp_world_raii_smoke.cpp` | `tests/smoke/cpp_world_raii_smoke.cpp` |
| `tests/physics_core_header_smoke.cpp` | `tests/smoke/physics_core_header_smoke.cpp` |
| `tests/subsystem_render_audio_animation_smoke.cpp` | `tests/smoke/subsystem_render_audio_animation_smoke.cpp` |

### Keep or Review Separately

| Current | Target | Notes |
|---|---|---|
| `tests/equivalence/*` | `tests/equivalence/*` | Keep in place |
| `tests/data/*` | `tests/data/*` | Keep in place |
| `tests/benchmark_suite.cpp` | `tests/benchmark_suite.cpp` or `tests/perf/benchmark_suite.cpp` | Optional later split |
| `tests/invariant_tests.cpp` | `tests/invariant_tests.cpp` or `tests/kernel/invariant_tests.cpp` | Decide based on ownership |
| `tests/regression_tests.cpp` | `tests/regression_tests.cpp` | Keep during transition |
| `tests/regression_tests.hpp` | `tests/regression_tests.hpp` | Keep during transition |
| `tests/REGRESSION_TEST_TEMPLATE.md` | `tests/REGRESSION_TEST_TEMPLATE.md` | Keep during transition |

## Sample Project Content Mapping

| Current | Target |
|---|---|
| `Packages/.gitkeep` | `samples/physics_sandbox_project/Packages/.gitkeep` |
| `Packages/*.physicsplugin` | `samples/physics_sandbox_project/Packages/*.physicsplugin` |
| `Packages/*.physicspackage` | `samples/physics_sandbox_project/Packages/*.physicspackage` |
| `Prefabs/*` | `samples/physics_sandbox_project/Prefabs/*` |
| `ProjectSettings/*` | `samples/physics_sandbox_project/ProjectSettings/*` |
| `Scenes/*` | `samples/physics_sandbox_project/Scenes/*` |

## Path-Resolution Mapping

The following conceptual path migrations must happen together with the sample-project move.

| Old assumption | New rule |
|---|---|
| `./Packages` | `<project_root>/Packages` |
| `./Prefabs` | `<project_root>/Prefabs` |
| `./ProjectSettings` | `<project_root>/ProjectSettings` |
| `./Scenes` | `<project_root>/Scenes` |

Recommended default:

- `project_root = samples/physics_sandbox_project`

## Files Likely Requiring Path Updates During Project-Root Migration

These are not guaranteed exhaustive, but they are the first places to check:

- `apps/sandbox_dwrite/main.cpp`
- `apps/sandbox_dwrite/application/editor_extension_state_service.cpp`
- `apps/sandbox_dwrite/application/editor_extension_state_service.hpp`
- packaging scripts
- installer script
- smoke tests that rely on repository-root content paths
- docs and quickstart instructions

## Conservative-To-Aggressive Header Migration

The repository should not stop at the conservative compatibility stage.

The conservative stage exists to reduce migration risk. The aggressive stage is still the intended end-state.

### Why Conservative First

The conservative stage solves these problems:

- new public header families are created
- internal code can start migrating without immediate breakage
- downstream callers are not broken all at once
- build and packaging can keep working while include paths transition

But the conservative stage does **not** solve the final structure problem.

If top-level module headers remain forever, the repository stays in a mixed state:

- new namespaced layout exists
- old flat layout still dominates the visual structure
- contributors keep guessing which layout is the real one

So the correct long-term path is:

1. conservative migration
2. stability window
3. aggressive cleanup

### New Explicit Phases

#### Phase 4.2: Public Header Canonicalization

Purpose:

- make namespaced public headers the canonical default
- push all internal code onto the new include layout
- demote top-level module headers to compatibility-only shims

Canonical include policy after this phase:

- `physics_core/...` for core public API
- `physics_content/...` for content/editor-content public API
- `physics_runtime/...` for runtime-support public API
- `physics2d/...` remains the compatibility facade family

Required work:

- scan and update includes under:
  - `apps/`
  - `src/`
  - `tests/`
  - `examples/`
  - `tools/`
- replace legacy top-level module includes such as:
  - `asset_database.hpp`
  - `prefab_schema.hpp`
  - `runtime_snapshot_repo.hpp`
  - `body.hpp`
- switch them to canonical namespaced paths such as:
  - `physics_content/asset_database.hpp`
  - `physics_content/prefab_schema.hpp`
  - `physics_runtime/runtime_snapshot_repo.hpp`
  - `physics_core/body.hpp`
- update README, examples, and package docs to teach the canonical paths
- keep top-level compatibility forwarding headers in place

Validation:

- all main targets compile
- regression tests pass
- header smoke tests pass
- packaging still works
- grep confirms internal code no longer uses legacy top-level module headers

Suggested commit:

- `Canonicalize internal public header usage`

#### Phase 9.5: Remove Legacy Top-Level Public Headers

Purpose:

- finish the migration
- make `include/` visually clean
- remove ambiguity about which public layout is canonical

This is the aggressive end-state.

Recommended top-level headers to preserve, if desired:

- `physics.hpp`
- `physics.h`
- optionally:
  - `physics_world.hpp`
  - `physics_world.h`
  - `physics_ext.hpp`
  - `physics_ext.h`

Recommended top-level headers to remove once migration is complete:

- `asset_database.hpp`
- `asset_fs_poll.hpp`
- `asset_fs_watch.hpp`
- `asset_hot_reload.hpp`
- `asset_importer.hpp`
- `asset_invalidation.hpp`
- `asset_pipeline.hpp`
- `asset_watch.hpp`
- `diagnostic_bundle.hpp`
- `editor_plugin.hpp`
- `prefab_schema.hpp`
- `prefab_semantics.hpp`
- `project_workspace.hpp`
- `runtime_snapshot_repo.hpp`
- `scene_schema.hpp`
- `session_recovery.hpp`
- `subsystem_render_audio_animation.hpp`
- `body.hpp`
- `collision.hpp`
- `constraint.hpp`
- `shape.hpp`
- `physics_math.hpp`

Preconditions:

- all internal code already uses canonical namespaced include paths
- tests already use canonical namespaced include paths
- examples/docs are already updated
- standalone package verification still passes

Required work:

- delete legacy top-level module headers
- keep only the explicitly approved umbrella entry points
- rerun full configure/build/test/package verification
- add a guardrail check preventing reintroduction of removed top-level headers in internal code

Validation:

- clean configure/build succeeds
- regression tests pass
- sample project still boots
- standalone package still builds and runs
- grep confirms no internal file includes removed top-level module headers

Suggested commit:

- `Remove legacy top-level public headers`

### Detailed Conservative-To-Aggressive Execution Plan

#### Step A: Establish Namespaced Header Families

Status:

- done in structure, but not yet complete in usage

Required state:

- `include/physics_core/` exists
- `include/physics_content/` exists
- `include/physics_runtime/` exists
- top-level legacy headers still exist as transition surface

#### Step B: Convert Internal Includes

Required work:

- update all source files under:
  - `apps/`
  - `src/`
  - `tests/`
  - `examples/`
  - `tools/`
- replace legacy include forms with canonical namespaced paths

Examples:

- `#include "asset_database.hpp"` -> `#include "physics_content/asset_database.hpp"`
- `#include "prefab_schema.hpp"` -> `#include "physics_content/prefab_schema.hpp"`
- `#include "runtime_snapshot_repo.hpp"` -> `#include "physics_runtime/runtime_snapshot_repo.hpp"`
- `#include "body.hpp"` -> `#include "physics_core/body.hpp"`

Rules:

- do not delete compatibility headers in this step
- do not mix behavior changes into this step
- preserve umbrella headers only if they are intended as long-term public entry points

Validation:

- grep repository for remaining legacy include usage
- rebuild all primary targets
- rerun regression and smoke coverage

Suggested commit:

- `Canonicalize internal public header usage`

#### Step C: Convert Docs And Examples

Required work:

- update README include examples
- update package docs
- update developer docs and templates
- update any examples that still show legacy top-level module includes

Goal:

- the repository teaches the new structure by default

Validation:

- grep docs/examples for removed legacy include forms

Suggested commit:

- `Update docs and examples to canonical include paths`

#### Step D: Hold Compatibility Window

Purpose:

- keep one stabilization window where:
  - internal code already uses canonical namespaced paths
  - compatibility headers still exist

Exit criteria:

- no internal code depends on legacy top-level module headers
- no active docs/examples recommend legacy module paths
- package verification stays green

#### Step E: Remove Legacy Module Headers

Purpose:

- complete the cleanup

Required work:

- delete top-level module headers that are no longer intended public defaults
- keep only approved umbrella headers at the top level
- rerun full validation from a clean build

Suggested keep-set:

- `physics.hpp`
- `physics.h`
- optional:
  - `physics_world.hpp`
  - `physics_world.h`
  - `physics_ext.hpp`
  - `physics_ext.h`

Suggested remove-set:

- all top-level content headers
- all top-level runtime headers
- all top-level core submodule headers such as `body.hpp`, `collision.hpp`, `constraint.hpp`, `shape.hpp`

Validation:

- clean configure/build succeeds
- regression tests pass
- sample project still boots
- package scripts still work

Suggested commit:

- `Remove legacy top-level public headers`

#### Step F: Add Guardrails

Purpose:

- prevent drift back to the old flat layout

Suggested guardrails:

- grep-based check that fails if internal code includes removed top-level module headers
- doc policy note stating canonical include paths
- optional smoke target that includes only namespaced families

Suggested commit:

- `Enforce canonical public header usage`

### Updated Recommended Commit Sequence

1. `Reorganize documentation layout`
2. `Reorganize test layout by layer`
3. `Reorganize scripts by purpose`
4. `Introduce namespaced public header layout with compatibility forwards`
5. `Canonicalize internal public header usage`
6. `Align source layout with public module boundaries`
7. `Move sample project content under samples/physics_sandbox_project`
8. `Make project_root explicit in startup and bootstrap flow`
9. `Update docs for repository and sample-project layout`
10. `Remove deprecated compatibility paths`
11. `Remove legacy top-level public headers`
12. `Enforce canonical public header usage`
