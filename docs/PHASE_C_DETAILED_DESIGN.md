# 阶段 C 详细设计（数据与内容管线）

更新时间：2026-03-05  
分支：`cpp-migration-baseline`  
前置条件：阶段 A/B 已封板（内核 C++ 化 + Runtime 架构固化）

执行跟踪：见 [PHASE_C_PROGRESS.md](./PHASE_C_PROGRESS.md)  
验收清单：见 [PHASE_C_ACCEPTANCE.md](./PHASE_C_ACCEPTANCE.md)

## 1. 文档目标

本设计文档用于落实 [ENGINE_ARCHITECTURE.md](./ENGINE_ARCHITECTURE.md) 中“阶段 C：数据与内容管线”要求，并提供可执行的落地计划。  
阶段 C 的主目标：

1. 定义并落地 Scene/Prefab 版本化序列化格式。
2. 建立资源导入与构建管线（至少纹理、字体、音频 3 类资产）。
3. 建立可验证的运行时热重载基础能力。

## 2. 范围定义

### 2.1 In Scope（本阶段必须完成）

1. 项目目录约定落地：`Assets/Scenes/Prefabs/ProjectSettings/Build/Cache`。
2. Scene/Prefab 稳定 schema（含版本号、稳定 ID、最小化写盘策略）。
3. 迁移工具基础能力：旧 schema -> 新 schema 的可重复迁移。
4. Asset Database：GUID、`.meta`、依赖图、增量导入。
5. Importer 管线：纹理/字体/音频导入，产出可运行时消费的中间资产。
6. 热重载链路：文件变更 -> 导入失效 -> 运行时更新（demo 场景可验证）。
7. B->C 过渡项：live simulation loop 后端所有权转向 facade 驱动路径，并保持现有 smoke 作为守门。

### 2.2 Out of Scope（本阶段不做）

1. 全量编辑器体验打磨（复杂 Inspector、资源浏览器完整交互）。
2. 动画系统、脚本生态、跨平台构建矩阵扩张。
3. 大规模渲染管线重构。

## 3. 成功标准（Definition of Success）

阶段 C 结束时必须同时满足：

1. Scene/Prefab 版本化生效，迁移工具可处理至少 1 个历史版本并提供 dry-run。
2. 资源导入支持纹理/字体/音频三类资产，增量导入与依赖失效正确。
3. 热重载链路在 demo 场景可重复通过（修改资源后运行时可观察到更新）。
4. 阶段 B 既有回归/smoke/架构门禁持续为绿。
5. 输出阶段 C 验收清单与证据链接。

## 4. 架构与数据流

### 4.1 逻辑分层

1. Authoring 层（源数据）：
   - `Assets/` 原始资产
   - `Scenes/` 场景数据
   - `Prefabs/` 预制体数据
   - `ProjectSettings/` 项目配置
2. Pipeline 层（导入与构建）：
   - 扫描/哈希
   - 依赖图更新
   - 导入器执行
   - 缓存与打包
3. Runtime 层（加载与热重载）：
   - 运行时资源索引加载
   - 变更通知
   - 实例刷新与错误回退

### 4.2 端到端流程

1. 扫描源目录并读取/创建 `.meta`。
2. 计算 `source_hash + importer_version + import_settings_hash`。
3. 判断缓存命中；未命中则执行导入并更新依赖图。
4. 生成构建产物与资产索引（供 runtime 查询）。
5. 文件变更时按依赖图触发重导入并广播热重载事件。

## 5. Scene/Prefab 版本化设计

### 5.1 Scene Schema（建议）

字段约束：

1. `schema_version`：整型，必填。
2. `scene_guid`：稳定 GUID，必填。
3. `entities[]`：实体数组，实体拥有稳定 `entity_id`。
4. `components`：组件存储采用稳定键顺序序列化，避免无意义 diff。

设计原则：

1. 保存顺序稳定。
2. 最小化写盘（仅写变更对象）。
3. 非法字段进入结构化错误报告，不静默丢弃。

### 5.2 Prefab Schema（建议）

1. 支持基础 Prefab、变体 Prefab、实例 Override。
2. Override 仅记录差异字段，不复制整对象。
3. 嵌套 Prefab 冲突遵循“最近层覆盖”。
4. 删除父字段后，子层悬挂 Override 进入修复流程并给出错误码。

### 5.3 迁移策略

1. 仅允许前向迁移（`vN -> vN+1`）。
2. 迁移步骤可组合（`v1->v2->v3`），每步可独立测试。
3. 提供 `dry-run` 输出：变更摘要、风险项、失败文件列表。
4. 迁移失败不得破坏原始文件（原子写入或临时文件替换）。

## 6. Asset Database 与 Importer 设计

### 6.1 AssetMeta

每个资产对应 `source.ext.meta`，至少包含：

