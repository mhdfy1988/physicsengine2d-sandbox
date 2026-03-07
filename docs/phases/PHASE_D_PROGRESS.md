# 阶段 D 进度（编辑器化与工具链完善）

更新时间�?026-03-06  
分支：`cpp-migration-baseline`  
状态：阶段 D 完成（已封板�?
> 跟踪基线：以 [PHASE_D_DETAILED_DESIGN.md](./PHASE_D_DETAILED_DESIGN.md) �?[PHASE_D_ACCEPTANCE.md](./PHASE_D_ACCEPTANCE.md) 为准�? 
> D1 证据包：�?[PHASE_D_EVIDENCE.md](../reports/PHASE_D_EVIDENCE.md)�?
## 已完�?
1. 已产出阶�?D 详细设计文档（范围、里程碑、门禁、预算、风险）�?2. 已规范化阶段 C 验收文档，明�?`phase-c-signoff` 为签收基线�?3. 已落�?D1 第一项能力：
   - 接入 Win32 原生文件监听触发 + 轮询 fallback（`asset_fs_watch`�?   - sandbox 热重载链路已切换�?watcher 抽象�?   - 既有门禁 `test/sandbox/hot_reload_smoke/arch/api` 全绿
4. D4 已收口：
   - Render2D / Animation / Audio workflow 已具�?smoke + demo
   - Script bridge 已打�?ECS/runtime 最小闭环并�?smoke
   - profiling / parallel benchmark 工具与报告已归档
   - 阶段 D gate suite 已纳�?D4 专项命令并全�?
## 进行�?
1. 无（阶段 D 已封板）�?
## 待完成（按里程碑�?
### D1：热重载与开发体验基线强�?
- [x] 原生文件监听后端接入 + 轮询 fallback
- [x] 热重载错误分类升级（taxonomy�?- [x] 无界�?smoke harness 接入
- [x] D1 验收证据归档

### D2：编辑器核心工作�?
- [x] Scene Tree 选择能力（层级点击切换）
- [x] Scene Tree 重命名能力（双击/F2�?- [x] Scene Tree 排序能力
- [x] Inspector 核心组件字段编辑闭环
- [x] 统一 command bus + Undo/Redo 主路�?- [x] Undo/Redo 覆盖核心编辑行为并有回归
- [x] GUID 安全资产引用编辑

### D3：PIE 与调试面�?
- [x] PIE 生命周期（Enter/Exit/Pause/Step�?- [x] 编辑�?运行态隔离与回切策略
- [x] 调试面板事件/错误/性能可视�?- [x] PIE 下热重载共存验证

### D4：子系统与工具链补齐

- [x] Render2D / Animation / Audio 工作流打�?- [x] 脚本桥接层最小可�?- [x] 并行调度优化首轮落地与基准对�?- [x] 阶段 D 封板报告与证据包

## 当前门禁状�?
1. 继承门禁（`test/sandbox/benchmark/arch/api/hot_reload_smoke`）当前保持通过�?2. D 阶段一键门�?`scripts/gate/run_phase_d_gate_suite.ps1` 已纳�?D4 专项命令并再次全绿�?3. 最新摘要：`artifacts/phase_d_gate_suite_20260306_163111/summary.md`�?4. D4 专项摘要：`artifacts/phase_d_d4_closure_20260306_162937/summary.md`�?
## 风险与关注点

1. headless harness 当前基于 `physics_tests` 聚合输出，后续可继续细化为独立二进制 smoke 以缩短执行时间�?2. D2-D3 若绕�?command bus，会导致 Undo/Redo �?PIE 隔离成本失控�?3. 当前并行调度首轮优化已把 960-body 重负载样例拉�?break-even 以上，但扩大收益仍需要更深层线程�?任务图工作�?
## 下一次更新触发条�?
1. 阶段 D 已封板；后续更新以新阶段计划文档为准�?
## 2026-03-06 D1 Taxonomy + Headless Smoke

- Added hot-reload runtime error taxonomy codes:
  - `APP_RUNTIME_ERROR_CODE_HOT_RELOAD_SCAN_FAILED`
  - `APP_RUNTIME_ERROR_CODE_HOT_RELOAD_IMPORT_FAILED`
  - `APP_RUNTIME_ERROR_CODE_HOT_RELOAD_BATCH_FAILED`
- Updated sandbox hot-reload error reporting path to use taxonomy-specific codes instead of generic pipeline mapping code.
- Added headless smoke script: `scripts/dev/hot_reload_smoke_headless.ps1`.
- Added runtime smoke assertion for taxonomy error propagation:
  - `tests/app_runtime_tick_smoke.cpp`
- Added one-click D gate runner:
  - `scripts/gate/run_phase_d_gate_suite.ps1`
