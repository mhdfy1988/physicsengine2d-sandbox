# PhysicsEngine2D 内核使用手册（API 中文版）

> AI 声明：本项目完全由 AI 生成（架构、代码、测试与文档）。


本文档面向“要真正接入并跑起来”的使用者，不只列 API 名称，还说明调用顺序、资源所有权、常见错误和工程集成方式。

## 1. 你要拿走哪些文件

跨项目复用时，请把以下内容作为同一版本工件一起提供：

- `include/*.h`（头文件，定义对外 API）
- `lib/libphysics2d.a`（静态库，编译产物）

不要只拷贝 `lib` 不拷贝 `include`，也不要只拷贝 `include` 不拷贝 `lib`。

## 2. 快速接入

### 2.1 最小编译命令

```bash
gcc your_app.c -I path/to/include path/to/lib/libphysics2d.a -lm
```

### 2.2 最小运行示例

```c
#include "physics.h"
#include <stdio.h>

int main(void) {
    PhysicsEngine* engine = physics_engine_create();
    if (!engine) return 1;

    // 地面（静态）
    Shape* ground_shape = shape_create_box(20.0f, 1.0f);
    RigidBody* ground = body_create(0.0f, -5.0f, 0.0f, ground_shape);
    body_set_type(ground, BODY_STATIC);

    // 小球（动态）
    Shape* ball_shape = shape_create_circle(0.5f);
    RigidBody* ball = body_create(0.0f, 5.0f, 1.0f, ball_shape);

    physics_engine_add_body(engine, ground);
    physics_engine_add_body(engine, ball);

    for (int i = 0; i < 120; ++i) {
        physics_engine_step(engine);
        printf("frame=%d, ball=(%.2f, %.2f)\\n", i, ball->position.x, ball->position.y);
    }

    physics_engine_free(engine);
    return 0;
}
```

## 3. 正确的生命周期（最重要）

推荐固定遵循以下顺序：

1. 创建引擎：`physics_engine_create`
2. 创建形状：`shape_create_*`
3. 创建刚体：`body_create`
4. 设置刚体属性：`body_set_type / body_set_friction / body_set_restitution ...`
5. 把刚体加入引擎：`physics_engine_add_body`
6. （可选）添加约束：`physics_engine_add_distance_constraint / physics_engine_add_spring_constraint`
7. 每帧推进：`physics_engine_step`
8. 结束时释放：`physics_engine_free`

如果你按这个顺序接入，绝大多数崩溃和“没反应”问题都会避免。

## 4. 所有权与内存管理

### 4.1 `PhysicsEngine` 是不透明类型

`PhysicsEngine` 不能直接访问内部字段，只能通过 `physics.h` 暴露的函数读写。

### 4.2 刚体与形状所有权

`RigidBody` 结构体里有：

- `Shape* shape`
- `int owns_shape`

默认场景下，你可以把“一个 shape 只给一个 body 使用”，由 body 管理 shape 的释放。

如果你要多个 body 共享同一个 shape，必须显式管理所有权，避免重复释放。

### 4.3 `remove` 和 `detach` 的区别

- `physics_engine_remove_body(engine, body)`
- `physics_engine_detach_body(engine, body)`

当前工程语义建议：

- `remove`：从引擎移除，并按引擎当前流程处理其资源生命周期
- `detach`：只从引擎摘除，返回指针，调用方继续持有并自行处理后续释放

当你不确定该用哪个，优先用 `detach`，逻辑更可控。

## 5. 每帧循环怎么写

### 5.1 推荐：一步到位

每帧直接调用：

- `physics_engine_step(engine)`

它会完成速度/位置更新、碰撞检测与求解等流程。

### 5.2 进阶：分阶段调试

当你要做诊断（例如检查某阶段数值异常）时可拆开调用：

- `physics_engine_update_velocities`
- `physics_engine_update_positions`
- `physics_engine_detect_collisions`
- `physics_engine_resolve_collisions`
- `physics_engine_clear_forces`

生产逻辑建议仍然使用 `physics_engine_step`，避免流程顺序不一致。

### 5.3 回调与事件钩子（架构调试推荐）

可通过 `physics_engine_set_callbacks` 注册统一回调：

- `on_pre_step`：帧开始
- `on_post_broadphase`：广相完成（拿到 pair_count）
- `on_post_narrowphase`：窄相完成（拿到 contact_count）
- `on_contact_created`：每个接触生成时触发
- `on_post_step`：帧结束（含 `PhysicsStepProfile`）

