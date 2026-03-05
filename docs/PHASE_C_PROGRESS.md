# 阶段 C 进度（数据与内容管线）

更新时间：2026-03-05  
分支：`cpp-migration-baseline`  
状态：已启动（文档基线已完成，功能开发未开始）

> 跟踪说明：阶段 C 以 [PHASE_C_DETAILED_DESIGN.md](./PHASE_C_DETAILED_DESIGN.md) 和 [PHASE_C_ACCEPTANCE.md](./PHASE_C_ACCEPTANCE.md) 为准。

## 已完成（启动准备）

1. 已产出阶段 C 详细设计文档（范围、架构、里程碑、DoD、风险、交付清单）。
2. 已产出阶段 C 验收清单（可勾选，覆盖 C1-C4 和封板门禁）。
3. 已在总架构文档阶段 C 条目中挂接详细设计入口。
4. 已确认阶段 B 封板结论可作为阶段 C 前置条件。

## 进行中

1. C1（Schema 与迁移基线）任务拆解与实现排期。

## 待完成（按里程碑）

### C1：Schema 与迁移基线

- [ ] 定稿 Scene/Prefab schema v1（字段、稳定 ID、写盘策略）
- [ ] 落地迁移工具骨架（支持 `dry-run`）
- [ ] 增加 schema round-trip 与 migration golden tests

### C2：Asset Database 与导入器

- [ ] 落地 GUID + `.meta` + 依赖图
- [ ] 接入纹理导入器
- [ ] 接入字体导入器
- [ ] 接入音频导入器
- [ ] 完成 importer determinism tests

### C3：热重载闭环

- [ ] 文件监听、去抖、批处理
- [ ] 导入失效与增量重导入
- [ ] runtime 局部刷新与失败回退
- [ ] demo 场景 hot-reload smoke

### C4：封板与验收

- [ ] 全量回归与 C 阶段新增测试全绿
- [ ] 产出 C 阶段验收证据包
- [ ] 产出 C 阶段封板报告
- [ ] 冻结阶段 C 数据协议与导入器基线

## 当前门禁状态

1. 阶段 B 封板门禁（`test/sandbox/benchmark/arch/api`）在 2026-03-05 记录为通过。
2. 阶段 C 新增测试尚未落地，当前无执行结果。

## 风险与关注点

1. B->C 迁移双路径漂移风险（C 引擎旧路径 vs facade 驱动路径）。
2. schema 初版变更频繁导致迁移工具反复调整。
3. 资源依赖图错误导致热重载“漏刷新”或“过度刷新”。

## 下一次更新触发条件

1. C1 schema v1 定稿并提交实现。
2. 迁移工具 `dry-run` 可用并有首批 golden 样例。
3. 任一 C 阶段门禁测试首次进入 CI。

## 2026-03-05 C1 Kickoff Implementation
- Added `scene_schema` v1 runtime module with deterministic save/load.
- Added legacy snapshot -> schema migration API with dry-run and atomic write.
- Added CLI tool: `bin/scene_migrate`.
- Added regression coverage: round-trip determinism + migration golden tests.
- Added sample data: `Scenes/sample_scene_v1.scene`.

## 2026-03-05 C1 Prefab Schema Baseline
- Added `prefab_schema` v1 load/save module with deterministic ordering.
- Added prefab regression coverage: round-trip determinism + variant fixture parse.
- Added prefab schema docs and sample file: `docs/PREFAB_SCHEMA_V1.md`, `Prefabs/sample_prefab_v1.prefab`.

## 2026-03-05 C2 Kickoff (Asset Database Baseline)
- Added `asset_database` baseline module: AssetMeta read/write, deterministic GUID and cache-key generation.
- Added dependency graph prototype with transitive query support.
- Added regression tests for AssetMeta determinism and dependency graph behavior.
- Added baseline docs/sample: `docs/ASSET_META_V1.md`, `Assets/hero.png.meta`.

## 2026-03-05 C2 Importer Skeleton + Invalidation
- Added `asset_importer` skeleton: texture/font/audio extension routing, deterministic meta + cache artifact output.
- Added `asset_invalidation` propagation utility: changed asset -> reverse dependents batch.
- Added regression tests: importer smoke and invalidation propagation.
- Added design note: `docs/ASSET_IMPORTER_V1.md`.

## 2026-03-05 C2 Incremental Reimport Queue
- Added `asset_pipeline` state machine: source registry, changed queue, dependency wiring, batch reimport execution.
- Added queue behavior guarantees: deduplicated change marks and no-op empty run.
- Added failure-path handling report for missing source during reimport.
- Added regression tests and design note: `tests/regression_asset_pipeline_tests.c`, `docs/ASSET_PIPELINE_V1.md`.

