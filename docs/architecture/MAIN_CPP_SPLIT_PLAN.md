# main.cpp 拆分方案

## 目标

在不推翻当前仓库分层的前提下，继续缩小 `apps/sandbox_dwrite/main.cpp` 的职责范围。

当前已经完成的前置整理：

- `application/editor_extension_state_service` 已经承接 editor startup state
- `application/editor_extension_host_service` 已经承接 startup wiring、builtin plugin 装配、默认路径与诊断文件编排
- `presentation/render/*`、`presentation/input/*`、`application/*` 已经有基础分层

## 当前状态

- 已完成：`Window Host`
  - `presentation/window/window_host.{hpp,cpp}`
- 已完成：`Input Router`
  - `presentation/input/input_router.{hpp,cpp}`
- 已完成：`Status Presenter`
  - `presentation/status/status_presenter.{hpp,cpp}`
- 已完成：`Editor Screen Renderer`
  - `presentation/render/editor_screen_renderer.{hpp,cpp}`
- 已完成：`Editor Session Service`
  - `application/editor_session_service.{hpp,cpp}`

说明：

- 这一轮完成的是“编排层/壳层外提”
- 低层的具体绘制函数、具体交互处理函数、具体场景操作函数仍有一部分保留在 `main.cpp`
- 但文档中规划的五个拆分方向都已经有了对应落地模块，并且 `main.cpp` 已开始通过这些模块完成调用

下一步不应该新造架构，而应该继续沿用现有边界：

- `domain/`: 纯命令、纯业务概念
- `application/`: 编排、状态协调、工作流
- `presentation/`: 窗口、输入、渲染、展示
- `infrastructure/`: repo、平台接缝、持久化

## 现状问题

`main.cpp` 仍然偏大，主要还混着这些职责：

- Win32 平台启动与窗口壳层
- `wnd_proc` 消息分发
- 输入事件解释与快捷键处理
- UI 各面板与状态栏的整帧渲染编排
- 编辑器会话级状态协调
- autosave / session recovery / diagnostic export 触发逻辑
- 部分运行时状态呈现逻辑

这会导致：

- 入口文件过长，阅读成本高
- 平台壳层和业务流程耦合
- 输入、渲染、工作流逻辑边界不清
- 后续继续加功能时很容易回流到 `main.cpp`

## 拆分原则

1. 不在这一轮重写全局状态模型。
2. 不把 UI 逻辑硬塞进 `application/`。
3. 不把业务流程硬塞进 `presentation/`。
4. 优先提取“编排”和“壳层”，避免先碰低层算法。
5. 每一步都保持可编译、可运行、可验证。

## 目标分解

### 1. Window Host

建议新增：

- `apps/sandbox_dwrite/presentation/window/window_host.hpp`
- `apps/sandbox_dwrite/presentation/window/window_host.cpp`

职责：

- Win32 窗口类注册
- 主窗口创建
- 消息循环
- DPI / COM / timer / icon 等平台启动壳层
- `wnd_proc` 外层入口

适合迁出的内容：

- `register_main_window_class`
- `create_main_window`
- `run_message_loop`
- `init_platform_ui`
- 与 `WNDCLASSEXW`、`CreateWindowExW`、`SetTimer` 直接相关的壳层逻辑

保留在 `main.cpp` 的内容：

- 应用级入口
- 需要直接接应用全局状态的初始化顺序控制

预期收益：

- `main.cpp` 先瘦一截
- 平台壳层与编辑器业务分离

### 2. Input Router

建议新增：

- `apps/sandbox_dwrite/presentation/input/input_router.hpp`
- `apps/sandbox_dwrite/presentation/input/input_router.cpp`

职责：

- 键盘快捷键解释
- 鼠标点击、拖拽、命中后的动作路由
- 菜单操作到应用命令的映射
- `WM_*` 到高层交互意图的转换

适合迁出的内容：

- 快捷键表驱动逻辑
- 鼠标按下/抬起/移动的交互路由
- 菜单命令与 `AppCommand` 的映射