## 6. 参数调优建议（实战）

### 6.1 基础稳定性参数

- `physics_engine_set_time_step(engine, dt)`
- `physics_engine_set_iterations(engine, iterations)`
- `physics_engine_set_damping(engine, damping)`

建议起点：

- `dt = 1.0f / 60.0f`
- `iterations = 8~12`
- `damping = 0.98f~1.0f`

### 6.2 广相（broadphase）

- `physics_engine_set_broadphase_use_grid(engine, 1)`
- `physics_engine_set_broadphase_cell_size(engine, cell_size)`

一般规则：`cell_size` 取“场景常见物体直径的 1~2 倍”。

### 6.3 自定义 Pipeline（高级）

可以注册自定义阶段构建器：

- `physics_engine_set_broadphase_builder(engine, builder, user)`
- `physics_engine_set_narrowphase_builder(engine, builder, user)`
- `physics_engine_reset_pipeline(engine)`（恢复默认实现）

自定义实现可使用这些辅助接口回写结果：

- `physics_engine_clear_broadphase_pairs`
- `physics_engine_add_broadphase_pair`
- `physics_engine_clear_contacts`
- `physics_engine_add_contact`

## 7. 常用读取接口怎么用

### 7.1 读取碰撞点（只读）

```c
int n = physics_engine_get_contact_count(engine);
for (int i = 0; i < n; ++i) {
    const CollisionManifold* m = physics_engine_get_contact(engine, i);
    if (!m) continue;
    // m->info.point / m->info.normal / m->info.penetration
}
```

### 7.2 读取约束并修改参数（通过索引 API）

```c
int count = physics_engine_get_constraint_count(engine);
for (int i = 0; i < count; ++i) {
    if (!physics_engine_constraint_is_active(engine, i)) continue;

    float k = physics_engine_constraint_get_stiffness(engine, i);
    physics_engine_constraint_set_stiffness(engine, i, k * 1.05f);
}
```

注意：`physics_engine_get_constraint` 返回 `const Constraint*`，不要直接改结构体字段。

## 8. 完整接入模板（推荐复制改造）

```c
#include "physics.h"

static PhysicsEngine* g_engine = NULL;

int physics_init(void) {
    g_engine = physics_engine_create();
    if (!g_engine) return 0;

    physics_engine_set_gravity(g_engine, vec2(0.0f, 9.8f));
    physics_engine_set_time_step(g_engine, 1.0f / 60.0f);
    physics_engine_set_iterations(g_engine, 10);
    physics_engine_set_damping(g_engine, 0.995f);

    Shape* floor_shape = shape_create_box(50.0f, 1.0f);
    RigidBody* floor_body = body_create(0.0f, 20.0f, 0.0f, floor_shape);
    body_set_type(floor_body, BODY_STATIC);
    physics_engine_add_body(g_engine, floor_body);

    return 1;
}

void physics_tick(void) {
    if (!g_engine) return;
    physics_engine_step(g_engine);
}

void physics_shutdown(void) {
    if (!g_engine) return;
    physics_engine_free(g_engine);
    g_engine = NULL;
}
```

## 9. 常见错误与排查

### 9.1 只拷贝了 `lib` 没拷贝 `include`

现象：编译找不到声明或类型不匹配。

### 9.2 头文件版本和静态库版本不一致

现象：能编译但运行异常、字段理解不一致。

处理：`include + lib` 必须来自同一版本发布。

### 9.3 把 `get_constraint` 返回值当可写对象

现象：编译报 `const` 相关错误，或强改导致未定义行为。

处理：使用 `physics_engine_constraint_set_*` 系列。

### 9.4 重复添加同一个刚体指针

现象：生命周期异常、潜在重复释放风险。

处理：确保每个 `RigidBody*` 仅 `add` 一次；移除后再决定是否重加。

### 9.5 直接访问 `PhysicsEngine` 内部字段

现象：编译失败（不透明类型）。

处理：改用 `physics_engine_get_* / set_*`。

## 11. 给内核维护者的同步规则

- 改实现：`src/*.c`
- 改对外能力：同步修改 `include/*.h` 与 `src/*.c`
- 回归验证：`make test`
- 变更检查：`make check-core`

