# 物理内核 C++ 化详细设计文档（阶段 A）

## 1. 文档目的

本设计文档用于落实 [ENGINE_ARCHITECTURE.md](./ENGINE_ARCHITECTURE.md) 的阶段 A：

1. 先完成物理内核从 C 到 C++ 的平滑迁移。
2. 保证功能等价、行为等价、性能不退化。
3. 在迁移完成前，冻结非关键扩张，避免目标漂移。

---

## 2. 重构边界

## 2.1 In Scope（本阶段必须完成）

1. 物理内核核心模块迁移为 C++ 实现。
2. 继续对外暴露稳定 C API（兼容现有调用方）。
3. 保持现有测试与 smoke 覆盖不下降。
4. 建立 C/C++ 等价验证与性能对比机制。

## 2.2 Out of Scope（本阶段不做）

1. 编辑器功能扩张（Inspector、资源浏览器等）。
2. Render2D/Audio/Animation 全量实现。
3. 脚本系统完整接入。
4. 跨平台工具链完善。

---

## 3. 成功标准（Definition of Success）

阶段 A 结束时必须同时满足：

1. `mingw32-make test` 保持 100% 通过。
2. 关键 C++ smoke 测试全部通过（wrapper/ECS/pipeline）。
3. C API 语义不变（错误码、边界行为、快照回放行为一致）。
4. 性能相对 C 基线退化不超过 5%（默认目标：不退化）。
5. C 旧实现可在短期窗口回滚，之后移除。

---

## 4. 当前代码基线（进入阶段 A 前）

1. C 内核稳定，回归测试齐全。
2. 已有 C++ 包装层（RAII、World、Snapshot、ECS 基础能力）。
3. ECS 已具备 Registry/Query/System/Pipeline（含阶段开关）。

结论：当前适合做“后端实现替换”，不适合继续做上层功能扩张。

---

## 5. 目标架构（阶段 A 结束态）

## 5.1 逻辑结构

1. API 兼容层：`include/*.h`（维持 C ABI）
2. C API 实现层：`src/c_api/*.cpp`（`extern "C"` 导出）
3. C++ 内核层：`src/core/*.cpp`, `src/physics2d/*.cpp`
4. 内部共享层：`src/internal/*.hpp`
5. 测试层：现有 C 回归 + C++ smoke + 等价测试

## 5.2 基本原则

1. 对外 ABI 稳定，对内实现可演进。
2. 先迁移实现，再优化设计；禁止迁移期大改 API。
3. 单次改动可回滚，避免大爆炸合并。

---

## 6. 目录与构建设计

建议目录演进（增量）：

1. `src/c_api/`：C 导出函数实现（`.cpp`）
2. `src/core/`：引擎生命周期、配置、调度
3. `src/physics2d/`：碰撞、求解、积分、约束
4. `src/internal/`：内部数据结构与工具
5. `tests/equivalence/`：C/C++ 行为一致性测试
6. `benchmarks/`：性能基线对比

构建要求：

1. 编译器切换到 C++17（已具备）并保留 C 头兼容。
2. C API 统一由 `extern "C"` 导出。
3. 保留开关：`PHYSICS_USE_CPP_KERNEL`（默认 ON，过渡期可 OFF）。

---

## 7. 模块迁移分解（WBS）

## 7.1 WBS-1 生命周期与配置

范围：

1. `create/free`
2. config sanitize/get/set
3. error channel

交付：

1. C API 调用路径切到 C++ 内核对象。
2. 单测与回归保持通过。

风险：

1. 初始化/销毁顺序变化导致资源泄漏。

门禁：

1. Address/Leak 检查（可选）无新增泄漏。

## 7.2 WBS-2 查询与变更 API

范围：

1. body/constraint/contact 查询
2. add/remove/detach body
3. constraint 增删改查

交付：

1. 句柄与索引语义不变。
2. 边界行为（空指针/越界）与旧实现一致。

## 7.3 WBS-3 仿真主流程

范围：

1. step orchestration
2. integrate/broadphase/narrowphase/solve/clear forces
3. profile 数据

交付：

1. 阶段顺序一致。
2. profile 字段语义一致。

## 7.4 WBS-4 Snapshot/Replay 与扩展点

