# Runtime 阶段 B 进度

更新时间：2026-03-05  
分支：`cpp-migration-baseline`

## 已完成

1. Runtime Facade 已落地并稳定运行：
   - `TickSnapshot`（帧序号、桥接校验、统计）
   - `last_events()`（接触创建、睡眠、唤醒）
   - `last_errors()`（桥接错误通道）
2. ECS-Physics 桥接已具备基础一致性保障：
   - 双向映射（entity -> body / body -> entity）
   - `validate_bridge()` 校验与映射错误统计
   - dirty sync + collider 变更触发受控 respawn
3. 编辑器最小运行时闭环已完成：
   - `AppRuntime` 事件化（tick/state/command）
   - 右侧调试面板显示运行时快照、历史、错误与拥塞状态
   - 状态栏与顶部工具栏拥塞提示（含红点跳转告警）
4. 运行时可观测性增强：
   - 事件总线 dropped 计数
   - 拥塞/恢复状态机与节流日志
   - 运行时错误码与可读标签展示
5. 测试与CI已覆盖关键路径：
   - 新增并接入 `cpp_runtime_error_channel_smoke`
   - 现有 C/C++ smoke、回归、benchmark、架构/API 检查持续全绿

## 未完成（B阶段收口项）

1. C++ `RuntimeFacade::last_errors()` 到编辑器事件总线的直连：
   - 当前编辑器仍主要消费 C 引擎 `last_error`（0/1 + code）
   - 尚未携带“多错误列表”完整透传
2. B阶段验收清单文档（封板版）：
   - 需单独产出“DoD逐条对照 + 证据链接 + 残留风险”
3. 错误通道 smoke 还需补两个场景：
   - 错误恢复场景
   - 多错误并发场景
4. B阶段封板回归报告：
   - 一次只修不加功能的收口回归与最终结论

## 当前门禁状态

1. `mingw32-make test`：通过（31/31 + app_runtime_smoke）
2. `mingw32-make sandbox`：通过
3. `mingw32-make benchmark`：通过
4. `check_arch_deps.ps1`：通过
5. `check_api_surface.ps1`：通过
