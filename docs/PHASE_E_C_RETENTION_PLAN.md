# 阶段 E C 保留模块清单与退出条件

更新时间：2026-03-06  
阶段：E3 App / Editor Path Convergence

## 1. 目的

阶段 E 明确全工程最终以 C++ 为主实现，但当前仓内仍保留一批 C 实现模块。  
这些模块不是未来长期鼓励扩张的主实现，而是迁移窗口中的兼容边界、平台壳层或已稳定的数据管线路径。

本清单用于冻结：

1. 当前允许保留的 C 实现路径。
2. 每类模块继续保留的原因。
3. 对应退出条件与计划移除阶段。

配套硬门禁见：`scripts/check_cpp_convergence.ps1` 与 [`phase_e_c_retention_allowlist.txt`](./phase_e_c_retention_allowlist.txt)。

## 2. 保留原则

1. 新能力默认落在 C++，不得再新增新的 `src/`、`apps/`、`tools/` 主实现 `.c` 文件。
2. 已保留的 C 模块可以维护，但若发生结构性扩展，应优先抽到 C++ foundation 或迁移为 `.cpp`。
3. 对外 C ABI、平台互操作入口、短期兼容层可以继续保留在 C。
4. 任何新增保留项都必须同时更新 allowlist 与本文件。

## 3. 保留模块分组

### 3.1 平台与编辑器壳层

范围：

1. `apps/sandbox_dwrite/main.c`
2. `apps/sandbox_dwrite/presentation/render/*.c`
3. `apps/sandbox_dwrite/presentation/input/*.c`
4. `apps/sandbox_dwrite/infrastructure/project_tree.c`
5. `apps/sandbox_dwrite/infrastructure/snapshot_repo.c`
6. `apps/sandbox_dwrite/infrastructure/ui_layout_repo.c`

保留原因：

1. 这些路径仍然直接承接 Win32 / DWrite / UI 外壳与现有编辑器流转。
2. 当前阶段优先保证主流程稳定，不在 Phase E 内对 UI 容器做大规模语言迁移。

退出条件：

1. 编辑器目录映射冻结后，`tools/editor/*` 新壳层完成。
2. Win32 / DWrite 访问收缩到适配层或少量边界模块。

计划移除阶段：

1. Phase F 启动目录迁移。
2. Phase G 前完成主入口与 UI 外壳的 C++ 主实现收敛。

### 3.2 编辑器应用服务层

范围：

1. `apps/sandbox_dwrite/application/app_controller.c`
2. `apps/sandbox_dwrite/application/app_runtime.c`
3. `apps/sandbox_dwrite/application/editor_command_bus.c`
4. `apps/sandbox_dwrite/application/history_service.c`
5. `apps/sandbox_dwrite/application/pie_lifecycle.c`
6. `apps/sandbox_dwrite/application/runtime_param_service.c`
7. `apps/sandbox_dwrite/application/scene_builder.c`
8. `apps/sandbox_dwrite/application/scene_catalog.c`
9. `apps/sandbox_dwrite/domain/app_command.c`
10. `apps/sandbox_dwrite/infrastructure/app_event_bus.c`

保留原因：

1. 这部分已经稳定接住 A-D 的 editor workflow、PIE、undo/redo、runtime bridge。
2. 继续重写风险高于收益，Phase E 先冻结边界与迁移顺序。

退出条件：

1. `app_runtime`、事件总线、command/history 边界抽象出稳定的 C++ facade。
2. editor 侧状态模型与 runtime bridge 合并到统一 C++ foundation / core 边界。

计划移除阶段：

1. Phase F 完成 app/editor 主状态对象设计。
2. Phase G 之前完成核心应用服务层迁移或收缩成纯 C ABI 壳。

### 3.3 内容与资源管线稳定层

范围：

1. `src/content/scene_schema.c`
2. `src/content/prefab_schema.c`
3. `src/content/asset_database.c`
4. `src/content/asset_invalidation.c`
5. `src/content/asset_pipeline.c`
6. `src/content/asset_watch.c`
7. `src/content/asset_hot_reload.c`
8. `src/content/asset_fs_poll.c`
9. `src/content/asset_fs_watch.c`
10. `src/content/subsystem_render_audio_animation.c`

保留原因：

1. schema、asset database、hot-reload 路径在阶段 C/D 已有稳定回归覆盖。
2. 这批模块大多承载数据协议与既有 C API，不适合在 Phase E 内一起大迁移。
3. `asset_importer` 已迁移到 C++，可以作为后续继续替换内容管线的模板。

退出条件：

1. `tools/asset_pipeline/*` 目录落位后，资源导入、失效传播、watcher 适配层完成拆分。
2. Prefab 深层编辑语义和工程对象模型稳定，schema 与 database 路径可以成组迁移。

计划移除阶段：

1. Phase F 处理 `scene/prefab/project` 语义补齐时同步推进。
2. Phase G 前把 watcher / import / pipeline 收敛到 C++ 主实现与 adapter 边界。

### 3.4 开发辅助工具

范围：

1. `tools/parallel_benchmark_compare.c`
2. `tools/phase_d_profile_capture.c`
3. `tools/subsystem_workflow_demo.c`

保留原因：

1. 这些是阶段 D 的专项工具，当前功能稳定、接口简单。
2. 相比先改为 C++，更重要的是先冻结它们在 `tools/dev/*` 的目标落点。

退出条件：

1. `tools/*` 目录映射生效。
2. 公共日志、filesystem、runtime config 接口在工具侧全面复用。

计划移除阶段：

1. Phase F 目录整理时迁到目标工具树。
2. 若有持续演进需求，新增实现直接以 C++ 落地，不在现有 C 文件上扩张。

## 4. 已完成的 C++ 收敛样板

1. `src/content/asset_importer.cpp`
   - 已切到 C++ 编译单元并复用 `physics_filesystem.hpp`
2. `tools/scene_migrate_main.cpp`
   - 已切到 C++ 主入口
3. `cpp/physics_foundation.hpp`
   - 提供 filesystem / logger / task runner / runtime path config 基线

## 5. 维护要求

1. `scripts/check_cpp_convergence.ps1` 必须保持为绿。
2. 若故意新增保留 C 路径，必须在同一提交中：
   - 更新 allowlist
   - 更新本文件
   - 说明保留原因与退出条件
