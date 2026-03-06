# Phase G Detailed Design

Date: 2026-03-06  
Status: Complete

## Scope

Phase G closes the remaining editor-facing architecture gap after the C++ convergence work:

1. Project / Workspace / Package / Settings persistence.
2. Prefab deep-edit semantics: nearest-layer override resolution, conflict visibility, dangling override repair.
3. Editor plugin lifecycle: scan, attach, initialize, disable, shutdown.

## Deliverables

1. Project system model
   - `include/project_workspace.hpp`
   - `src/content/project_workspace.cpp`
   - persisted examples under `ProjectSettings/` and `Packages/`
2. Prefab semantics model
   - `include/prefab_semantics.hpp`
   - `src/content/prefab_semantics.cpp`
   - sample nested prefabs under `Prefabs/phase_g_*.prefab`
3. Plugin extension model
   - `include/editor_plugin.hpp`
   - `src/content/editor_plugin.cpp`
   - sample plugin manifests under `Packages/*.physicsplugin`
4. Editor visibility path
   - `apps/sandbox_dwrite/main.cpp`
   - `apps/sandbox_dwrite/infrastructure/project_tree.cpp`

## Rules

1. Project / Workspace / Package / Settings are persisted in deterministic line-oriented text formats.
2. `Packages/` becomes part of the project tree and versioned workspace layout.
3. Prefab conflicts resolve by nearest override layer; shadowed overrides are reported as conflicts.
4. Dangling overrides are explicitly reported and repairable by pruning invalid entries.
5. Plugin discovery is manifest-driven and host bindings remain on the engine/editor side.
6. Missing or failing plugin implementations must not crash the editor; they move to disabled/failed state.

## Verification Strategy

1. Regression coverage for roundtrip persistence and bootstrap layout.
2. Regression coverage for prefab conflict analysis and dangling override repair.
3. Regression coverage for plugin scan/init/disable/shutdown and duplicate rejection.
4. Sandbox project panel shows current project model, plugin state summary, and prefab semantics summary.
