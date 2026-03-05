# 阶段 D 进度（编辑器化与工具链完善）

更新时间：2026-03-06  
分支：`cpp-migration-baseline`  
状态：D2 进行中（Scene Tree + Inspector + command bus 主路径已落地）

> 跟踪基线：以 [PHASE_D_DETAILED_DESIGN.md](./PHASE_D_DETAILED_DESIGN.md) 和 [PHASE_D_ACCEPTANCE.md](./PHASE_D_ACCEPTANCE.md) 为准。  
> D1 证据包：见 [PHASE_D_EVIDENCE.md](./PHASE_D_EVIDENCE.md)。

## 已完成

1. 已产出阶段 D 详细设计文档（范围、里程碑、门禁、预算、风险）。
2. 已规范化阶段 C 验收文档，明确 `phase-c-signoff` 为签收基线。
3. 已落地 D1 第一项能力：
   - 接入 Win32 原生文件监听触发 + 轮询 fallback（`asset_fs_watch`）
   - sandbox 热重载链路已切换到 watcher 抽象层
   - 既有门禁 `test/sandbox/hot_reload_smoke/arch/api` 全绿

## 进行中

1. D2 Undo/Redo 覆盖核心编辑行为扩展与回归加密。

## 待完成（按里程碑）

### D1：热重载与开发体验基线强化

- [x] 原生文件监听后端接入 + 轮询 fallback
- [x] 热重载错误分类升级（taxonomy）
- [x] 无界面 smoke harness 接入
- [x] D1 验收证据归档

### D2：编辑器核心工作流

- [x] Scene Tree 选择能力（层级点击切换）
- [x] Scene Tree 重命名能力（双击/F2）
- [x] Scene Tree 排序能力
- [x] Inspector 核心组件字段编辑闭环
- [x] 统一 command bus + Undo/Redo 主路径
- [ ] GUID 安全资产引用编辑

### D3：PIE 与调试面板

- [ ] PIE 生命周期（Enter/Exit/Pause/Step）
- [ ] 编辑态/运行态隔离与回切策略
- [ ] 调试面板事件/错误/性能可视化
- [ ] PIE 下热重载共存验证

### D4：子系统与工具链补齐

- [ ] Render2D / Animation / Audio 工作流打通
- [ ] 脚本桥接层最小可用
- [ ] 并行调度优化首轮落地与基准对比
- [ ] 阶段 D 封板报告与证据包

## 当前门禁状态

1. 继承门禁（`test/sandbox/benchmark/arch/api/hot_reload_smoke`）当前保持通过。
2. 新增门禁脚本 `scripts/hot_reload_smoke_headless.ps1` 已落地并通过本地执行。

## 风险与关注点

1. headless harness 当前基于 `physics_tests` 聚合输出，后续可继续细化为独立二进制 smoke 以缩短执行时间。
2. D2-D3 若绕过 command bus，会导致 Undo/Redo 和 PIE 隔离成本失控。
3. 子系统并行推进可能引入跨模块回归，需坚持小步合并和门禁前置。

## 下一次更新触发条件

1. D2 Undo/Redo 覆盖范围扩展到 Scene Tree/Inspector 多字段批量编辑回放。
2. `tests/editor_undo_redo_smoke.c` 增补多命令序列与边界场景断言。
3. D2 GUID 资产引用编辑路径进入首轮可回归状态。

## 2026-03-06 D1 Taxonomy + Headless Smoke

- Added hot-reload runtime error taxonomy codes:
  - `APP_RUNTIME_ERROR_CODE_HOT_RELOAD_SCAN_FAILED`
  - `APP_RUNTIME_ERROR_CODE_HOT_RELOAD_IMPORT_FAILED`
  - `APP_RUNTIME_ERROR_CODE_HOT_RELOAD_BATCH_FAILED`
- Updated sandbox hot-reload error reporting path to use taxonomy-specific codes instead of generic pipeline mapping code.
- Added headless smoke script: `scripts/hot_reload_smoke_headless.ps1`.
- Added runtime smoke assertion for taxonomy error propagation:
  - `tests/app_runtime_tick_smoke.c`
