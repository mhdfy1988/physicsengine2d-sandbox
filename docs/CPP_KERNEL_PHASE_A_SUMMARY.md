# 物理内核 C++ 化阶段 A 总结

更新时间：2026-03-05  
分支：`cpp-migration-baseline`

## 本阶段目标
1. 将物理内核实现从 C 迁移到 C++17。
2. 保持对外 C API 兼容，不引入破坏性 API 变更。
3. 在迁移过程中保持回归可验证、可回滚。

## 已完成项
1. `src` 内核实现已全部迁移为 `.cpp` 文件。
2. 目录分层完成：
   - `src/c_api/`
   - `src/core/`
   - `src/physics2d/`
3. 构建脚本已切换并验证：
   - `Makefile`：默认 C++17 内核，支持分层目录编译
   - `CMakeLists.txt`：同步新目录源文件清单
4. 回归测试通过：
   - `mingw32-make test`：`PASS (31/31)`
5. 基准测试通过：
   - `mingw32-make benchmark`：`avg_step_ms=0.2217`，`[PASS] benchmark gate`

## 兼容性结论
1. 对外头文件和 C API 调用方式保持不变。
2. 现有回归场景未出现行为回归。
3. 新增等价性测试覆盖了部分“旧路径 vs 新路径”API 语义一致性。

## 剩余工作（阶段 A 收口）
1. 从迁移前分支补采 C 内核 benchmark，形成 C vs C++ 对照表。
2. 明确 `PHYSICS_USE_CPP_KERNEL` 的退场策略（保留版本周期、移除条件）。
3. 在 CI 中固化等价测试与性能门禁。
