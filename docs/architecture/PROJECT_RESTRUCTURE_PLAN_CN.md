# 项目重构计划（中文版）

## 1. 目标与定位

这个仓库应明确定位为：

- 引擎代码
- 编辑器 / 应用代码
- 示例项目

也就是一个“引擎 + 编辑器 + 示例项目”的单仓库，而不是单纯的引擎仓库。

因此，仓库根目录应该主要表达“仓库结构”，而不是直接承载某个项目的运行内容。

## 2. 当前主要问题

当前结构的主要问题有：

- `include/` 顶层公开头过多，核心 / 内容 / 运行时接口混在一起
- `docs/` 过于扁平，API、架构、阶段文档、报告、todo 混在一起
- `tests/` 过于扁平，kernel / content / runtime / editor / smoke 混在一起
- `scripts/` 没有按用途分类
- `Packages/`、`Prefabs/`、`ProjectSettings/`、`Scenes/` 直接挂在仓库根目录
- 代码中存在“仓库根目录就是项目根目录”的路径假设

## 3. 目标结构

推荐的长期目标结构如下：

```text
Forge2D/
├─ .github/
├─ apps/
│  ├─ runtime_cli/
│  └─ sandbox_dwrite/
├─ assets/
├─ docs/
│  ├─ api/
│  ├─ architecture/
│  ├─ phases/
│  ├─ reports/
│  ├─ todos/
│  └─ archive/
├─ examples/
├─ include/
│  ├─ physics_core/
│  ├─ physics_content/
│  ├─ physics_runtime/
│  └─ physics2d/
├─ installer/
├─ samples/
│  └─ physics_sandbox_project/
│     ├─ Packages/
│     ├─ Prefabs/
│     ├─ ProjectSettings/
│     └─ Scenes/
├─ scripts/
│  ├─ check/
│  ├─ gate/
│  ├─ package/
│  └─ dev/
├─ src/
│  ├─ core/
│  ├─ content/
│  ├─ runtime/
│  ├─ c_api/
│  ├─ internal/
│  └─ physics2d/
├─ tests/
│  ├─ kernel/
│  ├─ content/
│  ├─ runtime/
│  ├─ editor/
│  ├─ smoke/
│  ├─ equivalence/
│  └─ data/
├─ tools/
├─ CMakeLists.txt
├─ Makefile
└─ README.md
```

## 4. 两个根目录概念

后续必须明确两个“根”的概念：

### 仓库根（repo root）

用于放置：

- 源码
- 文档
- 测试
- 工具
- 脚本

### 项目根（project root）

用于放置：

- `Packages/`
- `Prefabs/`
- `ProjectSettings/`
- `Scenes/`

对当前仓库，推荐的示例项目根是：

```text
samples/physics_sandbox_project/
```

也就是说：

- 仓库根不再等于项目根
- `samples/physics_sandbox_project/` 才是默认的 sample project root

## 5. 分阶段执行计划

## Phase 0：基线冻结

目的：

- 建立迁移前基线
- 识别所有硬编码路径与结构依赖

任务：

- 记录主要目标：
  - `physics_sandbox`
  - `physics_runtime_cli`
  - `physics_tests`
- 扫描对以下路径的硬编码依赖：
  - `Packages`
  - `Prefabs`
  - `ProjectSettings`
  - `Scenes`
- 记录当前脚本路径、文档路径、头文件路径依赖
- 跑一轮基线验证

验证：

- `check_arch_deps.ps1`
- 主要测试目标
- 主要 smoke
- 主要打包脚本

## Phase 1：整理 `docs/`

目的：

- 降低文档导航成本

目标：

- `docs/api/`
- `docs/architecture/`
- `docs/phases/`
- `docs/reports/`
- `docs/todos/`
- `docs/archive/`

任务：

- API / schema 文档移到 `docs/api/`
- 架构文档移到 `docs/architecture/`
- todo 文档移到 `docs/todos/`
- phase 的进度 / 设计 / 验收文档移到 `docs/phases/`
- evidence / final report / profile report 移到 `docs/reports/`
- 更新 README、脚本、文档内部链接

## Phase 2：整理 `tests/`

目的：

- 让测试结构直接表达模块边界

目标：

- `tests/kernel/`
- `tests/content/`
- `tests/runtime/`
- `tests/editor/`
- `tests/smoke/`
- `tests/equivalence/`
- `tests/data/`

