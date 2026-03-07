# Physics Core Extraction TODO

Date: 2026-03-06
Scope: follow-up plan for keeping the physics kernel independently reusable outside the current editor/runtime project

## Goal

Preserve and strengthen the ability to use the physics engine core as a standalone module in other projects.

Current state:

- `src/physics2d`
- `src/core`
- `src/c_api`

are still largely decoupled from `apps/`, and architecture checks currently enforce that boundary.

However, the current `physics2d` build target is no longer a pure physics-core artifact because it also bundles content/project/editor-support code from `src/content`.

## 1. Split Build Targets

Status: pending
Priority: high

### Problem

The current `physics2d` static library includes both:

- physics kernel implementation
- content/project/asset/session/editor-support systems

This makes the library heavier than necessary and weakens standalone reuse.

### Follow-up Direction

Introduce separate build targets:

1. `physics_kernel`
   - only:
     - `src/physics2d/*`
     - `src/core/*`
     - `src/c_api/*`
2. `physics_content`
   - content/project/prefab/asset/session/diagnostic systems
3. `physics_editor`
   - sandbox/editor app layer

Optional:

4. keep `physics2d` as a compatibility umbrella target temporarily

### Minimum Acceptance

- standalone physics target exists
- content/editor systems are not linked into that target
- tests can link either to pure kernel or to full stack explicitly

## 2. Tighten Architecture Guardrails

Status: pending
Priority: high

### Problem

Current dependency checks only guarantee that runtime core does not depend on `apps/`.
They do not assert that the pure physics core stays free of content/project/editor systems.

### Follow-up Direction

Extend [check_arch_deps.ps1](/d:/C_Project/simple_project/scripts/check/check_arch_deps.ps1) to enforce:

1. `src/physics2d/*`, `src/core/*`, `src/c_api/*` must not depend on:
   - `src/content/*`
   - `project_workspace`
   - `prefab_*`
   - `asset_*`
   - `editor_plugin`
   - `session_recovery`
   - `diagnostic_bundle`
2. public physics headers must not pull content/editor headers
3. standalone target include surface remains explicit

### Minimum Acceptance

- CI fails if physics core starts depending on content/editor systems
- architecture script protects the intended extraction boundary

## 3. Define Public Core API Surface

Status: pending
Priority: high

### Problem

The repository currently exposes both low-level C API and higher-level C++ wrappers, but it does not clearly separate:

- reusable physics-core headers
- full-engine/content headers

### Follow-up Direction

Create a clear public API split:

1. `include/physics_core/*`
   - pure physics C/C++ API surface
2. `include/physics2d/*`
   - optional higher-level wrappers / broader engine facade

or alternatively:

1. keep existing public names
2. formally mark which headers are core-only vs full-stack

### Minimum Acceptance

- core consumers can include physics-only headers without dragging content/project features
- documentation clearly states which headers belong to standalone kernel integration

## 4. Remove App-Layer Reuse Leaks

Status: pending
Priority: medium

### Problem

Some non-editor executables still reuse editor-side infrastructure. For example, `runtime_cli` currently depends on sandbox snapshot infrastructure.

### Follow-up Direction

Move reusable non-UI helpers out of `apps/sandbox_dwrite` into a neutral runtime/content location, for example:

- `src/runtime_support/*`
- or `src/content/snapshots/*`

Candidates:

1. snapshot load/save helpers currently under editor infrastructure
2. any reusable runtime-side serialization helpers

### Minimum Acceptance

- `runtime_cli` does not depend on `apps/sandbox_dwrite/infrastructure`
- non-UI tools only depend on core/content/runtime support layers

## 5. Separate Test Layers

Status: pending
Priority: medium

### Problem

Current tests mostly link against the combined library target, which hides whether a test is exercising:

- pure physics kernel
- content systems
- editor/runtime integration

### Follow-up Direction

Split tests into layers:

1. kernel tests
   - physics math/body/constraint/collision/step/query/runtime api
2. content tests
   - scene/prefab/project/asset/session/diagnostic
3. editor/app integration tests
   - undo/redo, PIE, runtime app bridge, sandbox-specific flows

### Minimum Acceptance

- at least one test target links only against `physics_kernel`
- integration tests explicitly depend on broader layers

## 6. Package a Standalone Core Deliverable

Status: pending
Priority: medium

### Problem

Even if the code is logically separable, there is not yet a clean standalone packaging path for external reuse.

### Follow-up Direction

Add a packaging/export path for the pure kernel:

1. core headers
2. standalone static library
3. minimal integration example
4. build metadata/version stamp

Optional:

5. example consumer app outside editor/runtime tree

### Minimum Acceptance

- standalone archive/package can be produced
- integration example builds only against core deliverable

## 7. Documentation Alignment

Status: pending
Priority: low

### Problem

Current documentation explains the full project, but does not clearly distinguish:

- physics kernel
- content/runtime systems
- editor/tooling

### Follow-up Direction

Add or update docs to describe:

1. what counts as standalone physics core
2. which targets/headers are safe for external reuse
3. which systems are full-project only
4. recommended extraction path for downstream adopters

### Minimum Acceptance

- one clear document defines the reusable core boundary
- build and integration docs mention the split

## Suggested Execution Order

1. Split build targets
2. Tighten architecture guardrails
3. Remove app-layer reuse leaks
4. Separate test layers
5. Define public core API surface
6. Package standalone core deliverable
7. Align documentation