如果你要把内核发给其他项目，请按版本发布（例如 `v1.0.x`），并保证 `include + lib` 是同一次构建/同一标签产物。

## 13. 全量 API 清单（按分类，含参数类型与示例）

字段说明：
- `参数` 列统一写法为 `参数名(类型): 示例`
- `返回结果` 是函数签名返回类型
- `结果说明` 包含成功/失败语义或边界行为

### 13.1 引擎接口（`physics.h`）

| 名称 | 说明 | 参数（参数类型 + 参数示例） | 返回结果 | 结果说明 |
|---|---|---|---|---|
| `physics_engine_create` | 创建引擎实例 | 无 | `PhysicsEngine*` | 成功返回非空，失败 `NULL` |
| `physics_engine_free` | 释放引擎及其管理对象 | `engine(PhysicsEngine*): engine` | `void` | `engine==NULL` 时忽略 |
| `physics_engine_set_gravity` | 设置全局重力 | `engine(PhysicsEngine*): engine`<br>`gravity(Vec2): vec2(0,9.8f)` | `void` | 无返回，空引擎忽略 |
| `physics_engine_set_time_step` | 设置步长 | `engine(PhysicsEngine*): engine`<br>`dt(float): 1.0f/60.0f` | `void` | `dt<=0` 忽略 |
| `physics_engine_set_iterations` | 设置迭代次数 | `engine(PhysicsEngine*): engine`<br>`iterations(int): 10` | `void` | `iterations<1` 忽略 |
| `physics_engine_set_damping` | 设置全局阻尼 | `engine(PhysicsEngine*): engine`<br>`damping(float): 0.99f` | `void` | 自动钳制到 `[0,1]` |
| `physics_engine_set_broadphase_cell_size` | 设置网格单元大小 | `engine(PhysicsEngine*): engine`<br>`cell_size(float): 8.0f` | `void` | `cell_size<=0` 忽略 |
| `physics_engine_set_broadphase_use_grid` | 开关网格广相 | `engine(PhysicsEngine*): engine`<br>`enable(int): 1` | `void` | 非 0 视为开启 |
| `physics_engine_get_gravity` | 读取重力 | `engine(const PhysicsEngine*): engine` | `Vec2` | 空引擎返回 `vec2(0,0)` |
| `physics_engine_get_time_step` | 读取步长 | `engine(const PhysicsEngine*): engine` | `float` | 空引擎返回 `0` |
| `physics_engine_get_iterations` | 读取迭代次数 | `engine(const PhysicsEngine*): engine` | `int` | 空引擎返回 `0` |
| `physics_engine_get_damping` | 读取阻尼 | `engine(const PhysicsEngine*): engine` | `float` | 空引擎返回 `0` |
| `physics_engine_get_broadphase_cell_size` | 读取网格单元 | `engine(const PhysicsEngine*): engine` | `float` | 空引擎返回 `0` |
| `physics_engine_get_broadphase_use_grid` | 读取网格开关 | `engine(const PhysicsEngine*): engine` | `int` | 空引擎返回 `0` |
| `physics_engine_get_body_count` | 读取刚体数量 | `engine(const PhysicsEngine*): engine` | `int` | 空引擎返回 `0` |
| `physics_engine_get_body` | 读取指定刚体 | `engine(const PhysicsEngine*): engine`<br>`index(int): 0` | `RigidBody*` | 越界返回 `NULL` |
| `physics_engine_get_contact_count` | 读取接触数量 | `engine(const PhysicsEngine*): engine` | `int` | 空引擎返回 `0` |
| `physics_engine_get_contact` | 读取指定接触 | `engine(const PhysicsEngine*): engine`<br>`index(int): 0` | `const CollisionManifold*` | 越界返回 `NULL` |
| `physics_engine_get_constraint_count` | 读取约束数量 | `engine(const PhysicsEngine*): engine` | `int` | 空引擎返回 `0` |
| `physics_engine_get_constraint` | 读取指定约束 | `engine(const PhysicsEngine*): engine`<br>`index(int): 0` | `const Constraint*` | 越界返回 `NULL` |
| `physics_engine_find_constraint_index` | 由指针查索引 | `engine(const PhysicsEngine*): engine`<br>`constraint(const Constraint*): cptr` | `int` | 找到 `>=0`，否则 `-1` |
| `physics_engine_constraint_is_active` | 读取约束激活状态 | `engine(const PhysicsEngine*): engine`<br>`index(int): 0` | `int` | 越界返回 `0` |
| `physics_engine_constraint_get_type` | 读取约束类型 | `engine(const PhysicsEngine*): engine`<br>`index(int): 0` | `ConstraintType` | 越界返回默认值 |
| `physics_engine_constraint_get_rest_length` | 读取静长 | `engine(const PhysicsEngine*): engine`<br>`index(int): 0` | `float` | 越界返回 `0` |
| `physics_engine_constraint_get_stiffness` | 读取刚度 | `engine(const PhysicsEngine*): engine`<br>`index(int): 0` | `float` | 越界返回 `0` |
| `physics_engine_constraint_get_damping` | 读取阻尼 | `engine(const PhysicsEngine*): engine`<br>`index(int): 0` | `float` | 越界返回 `0` |
| `physics_engine_constraint_get_break_force` | 读取断裂阈值 | `engine(const PhysicsEngine*): engine`<br>`index(int): 0` | `float` | 越界返回 `0` |
| `physics_engine_constraint_get_last_force` | 读取上一帧约束力 | `engine(const PhysicsEngine*): engine`<br>`index(int): 0` | `float` | 越界返回 `0` |
| `physics_engine_constraint_get_collide_connected` | 读取连接体碰撞开关 | `engine(const PhysicsEngine*): engine`<br>`index(int): 0` | `int` | 越界返回 `0` |
| `physics_engine_constraint_set_active` | 设置约束激活状态 | `engine(PhysicsEngine*): engine`<br>`index(int): 0`<br>`active(int): 1` | `void` | 越界忽略，值规范化为 `0/1` |
| `physics_engine_constraint_set_rest_length` | 设置静长 | `engine(PhysicsEngine*): engine`<br>`index(int): 0`<br>`rest_length(float): 2.0f` | `void` | 越界忽略，值钳制 `>=0` |
| `physics_engine_constraint_set_stiffness` | 设置刚度 | `engine(PhysicsEngine*): engine`<br>`index(int): 0`<br>`stiffness(float): 30.0f` | `void` | 越界忽略，值钳制 `>=0` |
| `physics_engine_constraint_set_damping` | 设置阻尼 | `engine(PhysicsEngine*): engine`<br>`index(int): 0`<br>`damping(float): 2.0f` | `void` | 越界忽略，值钳制 `>=0` |
| `physics_engine_constraint_set_break_force` | 设置断裂阈值 | `engine(PhysicsEngine*): engine`<br>`index(int): 0`<br>`break_force(float): 100.0f` | `void` | 越界忽略，值钳制 `>=0` |
| `physics_engine_constraint_set_collide_connected` | 设置连接体是否碰撞 | `engine(PhysicsEngine*): engine`<br>`index(int): 0`<br>`collide_connected(int): 0` | `void` | 越界忽略，值规范化 `0/1` |
| `physics_engine_get_broadphase_pair_count` | 读取广相候选对数 | `engine(const PhysicsEngine*): engine` | `int` | 空引擎返回 `0` |
| `physics_engine_add_body` | 添加刚体 | `engine(PhysicsEngine*): engine`<br>`body(RigidBody*): body` | `void` | 空指针/重复/超上限均忽略 |
| `physics_engine_remove_body` | 移除并释放刚体 | `engine(PhysicsEngine*): engine`<br>`body(RigidBody*): body` | `void` | 内部 detach 成功后 `body_free` |
| `physics_engine_detach_body` | 移除但不释放刚体 | `engine(PhysicsEngine*): engine`<br>`body(RigidBody*): body` | `RigidBody*` | 成功返回 body，失败 `NULL` |
| `physics_engine_add_distance_constraint` | 添加距离约束 | `engine(PhysicsEngine*): engine`<br>`a(RigidBody*): a`<br>`b(RigidBody*): b`<br>`world_anchor_a(Vec2): vec2(0,0)`<br>`world_anchor_b(Vec2): vec2(1,0)`<br>`stiffness(float): 0.5f`<br>`collide_connected(int): 1` | `Constraint*` | 失败返回 `NULL` |
| `physics_engine_add_spring_constraint` | 添加弹簧约束 | `engine(PhysicsEngine*): engine`<br>`a(RigidBody*): a`<br>`b(RigidBody*): b`<br>`world_anchor_a(Vec2): vec2(0,0)`<br>`world_anchor_b(Vec2): vec2(1,0)`<br>`rest_length(float): 2.0f`<br>`stiffness(float): 30.0f`<br>`damping(float): 2.0f`<br>`collide_connected(int): 0` | `Constraint*` | 失败返回 `NULL` |
| `physics_engine_clear_constraints` | 清空所有约束 | `engine(PhysicsEngine*): engine` | `void` | 空引擎忽略 |
| `physics_engine_step` | 执行一帧完整仿真 | `engine(PhysicsEngine*): engine` | `void` | 推荐主循环每帧调用 |
| `physics_engine_update_positions` | 仅更新位置/角度 | `engine(PhysicsEngine*): engine` | `void` | 分阶段调试用 |
| `physics_engine_update_velocities` | 仅更新速度 | `engine(PhysicsEngine*): engine` | `void` | 分阶段调试用 |
| `physics_engine_detect_collisions` | 仅做碰撞检测 | `engine(PhysicsEngine*): engine` | `void` | 会刷新 contact 缓存 |
| `physics_engine_resolve_collisions` | 仅做碰撞/约束求解 | `engine(PhysicsEngine*): engine` | `void` | 依赖已有 contact |
| `physics_engine_clear_forces` | 清空所有力和力矩 | `engine(PhysicsEngine*): engine` | `void` | 通常每帧末调用 |
| `physics_engine_raycast` | 线段射线拾取最近刚体 | `engine(PhysicsEngine*): engine`<br>`start(Vec2): vec2(0,0)`<br>`end(Vec2): vec2(10,0)` | `RigidBody*` | 未命中返回 `NULL` |
| `physics_engine_get_bodies_in_area` | 查询圆形区域内刚体 | `engine(PhysicsEngine*): engine`<br>`center(Vec2): vec2(0,0)`<br>`radius(float): 5.0f`<br>`out_bodies(RigidBody**): list`<br>`max_bodies(int): 32` | `int` | 返回写入数量，参数非法返回 `0` |

