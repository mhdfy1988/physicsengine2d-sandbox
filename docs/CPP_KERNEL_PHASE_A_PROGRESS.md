# 物理内核 C++ 化阶段 A 进度

更新时间：2026-03-05
分支：`cpp-migration-baseline`

## 已完成
1. `src/` 物理内核实现已全部迁移为 `.cpp`（C API 保持不变）。
2. 构建链已支持并默认启用 C++17 内核路径（`PHYSICS_USE_CPP_KERNEL=1`）。
3. 目录分层已落地：
   - `src/c_api/`：C API 导出实现
   - `src/core/`：生命周期、世界状态、并行、快照等核心能力
   - `src/physics2d/`：2D 物理算法与求解模块
4. 回归验证：`mingw32-make clean; mingw32-make test` 通过（`PASS 29/29`）。
5. 已新增 `tests/equivalence/` 等价性测试并接入回归（当前 `PASS 31/31`）。

## 待完成（阶段 A 收尾）
1. 增补性能对比记录：固定场景下输出 step 时间基线与对比结论（已新增 C++ 基线记录，待补 C 对照）。

## 风险与说明
1. 当前环境缺少 `cmake` 可执行文件，已用 `mingw32-make` 完成持续回归验证。
2. 目录分层已完成，但尚未引入更细粒度的等价测试与性能门禁。
