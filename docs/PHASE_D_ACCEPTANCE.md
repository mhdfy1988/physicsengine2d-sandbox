# 阶段 D 验收清单（编辑器化与工具链完善）

更新时间：2026-03-06  
状态：D4 完成，阶段 D 完成

对应设计：[`PHASE_D_DETAILED_DESIGN.md`](./PHASE_D_DETAILED_DESIGN.md)  
进度跟踪：[`PHASE_D_PROGRESS.md`](./PHASE_D_PROGRESS.md)  
证据归档：[`PHASE_D_EVIDENCE.md`](./PHASE_D_EVIDENCE.md)

## A. D1 热重载与开发体验基线强化

- [x] 原生文件监听后端已接入（保留轮询 fallback）
- [x] sandbox 热重载运行路径已切换到 watcher 抽象层
- [x] 热重载错误分类升级为细粒度 taxonomy
- [x] 无界面 hot-reload smoke harness 可在 CI 执行
- [x] D1 证据包归档（命令、日志、关键产物）

## B. D2 编辑器核心工作流

- [x] Scene Tree 支持场景项选择同步（层级点击切换到指定场景）
- [x] Scene Tree 支持场景重命名（双击或 `F2`）
- [x] Scene Tree 支持场景排序
- [x] Inspector 支持核心组件字段编辑与约束校验
- [x] 统一命令总线驱动编辑操作
- [x] Undo/Redo 覆盖核心编辑行为并有回归
- [x] 资产引用编辑默认走 GUID 安全路径

## C. D3 PIE 与调试面板

- [x] PIE 生命周期（Enter/Exit/Pause/Step）完整可用
- [x] 编辑态与运行态状态隔离，回切行为可预测
- [x] 调试面板可查看事件流/错误流/关键性能指标
- [x] PIE 过程中热重载可观测且失败可回退

## D. D4 子系统与工具链补齐

- [x] Render2D 工作流可演示并可回归
- [x] Animation 工作流可演示并可回归
- [x] Audio 工作流可演示并可回归
- [x] 脚本桥接层最小可用且有 smoke 覆盖
- [x] 并行调度优化有基准对比报告

## E. 阶段门禁与封板

- [x] `mingw32-make test` 持续通过
- [x] `mingw32-make sandbox` 持续通过
- [x] `mingw32-make benchmark` 持续通过
- [x] `scripts/check_arch_deps.ps1` 持续通过
- [x] `scripts/check_api_surface.ps1` 持续通过
- [x] `scripts/hot_reload_smoke.ps1` 持续通过
- [x] `scripts/hot_reload_smoke_headless.ps1` 通过并纳入门禁
- [x] `scripts/run_phase_d_gate_suite.ps1` 一键门禁通过并归档摘要
- [x] 阶段 D 证据归档完成
- [x] 阶段 D 封板报告输出

## 2026-03-06 D1 Snapshot

- [x] 新增 watcher 抽象：`include/asset_fs_watch.hpp`, `src/content/asset_fs_watch.cpp`
- [x] sandbox 接入 watcher 抽象：`apps/sandbox_dwrite/main.cpp`
- [x] 构建接入：`Makefile`
- [x] 新增热重载错误 taxonomy：`apps/sandbox_dwrite/infrastructure/app_event_bus.hpp`, `apps/sandbox_dwrite/main.cpp`
- [x] 新增 headless smoke：`scripts/hot_reload_smoke_headless.ps1`
- [x] 新增 D 阶段一键门禁：`scripts/run_phase_d_gate_suite.ps1`
- [x] runtime smoke 增补 taxonomy 断言：`tests/app_runtime_tick_smoke.cpp`
- [x] 验证通过：`mingw32-make test`, `mingw32-make sandbox`, `mingw32-make benchmark`, `scripts/hot_reload_smoke.ps1`, `scripts/hot_reload_smoke_headless.ps1 -SkipBuild`, `scripts/check_arch_deps.ps1`, `scripts/check_api_surface.ps1`
- [x] 门禁摘要：`artifacts/phase_d_gate_suite_20260306_010737/summary.md`

## 2026-03-06 D2 Kickoff Snapshot (Scene Tree)

- [x] Scene Tree 从“单行当前场景”升级为“全场景列表”并支持逐项点击切换
- [x] 场景重命名能力接入：层级双击或 `F2` 打开名称输入
- [x] 层级过滤已对场景列表生效（无匹配时显示占位提示）
- [x] 验证通过：`mingw32-make test`, `mingw32-make sandbox`, `mingw32-make benchmark`, `scripts/check_arch_deps.ps1`, `scripts/check_api_surface.ps1`, `scripts/hot_reload_smoke.ps1`, `scripts/hot_reload_smoke_headless.ps1 -SkipBuild`
- [x] 门禁摘要：`artifacts/phase_d_gate_suite_20260306_012203/summary.md`

