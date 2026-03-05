# 阶段 D 验收清单（编辑器化与工具链完善）

更新时间：2026-03-06  
状态：进行中（D1）

对应设计：[`PHASE_D_DETAILED_DESIGN.md`](./PHASE_D_DETAILED_DESIGN.md)  
进度跟踪：[`PHASE_D_PROGRESS.md`](./PHASE_D_PROGRESS.md)

## A. D1 热重载与开发体验基线强化

- [x] 原生文件监听后端已接入（保留轮询 fallback）
- [x] sandbox 热重载运行路径已切换到 watcher 抽象层
- [x] 热重载错误分类升级为细粒度 taxonomy
- [x] 无界面 hot-reload smoke harness 可在 CI 执行
- [ ] D1 证据包归档（命令、日志、关键产物）

## B. D2 编辑器核心工作流

- [ ] Scene Tree 支持基础层级编辑（选择、重命名、排序）
- [ ] Inspector 支持核心组件字段编辑与约束校验
- [ ] 统一命令总线驱动编辑操作
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
