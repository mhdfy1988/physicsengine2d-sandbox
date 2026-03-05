# Phase C Commit Plan

Date: 2026-03-06

## Commit 1: Content Core (schema + pipeline)

Scope:
1. `include/scene_schema.h`, `src/content/scene_schema.c`, `tools/scene_migrate_main.c`
2. `include/prefab_schema.h`, `src/content/prefab_schema.c`
3. `include/asset_database.h`, `src/content/asset_database.c`
4. `include/asset_importer.h`, `src/content/asset_importer.c`
5. `include/asset_invalidation.h`, `src/content/asset_invalidation.c`
6. `include/asset_pipeline.h`, `src/content/asset_pipeline.c`

Message:
`feat(content): add schema v1, migration tool, and deterministic asset pipeline baseline`

## Commit 2: Runtime Hot-Reload Integration

Scope:
1. `include/asset_watch.h`, `src/content/asset_watch.c`
2. `include/asset_hot_reload.h`, `src/content/asset_hot_reload.c`
3. `include/asset_fs_poll.h`, `src/content/asset_fs_poll.c`
4. `apps/sandbox_dwrite/infrastructure/app_event_bus.h`
5. `apps/sandbox_dwrite/application/app_runtime.h`
6. `apps/sandbox_dwrite/application/app_runtime.c`
7. `apps/sandbox_dwrite/main.c`
8. `apps/sandbox_dwrite/application/app_controller.c`

Message:
`feat(runtime): integrate hot-reload watch/poll pipeline into sandbox loop and event bus`

## Commit 3: Regression Coverage

Scope:
1. `tests/regression_scene_schema_tests.c`
2. `tests/regression_prefab_schema_tests.c`
3. `tests/regression_asset_database_tests.c`
4. `tests/regression_asset_importer_tests.c`
5. `tests/regression_asset_pipeline_tests.c`
6. `tests/regression_asset_hot_reload_tests.c`
7. `tests/regression_tests.h`
8. `tests/app_runtime_tick_smoke.c`
9. `tests/data/*`

Message:
`test(content): add regression and smoke coverage for schema, pipeline, and hot-reload`

## Commit 4: Build Wiring and Scripts

Scope:
1. `Makefile`
2. `CMakeLists.txt`
3. `scripts/hot_reload_smoke.ps1`
4. `scripts/run_phase_c_gate_suite.ps1`
5. `.gitignore`

Message:
`build(ci): wire phase-c modules and add gate/smoke automation scripts`

## Commit 5: Docs and Sign-Off Pack

Scope:
1. `docs/SCENE_SCHEMA_V1.md`
2. `docs/PREFAB_SCHEMA_V1.md`
3. `docs/ASSET_META_V1.md`
4. `docs/ASSET_IMPORTER_V1.md`
5. `docs/ASSET_PIPELINE_V1.md`
6. `docs/ASSET_HOT_RELOAD_V1.md`
7. `docs/PHASE_C_DETAILED_DESIGN.md`
8. `docs/PHASE_C_PROGRESS.md`
9. `docs/PHASE_C_ACCEPTANCE.md`
10. `docs/PHASE_C_EVIDENCE.md`
11. `docs/PHASE_C_GATE_SUITE_20260306_003435.md`
12. `docs/PHASE_C_FINAL_REPORT.md`

Message:
`docs(phase-c): publish acceptance, evidence pack, and final seal report`

## Optional Commit 6: Sample Project Artifacts

Scope:
1. `Scenes/*`
2. `Prefabs/*`
3. `ProjectSettings/*`
4. `assets/hero.png.meta`

Message:
`chore(samples): add phase-c sample scene/prefab/settings assets`
