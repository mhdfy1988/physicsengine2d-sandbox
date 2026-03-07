# 前台模块化、组件化与样式统一化重构方案

## 1. 目标

本方案面向 `apps/sandbox_dwrite` 前台代码，统一解决三类问题：

1. 模块化不足：前台功能仍然大量堆积在 `main.cpp`，业务编排、渲染、输入、状态拼装边界不清。
2. 组件化不足：当前复用更多停留在基础绘制函数层，缺少稳定的共享 UI 组件层。
3. 样式统一化不足：颜色、尺寸、间距、状态反馈分散，后续演进容易继续失控。

本次重构的目标不是单纯把大文件拆小，而是建立一套可长期扩展的前台结构：

1. `main.cpp` 退化为启动入口和依赖装配层。
2. 前台按功能场景拆模块，而不是只按 `render/input/status` 这类技术类型拆文件。
3. 建立共享组件层，沉淀面板、按钮、标签页、树节点、弹窗等可复用能力。
4. 建立统一设计系统，收口视觉 token 和组件样式规则。
5. 新功能默认落在前台模块、共享组件或应用编排层，不再回流到入口文件。

---

## 2. 现状判断

当前仓库已经具备一定分层基础：

- `presentation/window`
- `presentation/input`
- `presentation/render`
- `presentation/status`
- `application/*`

但现状仍然属于“技术分层抽壳”，还不是真正的前台模块化。主要问题如下：

1. `apps/sandbox_dwrite/main.cpp` 仍然承载大量前台核心逻辑，包括：
   - 整帧布局计算
   - 左侧层级树、右侧 inspector、底部面板、状态栏、弹窗渲染
   - 大量输入事件解释和交互状态更新
   - 运行时状态与编辑器状态的拼装
2. `editor_screen_renderer` 当前更像整帧回调调度器，而不是独立屏幕模块。
3. `input_router` 当前更像 Win32 消息分发适配器，而不是 UI 交互系统。
4. `status_presenter` 只抽出了一部分文案格式化逻辑，状态展示仍未完整模块化。
5. `ui_primitives / ui_text / ui_widgets / ui_icons` 提供了基础绘制能力，但缺少稳定的组件抽象。
6. 样式值容易分散在各个模块实现中，导致后续视觉一致性难以维护。

结论：

前台下一轮重构不应继续只做“搬函数出 `main.cpp`”，而应同时推进：

1. 前台功能模块化
2. 共享组件化
3. 设计 token 与样式统一化

---

## 3. 重构原则

### 3.1 总体原则

1. 先建立清晰边界，再迁移实现。
2. 优先抽编排层和展示层，不先重写底层图元绘制。
3. 每一步都保持可编译、可运行、可回归。
4. 不为了“纯架构”引入过重抽象，所有抽象都必须服务于当前前台演进。

### 3.2 边界原则

1. `presentation` 只负责界面表达和交互意图产生，不直接承载业务流程。
2. `application` 负责状态编排、服务协作、跨模块联动。
3. `domain` 不出现 Win32/D2D 相关类型。
4. `infrastructure` 只提供平台与技术适配，不决定前台业务行为。
5. 样式规则不散落在功能模块内部，而统一收口到设计系统与组件样式层。

### 3.3 演进原则

1. 允许短期兼容旧接口，但新能力不得继续堆入 `main.cpp`。
2. 优先把“重复结构”和“重复视觉规则”抽成共享组件。
3. 先稳定壳层和组件层，再处理最复杂的会话编排逻辑。

---

## 4. 目标结构

建议将 `apps/sandbox_dwrite` 前台结构演进为：

```text
apps/sandbox_dwrite/
|-- application/
|   `-- workbench/
|       |-- workbench_service.hpp
|       |-- workbench_service.cpp
|       |-- workbench_view_state.hpp
|       |-- ui_intent.hpp
|       `-- ui_action_result.hpp
|-- presentation/
|   |-- design_system/
|   |   |-- ui_tokens.hpp
|   |   |-- ui_theme.hpp
|   |   |-- ui_theme.cpp
|   |   `-- ui_component_style.hpp
|   |-- components/
|   |   |-- panel/
|   |   |-- button/
|   |   |-- tabs/
|   |   |-- status_chip/
|   |   |-- tree_item/
|   |   |-- modal/
|   |   |-- input_field/
|   |   |-- toolbar/
|   |   `-- scrollbar/
|   |-- shell/
|   |-- topbar/
|   |-- stage/
|   |-- overlay/
|   |-- modal/
|   |-- input/
|   `-- panels/
|       |-- hierarchy/
|       |-- inspector/
|       `-- console/
`-- infrastructure/
    |-- win32/
    `-- d2d/
```

