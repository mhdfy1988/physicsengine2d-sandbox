# 阶段 E 目录映射冻结说明

更新时间：2026-03-06  
阶段：E4 Directory Mapping Freeze

## 1. 目标

阶段 E 不做一次性大规模重命名，但必须先冻结“当前目录 -> 目标目录”的稳定映射，避免后续 F/G 阶段继续沿着旧结构扩张。

目标分层沿用 [`ENGINE_ARCHITECTURE.md`](./ENGINE_ARCHITECTURE.md) 第 6 节：

1. `engine/foundation/*`
2. `engine/core/*`
3. `engine/ecs/*`
4. `engine/systems/*`
5. `tools/*`
6. `third_party_adapters/*`

## 2. 冻结规则

1. Phase E 之后新增模块命名和职责，必须按目标落点设计。
2. 在真正重命名前，允许保留旧路径，但不允许继续扩张与目标目录冲突的平行结构。
3. `third_party_adapters/*` 作为未来显式平台/第三方隔离层，只接受 adapter 语义模块。

## 3. 当前到目标的主映射

### 3.1 Foundation / Core / ECS

1. `cpp/physics_filesystem.hpp` -> `engine/foundation/filesystem/*`
2. `cpp/physics_logger.hpp` -> `engine/foundation/logging/*`
3. `cpp/physics_task_runner.hpp` -> `engine/foundation/tasks/*`
4. `cpp/physics_runtime_config.hpp` -> `engine/foundation/config/*`
5. `src/core/*.cpp` -> `engine/core/*`
6. `src/c_api/*.cpp` -> `engine/core/c_api/*`
7. `cpp/physics_ecs*.hpp` -> `engine/ecs/*`

### 3.2 Systems

1. `src/physics2d/*.cpp` -> `engine/systems/physics2d/*`
2. `src/content/subsystem_render_audio_animation.cpp` -> `engine/systems/render_audio_animation/*`
3. `src/content/scene_schema.cpp` -> `engine/systems/scene/schema/*`
4. `src/content/prefab_schema.cpp` -> `engine/systems/prefab/schema/*`
5. `src/content/asset_*.cpp` -> `engine/systems/assets/*`

### 3.3 Editor / Tools

1. `apps/sandbox_dwrite/*` -> `tools/editor/*`
2. `tools/scene_migrate_main.cpp` -> `tools/asset_pipeline/migrations/scene_migrate_main.cpp`
3. `tools/subsystem_workflow_demo.cpp` -> `tools/dev/subsystem_workflow_demo.*`
4. `tools/parallel_benchmark_compare.cpp` -> `tools/dev/parallel_benchmark_compare.*`
5. `tools/phase_d_profile_capture.cpp` -> `tools/dev/phase_profile_capture.*`

### 3.4 Third-party Adapters

1. Win32 / DWrite / shell / filesystem watcher 平台互操作 -> `third_party_adapters/win32/*`
2. 未来字体、音频、资源格式库适配 -> `third_party_adapters/<vendor_or_api>/*`
3. 当前直接散落在 editor/app/content 中的第三方访问，后续迁移时优先被抽到 adapter 层。

## 4. 依赖方向冻结

1. `tools/* -> engine/*` 允许，反向禁止。
2. `engine/systems/* -> engine/ecs + engine/core + engine/foundation` 允许。
3. `engine/core -> engine/foundation` 允许。
4. `engine/foundation` 不反向依赖 `tools/*` 或 `engine/systems/*`。
5. 第三方访问必须尽量经 `third_party_adapters/*` 暴露，避免再新增直接平台穿透。

## 5. 分阶段搬迁顺序

### 5.1 优先级 1

1. `cpp/*` foundation 能力继续成组搬到 `engine/foundation/*`
2. `tools/scene_migrate_main.cpp` 与后续迁移工具搬到 `tools/asset_pipeline/migrations/*`

### 5.2 优先级 2

1. `apps/sandbox_dwrite/*` 逐步拆成 `tools/editor/application/*`、`tools/editor/presentation/*`、`tools/editor/infrastructure/*`
2. `src/content/asset_*` 收敛到 `engine/systems/assets/*`

### 5.3 优先级 3

1. Win32 / DWrite / watcher 适配从 editor/content 中下沉到 `third_party_adapters/win32/*`
2. `scene/prefab/project` 相关数据对象在 Phase F 工程模型稳定后再统一落位

## 6. 不在 Phase E 做的事

1. 不在本阶段批量改 include 路径和目录树。
2. 不在本阶段一次性移动 `apps/`、`src/content/`、`tools/` 的全部文件。
3. 不把尚未设计稳定的 Project/Workspace/Package 模型提前塞进目录树。