- Local verification:
  - `mingw32-make test`
  - `mingw32-make sandbox`
  - `mingw32-make benchmark`
  - `scripts/check/check_arch_deps.ps1`
  - `scripts/check/check_api_surface.ps1`
  - `scripts/dev/hot_reload_smoke.ps1`
  - `scripts/dev/hot_reload_smoke_headless.ps1 -SkipBuild`
  - `scripts/gate/run_phase_d_gate_suite.ps1` (PASS, summary: `artifacts/phase_d_gate_suite_20260306_010737/summary.md`)

## 2026-03-06 D2 Kickoff (Scene Tree Select + Rename)

- Upgraded hierarchy scene section from single current-scene row to full scene list with per-row selection.
- Added scene rename entry:
  - double-click a scene row in hierarchy
  - press `F2` while hierarchy area is focused
- Added runtime editable scene names (initialized from catalog defaults) and preserved fallback semantics.
- Updated value-input flow:
  - text targets now bypass numeric-only validation path
  - new scene-name target and modal title mapping added
- Touched files:
  - `apps/sandbox_dwrite/main.cpp`
  - `apps/sandbox_dwrite/application/scene_catalog.hpp`
  - `apps/sandbox_dwrite/application/scene_catalog.cpp`
- Verification:
  - `mingw32-make test`
  - `mingw32-make sandbox`
  - `mingw32-make benchmark`
  - `scripts/check/check_arch_deps.ps1`
  - `scripts/check/check_api_surface.ps1`
  - `scripts/dev/hot_reload_smoke.ps1`
  - `scripts/dev/hot_reload_smoke_headless.ps1 -SkipBuild`
  - `scripts/gate/run_phase_d_gate_suite.ps1` (PASS, summary: `artifacts/phase_d_gate_suite_20260306_012203/summary.md`)

## 2026-03-06 D2 Scene Tree Ordering (Sort)

- Added scene ordering state and reorder helpers:
  - scene list now renders by `scene_order` instead of raw catalog index
  - scene content-height/filtering path switched to ordered traversal
- Added hierarchy reorder shortcuts:
  - `Alt+Up`: move current scene up by one position
  - `Alt+Down`: move current scene down by one position
  - `Alt+Home`: reset scene order to default
- Updated help modal usage text with sorting shortcut hints.
- Touched files:
  - `apps/sandbox_dwrite/main.cpp`
- Verification:
  - `mingw32-make test`
  - `mingw32-make sandbox`
  - `mingw32-make benchmark`
  - `scripts/check/check_arch_deps.ps1`
  - `scripts/check/check_api_surface.ps1`
  - `scripts/dev/hot_reload_smoke.ps1`
  - `scripts/dev/hot_reload_smoke_headless.ps1 -SkipBuild`
  - `scripts/gate/run_phase_d_gate_suite.ps1` (PASS, summary: `artifacts/phase_d_gate_suite_20260306_013031/summary.md`)

## 2026-03-06 D2 Inspector Core Editing + Constraint Validation

- Added strict numeric parsing for inspector/debug value input:
  - reject non-canonical numeric strings
  - reject NaN/Inf
- Added inspector field-level validation and range guards:
  - body core fields: mass/position/velocity/angular/damping/restitution
  - constraint core fields: rest_length/stiffness/damping/break_force/collide_connected/spring preset
- Added inspector inline micro-adjust workflow:
  - per-row `- / +` buttons in inspector
  - keyboard micro-adjust: `Left/Right`, `-`, `+` on focused inspector row
  - boundary-aware step clamp and no-op suppression
- Updated inspector hints and help text to reflect validation ranges and micro-adjust controls.
- Touched files:
  - `apps/sandbox_dwrite/main.cpp`
- Verification:
  - `mingw32-make test`
  - `mingw32-make sandbox`
  - `mingw32-make benchmark`
  - `scripts/check/check_arch_deps.ps1`
  - `scripts/check/check_api_surface.ps1`
  - `scripts/dev/hot_reload_smoke.ps1`
  - `scripts/dev/hot_reload_smoke_headless.ps1 -SkipBuild`
  - `scripts/gate/run_phase_d_gate_suite.ps1` (PASS, summary: `artifacts/phase_d_gate_suite_20260306_014251/summary.md`)

## 2026-03-06 D2 Command Bus Main Path + Undo/Redo Smoke

- Added unified editor command bus module:
  - `EditorCommand` / `EditorCommandCallbacks`
  - dispatch coverage for scene rename, scene order move/reset, inspector set value
- Routed main editor write-paths through command bus dispatch:
  - Scene rename submit
  - Scene order shortcuts (`Alt+Up/Down/Home`)
  - Inspector value commit and micro-adjust