说明：

1. `design_system` 负责样式统一化。
2. `components` 负责可复用 UI 组件。
3. `shell/topbar/stage/overlay/modal/panels/*` 负责按前台功能区域模块化。
4. `application/workbench` 负责前台状态编排和 UI Intent 消费。

---

## 5. 分层职责

### 5.1 application/workbench

职责：

1. 聚合 runtime、history、scene、session、catalog、hot reload 等数据源。
2. 生成各前台模块所需的只读 ViewModel。
3. 接收 `UiIntent` 并调用已有 application service 或 domain command。
4. 处理跨面板联动、跨服务联动和工作流编排。

不负责：

1. 直接绘制 UI。
2. 直接处理 Win32 消息。
3. 决定具体样式细节。

建议核心对象：

1. `WorkbenchService`
2. `WorkbenchViewState`
3. `UiIntent`
4. `UiActionResult`

### 5.2 presentation/design_system

职责：

1. 定义颜色、字号、间距、圆角、控件高度、边框、层级等 token。
2. 统一浅色/深色主题或后续主题扩展能力。
3. 为共享组件输出统一样式描述。

不负责：

1. 直接承载业务数据。
2. 直接感知具体面板业务。

### 5.3 presentation/components

职责：

1. 把高频 UI 结构抽成稳定组件。
2. 依赖 design system 输出视觉一致的渲染和交互。
3. 为上层功能模块提供标准 API。

不负责：

1. 直接操作应用业务状态。
2. 处理跨面板业务联动。

### 5.4 presentation/feature modules

职责：

1. 消费 ViewModel。
2. 组合共享组件与基础图元绘制。
3. 完成模块级布局、渲染、命中测试与交互意图生成。

不负责：

1. 持有全局工作流状态。
2. 直接调用复杂业务编排流程。

### 5.5 infrastructure

职责：

1. Win32、D2D、DWrite、WIC 等技术能力适配。
2. 窗口、消息循环、资源创建、平台工具函数封装。

不负责：

1. 前台模块业务。
2. 组件语义。

---

## 6. 前台模块化方案

### 6.1 模块拆分原则

前台模块应按“功能区域 + 交互职责”拆分，而不是继续按 `render/input/status` 细分。

每个前台模块建议具备统一结构：