任务：

- kernel 回归测试移到 `tests/kernel/`
- content 回归测试移到 `tests/content/`
- runtime 相关测试移到 `tests/runtime/`
- editor 相关测试移到 `tests/editor/`
- smoke 测试移到 `tests/smoke/`
- 保持 `equivalence/` 与 `data/`
- 更新 `CMakeLists.txt` 与 `Makefile`

## Phase 3：整理 `scripts/`

目的：

- 让脚本路径表达用途

目标：

- `scripts/check/`
- `scripts/gate/`
- `scripts/package/`
- `scripts/dev/`

任务：

- 检查脚本移到 `check/`
- gate runner 移到 `gate/`
- 打包脚本移到 `package/`
- 开发辅助脚本移到 `dev/`
- 修正脚本内部路径
- 修正 CI 和文档引用

## Phase 4：建立公开头的命名空间结构

目的：

- 建立清晰的公开 API 目录

目标：

- `include/physics_core/`
- `include/physics_content/`
- `include/physics_runtime/`
- `include/physics2d/`

说明：

这一步先建立新结构，不立即删除旧顶层头。

旧顶层头先保留，避免一次性打爆兼容性。

## Phase 4.2：公开头规范化（保守版）

目的：

- 让新的 namespaced 头路径成为“默认正式路径”
- 把旧顶层头降级为兼容层

核心规则：

- `physics_core/...` 用于核心物理公开 API
- `physics_content/...` 用于内容 / prefab / scene / workspace / plugin
- `physics_runtime/...` 用于运行时支撑
- `physics2d/...` 保留兼容 facade

要做的事：

- 扫描并替换 `apps/`、`src/`、`tests/`、`examples/`、`tools/` 中的旧 include
- 把：
  - `asset_database.hpp`
  - `prefab_schema.hpp`
  - `runtime_snapshot_repo.hpp`
  - `body.hpp`
  这类旧顶层引用，改为：
  - `physics_content/asset_database.hpp`
  - `physics_content/prefab_schema.hpp`
  - `physics_runtime/runtime_snapshot_repo.hpp`
  - `physics_core/body.hpp`
- 更新 README、示例、打包文档中的 include 示例
- 顶层旧头保留为 forwarding headers

这一步结束后的状态：

- 新结构已经是“官方默认路径”
- 旧结构还活着，但只作为兼容层

## Phase 5：整理 `src/`

目的：

- 让实现层目录与模块边界对应

目标：

- `src/core/`
- `src/content/`
- `src/runtime/`
- `src/c_api/`
- `src/internal/`
- `src/physics2d/`

任务：

- 将 `src/runtime_support/` 更名为 `src/runtime/`
- 同步更新构建系统与脚本引用

## Phase 6：移动示例项目内容

目的：

- 把项目内容从仓库根移走

迁移目标：

- `Packages/` -> `samples/physics_sandbox_project/Packages/`
- `Prefabs/` -> `samples/physics_sandbox_project/Prefabs/`
- `ProjectSettings/` -> `samples/physics_sandbox_project/ProjectSettings/`
- `Scenes/` -> `samples/physics_sandbox_project/Scenes/`

这一步是结构重构的关键节点。

## Phase 7：把 `project_root` 做成正式概念

目的：

- 不再依赖“仓库根就是项目根”

要做的事：

- 在 startup / bootstrap 配置中引入显式 `project_root`
- 所有内容路径改为从 `project_root` 派生
- 不再默认使用：
  - `./Packages`
  - `./Prefabs`
  - `./ProjectSettings`
  - `./Scenes`
- 改为：
  - `<project_root>/Packages`
  - `<project_root>/Prefabs`
  - `<project_root>/ProjectSettings`
  - `<project_root>/Scenes`

对当前仓库：

- 默认 `project_root = samples/physics_sandbox_project`

## Phase 8：更新仓库文档与开发入口

目的：

- 让新结构对开发者是清晰的

任务：

- 更新 `README.md`
- 更新 `QUICKSTART.md`
- 更新项目结构说明
- 明确说明：
  - repo root 与 project root 的区别
  - sample project 的位置
  - 公开头的 canonical 路径
  - 脚本的新路径

## Phase 9：移除一般兼容路径

目的：

- 清理不再需要的过渡层

