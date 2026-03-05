# Runtime 阶段 B 验收清单

更新时间：2026-03-05

## A. 边界与架构

- [x] Runtime Facade 作为统一入口（Tick/Event/Error）
- [x] ECS 与 Physics2D 桥接有一致性校验
- [x] 架构依赖检查脚本持续通过
- [ ] 编辑器事件总线直连 C++ `last_errors()` 多错误列表

## B. 运行时能力

- [x] Tick snapshot 可用于状态栏/调试面板
- [x] Runtime event（接触/睡眠/唤醒）可消费
- [x] Runtime error 语义（code/name/severity）已统一
- [x] 事件总线拥塞检测、告警、恢复状态机完成

## C. 工具链与体验

- [x] 编辑器运行/暂停/单步走统一 runtime 通道
- [x] 调试面板可查看运行时快照与历史
- [x] 拥塞红点可点击跳转警告日志
- [x] 错误日志具备可读文本标签

## D. 测试与CI

- [x] C 回归测试通过
- [x] benchmark 通过
- [x] C++ runtime error smoke（基础）通过
- [ ] C++ runtime error smoke（恢复场景）完成
- [ ] C++ runtime error smoke（多错误并发）完成
- [x] API surface/arch guardrails 通过

## E. 进入阶段 C 前置项

- [ ] 输出 B 阶段封板报告（证据链接 + 风险清单）
- [ ] 冻结 B 阶段接口基线（防止 C 阶段重构回归）