范围：

1. capture/apply/replay
2. pipeline plugin / callbacks / job system

交付：

1. 快照兼容现有数据结构。
2. 扩展点行为保持兼容。

---

## 8. 接口与兼容策略

## 8.1 C API 兼容原则

1. 函数签名不变。
2. 结构体布局不变（对外可见者）。
3. 错误码与字符串映射不变。

## 8.2 C++ 内核接口（内部）

建议核心类（内部，不直接暴露给外部）：

1. `PhysicsKernel`
2. `WorldState`
3. `CollisionPipeline`
4. `Solver`
5. `SnapshotService`

说明：

1. C API 仅做参数校验与转发。
2. 业务逻辑集中在 C++ 内核类中。

## 8.3 弃用策略

1. 阶段 A 不新增破坏性 API。
2. 如必须弃用，需先标记 deprecated，至少跨 1 个稳定版本再移除。

---

## 9. 数据结构迁移策略

1. 优先保留现有 POD 数据结构，降低 ABI 风险。
2. 内部管理对象可用 RAII 容器接管生命周期。
3. 热路径避免 `std::function`、异常、频繁堆分配。
4. 对性能敏感结构保留连续内存布局。

---

## 10. 等价性测试设计

## 10.1 测试分层

1. 回归测试：现有 `tests/regression_*`
2. C++ smoke：现有 `tests/cpp_*_smoke.cpp`
3. 等价测试：新增 `tests/equivalence/*`

## 10.2 等价测试维度

1. 创建/销毁/重置
2. 配置读取写入
3. 碰撞与约束关键行为
4. 快照捕获与回放结果
5. 错误路径与错误码

## 10.3 判定标准

1. 离散状态一致：严格相等（计数、错误码、active 标志等）。
2. 浮点状态一致：在 epsilon 内。
3. 事件顺序一致：按定义顺序严格比对。

---

## 11. 性能基线与预算

## 11.1 基线采集

1. 以当前主分支 C 内核作为 baseline。
2. 固定场景、固定步数、固定硬件下采样。

## 11.2 指标

1. 平均 step 时间
2. P95 step 时间
3. 峰值内存
4. 启动耗时（create + warmup）

## 11.3 预算

1. 默认不退化。
2. 允许短期波动上限 5%，超限必须阻断合并并出优化计划。

---

## 12. 迁移节奏与分支策略

1. 主工作分支：`cpp-migration-baseline`
2. 小步提交：每个提交只覆盖一个工作包或一个可验证子任务。
3. 每次提交必须附：
4. 变更模块清单
5. 测试结果摘要
6. 风险与回滚说明

---

## 13. CI 门禁设计

每次 PR 必跑：

1. `mingw32-make test`
2. 关键 C++ smoke 集
3. 等价测试集（阶段 A 完整后设为必选）

建议门禁：

1. 依赖规则扫描（禁止跨层 include）
2. API Surface 差异检查（防止误改公开 API）
3. 基准性能回归检查（夜间任务）

---

## 14. 风险清单与回滚策略

## 14.1 主要风险

1. ABI 误变更导致上层调用崩溃。
2. 行为微差导致回放/测试不稳定。
3. 性能退化超预算。
4. 迁移任务外溢（被编辑器需求打断）。

## 14.2 对策

1. C API 冻结 + surface 对比。
2. 引入等价测试与固定 seed。
3. 基准测试自动化。
4. 设“范围守门”规则：阶段 A 禁止并行做大功能。

## 14.3 回滚

1. 保留 `PHYSICS_USE_CPP_KERNEL=OFF` 回退通道。
2. 重大问题按模块级回滚，不回滚整分支历史。

---

## 15. 交付清单（阶段 A 完成时）

1. C++ 内核主实现代码与模块文档。
2. C API 兼容验证报告。
3. 等价测试报告。
4. 性能对比报告（C baseline vs C++ kernel）。
5. 迁移总结（已迁移/待优化项）。

---

## 16. 执行纪律（防跑偏）

1. 任何“工具层新功能”需求默认延后到阶段 B/C/D。
2. 阶段 A 期间新增代码优先服务“内核迁移主线”。
3. 不允许未测提交合并。
4. 设计变更必须先更新本文件再改代码。
