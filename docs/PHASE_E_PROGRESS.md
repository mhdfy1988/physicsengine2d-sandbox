# 阶段 E 进度（全工程 C++ 收敛与目录落地）

更新时间：2026-03-06  
分支：`cpp-migration-baseline`  
状态：阶段 E 完成，已封板

> 跟踪基线：以 [PHASE_E_DETAILED_DESIGN.md](./PHASE_E_DETAILED_DESIGN.md) 和 [PHASE_E_ACCEPTANCE.md](./PHASE_E_ACCEPTANCE.md) 为准。

## 已完成

1. E1 Foundation Baseline：
   - 新增 `filesystem / task runner / logger / runtime config` C++ foundation 原语
   - `cpp_foundation_smoke` 接入 `mingw32-make test`
2. E2 Tool / Pipeline Entry Migration：
   - `tools/scene_migrate_main.cpp` 切到 C++ 主入口
   - `src/content/asset_importer.cpp` 切到 C++ 编译单元并复用 foundation filesystem
   - `scripts/check_cpp_convergence.ps1` + allowlist 固化“不再新增新的 C 主实现模块”
3. E3 App / Editor Path Convergence：
   - 输出 [`PHASE_E_C_RETENTION_PLAN.md`](./PHASE_E_C_RETENTION_PLAN.md)
   - 冻结 C 保留模块、原因、退出条件与计划移除阶段
4. E4 Directory Mapping Freeze：
   - 输出 [`PHASE_E_DIRECTORY_MAPPING.md`](./PHASE_E_DIRECTORY_MAPPING.md)
   - 冻结 `engine/*` / `tools/*` / `third_party_adapters/*` 映射与搬迁顺序
5. Phase E gate suite：
   - 输出 `artifacts/phase_e_gate_suite_20260306_170958/summary.md`

## 2026-03-06 Phase E Kickoff

1. 建立阶段 E 设计 / 验收 / 进度文档基线。
2. 落地第一批可复用 C++ foundation。
3. 选择 `scene_migrate` 与 `asset_importer` 作为迁移样板。

## 2026-03-06 Phase E Closure

1. Makefile 链接模型完成对 C++ runtime 的适配，`libphysics2d.a` 中的 C++ 对象不再阻断 C 目标链接。
2. CMakeLists 同步补齐混合库链接语言与 `asset_fs_watch.c` 源列表。
3. Phase E 新增收敛护栏已落地：
   - `scripts/check_cpp_convergence.ps1`
   - [`phase_e_c_retention_allowlist.txt`](./phase_e_c_retention_allowlist.txt)
4. 阶段文档已完成：
   - [`PHASE_E_C_RETENTION_PLAN.md`](./PHASE_E_C_RETENTION_PLAN.md)
   - [`PHASE_E_DIRECTORY_MAPPING.md`](./PHASE_E_DIRECTORY_MAPPING.md)
   - [`PHASE_E_EVIDENCE.md`](./PHASE_E_EVIDENCE.md)
   - [`PHASE_E_FINAL_REPORT.md`](./PHASE_E_FINAL_REPORT.md)

## 最终验证

1. `powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\run_phase_e_gate_suite.ps1`
   - PASS
   - 归档：`artifacts/phase_e_gate_suite_20260306_170958/summary.md`
2. 单项门禁也已通过：
   - `mingw32-make test`
   - `mingw32-make sandbox`
   - `mingw32-make benchmark`
   - `mingw32-make parallel-benchmark-compare`
   - `mingw32-make subsystem-workflow-demo`
   - `mingw32-make phase-d-profile`
   - `mingw32-make scene-migrate`
   - `bin/scene_migrate.exe --in tests/data/scene_snapshot_v0.txt --out _tmp_phase_e_scene_v1.scene`
   - `scripts/check_arch_deps.ps1`
   - `scripts/check_api_surface.ps1`
   - `scripts/check_cpp_convergence.ps1`
