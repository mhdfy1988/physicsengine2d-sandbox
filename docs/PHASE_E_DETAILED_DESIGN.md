# 阶段 E 详细设计（全工程 C++ 收敛与目录落地）

更新时间：2026-03-06  
分支：`cpp-migration-baseline`  
前置条件：阶段 D 已封板（编辑器主流程与工具链闭环就绪）

架构基线：[`ENGINE_ARCHITECTURE.md`](./ENGINE_ARCHITECTURE.md)  
进度跟踪：[`PHASE_E_PROGRESS.md`](./PHASE_E_PROGRESS.md)  
验收清单：[`PHASE_E_ACCEPTANCE.md`](./PHASE_E_ACCEPTANCE.md)

## 1. 文档目标

阶段 E 的目标不是继续扩功能，而是把已经完成的 A-D 成果收敛到更稳定的长期代码形态：

1. 明确“全工程以 C++ 为主实现”的执行边界。
2. 停止新的 C 主实现扩张。
3. 为 editor / tools / asset pipeline / app 主路径后续迁移提供 foundation 底座。
4. 为目录重整和兼容层收缩建立可验证路线。

## 2. 范围定义

### 2.1 In Scope

1. 新增 C++ foundation 原语：
   - 文件系统抽象
   - 任务分发抽象
   - 统一日志 / trace 基础接口
   - 运行路径 / 配置对象
2. tools / pipeline / app 侧后续迁移所需的公共 header-only 或轻量 C++ 模块。
3. 约束新代码路径：
   - 默认不再新增新的 C 主实现模块
   - C 保留路径必须声明“保留原因 + 退出条件”
4. 目录重整策略和阶段性映射说明。
5. 对应 smoke / regression / 构建门禁接入。

### 2.2 Out of Scope

1. 一次性把 `apps/`、`tools/`、`src/content/` 全部重命名并整体搬迁。
2. Prefab 冲突面板与插件系统完整落地。
3. 发布矩阵、崩溃诊断、工作会话恢复。

## 3. 成功标准（Definition of Success）

阶段 E 结束时必须同时满足：

1. 新增主路径默认使用 C++ 实现，不再扩张新的 C 主实现模块。
2. 至少一套可复用的 C++ foundation 能力稳定落地，并被 smoke 覆盖。
3. editor / tools / pipeline / app 的后续收敛边界有明确文档和阶段性路线。
4. 现有 A-D 门禁持续为绿。

## 4. 设计原则

1. 先补“可复用底座”，再逐步迁移高层主路径。
2. 迁移优先减少语言边界，不优先追求目录美观。
3. C 兼容层只保留在 ABI 或平台互操作必须存在的位置。
4. 同一能力禁止在 C 和 C++ 各自继续平行演化。

## 5. 模块设计

### 5.1 Filesystem

提供 C++ 路径与文件操作薄封装，约束：

1. 统一使用 `std::filesystem::path`。
2. 所有工具路径拼接收敛到同一套接口。
3. 失败返回显式 `bool`/结果对象，不抛出未处理异常到调用边界。

### 5.2 Task Runner

提供小型并行分发原语，约束：

1. 支持 inline fallback。
2. 支持范围分块 `parallel_for(begin, end)`。
3. 不在系统内部隐式共享可变状态。

### 5.3 Logger / Trace

提供统一日志记录接口，约束：

1. 统一 level 语义：`debug/info/warn/error/fatal`。
2. sink 可替换，便于 editor / tool / test 捕获。
3. trace 字段可选，但结构保持稳定。

### 5.4 Runtime Path Config

提供运行路径 / 项目路径对象，约束：

1. 派生 `Assets/Scenes/Prefabs/ProjectSettings/Build/Cache`。
2. 支持最小目录创建。
3. 支持后续 project/workspace 对象在其上扩展。

## 6. 里程碑拆解

### E1：Foundation Baseline

1. Filesystem / Task Runner / Logger / Runtime Path Config 落地。
2. 新增 `cpp_foundation_smoke`。
3. 接入 `mingw32-make test`。

### E2：Tool / Pipeline Entry Migration

1. 工具入口逐步迁移到 C++ 主实现。
2. pipeline 公共路径优先收敛到 C++ foundation。

### E3：App / Editor Path Convergence

1. `app_runtime` / sandbox 边界逐步收敛到 C++ 主实现。
2. C 保留模块清单与退出条件文档化。

### E4：Directory Mapping Freeze

1. 形成 `engine/*`、`tools/*`、`third_party_adapters/*` 的稳定映射计划。
2. 输出阶段 E 封板文档和迁移基线。

## 7. 测试与门禁

继承门禁：

1. `mingw32-make test`
2. `mingw32-make sandbox`
3. `mingw32-make benchmark`
4. `scripts/check_arch_deps.ps1`
5. `scripts/check_api_surface.ps1`

阶段 E 新增门禁：

1. `tests/cpp_foundation_smoke.cpp`

## 8. 风险与应对

1. 语言收敛过程中引入新的重复封装：
   - 对策：新能力统一先落在 C++ foundation，再决定是否暴露给 C 边界。
2. 大规模重命名导致回归面失控：
   - 对策：分里程碑迁移，优先收敛公共模块，不做一次性目录搬迁。
3. C/C++ 混编行为差异导致隐性兼容问题：
   - 对策：先用 smoke 固化基础语义，再推动上层迁移。