- Added one-click D gate runner:
  - `scripts/run_phase_d_gate_suite.ps1`
- Local verification:
  - `mingw32-make test`
  - `mingw32-make sandbox`
  - `mingw32-make benchmark`
  - `scripts/check_arch_deps.ps1`
  - `scripts/check_api_surface.ps1`
  - `scripts/hot_reload_smoke.ps1`
  - `scripts/hot_reload_smoke_headless.ps1 -SkipBuild`
  - `scripts/run_phase_d_gate_suite.ps1` (PASS, summary: `artifacts/phase_d_gate_suite_20260306_010737/summary.md`)

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
  - `apps/sandbox_dwrite/main.c`
  - `apps/sandbox_dwrite/application/scene_catalog.h`
  - `apps/sandbox_dwrite/application/scene_catalog.c`
- Verification:
  - `mingw32-make test`
  - `mingw32-make sandbox`
  - `mingw32-make benchmark`
  - `scripts/check_arch_deps.ps1`
  - `scripts/check_api_surface.ps1`
  - `scripts/hot_reload_smoke.ps1`
  - `scripts/hot_reload_smoke_headless.ps1 -SkipBuild`
  - `scripts/run_phase_d_gate_suite.ps1` (PASS, summary: `artifacts/phase_d_gate_suite_20260306_012203/summary.md`)

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
  - `apps/sandbox_dwrite/main.c`
- Verification:
  - `mingw32-make test`
  - `mingw32-make sandbox`
  - `mingw32-make benchmark`
  - `scripts/check_arch_deps.ps1`
  - `scripts/check_api_surface.ps1`
  - `scripts/hot_reload_smoke.ps1`
  - `scripts/hot_reload_smoke_headless.ps1 -SkipBuild`
  - `scripts/run_phase_d_gate_suite.ps1` (PASS, summary: `artifacts/phase_d_gate_suite_20260306_013031/summary.md`)

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
  - `apps/sandbox_dwrite/main.c`
- Verification:
  - `mingw32-make test`
  - `mingw32-make sandbox`
  - `mingw32-make benchmark`
  - `scripts/check_arch_deps.ps1`
  - `scripts/check_api_surface.ps1`
  - `scripts/hot_reload_smoke.ps1`
  - `scripts/hot_reload_smoke_headless.ps1 -SkipBuild`
  - `scripts/run_phase_d_gate_suite.ps1` (PASS, summary: `artifacts/phase_d_gate_suite_20260306_014251/summary.md`)

## 2026-03-06 D2 Command Bus Main Path + Undo/Redo Smoke

- Added unified editor command bus module:
  - `EditorCommand` / `EditorCommandCallbacks`
  - dispatch coverage for scene rename, scene order move/reset, inspector set value
- Routed main editor write-paths through command bus dispatch:
  - Scene rename submit
  - Scene order shortcuts (`Alt+Up/Down/Home`)
  - Inspector value commit and micro-adjust
- Integrated initial undo/redo smoke gate:
  - added `tests/editor_undo_redo_smoke.c`
  - wired into `mingw32-make test` pipeline
- Touched files:
  - `apps/sandbox_dwrite/application/editor_command_bus.h`
  - `apps/sandbox_dwrite/application/editor_command_bus.c`
  - `apps/sandbox_dwrite/main.c`
  - `tests/editor_undo_redo_smoke.c`
  - `Makefile`
- Verification:
  - `mingw32-make test` (includes `editor_undo_redo_smoke`)
  - `mingw32-make sandbox`
  - `mingw32-make benchmark`
  - `scripts/check_arch_deps.ps1`
  - `scripts/check_api_surface.ps1`
  - `scripts/hot_reload_smoke.ps1`
  - `scripts/hot_reload_smoke_headless.ps1 -SkipBuild`
  - `scripts/run_phase_d_gate_suite.ps1` (PASS, summary: `artifacts/phase_d_gate_suite_20260306_015548/summary.md`)