### 13.2 刚体接口（`body.h`）

| 名称 | 说明 | 参数（参数类型 + 参数示例） | 返回结果 | 结果说明 |
|---|---|---|---|---|
| `body_create` | 创建刚体 | `x(float): 0.0f`<br>`y(float): 5.0f`<br>`mass(float): 1.0f`<br>`shape(Shape*): shape_create_circle(0.5f)` | `RigidBody*` | `shape==NULL` 或分配失败返回 `NULL` |
| `body_free` | 释放刚体 | `body(RigidBody*): body` | `void` | `owns_shape=1` 时会释放 `shape` |
| `body_set_type` | 设置刚体类型 | `body(RigidBody*): body`<br>`type(BodyType): BODY_STATIC` | `void` | 空指针忽略 |
| `body_set_gravity` | 设置自定义重力 | `body(RigidBody*): body`<br>`g(Vec2): vec2(0,15)` | `void` | 同时启用 `use_custom_gravity` |
| `body_set_friction` | 设置摩擦系数 | `body(RigidBody*): body`<br>`friction(float): 0.4f` | `void` | 空指针忽略 |
| `body_set_restitution` | 设置弹性系数 | `body(RigidBody*): body`<br>`restitution(float): 0.6f` | `void` | 空指针忽略 |
| `body_set_shape_ownership` | 设置 shape 所有权 | `body(RigidBody*): body`<br>`owns_shape(int): 0` | `void` | 值规范化为 `0/1` |
| `body_apply_force` | 施加连续力 | `body(RigidBody*): body`<br>`force(Vec2): vec2(10,0)` | `void` | 力会累加 |
| `body_apply_force_at` | 在世界点施加力 | `body(RigidBody*): body`<br>`force(Vec2): vec2(10,0)`<br>`point(Vec2): vec2(1,1)` | `void` | 同时产生力矩 |
| `body_apply_impulse` | 施加线性冲量 | `body(RigidBody*): body`<br>`impulse(Vec2): vec2(3,0)` | `void` | 立即改变速度 |
| `body_apply_angular_impulse` | 施加角冲量 | `body(RigidBody*): body`<br>`impulse(float): 0.2f` | `void` | 立即改变角速度 |
| `body_apply_torque` | 施加力矩 | `body(RigidBody*): body`<br>`torque(float): 1.5f` | `void` | 力矩会累加 |
| `body_get_velocity_at` | 取刚体某点速度 | `body(RigidBody*): body`<br>`point(Vec2): vec2(2,2)` | `Vec2` | 空指针返回零向量 |
| `body_get_world_point` | 局部点转世界点 | `body(RigidBody*): body`<br>`local_point(Vec2): vec2(0.5f,0)` | `Vec2` | 空指针返回输入点 |
| `body_get_local_point` | 世界点转局部点 | `body(RigidBody*): body`<br>`world_point(Vec2): vec2(3,4)` | `Vec2` | 空指针返回输入点 |

