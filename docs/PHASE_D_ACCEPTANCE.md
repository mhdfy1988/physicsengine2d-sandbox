# 阶段 D 验收清单（编辑器化与工具链完善）

更新时间：2026-03-06  
状态：进行中（D2）

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
- [ ] Undo/Redo 覆盖核心编辑行为并有回归
- [ ] 资产引用编辑默认走 GUID 安全路径

## C. D3 PIE 与调试面板

- [ ] PIE 生命周期（Enter/Exit/Pause/Step）完整可用
- [ ] 编辑态与运行态状态隔离，回切行为可预测
- [ ] 调试面板可查看事件流/错误流/关键性能指标
- [ ] PIE 过程中热重载可观测且失败可回退

## D. D4 子系统与工具链补齐

- [ ] Render2D 工作流可演示并可回归
- [ ] Animation 工作流可演示并可回归
- [ ] Audio 工作流可演示并可回归
- [ ] 脚本桥接层最小可用且有 smoke 覆盖
- [ ] 并行调度优化有基准对比报告

## E. 阶段门禁与封板

- [x] `mingw32-make test` 持续通过
- [x] `mingw32-make sandbox` 持续通过
- [x] `mingw32-make benchmark` 持续通过
- [x] `scripts/check_arch_deps.ps1` 持续通过
- [x] `scripts/check_api_surface.ps1` 持续通过
- [x] `scripts/hot_reload_smoke.ps1` 持续通过
- [x] `scripts/hot_reload_smoke_headless.ps1` 通过并纳入门禁
- [x] `scripts/run_phase_d_gate_suite.ps1` 一键门禁通过并归档摘要
- [ ] 阶段 D 证据归档完成
- [ ] 阶段 D 封板报告输出

## 2026-03-06 D1 Snapshot

- [x] 新增 watcher 抽象：`include/asset_fs_watch.h`, `src/content/asset_fs_watch.c`
- [x] sandbox 接入 watcher 抽象：`apps/sandbox_dwrite/main.c`
- [x] 构建接入：`Makefile`
- [x] 新增热重载错误 taxonomy：`apps/sandbox_dwrite/infrastructure/app_event_bus.h`, `apps/sandbox_dwrite/main.c`
- [x] 新增 headless smoke：`scripts/hot_reload_smoke_headless.ps1`
- [x] 新增 D 阶段一键门禁：`scripts/run_phase_d_gate_suite.ps1`
- [x] runtime smoke 增补 taxonomy 断言：`tests/app_runtime_tick_smoke.c`
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
- [x] 新增 `tests/editor_undo_redo_smoke.c` 并接入 `mingw32-make test`
- [x] 验证通过：`mingw32-make test`, `mingw32-make sandbox`, `mingw32-make benchmark`, `scripts/check_arch_deps.ps1`, `scripts/check_api_surface.ps1`, `scripts/hot_reload_smoke.ps1`, `scripts/hot_reload_smoke_headless.ps1 -SkipBuild`
- [x] 门禁摘要：`artifacts/phase_d_gate_suite_20260306_015548/summary.md`
