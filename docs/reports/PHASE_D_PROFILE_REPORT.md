# 阶段 D Profiling / Benchmark 报告

更新时间：2026-03-06  
分支：`cpp-migration-baseline`

证据源：

1. `artifacts/phase_d_gate_suite_20260306_163111/gate_parallel_benchmark_compare.stdout.log`
2. `artifacts/phase_d_gate_suite_20260306_163111/gate_phase_d_profile.stdout.log`
3. `artifacts/phase_d_d4_closure_20260306_162937/summary.md`

## 1. 命令与环境

1. `mingw32-make parallel-benchmark-compare`
   - 默认负载：`bodies=960`, `warmup=120`, `steps=480`
2. `mingw32-make phase-d-profile`
   - 默认负载：`bodies=960`, `warmup=120`, `steps=240`, `import_runs=5`
3. 主机信息：
   - `logical_cpus=16`

## 2. CPU 关键路径剖析

`phase-d-profile` 采集的是引擎内部 step profile，反映 `physics_internal_step()` 内部分阶段耗时。

- 平均总耗时：`6.7591 ms`
- `integrate`: `0.3513 ms`
- `broadphase`: `1.5961 ms`
- `narrowphase`: `1.3968 ms`
- `solve`: `3.3958 ms`
- `clear_forces`: `0.0059 ms`
- 同场景观测：
  - `pairs=256`
  - `contacts=254`

结论：

1. `solve` 仍是当前 D4 负载下的主耗时段，约占总 step 时间的一半。
2. `broadphase + narrowphase` 合计约 `2.99 ms`，说明碰撞管线仍是下一轮优化的重要候选。

## 3. 内存与加载时长

`phase-d-profile` 当前输出的是“运行时分配估算”，不是 OS working set。估算公式包括：

1. `sizeof(PhysicsEngine)`
2. `pair_stamp` 堆内存
3. `scratch` arena 堆内存
4. 活跃 `RigidBody` 堆对象
5. `SubsystemWorkflowWorld` 结构大小

本次结果：

- engine struct: `586160 bytes`
- pair stamp: `2097152 bytes`
- scratch arena: `1130496 bytes`
- active bodies: `115320 bytes`
- runtime total estimated: `3929128 bytes`
- workflow world: `8088 bytes`
- texture import avg: `0.6618 ms`
- audio import avg: `0.6439 ms`

结论：

1. D4 子系统工作流额外 staging 内存很小，主量级仍在物理世界和 broadphase 辅助结构。
2. 当前示例资源导入延迟低于 `1 ms`，足以支撑 demo / smoke / 回归闭环。

## 4. 并行调度对比

`parallel-benchmark-compare` 使用外部 wall-clock，对比的是完整 step 墙钟时间。

- sequential: `8.9448 ms`
- threaded / 2 workers: `8.9441 ms` (`1.000x`)
- threaded / 4 workers: `8.9314 ms` (`1.001x`)

`phase-d-profile` 也输出了外部 wall-clock 并行对比：

- sequential: `6.8351 ms`
- threaded / 2 workers: `6.8058 ms` (`1.004x`)
- threaded / 4 workers: `6.7934 ms` (`1.006x`)

实现变化：

1. `src/core/physics_parallel.cpp` 的 Win32 路径已从“每次 dispatch 新建线程”改为 `QueueUserWorkItem` 线程池分发。
2. `tools/parallel_benchmark_compare.cpp` 已从 `clock()` 切换到高精度 wall-clock，避免多线程 CPU 时间累计导致的错误结论。

结论：

1. 首轮优化的主要收益是把 960-body 样例从明显回退拉回到 break-even 以上。
2. 当前收益仍然很小，说明下一轮如果要继续放大并行红利，需要更深层的任务图/常驻 worker 设计，而不仅是 dispatch 方式优化。

## 5. 结论

阶段 D 的 profiling / benchmark 目标已经满足“有工具、有数据、有报告”的最低闭环：

1. CPU 关键路径可量化。
2. 内存与加载时长有可复测样例。
3. 并行调度优化有实现改动和前后基准对比。
