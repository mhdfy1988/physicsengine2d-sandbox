# Runtime 架构固化阶段 B 进度

更新时间：2026-03-05  
分支：`cpp-migration-baseline`

## 已完成
1. B 阶段详细设计文档已创建：`RUNTIME_PHASE_B_DETAILED_DESIGN.md`。
2. Runtime Facade 已落地：`cpp/physics_runtime_facade.hpp`。
3. Runtime Facade 支持 `TickSnapshot` 导出：
   - 帧序号
   - spawn/sync/cleanup 统计
   - mapping 错误计数
   - bridge 校验报告
4. ECS-Physics 桥接已增强为双向映射并可校验一致性。
5. 已完成 ECS->Physics 增量脏标记同步：
   - Transform/BodySpec 脏更新仅同步变更体
   - Collider 脏更新触发受控 respawn
5. 新增并接入 smoke：
   - `cpp_runtime_facade_smoke`
   - `cpp_ecs_bridge_smoke`
   - `cpp_ecs_dirty_sync_smoke`
6. CI 门禁已升级：
   - Windows: `test + benchmark + arch/api checks`
   - Ubuntu: `cmake tests + benchmark + selected cpp smoke`
7. 已增加 Physics->ECS 最小事件桥：
   - `ContactCreated`
   - `BodySleep`
   - `BodyWake`
   并通过 `runtime.last_events()` 暴露给上层。
8. 已补充编辑器近似控制流 smoke：
   - `cpp_runtime_pause_step_smoke`
   覆盖暂停（不步进）/单步恢复/再次暂停行为。
9. 已把 runtime 快照接到编辑器最小闭环：
   - `AppRuntime` 支持 `RuntimeTick` 事件与 `last_snapshot`
   - `main.c` 每帧上报并消费快照，状态栏优先展示该快照指标。
10. 已新增 editor 通路最小集成测试：
    - `tests/app_runtime_tick_smoke.c`
    - 已接入 `mingw32-make test` 与 CMake `BUILD_TESTS`。
11. editor 运行控制进一步事件化：
    - 新增 `APP_CMD_STEP_ONCE`
    - `N` 键与菜单单步都通过 `AppRuntime` 分发，不再走 main 直连分支。

## 当前状态
1. `mingw32-make test`：`PASS (31/31)`。
2. `mingw32-make benchmark`：`PASS`。
3. `check_arch_deps.ps1` / `check_api_surface.ps1`：通过。

## 下一步（B2 收口）
1. 把“暂停/单步/恢复”状态变化补充为统一 runtime 事件条目（供 UI/日志面板直接消费）。