```text
<module>/
|-- <module>_view_model.hpp
|-- <module>_renderer.hpp
|-- <module>_renderer.cpp
|-- <module>_interaction.hpp
|-- <module>_interaction.cpp
`-- <module>_intent.hpp
```

说明：

1. `ViewModel`：只读数据。
2. `Renderer`：纯渲染与模块内部布局。
3. `Interaction`：命中测试、滚动、点击、拖拽解释。
4. `Intent`：模块对外发出的高层交互意图。

### 6.2 建议模块清单

#### 6.2.1 shell

职责：

1. 整帧布局计算。
2. 顶部、左右、底部、状态栏区域分发。
3. 整体 begin/end render 调度。

要求：

1. 不承载具体面板业务。
2. 不直接格式化状态文案。

#### 6.2.2 topbar

职责：

1. 菜单栏渲染。
2. 工具栏渲染。
3. 窗口控制区组合。

输出：

1. 菜单点击意图。
2. 工具按钮意图。

#### 6.2.3 panels/hierarchy

职责：

1. 左侧层级树、项目浏览、展开折叠。
2. 滚动条展示与命中测试。
3. 节点选中与节点交互意图。

#### 6.2.4 panels/inspector

职责：

1. 右侧属性检查与编辑入口。
2. 属性分组渲染。
3. 对数值输入、切换开关、操作按钮发出修改意图。

#### 6.2.5 panels/console

职责：

1. 底部 tab 切换。
2. 日志、性能、运行时调试信息展示。
3. 滚动与筛选交互。

#### 6.2.6 stage

职责：

1. 中央画布渲染。
2. 选中态、拖拽、缩放、平移、可视化辅助元素。
3. 舞台相关命中测试与输入解释。

#### 6.2.7 overlay

职责：

1. 状态栏展示。
2. hot reload、runtime error、bus 健康度、启动状态提示。
3. 浮层或轻量提示信息。

#### 6.2.8 modal

职责：

1. help/config/value input 等弹窗统一收口。
2. 统一弹窗标题、内容区、操作区和遮罩行为。

#### 6.2.9 input

职责：

1. Win32 消息到 UI 事件的统一适配。
2. 快捷键解析。
3. 鼠标事件路由。
4. 最终统一产出 `UiIntent`。

要求：

1. 不直接修改复杂应用状态。
2. 不再让多个区域各自藏消息分支。

---

## 7. 组件化方案

### 7.1 组件层次

建议将组件划分为三层：

#### 7.1.1 基础绘制层

保留和延续现有能力：

1. `ui_primitives`
2. `ui_text`
3. `ui_icons`

职责：

1. 绘制矩形、圆角矩形、边框、文本、图标。
2. 提供最底层图形能力，不感知组件语义。

#### 7.1.2 通用组件层

新增真正可复用的共享组件：

1. `Panel`
2. `Button`
3. `IconButton`
4. `Tabs`
5. `ToolbarItem`
6. `StatusChip`
7. `TreeItem`
8. `InputField`
9. `Modal`
10. `ScrollBar`

每个组件建议具备：

1. `Props`
2. `Style`
3. `State`
4. `Renderer`
5. `HitTest` 或 `Interaction`

#### 7.1.3 场景组合组件层

为单个功能模块服务的半业务组件，例如：

1. `InspectorSection`
2. `PropertyEditorRow`
3. `HierarchyNodeRow`
4. `ConsoleLogRow`
5. `StatusBarItem`

说明：

1. 这类组件不一定全局复用，但应服从统一样式规则和交互模型。

### 7.2 优先抽取的共享组件

建议优先处理以下高频复用组件：

1. `Panel`
2. `Button`
3. `Tabs`
4. `StatusChip`
5. `TreeItem`
6. `Modal`
7. `ScrollBar`

原因：

1. 这些组件横跨多个前台区域。
2. 它们对视觉统一和代码复用收益最高。
3. 它们最容易在当前实现中出现重复逻辑和重复样式。

### 7.3 组件 API 约束

共享组件必须满足：

1. 业务模块只选择变体，不直接写死颜色和尺寸。
2. 组件状态统一采用标准状态枚举，如 `normal / hover / active / selected / disabled`。
3. 组件高度、边距、圆角优先从 design system 读取。
4. 组件不得直接依赖 application service 或 domain object。

---

## 8. 样式统一化方案

### 8.1 目标

样式统一化的目标不是简单提取几个常量，而是建立一套长期可维护的设计系统约束。

必须统一的内容包括：

1. 颜色体系
2. 字号体系
3. 间距体系
4. 圆角体系
5. 控件高度体系
6. 状态反馈体系
7. 面板层级和视觉节奏

### 8.2 设计 token

建议建立如下 token 分类：

#### 8.2.1 色彩 token

例如：

1. `color_bg_canvas`
2. `color_bg_panel`
3. `color_bg_panel_alt`
4. `color_border_muted`
5. `color_border_focus`
6. `color_text_primary`
7. `color_text_secondary`
8. `color_text_disabled`
9. `color_status_success`
10. `color_status_warning`
11. `color_status_error`
12. `color_status_info`

#### 8.2.2 间距 token

例如：

1. `space_4`
2. `space_8`
3. `space_12`
4. `space_16`
5. `space_20`
6. `space_24`

#### 8.2.3 尺寸 token

例如：

1. `control_h_sm`
2. `control_h_md`
3. `control_h_lg`
4. `toolbar_h`
5. `status_bar_h`
6. `panel_header_h`
7. `tree_row_h`

#### 8.2.4 圆角 token

例如：

1. `radius_sm`
2. `radius_md`
3. `radius_lg`

#### 8.2.5 字体 token

例如：

1. `font_title`
2. `font_body`
3. `font_caption`
4. `font_mono`

### 8.3 组件样式模型

除 token 外，还应建立组件级样式结构，例如：

1. `PanelStyle`
2. `ButtonStyle`
3. `TabStyle`
4. `TreeItemStyle`
5. `StatusChipStyle`
6. `ModalStyle`

作用：

1. 将 token 组合成具体组件样式。
2. 让上层模块按 variant 使用，而不是重复拼颜色和间距。

### 8.4 统一交互状态视觉

所有组件都应统一处理以下状态：

1. `normal`
2. `hover`
3. `active`
4. `selected`
5. `focused`
6. `disabled`

必须保证：

1. 同类状态在不同组件上的视觉反馈强度一致。
2. 选中态、焦点态、危险态有统一规则。
3. 不同模块不得私自定义新的局部交互颜色体系。

### 8.5 布局节奏统一

样式统一化还应覆盖布局节奏：

1. 工具栏高度统一
2. 状态栏高度统一
3. 面板标题区高度统一
4. 按钮和输入框高度统一
5. 树节点和列表行高统一
6. 面板内边距和模块间距统一

---

## 9. UI 事件与状态流

### 9.1 核心思想

前台应逐步收口为以下流向：

```text
Win32 Message
-> Input Adapter
-> UiIntent
-> WorkbenchService
-> WorkbenchViewState
-> Feature Module Renderer
-> Shared Components
```

### 9.2 规则

1. Win32 消息只在输入适配层出现。
2. 前台模块不直接处理平台消息，而消费高层意图。
3. 前台模块不直接修改复杂业务状态，而通过 `UiIntent` 请求应用层处理。
4. 应用层更新状态后，重新生成 ViewState 供界面消费。

### 9.3 收益

1. 交互入口统一。
2. 键盘、鼠标、菜单、快捷键可以汇总到同一套处理链。
3. 前台模块更容易测试。
4. 输入逻辑不会继续散落在 `wnd_proc` 或不同渲染函数附近。

---

## 10. 迁移顺序

建议按风险从低到高推进。

### Phase 1：建立设计系统基础

目标：

1. 引入 `design_system`
2. 抽取 token 和基础组件样式结构
3. 禁止新增写死颜色、尺寸、间距

输出：

1. `UiTokens`
2. `UiTheme`
3. `UiComponentStyle`

### Phase 2：抽取共享组件

目标：

1. 基于 design system 抽取共享组件
2. 用共享组件替换当前重复 UI 片段

优先组件：

1. `Panel`
2. `Button`
3. `Tabs`
4. `StatusChip`
5. `TreeItem`
6. `Modal`
7. `ScrollBar`

### Phase 3：提取前台 shell

目标：

1. 提取整帧布局和渲染调度
2. 让 `main.cpp` 不再承载整帧 UI 组织

迁移内容：

1. 区域布局计算
2. begin/end render
3. 各区域调度入口

### Phase 4：按面板拆功能模块

建议顺序：

1. `panels/hierarchy`
2. `panels/inspector`
3. `panels/console`
4. `overlay`
5. `modal`
6. `stage`

原因：

1. hierarchy/inspector/console 结构相对明确，收益高。
2. stage 交互复杂，放后面风险更低。

### Phase 5：统一输入意图系统

目标：

1. 将键盘、鼠标、菜单、快捷键统一收口到 `UiIntent`
2. 让输入系统从消息分发层升级为交互适配层

### Phase 6：收口 workbench 编排

目标：

1. 将 autosave、session、history、runtime、hot reload 等跨模块联动归入 application/workbench
2. 界面模块不再直接操纵复杂流程

---

## 11. 建议提交拆分

建议按下列提交顺序推进：

1. `Introduce frontend design system tokens and theme`
2. `Extract shared UI components for sandbox frontend`
3. `Extract editor shell layout and render orchestration`
4. `Extract hierarchy panel module`
5. `Extract inspector panel module`
6. `Extract console and overlay modules`
7. `Unify frontend input intents and routing`
8. `Extract workbench orchestration from sandbox main`

---

## 12. 验收标准

本重构至少应满足以下验收标准：

1. `main.cpp` 行数明显下降，并且不再包含大段面板实现。
2. 前台主要区域都具备独立模块边界。
3. 共享组件已替换高频重复 UI 结构。
4. 样式值已大幅收口到 design system 和组件样式层。
5. 输入系统可以统一生成 `UiIntent`。
6. 前台模块主要通过 ViewModel 驱动，而不是直接依赖散落全局状态。
7. 编译、启动、关键交互和相关 smoke/regression 不回归。

建议量化目标：

1. `main.cpp` 收缩到 1500 行以内。
2. 至少完成 `Panel / Button / Tabs / TreeItem / Modal / StatusChip` 六类共享组件。
3. 至少完成 `hierarchy / inspector / console / overlay` 四个独立前台模块。

---

## 13. 当前建议

如果立刻启动本轮前台重构，建议从以下顺序开始：

1. 先建 `design_system`
2. 再抽共享组件
3. 然后提取 `shell`
4. 再拆 `hierarchy / inspector / console`
5. 最后统一输入意图与 workbench 编排

原因：

1. 如果先拆复杂流程而没有统一组件和样式基础，后面还会返工。
2. 如果没有 shell 和模块边界，`main.cpp` 只会从“大文件”变成“多个薄文件 + 继续耦合”。
3. 先统一样式和组件，后续模块化迁移可以同步受益。

一句话总结：

这轮前台重构不应只做“拆文件”，而应把前台正式演进为“工作台编排层 + 功能模块层 + 共享组件层 + 设计系统层”的结构。