### 13.3 形状接口（`shape.h`）

| 名称 | 说明 | 参数（参数类型 + 参数示例） | 返回结果 | 结果说明 |
|---|---|---|---|---|
| `shape_create_circle` | 创建圆形 | `radius(float): 0.5f` | `Shape*` | 分配失败返回 `NULL` |
| `shape_create_box` | 创建矩形（多边形） | `width(float): 2.0f`<br>`height(float): 1.0f` | `Shape*` | 分配失败返回 `NULL` |
| `shape_free` | 释放形状 | `shape(Shape*): shape` | `void` | `shape==NULL` 安全 |
| `shape_get_area` | 计算面积 | `shape(Shape*): shape` | `float` | 非法输入返回 `0` |
| `shape_get_moment_of_inertia` | 计算惯性矩 | `shape(Shape*): shape`<br>`mass(float): 1.0f` | `float` | 非法输入返回 `0` |
| `shape_get_center_of_mass` | 计算质心 | `shape(Shape*): shape` | `Vec2` | 非法输入返回 `vec2(0,0)` |

### 13.4 碰撞接口（`collision.h`）

| 名称 | 说明 | 参数（参数类型 + 参数示例） | 返回结果 | 结果说明 |
|---|---|---|---|---|
| `collision_circle_circle` | 圆-圆检测 | `a(RigidBody*): a`<br>`b(RigidBody*): b`<br>`info(CollisionInfo*): &info` | `int` | `1` 碰撞，`0` 不碰撞 |
| `collision_circle_polygon` | 圆-多边形检测 | `a(RigidBody*): circleBody`<br>`b(RigidBody*): polyBody`<br>`info(CollisionInfo*): &info` | `int` | `1/0` 表示命中与否 |
| `collision_polygon_polygon` | 多边形-多边形检测 | `a(RigidBody*): a`<br>`b(RigidBody*): b`<br>`info(CollisionInfo*): &info` | `int` | `1/0` 表示命中与否 |
| `collision_detect` | 通用检测入口 | `a(RigidBody*): a`<br>`b(RigidBody*): b`<br>`info(CollisionInfo*): &info` | `int` | 输入无效时返回 `0` |
| `sat_test_circle` | SAT 圆形相关测试 | `a(RigidBody*): a`<br>`b(RigidBody*): b`<br>`info(CollisionInfo*): &info` | `int` | `1/0` |
| `sat_test_polygon` | SAT 多边形测试 | `a(RigidBody*): a`<br>`b(RigidBody*): b`<br>`info(CollisionInfo*): &info` | `int` | `1/0` |
| `collision_resolve` | 碰撞响应总入口 | `manifold(CollisionManifold*): &m` | `void` | 当前实现主要处理速度响应 |
| `collision_resolve_velocity` | 速度层碰撞求解 | `manifold(CollisionManifold*): &m` | `void` | 直接更新速度/角速度 |
| `collision_resolve_position` | 位置层穿透修正 | `manifold(CollisionManifold*): &m` | `void` | 修正位置重叠 |

