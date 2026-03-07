# Runtime 阶段 B 详细设计

更新时间：2026-03-05  
分支：`cpp-migration-baseline`  
前置条件：阶段 A（物理内核 C++ 化）已完成

## 1. 目标

阶段 B 的目标是把 C++ 物理内核接入可持续迭代的 Runtime/ECS/工具链闭环，重点是：

1. 统一运行时边界与调用路径。
2. 保证 ECS 与 Physics2D 同步一致性。
3. 建立可观测、可回归、可门禁的工程基线。

## 2. 范围

### 2.1 In Scope

1. Runtime Facade 语义固化（Tick/Event/Error）。
2. ECS-Physics 桥接（Spawn/PreSync/Step/Sync/Cleanup）。
3. 编辑器最小闭环接入（运行/暂停/单步/状态可视化）。
4. 回归、smoke、benchmark 与架构/API门禁。

### 2.2 Out of Scope

1. 大规模新功能（动画、脚本生态等）。
2. 全量资源格式重构。
3. 全平台构建矩阵扩张（阶段 B 以当前平台稳定为先）。

## 3. 架构边界

1. `tools/editor -> app_runtime -> runtime facade -> core/physics2d`
2. `physics2d` 不依赖 `tools/*`
3. ECS 不直接耦合编辑器 UI 逻辑
4. 第三方能力通过适配层引入

## 4. Runtime 模型

### 4.1 Tick Snapshot

输出：

1. 帧序号
2. pipeline 统计
3. 桥接一致性报告
4. 事件总线健康信息（编辑器侧）

### 4.2 Runtime Event

输出：

1. `ContactCreated`
2. `BodySleep`
3. `BodyWake`

### 4.3 Runtime Error

输出：

1. 桥接错误列表（`last_errors()`）
2. 错误 code/name/severity 统一语义（见 `RUNTIME_ERROR_SEMANTICS.md`）

## 5. ECS-Physics 桥接策略

1. Spawn：仅对可生成实体创建刚体。
2. PreSync：脏标记增量同步，Collider 变更受控 respawn。
3. Step：单点推进物理。
4. Sync：回写 Transform 并校验桥接一致性。
5. Cleanup：延迟销毁与句柄失效处理。

## 6. 编辑器接入原则

1. 编辑器行为尽量走 `AppRuntime` 事件化通道。
2. 面板/状态栏显示优先消费 runtime snapshot。
3. 告警可定位（点击红点进入警告日志）。

## 7. 质量门禁

每次关键提交至少满足：

1. `mingw32-make test`
2. `mingw32-make sandbox`
3. `mingw32-make benchmark`
4. `check_arch_deps.ps1`
5. `check_api_surface.ps1`

CI 中额外运行 selected C++ smoke。

## 8. 阶段 B DoD

1. Runtime Facade 稳定输出 Tick/Event/Error。
2. ECS-Physics 桥接有一致性校验与 smoke 覆盖。
3. 编辑器形成最小运行时闭环并具备可观测性。
4. 架构/API门禁稳定执行。
5. 产出封板验收清单与残留风险说明。

## 9. 残留工作（进入封板前）

1. `last_errors()` 多错误列表透传到编辑器事件总线。
2. 补齐错误恢复/多错误并发 smoke。
3. 输出 B 阶段封板报告。
