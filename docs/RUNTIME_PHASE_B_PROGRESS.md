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
5. 新增并接入 smoke：
   - `cpp_runtime_facade_smoke`
   - `cpp_ecs_bridge_smoke`
6. CI 门禁已升级：
   - Windows: `test + benchmark + arch/api checks`
   - Ubuntu: `cmake tests + benchmark + selected cpp smoke`

## 当前状态
1. `mingw32-make test`：`PASS (31/31)`。
2. `mingw32-make benchmark`：`PASS`。
3. `check_arch_deps.ps1` / `check_api_surface.ps1`：通过。

## 下一步（B2 后半段）
1. 增加 ECS->Physics 增量同步路径（参数变更脏标记）。
2. 增加 Physics->ECS 事件桥（contact/sleep/wakeup 最小集）。
3. 为编辑器最小闭环准备 runtime 调试快照导出接口。