任务：

- 删除旧脚本路径兼容
- 删除旧目录结构残留
- 收紧 guardrail

## Phase 9.5：移除旧顶层模块头（激进版）

目的：

- 完成头文件层面的最终收口
- 让 `include/` 真正干净

这是保守版之后的终局步骤，不应跳过。

### 允许保留在顶层的头（可选）

建议长期只保留少量 umbrella 入口：

- `physics.hpp`
- `physics.h`
- 可选：
  - `physics_world.hpp`
  - `physics_world.h`
  - `physics_ext.hpp`
  - `physics_ext.h`

### 建议最终删除的顶层头

- `asset_database.hpp`
- `asset_fs_poll.hpp`
- `asset_fs_watch.hpp`
- `asset_hot_reload.hpp`
- `asset_importer.hpp`
- `asset_invalidation.hpp`
- `asset_pipeline.hpp`
- `asset_watch.hpp`
- `diagnostic_bundle.hpp`
- `editor_plugin.hpp`
- `prefab_schema.hpp`
- `prefab_semantics.hpp`
- `project_workspace.hpp`
- `runtime_snapshot_repo.hpp`
- `scene_schema.hpp`
- `session_recovery.hpp`
- `subsystem_render_audio_animation.hpp`
- `body.hpp`
- `collision.hpp`
- `constraint.hpp`
- `shape.hpp`
- `physics_math.hpp`

### 前置条件

- 内部代码全部已经切到 namespaced 头路径
- 测试全部已经切到 namespaced 路径
- 示例与文档也已经切换完成
- 打包与 smoke 都验证通过

### 结果

这一步做完之后，`include/` 顶层就不会再像现在这样堆很多散头文件。

## 6. 保守版到激进版的详细头文件迁移路径

### Step A：建立 namespaced 头族

状态：

- 已建立骨架

内容：

- `physics_core/`
- `physics_content/`
- `physics_runtime/`

### Step B：切换内部 include

要求：

- `apps/`
- `src/`
- `tests/`
- `examples/`
- `tools/`

全部改为新的 canonical include 路径。

### Step C：切换文档与示例

要求：

- README 示例改为新路径
- 打包文档改为新路径
- 开发模板改为新路径

### Step D：兼容观察期

要求：

- 保留旧顶层头
- 但仓库内部不再使用它们

### Step E：删除旧顶层模块头

要求：

- 真正删除不再需要的顶层模块头
- 只保留少量明确需要的 umbrella 头

### Step F：增加 guardrail

要求：

- 增加 grep / check，禁止内部代码重新 include 被淘汰的顶层头

## 7. 验证矩阵

每个阶段完成后，至少应验证：

- `scripts/check/check_arch_deps.ps1`
- 主回归测试
- `physics_kernel_tests`
- `physics_content_tests`
- 关键 smoke
- `physics_sandbox` 编译
- `physics_runtime_cli` 编译 / 运行
- 打包脚本

头文件迁移阶段额外验证：

- `physics_core_header_smoke`
- namespaced include 示例
- standalone package 产出

## 8. 推荐提交顺序

1. `Reorganize documentation layout`
2. `Reorganize test layout by layer`
3. `Reorganize scripts by purpose`
4. `Introduce namespaced public header layout with compatibility forwards`
5. `Canonicalize internal public header usage`
6. `Align source layout with public module boundaries`
7. `Move sample project content under samples/physics_sandbox_project`
8. `Make project_root explicit in startup and bootstrap flow`
9. `Update docs for repository and sample-project layout`
10. `Remove deprecated compatibility paths`
11. `Remove legacy top-level public headers`
12. `Enforce canonical public header usage`

## 9. 当前建议的下一步

如果继续执行计划，当前最值得做的是：

### 下一步 1

执行 `Phase 4.2`

也就是：

- 把仓库内部所有 include 批量切到：
  - `physics_core/...`
  - `physics_content/...`
  - `physics_runtime/...`

### 下一步 2

确认一轮稳定后，执行 `Phase 9.5`

也就是：

- 删除大部分顶层旧模块头
- 让 `include/` 顶层真正收干净

一句话总结：

现在的结构已经从“完全混乱”走到了“兼容过渡态”；  
后续必须继续推进到“激进收口态”，否则 `include/` 会长期停在新旧并存的半完成状态。