- Integrated initial undo/redo smoke gate:
- added `tests/editor_undo_redo_smoke.cpp`
  - wired into `mingw32-make test` pipeline
- Touched files:
  - `apps/sandbox_dwrite/application/editor_command_bus.hpp`
  - `apps/sandbox_dwrite/application/editor_command_bus.cpp`
  - `apps/sandbox_dwrite/main.cpp`
  - `tests/editor_undo_redo_smoke.cpp`
  - `Makefile`
- Verification:
  - `mingw32-make test` (includes `editor_undo_redo_smoke`)
  - `mingw32-make sandbox`
  - `mingw32-make benchmark`
  - `scripts/check/check_arch_deps.ps1`
  - `scripts/check/check_api_surface.ps1`
  - `scripts/dev/hot_reload_smoke.ps1`
  - `scripts/dev/hot_reload_smoke_headless.ps1 -SkipBuild`
  - `scripts/gate/run_phase_d_gate_suite.ps1` (PASS, summary: `artifacts/phase_d_gate_suite_20260306_015548/summary.md`)

## 2026-03-06 D2 Undo/Redo Core Coverage Expansion

- Extended snapshot persistence to include editor metadata:
  - scene active index
  - scene order
  - scene display names (UTF-8 hex encoded metadata lines)
- Hooked scene rename and scene order commands into history stack:
  - rename/order changes now generate undoable snapshots
- Expanded `editor_undo_redo_smoke` to verify mixed command sequences:
  - rename + order move/reset + inspector edits
  - multi-step undo/redo replay assertions
- Touched files:
  - `apps/sandbox_dwrite/main.cpp`
  - `tests/editor_undo_redo_smoke.cpp`
- Verification:
  - `mingw32-make test` (includes extended `editor_undo_redo_smoke`)
  - `mingw32-make sandbox`
  - `mingw32-make benchmark`
  - `scripts/check/check_arch_deps.ps1`
  - `scripts/check/check_api_surface.ps1`
  - `scripts/dev/hot_reload_smoke.ps1`
  - `scripts/dev/hot_reload_smoke_headless.ps1 -SkipBuild`
  - `scripts/gate/run_phase_d_gate_suite.ps1` (PASS, summary: `artifacts/phase_d_gate_suite_20260306_020552/summary.md`)

## 2026-03-06 D2 GUID-Safe Asset Reference Editing Closure

- Added scene-level asset reference GUID field:
  - `SceneConfig.asset_ref_guid` (default `asset://none`)
- Added GUID-safe validation path:
  - input must satisfy `asset://<safe-id>`
  - non-ASCII/path-like references are rejected in editor input
  - shared validator added: `asset_meta_is_valid_guid()`
- Routed scene asset reference edits through command bus:
  - new command `EDITOR_CMD_SCENE_ASSET_REF_SET`
  - callback path now pushes history snapshots for undo/redo
- Extended snapshot editor metadata:
  - persisted `SCENE_ASSET_GUID <idx> <hex>` records
  - restored into scene config on snapshot load
- Updated editor interaction:
  - `F3` opens current-scene asset GUID edit input
  - hierarchy scene header now shows `F2/F3` edit hints
- Expanded regression/smoke:
  - `tests/regression_asset_database_tests.cpp` now validates GUID format rules
  - `tests/editor_undo_redo_smoke.cpp` now covers scene asset GUID command with multi-step undo/redo replay
- Touched files:
  - `include/asset_database.hpp`
  - `src/content/asset_database.cpp`
  - `apps/sandbox_dwrite/application/scene_catalog.hpp`
  - `apps/sandbox_dwrite/application/scene_catalog.cpp`
  - `apps/sandbox_dwrite/application/editor_command_bus.hpp`
  - `apps/sandbox_dwrite/application/editor_command_bus.cpp`
  - `apps/sandbox_dwrite/main.cpp`
  - `tests/regression_asset_database_tests.cpp`
  - `tests/editor_undo_redo_smoke.cpp`
- Verification:
  - `mingw32-make test`
  - `scripts/gate/run_phase_d_gate_suite.ps1` (PASS, summary: `artifacts/phase_d_gate_suite_20260306_022153/summary.md`)

## 2026-03-06 D3 PIE Lifecycle + State Isolation Kickoff

- Added a dedicated PIE lifecycle module:
  - `apps/sandbox_dwrite/application/pie_lifecycle.hpp`
  - `apps/sandbox_dwrite/application/pie_lifecycle.cpp`
- Wired sandbox runtime controls to PIE semantics:
  - `Space`: enter PIE (first trigger) and run/pause toggle
  - `N`: single-step in PIE paused state
  - `Esc`: exit PIE and restore frozen editor snapshot
