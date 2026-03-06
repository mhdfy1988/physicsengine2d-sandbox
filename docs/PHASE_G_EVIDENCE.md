# Phase G Evidence

Date: 2026-03-06  
Scope: Project system, prefab semantics, and editor plugin lifecycle closure

## Code Evidence

1. `include/project_workspace.hpp`
2. `src/content/project_workspace.cpp`
3. `include/prefab_semantics.hpp`
4. `src/content/prefab_semantics.cpp`
5. `include/editor_plugin.hpp`
6. `src/content/editor_plugin.cpp`
7. `apps/sandbox_dwrite/main.cpp`
8. `apps/sandbox_dwrite/infrastructure/project_tree.cpp`

## Data / Sample Evidence

1. `ProjectSettings/workspace.physicsworkspace`
2. `ProjectSettings/project.physicsproject`
3. `ProjectSettings/editor.physicssettings`
4. `Packages/core_tools.physicspackage`
5. `Packages/builtin.scene_inspector.physicsplugin`
6. `Packages/builtin.failing_menu.physicsplugin`
7. `Prefabs/phase_g_base.prefab`
8. `Prefabs/phase_g_nested.prefab`
9. `Prefabs/phase_g_variant.prefab`

## Test Evidence

1. `tests/regression_project_workspace_tests.cpp`
2. `tests/regression_prefab_semantics_tests.cpp`
3. `tests/regression_editor_plugin_tests.cpp`

## Gate Archive

1. `artifacts/phase_g_gate_suite_20260306_193420/summary.md`