### 13.5 约束接口（`constraint.h`）

| 名称 | 说明 | 参数（参数类型 + 参数示例） | 返回结果 | 结果说明 |
|---|---|---|---|---|
| `constraint_init_distance` | 初始化距离约束 | `c(Constraint*): &c`<br>`a(RigidBody*): a`<br>`b(RigidBody*): b`<br>`world_anchor_a(Vec2): vec2(0,0)`<br>`world_anchor_b(Vec2): vec2(1,0)`<br>`stiffness(float): 0.5f`<br>`collide_connected(int): 1` | `void` | 空指针输入会忽略 |
| `constraint_init_spring` | 初始化弹簧约束 | `c(Constraint*): &c`<br>`a(RigidBody*): a`<br>`b(RigidBody*): b`<br>`world_anchor_a(Vec2): vec2(0,0)`<br>`world_anchor_b(Vec2): vec2(1,0)`<br>`rest_length(float): 2.0f`<br>`stiffness(float): 30.0f`<br>`damping(float): 2.0f`<br>`collide_connected(int): 0` | `void` | 空指针输入会忽略 |
| `constraint_warm_start` | 使用上帧冲量预热 | `c(Constraint*): &c` | `void` | 非激活或无效约束会忽略 |
| `constraint_solve_velocity` | 速度层约束求解 | `c(Constraint*): &c`<br>`dt(float): 1.0f/60.0f` | `void` | 可更新 `last_force/active` |
| `constraint_solve_position` | 位置层约束修正 | `c(Constraint*): &c` | `void` | 主要用于抑制漂移 |