## 2026-03-06 D2 Scene Tree Ordering Snapshot

- [x] Scene Tree 已支持排序视图（渲染与过滤均按 `scene_order` 生效）
- [x] 排序快捷键已接入：`Alt+Up` / `Alt+Down` / `Alt+Home`
- [x] 帮助文档已同步排序操作提示
- [x] 验证通过：`mingw32-make test`, `mingw32-make sandbox`, `mingw32-make benchmark`, `scripts/check_arch_deps.ps1`, `scripts/check_api_surface.ps1`, `scripts/hot_reload_smoke.ps1`, `scripts/hot_reload_smoke_headless.ps1 -SkipBuild`
- [x] 门禁摘要：`artifacts/phase_d_gate_suite_20260306_013031/summary.md`

## 2026-03-06 D2 Inspector Core Editing Snapshot

- [x] Inspector 输入路径升级为严格数值解析（拒绝非法尾随字符与 NaN/Inf）
- [x] Inspector 核心字段已接入范围校验（物体 + 约束）
- [x] Inspector 支持行内 `- / +` 与键盘 `Left/Right/-/+` 微调
- [x] 帮助文案与字段提示已同步可编辑范围
- [x] 验证通过：`mingw32-make test`, `mingw32-make sandbox`, `mingw32-make benchmark`, `scripts/check_arch_deps.ps1`, `scripts/check_api_surface.ps1`, `scripts/hot_reload_smoke.ps1`, `scripts/hot_reload_smoke_headless.ps1 -SkipBuild`
- [x] 门禁摘要：`artifacts/phase_d_gate_suite_20260306_014251/summary.md`

## 2026-03-06 D2 Command Bus Snapshot

- [x] 新增 `editor_command_bus` 模块并接入主编辑路径
- [x] Scene Rename / Scene Order / Inspector Set Value 已统一走 command bus dispatch
- [x] 新增 `tests/editor_undo_redo_smoke.cpp` 并接入 `mingw32-make test`
- [x] 验证通过：`mingw32-make test`, `mingw32-make sandbox`, `mingw32-make benchmark`, `scripts/check_arch_deps.ps1`, `scripts/check_api_surface.ps1`, `scripts/hot_reload_smoke.ps1`, `scripts/hot_reload_smoke_headless.ps1 -SkipBuild`
- [x] 门禁摘要：`artifacts/phase_d_gate_suite_20260306_015548/summary.md`

## 2026-03-06 D2 Undo/Redo Core Coverage Snapshot

- [x] 历史快照已包含编辑器元数据（场景名/场景顺序/当前场景索引）
- [x] Scene Rename 与 Scene Order 已接入可撤销路径
- [x] `tests/editor_undo_redo_smoke.cpp` 已覆盖混合命令序列的多步 undo/redo 回放
- [x] 验证通过：`mingw32-make test`, `mingw32-make sandbox`, `mingw32-make benchmark`, `scripts/check_arch_deps.ps1`, `scripts/check_api_surface.ps1`, `scripts/hot_reload_smoke.ps1`, `scripts/hot_reload_smoke_headless.ps1 -SkipBuild`
- [x] 门禁摘要：`artifacts/phase_d_gate_suite_20260306_020552/summary.md`

## 2026-03-06 D2 GUID Asset Reference Safety Snapshot

- [x] Scene config now carries GUID-only asset reference field (`asset_ref_guid`).
- [x] Editor input path enforces `asset://<safe-id>` validation (rejects path-like/non-ASCII values).
- [x] Command bus extended with `EDITOR_CMD_SCENE_ASSET_REF_SET` and integrated into undo/redo.
- [x] Snapshot metadata now persists/restores per-scene `SCENE_ASSET_GUID` entries.
- [x] Regression + smoke coverage updated:
  - `tests/regression_asset_database_tests.cpp` (GUID validator rules)
  - `tests/editor_undo_redo_smoke.cpp` (scene asset GUID command + replay)
- [x] Verification passed:
  - `mingw32-make test`
  - `scripts/run_phase_d_gate_suite.ps1` (summary: `artifacts/phase_d_gate_suite_20260306_022153/summary.md`)

## 2026-03-06 D3 PIE Lifecycle + State Isolation Snapshot