## 2026-03-05 C2 Structured Import Parameters
- Added typed importer parameter model (texture/font/audio) with deterministic fingerprint generation.
- Integrated parameter fingerprint into importer cache-key path.
- Added regression test ensuring same parameters => same cache key, different parameters => different cache key.

## 2026-03-05 C2 Meta Parameter Deserialization + Batch Marking
- Extended AssetMeta with structured importer parameter fields and serialization support.
- Pipeline source registration now auto-loads `<source>.meta` parameters when available.
- Added batch source-change marking API for deduplicated event windows.
- Added regression coverage for meta-parameter deserialization in pipeline reimport path.

## 2026-03-05 C3 Kickoff (Watch + Hot-Reload Bridge)
- Added `asset_watch` debounce batch module for file-change event windows.
- Added `asset_hot_reload` bridge service: watch batch -> pipeline mark -> reimport tick.
- Added regression coverage for debounce behavior and hot-reload pipeline bridge flow.
- Added design note: `docs/ASSET_HOT_RELOAD_V1.md`.

## 2026-03-05 C3 Poll Source + Runtime Event Bridge
- Added `asset_fs_poll` source module (exists/mtime/size polling) and bridge to hot-reload change events.
- Extended runtime event model with hot-reload batch event snapshot.
- Added regression coverage for fs-poll bridge and runtime hot-reload event publication.

## 2026-03-05 C3 Runtime Loop Integration (Sandbox)
- Integrated `asset_hot_reload` service and `asset_fs_poll` into `apps/sandbox_dwrite/main.c` startup and tick loop.
- Added recursive asset source auto-registration from `assets/` (supported importer extensions only).
- Added runtime handling for `APP_EVENT_HOT_RELOAD_BATCH` with editor log output and in-memory counters.
- Added status-bar hot-reload summary (`+imported / -failed`) with recent-activity highlighting.

## 2026-03-05 C3 Runtime Robustness Enhancements
- Added periodic runtime discovery for newly added files under `assets/` and automatic watch-list expansion.
- Added hot-reload failure bridging into runtime unified error channel (`APP_RUNTIME_ERROR_CODE_PIPELINE_MAPPING_ERRORS`).
- Added regression case for missing-source hot-reload failure contract and kept existing hot-reload bridge tests green.
- Added rollback-safety regression: failed reimport must preserve previous successful artifact content.
- Added `scripts/hot_reload_smoke.ps1` for end-to-end sandbox smoke (launch, mutate asset twice, verify artifact updates).

## 2026-03-06 C3 Polling/Smoke Stabilization
- Upgraded `asset_fs_poll` file timestamp query from `_stat64` seconds granularity to Windows `GetFileAttributesExA` high-resolution write time.
- Fixed poll re-watch behavior: re-registering an already watched path no longer resets poll baseline state.
- Added regression test `regression_test_asset_fs_poll_rewatch_does_not_swallow_change`.
- Improved `scripts/hot_reload_smoke.ps1`:
  - explicit sandbox `WorkingDirectory`
  - resilient second-stage retries to tolerate polling race windows
  - preflight mode retained via `-SkipLaunch`
- Evidence:
  - `mingw32-make test` => `49/49 PASS` + `app_runtime_smoke PASS`
  - `powershell -File scripts/hot_reload_smoke.ps1` => `PASS`

## 2026-03-06 C-Phase Gate Suite (Full)
- `mingw32-make test` passed (`physics_tests 49/49`, `app_runtime_smoke PASS`).
- `mingw32-make sandbox` passed.
- `mingw32-make benchmark` passed (`avg_step_ms=0.2233`, gate `8.0000`).
- `scripts/check_arch_deps.ps1` passed.
- `scripts/check_api_surface.ps1` passed.
- `scripts/hot_reload_smoke.ps1` passed end-to-end.

## 2026-03-06 C4 Seal Report Published
- Produced closure report: `docs/PHASE_C_FINAL_REPORT.md`.
- Included closure verdict, full quality-gate evidence, residual risks, and Phase D entry recommendations.

## 2026-03-06 Phase C Closure Verdict
- Phase C is now in **closure-candidate** state.
- All planned C-phase gate commands are green in local verification set.
- Remaining follow-up belongs to Phase D scope (native watch backend, CI headless smoke harness, finer error taxonomy).

## 2026-03-06 Evidence Archive Published
- Added `docs/PHASE_C_EVIDENCE.md` as command/result and key-file evidence bundle for C-stage sign-off review.
- Added one-click gate runner `scripts/run_phase_c_gate_suite.ps1`, with versioned summary `docs/PHASE_C_GATE_SUITE_20260306_003435.md`.
- Added commit split guide `docs/PHASE_C_COMMIT_PLAN.md` for structured merge and review.