### 13.6 数学接口（`physics_math.h`）

| 名称 | 说明 | 参数（参数类型 + 参数示例） | 返回结果 | 结果说明 |
|---|---|---|---|---|
| `vec2` | 构造二维向量 | `x(float): 1.0f`<br>`y(float): 2.0f` | `Vec2` | 返回 `{x,y}` |
| `vec2_add` | 向量加法 | `a(Vec2): vec2(1,2)`<br>`b(Vec2): vec2(3,4)` | `Vec2` | 返回 `a+b` |
| `vec2_sub` | 向量减法 | `a(Vec2): vec2(3,4)`<br>`b(Vec2): vec2(1,2)` | `Vec2` | 返回 `a-b` |
| `vec2_scale` | 向量数乘 | `v(Vec2): vec2(1,2)`<br>`s(float): 2.0f` | `Vec2` | 返回 `v*s` |
| `vec2_negate` | 向量取反 | `v(Vec2): vec2(1,-2)` | `Vec2` | 返回 `-v` |
| `vec2_dot` | 点积 | `a(Vec2): vec2(1,0)`<br>`b(Vec2): vec2(0,1)` | `float` | 返回标量点积 |
| `vec2_cross` | 2D 叉积标量 | `a(Vec2): vec2(1,0)`<br>`b(Vec2): vec2(0,1)` | `float` | 返回标量叉积 |
| `vec2_length_sq` | 长度平方 | `v(Vec2): vec2(3,4)` | `float` | 返回 `25` |
| `vec2_length` | 向量长度 | `v(Vec2): vec2(3,4)` | `float` | 返回 `5` |
| `vec2_normalize` | 单位化向量 | `v(Vec2): vec2(3,4)` | `Vec2` | 返回单位向量 |
| `vec2_rotate` | 旋转向量 | `v(Vec2): vec2(1,0)`<br>`angle(float): PI*0.5f` | `Vec2` | 按弧度旋转 |
| `mat2` | 构造 2x2 矩阵 | `a(float):1`<br>`b(float):0`<br>`c(float):0`<br>`d(float):1` | `Mat2` | 返回矩阵对象 |
| `mat2_identity` | 单位矩阵 | 无 | `Mat2` | 返回 `I` |
| `mat2_rotation` | 旋转矩阵 | `angle(float): PI/4` | `Mat2` | 返回旋转矩阵 |
| `mat2_mul_vec2` | 矩阵乘向量 | `m(Mat2): mat2_identity()`<br>`v(Vec2): vec2(1,2)` | `Vec2` | 返回 `m*v` |
| `mat2_mul` | 矩阵乘矩阵 | `a(Mat2): ma`<br>`b(Mat2): mb` | `Mat2` | 返回 `a*b` |
| `mat2_transpose` | 矩阵转置 | `m(Mat2): m` | `Mat2` | 返回 `m^T` |
| `mat2_invert` | 矩阵求逆 | `m(Mat2): m` | `Mat2` | 不可逆时依实现处理 |
| `clamp` | 数值钳制 | `value(float):1.2f`<br>`min(float):0.0f`<br>`max(float):1.0f` | `float` | 返回区间内值 |
| `min_f` | 浮点最小值 | `a(float):1.0f`<br>`b(float):2.0f` | `float` | 返回较小值 |
| `max_f` | 浮点最大值 | `a(float):1.0f`<br>`b(float):2.0f` | `float` | 返回较大值 |
| `is_equal` | 浮点近似比较 | `a(float):0.1f+0.2f`<br>`b(float):0.3f`<br>`epsilon(float):1e-6f` | `int` | 相等返回非 0，否则 0 |
