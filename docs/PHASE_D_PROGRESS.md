# 阶段 D 进度（编辑器化与工具链完善）

更新时间：2026-03-06  
分支：`cpp-migration-baseline`  
状态：已启动（D1 进行中）

> 跟踪基线：以 [PHASE_D_DETAILED_DESIGN.md](./PHASE_D_DETAILED_DESIGN.md) 和 [PHASE_D_ACCEPTANCE.md](./PHASE_D_ACCEPTANCE.md) 为准。

## 已完成

1. 已产出阶段 D 详细设计文档（范围、里程碑、门禁、预算、风险）。
2. 已规范化阶段 C 验收文档，明确 `phase-c-signoff` 为签收基线。
3. 已落地 D1 第一项能力：
   - 接入 Win32 原生文件监听触发 + 轮询 fallback（`asset_fs_watch`）
   - sandbox 热重载链路已切换到 watcher 抽象层
   - 既有门禁 `test/sandbox/hot_reload_smoke/arch/api` 全绿

## 进行中

1. D1 热重载错误分类升级（从共享错误码提升到细粒度 taxonomy）。
2. D1 无界面 hot-reload smoke harness（CI 友好）设计与落地。

## 待完成（按里程碑）

### D1：热重载与开发体验基线强化

- [x] 原生文件监听后端接入 + 轮询 fallback
- [ ] 热重载错误分类升级（taxonomy）
- [ ] 无界面 smoke harness 接入
- [ ] D1 验收证据归档

### D2：编辑器核心工作流

- [ ] Scene Tree 基础编辑能力（选择/重命名/排序）
- [ ] Inspector 核心组件字段编辑闭环
- [ ] 统一 command bus + Undo/Redo 主路径
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
2. D 阶段新增门禁尚未全部落地（`hot_reload_smoke_headless` 等待实现）。

## 风险与关注点

1. D1 若不先完成 headless harness，CI 对热重载的覆盖仍受桌面环境限制。
2. D2-D3 若绕过 command bus，会导致 Undo/Redo 和 PIE 隔离成本失控。
3. 子系统并行推进可能引入跨模块回归，需坚持小步合并和门禁前置。

## 下一次更新触发条件

1. 完成 D1 错误 taxonomy 并通过回归。
2. `scripts/hot_reload_smoke_headless.ps1` 首次通过并进入门禁。
3. D2 第一条可用编辑链路（Scene Tree 或 Inspector）完成。