保留在 `main.cpp` 的内容：

- `wnd_proc` 调用入口
- 必须直接接 Win32 API 的极薄壳

预期收益：

- 输入逻辑不再散在消息处理中
- 快捷键和交互映射更容易维护

### 3. Status Presenter

建议新增：

- `apps/sandbox_dwrite/presentation/status/status_presenter.hpp`
- `apps/sandbox_dwrite/presentation/status/status_presenter.cpp`

职责：

- 启动状态文本呈现
- runtime error 状态文本呈现
- hot reload 状态文本呈现
- 诊断和状态栏文案格式化

适合迁出的内容：

- 各种状态文本拼装
- UI 状态栏与信息条展示格式逻辑

预期收益：

- 文案逻辑不再和入口文件混在一起
- 状态条修改更集中

### 4. Editor Screen Renderer

建议新增：

- `apps/sandbox_dwrite/presentation/render/editor_screen_renderer.hpp`
- `apps/sandbox_dwrite/presentation/render/editor_screen_renderer.cpp`

职责：

- 整帧 UI 编排
- 左侧树、右侧 inspector、底部面板、状态栏、overlay 的组合绘制
- 将 `ui_primitives / ui_text / ui_widgets` 组装成完整屏幕

适合迁出的内容：

- 每个面板的绘制编排
- 状态区、列表区、树区、调试区的组合渲染

保留在 `main.cpp` 的内容：

- 极少量每帧入口和调用顺序

预期收益：

- 渲染编排真正回到 `presentation/render`
- `main.cpp` 不再承担整帧 UI 组织

### 5. Editor Session Service

建议新增：

- `apps/sandbox_dwrite/application/editor_session_service.hpp`
- `apps/sandbox_dwrite/application/editor_session_service.cpp`

职责：

- 场景切换后的高层状态联动
- 选择集变化后的 inspector / history / runtime 联动
- autosave / session recovery 触发编排
- PIE、history、snapshot 等编辑器会话级流程协调

适合迁出的内容：

- 会话级 workflow
- 跨多个 application service 的编排逻辑

不建议在第一步就拆的原因：

- 涉及状态面最广
- 风险比 window/input/render 拆分大

预期收益：

- 编辑器行为的编排层从 UI 壳子中独立出来
- 后续新功能不会持续堆回 `main.cpp`

## 推荐执行顺序

建议按风险从低到高推进：

1. `Window Host`
2. `Input Router`
3. `Status Presenter`
4. `Editor Screen Renderer`
5. `Editor Session Service`

原因：

- `Window Host` 最偏平台壳层，拆出来收益高、风险低
- `Input Router` 和 `Status Presenter` 属于表现层整理，容易控风险
- `Editor Screen Renderer` 改动面更大，但仍属于 presentation
- `Editor Session Service` 最后做，因为它会碰最多应用状态

## 每一步的验收标准

每一阶段至少要验证：

- `physics_sandbox` 编译通过
- `physics_tests` 通过
- `physics_editor_extension_state_service_smoke` 通过
- `scripts/package/package.ps1` 通过

如果拆到了输入或渲染：

- sandbox 可以正常启动
- 关键交互没有明显回归

## 推荐提交拆分

1. `Extract window host shell from sandbox main`
2. `Extract sandbox input routing from main`
3. `Extract status presentation formatting`
4. `Extract editor screen rendering composition`
5. `Extract editor session workflow orchestration`

## 当前建议

如果继续推进，下一步最合适的是：

### Step 1

优先做 `Window Host`。

原因：

- 它最符合“平台壳层回收”的目标
- 对现有 `application` 与 `presentation` 边界最自然
- 能立刻让 `main.cpp` 变短
- 不需要先重构状态模型

一句话总结：

下一轮对 `main.cpp` 的继续拆分，应该先从 Win32 窗口壳层开始，而不是一上来碰最复杂的编辑器会话逻辑。
