# Runtime 架构固化详细设计（阶段 B）

更新时间：2026-03-05  
主分支：`cpp-migration-baseline`  
前置条件：阶段 A（物理内核 C++ 化）完成

## 1. 文档目标
本设计文档用于落实 `ENGINE_ARCHITECTURE.md` 的阶段 B，目标是将“已完成 C++ 化的物理内核”与 ECS/Runtime/工具链稳定接轨，形成可持续迭代的工程基线。

## 2. 范围定义
## 2.1 In Scope（阶段 B 必做）
1. Runtime API 边界固化（对外语义稳定、对内模块职责清晰）。
2. ECS 与 Physics2D 的对齐层设计与落地（创建/同步/销毁/查询）。
3. Pipeline 执行阶段标准化（Spawn/Step/Sync/Cleanup）。
4. CI 门禁落地（回归、等价、benchmark、架构依赖检查）。
5. 最小工具闭环（Editor 最小工作流接 Runtime，而非旁路逻辑）。

## 2.2 Out of Scope（阶段 B 不做）
1. 大规模新功能扩展（如完整动画、完整音频编辑器、复杂脚本生态）。
2. 大范围数据格式重构（Scene/Prefab schema 大改属于后续阶段）。
3. 跨平台全覆盖构建链完善（本阶段优先现有平台稳定性）。

## 3. 成功标准（Definition of Done）
阶段 B 结束时必须同时满足：
1. Runtime 核心链路在 CI 中稳定通过：`test + equivalence + benchmark + arch-check`。
2. ECS <-> Physics2D 的数据流具备可验证一致性，核心回归无行为漂移。
3. 编辑器路径通过统一 Runtime API 驱动（无“编辑器私有物理旁路”）。
4. 模块依赖方向符合架构规则，无新增跨层违规。

## 4. 目标架构（阶段 B 结束态）
当前已形成目录分层：
1. `src/c_api/`
2. `src/core/`
3. `src/physics2d/`

阶段 B 增量目标：
1. ECS 侧通过稳定桥接层调用 Physics2D，不直接耦合底层实现细节。
2. Runtime Pipeline 由阶段配置驱动，支持工具态/运行态一致执行框架。
3. 工具层只通过 Runtime API 访问引擎能力。

## 5. 模块边界与依赖规则
1. `tools/* -> runtime facade -> src/core + src/physics2d` 允许。
2. `src/physics2d` 不依赖 `tools/*`。
3. ECS 模块不直接依赖编辑器 UI 或资源面板逻辑。
4. 第三方能力统一经适配层引入，不允许直接散落在业务系统中。

## 6. ECS 与 Physics2D 对齐设计
## 6.1 组件最小集（建议）
1. `Transform2D`：位置、旋转、缩放（或最小字段子集）。
2. `RigidBody2D`：质量、速度、阻尼、类型、睡眠状态。
3. `Collider2D`：几何参数、材质参数、碰撞过滤。
4. `Constraint2D`（可选第一批支持距离约束）。

## 6.2 生命周期同步点
1. Spawn：从 ECS 实体构建/绑定 Physics2D 对象句柄。
2. Step：由 Physics2D 推进仿真。
3. Sync：将物理结果回写到 ECS（Transform/速度状态等）。
4. Cleanup：延迟销毁与句柄失效处理，避免遍历期结构突变。

## 6.3 一致性约束
1. 每个带 `RigidBody2D` 的实体必须有稳定物理句柄映射。
2. 实体删除后句柄必须在同帧或受控阶段失效。
3. 同步回写必须具备阶段顺序保证，避免读写竞争。

## 7. Runtime Pipeline 固化
## 7.1 阶段定义
1. `PrePhysics`：收集输入、应用外部参数变更。
2. `PhysicsStep`：调用 `physics2d` 步进。
3. `PostPhysicsSync`：回写 ECS 状态，触发事件。
4. `LateUpdate/Cleanup`：资源回收、延迟销毁、统计采样。

## 7.2 调度原则
1. 写同一组件集的系统不得并行。
2. 读系统可并行，但必须标注只读语义。
3. 系统顺序由 Pipeline 配置显式声明，禁止隐式顺序依赖。

## 8. 工具层接入原则
1. 编辑器运行模式（PIE）必须走同一 Runtime 主循环接口。
2. 编辑器调参通过 Runtime Config API 生效，不直接写内核内部状态。
3. 调试可视化（碰撞体、约束、事件）基于 Runtime 观测接口输出。

## 9. CI 与质量门禁
每次 PR 必跑：
1. `mingw32-make test`
2. `mingw32-make benchmark`
3. 架构依赖检查脚本（禁止跨层 include/调用）
4. API surface 差异检查（防止误改公共接口）

建议追加：
1. PR 评论中自动附 benchmark 与基线差异百分比。
2. 性能超预算（默认 5%）自动标红并阻断合并。

## 10. 里程碑拆分（B1/B2/B3）
## B1：边界固化（1 周）
1. 明确 Runtime Facade 接口与调用路径。
2. 补齐依赖规则检查脚本与 CI 初版。
3. 验收：无新增跨层依赖违规，主回归稳定。

## B2：ECS-Physics 桥接落地（1-2 周）
1. 完成组件-物理对象映射与同步流程。
2. 接入 Spawn/Step/Sync/Cleanup 完整链路。
3. 验收：桥接相关回归与等价测试通过。

## B3：工具最小闭环（1 周）
1. 编辑器最小“编辑->运行->暂停->回放”闭环。
2. 输出运行时调试信息与基础性能统计。
3. 验收：闭环流程可稳定复现，问题可回放。

## 11. 测试策略
1. 回归测试：保持现有 `31/31` 为下限。
2. 新增桥接测试：实体创建/销毁、句柄失效、同步一致性。
3. 新增工具链 smoke：PIE 启停、参数回写、回放一致性。
4. benchmark：延续阶段 A 基线对比方式，持续追踪波动。

## 12. 风险与回滚
主要风险：
1. ECS 与物理同步顺序错误导致帧间抖动或状态撕裂。
2. 工具层绕过 Runtime API，形成双轨逻辑。
3. CI 门禁缺失导致架构退化。

回滚策略：
1. 按桥接层提交粒度回滚，不回滚整阶段历史。
2. 保留阶段性开关策略（参考 `CPP_KERNEL_SWITCH_POLICY.md`）。
3. 任何性能回退超过阈值时阻断合并并先修复。

## 13. 交付清单
1. 阶段 B 代码变更（桥接层 + pipeline + 工具接入）。
2. 阶段 B 测试与 benchmark 报告。
3. CI 配置与门禁说明文档。
4. 阶段 B 结束总结（风险项、遗留项、进入 C 阶段建议）。
