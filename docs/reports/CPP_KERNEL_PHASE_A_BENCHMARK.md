# 物理内核阶段 A 性能记录（C++ 内核）

更新时间：2026-03-05  
分支：`cpp-migration-baseline`

## 环境与命令
1. 统一命令：`mingw32-make benchmark`
2. C 基线（迁移前提交 `1240fa3`）：
   - `avg_step_ms=0.2133`
   - `gate=8.0000`
   - 结果：`[PASS] benchmark gate`
3. C++ 当前（`cpp-migration-baseline`）：
   - `avg_step_ms=0.2200`
   - `gate=8.0000`
   - 结果：`[PASS] benchmark gate`

## 对照结论
1. C++ 相对 C 基线变化：`+3.14%`（`(0.2200 - 0.2133) / 0.2133`）。
2. 在阶段 A 设定的 `<=5%` 预算内，满足门限。

## 结论
1. 当前 C++ 内核在现有基准门限下通过。
2. C vs C++ 对照已补齐，可作为阶段 B/C 的回归参考点。

## 说明
1. C 基线通过临时 worktree 在迁移前提交复测得到，避免污染当前分支。