1. `guid`
2. `asset_type`
3. `importer_id`
4. `importer_version`
5. `import_settings`
6. `source_hash`
7. `dependencies[]`

### 6.2 导入器接口约束

1. 输入固定则输出可复现（纯函数化约束）。
2. 失败返回结构化错误（`code/name/message/path`）。
3. 不允许写出业务目录外路径。

### 6.3 首批资产类型

1. Texture：`png/jpg/bmp`（运行时统一到项目定义的目标格式）。
2. Font：`ttf/otf`（生成运行时字形缓存或索引）。
3. Audio：`wav/ogg`（按平台策略转换或透传）。

## 7. 缓存、失效与增量导入

1. 缓存键：`source_hash + importer_version + import_settings_hash`。
2. 依赖变化触发上游重导入（依赖图传播）。
3. 清理 `Cache/` 不影响源资产和项目数据。
4. 导入结果必须可重建（删除缓存后可由源资产恢复）。

## 8. 运行时热重载设计

### 8.1 基础链路

1. 文件系统监听（按目录与类型过滤）。
2. 事件去抖（debounce）与批处理。
3. 触发导入失效和重导入。
4. runtime 收到变更事件后按资源类型执行局部刷新。

### 8.2 一致性与回退

1. 热重载失败时保持旧资源继续可用。
2. 错误信息写入统一 runtime 错误通道并可在编辑器查看。
3. 多资源联动变更时使用事务批次，避免半更新状态。

## 9. B->C 过渡与兼容策略

结合 [RUNTIME_PHASE_B_FINAL_REPORT.md](./RUNTIME_PHASE_B_FINAL_REPORT.md) 的建议，阶段 C 首批工作必须包含：

1. 将 live simulation loop 后端所有权迁移到 facade 驱动路径。
2. 保留并持续执行现有 smoke 集合作为迁移护栏。
3. 迁移期间保持 snapshot/event/error 协议字段兼容。
4. 将编辑器事件总线与 facade 多错误 payload 完整统一。

## 10. 测试与 CI 门禁

### 10.1 保留门禁（来自阶段 B）

1. `mingw32-make test`
2. `mingw32-make sandbox`
3. `mingw32-make benchmark`
4. `scripts/check_arch_deps.ps1`
5. `scripts/check_api_surface.ps1`

### 10.2 阶段 C 新增测试

1. Schema round-trip：Scene/Prefab 读写稳定性测试。
2. Migration golden tests：历史版本迁移结果对照。
3. Importer determinism tests：重复导入产物一致性。
4. Hot-reload smoke：demo 场景热重载成功与失败回退。
5. Asset dependency invalidation tests：依赖传播重导入正确性。

## 11. 里程碑拆解（C1-C4）

### C1：Schema 与迁移基线

1. 定稿 Scene/Prefab schema v1。
2. 完成迁移框架与 `dry-run`。
3. 交付 schema 说明和示例数据。

### C2：Asset Database 与首批导入器

1. `.meta` 与 GUID 体系落地。
2. 纹理/字体/音频导入器接入。
3. 依赖图与增量导入机制可用。

### C3：热重载闭环

1. 文件监听 -> 导入 -> runtime 刷新闭环跑通。
2. 失败回退与错误可观测性达标。
3. demo 场景热重载 smoke 通过。

### C4：封板与验收

1. 运行全部回归和新增测试。
2. 输出 C 阶段验收清单（逐条 DoD 对照 + 证据链接）。
3. 输出 C 阶段封板报告与残留风险清单。

## 12. 风险与应对

1. Schema 演进导致历史项目不可读：
   - 对策：强制迁移工具入口，禁手工批量改 schema。
2. 导入缓存污染导致随机失败：
   - 对策：缓存键严格化 + 一键清理重建验证。
3. 热重载引发运行时不一致：
   - 对策：事务批处理 + 失败回退 + 可观测日志。
4. B->C 迁移期间双路径漂移：
   - 对策：保持 smoke 常绿并优先切换 live path 所有权。

## 13. 交付清单

1. 阶段 C 详细设计文档（本文件）。
2. 阶段 C 验收清单（见 [PHASE_C_ACCEPTANCE.md](./PHASE_C_ACCEPTANCE.md)）。
3. 阶段 C 进度文档（见 [PHASE_C_PROGRESS.md](./PHASE_C_PROGRESS.md)）。
4. Scene/Prefab schema 文档与示例。
5. 迁移工具与使用说明。
6. 资源导入器文档（纹理/字体/音频）。
7. 热重载验证报告（demo 证据）。
8. 阶段 C 封板报告。

## 14. 执行纪律

1. 涉及 schema 或导入行为变更，必须同步更新文档与测试。
2. 不允许跳过迁移工具直接改线上项目数据格式。
3. 未通过门禁不得合并。
4. 阶段 C 期间避免并行引入与数据管线无关的大功能。