- Added edit/runtime isolation guards in PIE:
  - scene switch/reorder/rename/GUID edit paths are blocked while PIE is active
  - autosave no longer writes runtime-mutated state during PIE session
- Updated physics menu run action to use command callback dispatch instead of direct `running` bit flip.
- Added D3 gate smoke:
  - `tests/editor_pie_lifecycle_smoke.cpp`
  - verifies enter/exit idempotence and save/load failure behavior
- Touched files:
  - `apps/sandbox_dwrite/main.cpp`
  - `apps/sandbox_dwrite/presentation/input/menu_view_physics_window_actions.hpp`
  - `apps/sandbox_dwrite/presentation/input/menu_view_physics_window_actions.cpp`
  - `Makefile`
  - `tests/editor_pie_lifecycle_smoke.cpp`
- Verification:
  - `mingw32-make test` (includes `editor_pie_lifecycle_smoke`)
  - `mingw32-make sandbox`
  - `scripts/check/check_arch_deps.ps1`
  - `scripts/check/check_api_surface.ps1`

## 2026-03-06 D3 Debug Observability + PIE Hot-Reload Closure

- Extended runtime hot-reload snapshot payload:
  - `AppHotReloadSnapshot.pie_active`
  - `AppHotReloadSnapshot.rollback_retained`
- Updated sandbox D3 debug panel coverage:
  - recent event flow history
  - recent error flow history
  - compact performance summary
  - recent hot-reload batch history with imported GUID details
- Added explicit PIE/hot-reload coexistence surfacing:
  - batches now show whether they happened during PIE or editor mode
  - failed batches explicitly surface retained previous-cache semantics
- Fixed PIE lifecycle log strings to readable text (removed mojibake output).
- Touched files:
  - `apps/sandbox_dwrite/main.cpp`
  - `apps/sandbox_dwrite/infrastructure/app_event_bus.hpp`
  - `apps/sandbox_dwrite/application/app_runtime.hpp`
  - `apps/sandbox_dwrite/application/app_runtime.cpp`
  - `apps/sandbox_dwrite/application/pie_lifecycle.cpp`
  - `tests/app_runtime_tick_smoke.cpp`
- Verification:
  - `mingw32-make test`
  - `mingw32-make sandbox`
  - `scripts/check/check_arch_deps.ps1`
  - `scripts/check/check_api_surface.ps1`
  - `scripts/dev/hot_reload_smoke_headless.ps1 -SkipBuild`
  - `scripts/gate/run_phase_d_gate_suite.ps1` (PASS, summary: `artifacts/phase_d_gate_suite_20260306_160618/summary.md`)

## 2026-03-06 D4 Subsystem Workflow Smoke Kickoff

- Added a minimal workflow module for Render2D / Animation / Audio:
  - bind imported texture/audio GUIDs to runtime bodies
  - drive body transforms from animation keyframes
  - emit draw commands and audio play commands for smoke verification
- New files:
  - `include/subsystem_render_audio_animation.hpp`
  - `src/content/subsystem_render_audio_animation.cpp`
  - `tests/subsystem_render_audio_animation_smoke.cpp`
- Build wiring updated:
  - `Makefile` (`mingw32-make test` now runs the D4 smoke)
  - `CMakeLists.txt` (core source list and sandbox source list synchronized)
- Verification:
  - `mingw32-make test`
  - `mingw32-make sandbox`
  - `scripts/gate/run_phase_d_gate_suite.ps1` (PASS, summary: `artifacts/phase_d_gate_suite_20260306_160618/summary.md`)

## 2026-03-06 D4 Closure (Workflow + Script Bridge + Profiling)

- Added a minimal C++ script bridge surface:
  - `include/physics2d/physics_script_bridge.hpp`
  - `tests/cpp_script_bridge_smoke.cpp`
- Added D4 demo / profiling utilities:
  - `tools/subsystem_workflow_demo.cpp`
  - `tools/parallel_benchmark_compare.cpp`
  - `tools/phase_d_profile_capture.cpp`
- Updated Win32 parallel dispatch to use thread-pool work items instead of per-dispatch thread creation:
  - `src/core/physics_parallel.cpp`
- Extended one-click gate coverage:
  - `scripts/gate/run_phase_d_gate_suite.ps1` now also runs `parallel-benchmark-compare`, `subsystem-workflow-demo`, and `phase-d-profile`
- Verification:
  - `bin/subsystem_render_audio_animation_smoke.exe`
  - `bin/cpp_script_bridge_smoke.exe`
  - `mingw32-make subsystem-workflow-demo`
  - `mingw32-make parallel-benchmark-compare`
  - `mingw32-make phase-d-profile`
  - `scripts/gate/run_phase_d_gate_suite.ps1` (PASS, summary: `artifacts/phase_d_gate_suite_20260306_163111/summary.md`)