- [x] PIE lifecycle is wired and usable in sandbox:
  - Enter: first `Space`
  - Pause/Resume: `Space`
  - Step: `N` (paused single-frame tick)
  - Exit: `Esc` (restore editor snapshot)
- [x] Runtime/editor isolation guardrails are active:
  - PIE exit restores frozen editor-state snapshot
  - scene switch/reorder/rename/GUID-edit paths are blocked while PIE is active
  - autosave is suppressed during PIE session
- [x] Added D3 smoke gate:
  - `tests/editor_pie_lifecycle_smoke.cpp`
  - included in `mingw32-make test`
- [x] Verification passed:
  - `mingw32-make test`
  - `mingw32-make sandbox`
  - `scripts/check_arch_deps.ps1`
  - `scripts/check_api_surface.ps1`

## 2026-03-06 D3 Debug Observability + PIE Hot-Reload Closure

- [x] Debug panel now exposes the remaining D3 visibility surfaces:
  - recent event flow history (commands / PIE state changes / bus recovery / hot-reload batches)
  - recent error flow history (runtime taxonomy errors / event-bus drops / hot-reload failures)
  - compact performance summary (FPS avg/min/max, step avg/peak, event-drop and reload totals)
- [x] PIE + hot-reload coexistence is now directly observable in UI:
  - each hot-reload batch records whether it happened in editor mode or PIE
  - failed hot-reload batches surface "rollback retained" semantics in panel history
  - imported asset GUIDs from recent batches are visible from the debug panel
- [x] Runtime snapshot contract extended for D3 UI:
  - `AppHotReloadSnapshot` now carries `pie_active` and `rollback_retained`
  - smoke coverage updated in `tests/app_runtime_tick_smoke.cpp`
- [x] PIE lifecycle log text no longer emits mojibake in sandbox logs.
- [x] Verification passed:
  - `mingw32-make test`
  - `mingw32-make sandbox`
  - `scripts/check_arch_deps.ps1`
  - `scripts/check_api_surface.ps1`
  - `scripts/hot_reload_smoke_headless.ps1 -SkipBuild`
  - `scripts/run_phase_d_gate_suite.ps1` (summary: `artifacts/phase_d_gate_suite_20260306_160618/summary.md`)

## 2026-03-06 D4 Subsystem Workflow Smoke Kickoff

- [x] Added a minimal Render2D / Animation / Audio workflow module:
  - `include/subsystem_render_audio_animation.hpp`
  - `src/content/subsystem_render_audio_animation.cpp`
- [x] Workflow smoke covers an end-to-end minimum path:
  - texture/audio asset import -> GUID binding
  - animation keyframe tick -> body transform update
  - draw command build
  - audio play command collection
- [x] Added D4 smoke gate:
  - `tests/subsystem_render_audio_animation_smoke.cpp`
  - included in `mingw32-make test`
- [x] Build-system wiring updated:
  - `Makefile`
  - `CMakeLists.txt`
- [x] Verification passed:
  - `mingw32-make test`
  - `mingw32-make sandbox`
  - `scripts/run_phase_d_gate_suite.ps1` (summary: `artifacts/phase_d_gate_suite_20260306_160618/summary.md`)

## 2026-03-06 D4 Closure Snapshot

- [x] Render2D / Animation / Audio workflow now has both regression and demo surfaces:
  - smoke: `tests/subsystem_render_audio_animation_smoke.cpp`
  - demo: `tools/subsystem_workflow_demo.cpp`
- [x] Minimal script bridge path is complete and regression-covered:
  - bridge: `include/physics2d/physics_script_bridge.hpp`
  - smoke: `tests/cpp_script_bridge_smoke.cpp`
- [x] First-pass profiling and parallel scheduling evidence is archived:
  - scheduling path: `src/core/physics_parallel.cpp`
  - compare tool: `tools/parallel_benchmark_compare.cpp`
  - profile capture: `tools/phase_d_profile_capture.cpp`
  - report: [`PHASE_D_PROFILE_REPORT.md`](./PHASE_D_PROFILE_REPORT.md)
- [x] Closure verification passed:
  - `bin/subsystem_render_audio_animation_smoke.exe`
  - `bin/cpp_script_bridge_smoke.exe`
  - `mingw32-make subsystem-workflow-demo`
  - `mingw32-make parallel-benchmark-compare`
  - `mingw32-make phase-d-profile`
  - `scripts/run_phase_d_gate_suite.ps1` (summary: `artifacts/phase_d_gate_suite_20260306_163111/summary.md`)
  - dedicated D4 archive: `artifacts/phase_d_d4_closure_20260306_162937/summary.md`
