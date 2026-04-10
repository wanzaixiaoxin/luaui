# LuaUI 现代化改进推进方案

## 现状盘点

在制定路线之前，先明确已有的基础设施：

| 模块 | 已有 | 缺失 |
|------|------|------|
| 动画 | `IAnimation` / `D2DAnimation` / `Timeline` / 30 种 Easing 全部实现 | ~~帧驱动 (`WM_TIMER`)~~ ✅ / ~~`Window` 未持有 `Timeline`~~ ✅ / ~~`AnimationValue` 无 `Color` 类型~~ ✅；无 `VisualStateManager` |
| 主题 | `Theme` / `Style` / `Setter` / `ResourceDictionary` 类存在 | 控件不读取 `Theme`；无 `IStyleable` 实现；无 `DependencyProperty`；无 Dark Mode |
| 渲染 | 完整的 D2D 渲染管线 + 脏矩形优化 | 被动式渲染（仅 `WM_PAINT` 驱动） |
| 输入 | 鼠标事件路由 + 焦点管理 + 鼠标捕获 | IME 缺失；无 DragDrop |

**关键洞察**：动画引擎已写好 80%，只差"最后一公里"——帧驱动；主题系统有壳无肉。因此推进策略是 **先激活动画引擎（ROI 最高）**，再逐步贯通主题，最后补全控件交互。

---

## 分阶段推进路线

### ✅ 阶段一：激活动画引擎（已完成 2026-04-08）

**目标**：让已有的 Animation 代码跑起来，Button 率先拥有过渡动画

- [x] **步骤 1.1** Window 集成 AnimationTimeline
- [x] **步骤 1.2** 帧驱动机制（WM_TIMER + QPC deltaTime + 自动启停）
- [x] **步骤 1.3** AnimationValue 扩展 Color 类型（r/g/b/a 四通道 Lerp）
- [x] **步骤 1.4** Button 率先使用动画（Hover 150ms / Pressed 100ms CubicOut）

改动文件：`IAnimation.h`, `D2DAnimation.h/cpp`, `Window.h/cpp`, `Button.h/cpp`

---

### ✅ 阶段二：P0 交互修复（已完成 2026-04-08）

**目标**：修复基本交互逻辑缺陷，不依赖动画引擎

- [x] **步骤 2.1** Button OnMouseUp 坐标校验（已在 Window::HandleMouseUp 中通过 HitTest 实现）
- [x] **步骤 2.2** CheckBox/RadioButton 禁用态（灰色边框/勾选色/文字色 + OnClick 拦截）
- [x] **步骤 2.3** CheckBox/RadioButton 文本宽度（`fontSize*0.6` 替代固定 `*7`）
- [x] **步骤 2.4** Slider 鼠标滚轮（`OnMouseWheel` + `SmallChange` 步进）
- [x] **步骤 2.5** Slider Hover 视觉（thumb 12→16 + OnMouseEnter/Leave）

改动文件：`CheckBox.h/cpp`, `Slider.h/cpp`

---

### ✅ 阶段三：主题系统贯通（已完成 2026-04-08）

**目标**：控件颜色从硬编码 → ThemeResource，支持 Dark Mode

- [x] **步骤 3.1** 扩展 ResourceDictionary（Color/float/Thickness/CornerRadius 支持）
- [x] **步骤 3.2** 创建 ThemeKeys.h + Light/Dark 主题工厂函数（CreateLightTheme/CreateDarkTheme）
- [x] **步骤 3.3** Control 基类添加 ApplyTheme() 虚函数 + 自动注册/注销 Theme 回调
- [x] **步骤 3.4** Button 改造为 Theme 驱动（示范）
- [x] **步骤 3.5** CheckBox/RadioButton 改造
- [x] **步骤 3.5** Slider 改造
- [x] **步骤 3.5** TextBox 改造
- [x] **步骤 3.5** ProgressBar 改造

改动文件：`Theme.h/cpp`, `ThemeKeys.h`, `ResourceDictionary.h/cpp`, `Control.h/cpp`, `Button.h/cpp`, `CheckBox.h/cpp`, `Slider.h/cpp`, `TextBox.h/cpp`, `ProgressBar.h/cpp`

**验收标准**：Theme::GetCurrent().ApplyResources(CreateDarkTheme()) 后，Button/CheckBox/Slider/TextBox/ProgressBar 颜色即时更新。

---

### 阶段四：动画扩展到所有控件（~3 天）

**目标**：所有交互控件拥有过渡动画

#### 步骤 4.1 VisualStateManager（简化版）

- 添加 `VisualState` 类（name + Setter 列表）
- 添加 `VisualStateManager` 到 `Control`
  - `GoToState("Normal")`, `GoToState("Hovered")`, `GoToState("Pressed")`, `GoToState("Disabled")`
- 状态切换自动创建过渡动画（使用当前值 → 目标值，duration=150ms）
- 控件只需声明状态，无需手写动画代码

#### 步骤 4.2 各控件接入 VSM

- `CheckBox`: Normal → Hovered 过渡
- `RadioButton`: 同上
- `TabItem`: 选中/未选中过渡
- `Slider`: Hover 放大/缩小过渡

#### 步骤 4.3 ProgressBar/ProgressRing 动画

- 不确定模式：Timeline 中添加循环动画
  - `ProgressBar`: block 滑动（Duration=2000ms, Iterations=-1）
  - `ProgressRing`: 弧形旋转（Duration=1500ms, Iterations=-1）
- `ProgressRing` 改用 `ID2D1PathGeometry` 画弧
- 自动启用 Timer

---

### 阶段五：TextBlock/TextBox 增强（~3 天）

**目标**：文本展示和编辑达到基本可用水平

#### 步骤 5.1 TextBlock 增强

- `TextAlignment` 属性（Left/Center/Right/Justify）
- `TextTrimming`（CharacterEllipsis / WordEllipsis）
- `TextWrapping`（Wrap / NoWrap）
- `FontFamily` 属性
- `MaxLines` 属性

#### 步骤 5.2 TextBox 多行模式

- `AcceptsReturn` 属性（允许换行）
- `TextWrapping` 支持
- 垂直滚动 + 多行光标导航
- 行号显示（可选）

#### 步骤 5.3 TextBox IME 集成（TSF）

- `WM_IME_STARTCOMPOSITION` / `WM_IME_COMPOSITION`
- 候选窗口定位到光标位置
- 组合文本临时显示
- 这是最大的单项工程，可能需要 2-3 天

---

### 阶段六：容器/复杂控件增强（~4 天）

#### 步骤 6.1 ScrollViewer 水平滚动 + 平滑滚动

- 水平滚动条交互（对称已有垂直逻辑）
- 平滑滚动（惯性动画）
- `ScrollChanged` 事件

#### 步骤 6.2 TabControl 溢出滚动 + 键盘切换

- 标签溢出时显示左右箭头
- 点击箭头滚动标签栏
- 选中标签自动滚入可见区

#### 步骤 6.3 DataGrid 基础增强

- 列宽拖拽调整
- 排序实现（升/降序切换）
- 虚拟化（仅渲染可见行）

#### 步骤 6.4 Image 异步加载

- 后台线程解码图片
- 加载完成后淡入动画
- 加载中占位符 + 失败占位符

---

## 依赖关系

```
阶段一 ──→ 阶段四（动画引擎是 VSM 的基础）
  │
  ↓
阶段二（独立，可与阶段三并行）
  │
阶段三 ──→ 阶段四（Theme 是 VSM 目标值的来源）
            │
            ↓
          阶段五（独立）
            │
            ↓
          阶段六（依赖阶段一）
```

## 工程量估算

| 阶段 | 天数 | 依赖 | 可独立并行 |
|------|------|------|------------|
| 一：激活动画引擎 | 3 | 无 | - |
| 二：P0 交互修复 | 2 | 无 | 可与阶段一并行 |
| 三：主题系统贯通 | 4 | 无 | 可与阶段一/二并行 |
| 四：动画扩展 | 3 | 阶段一 + 阶段三 | - |
| 五：文本增强 | 3 | 无 | 可与阶段四并行 |
| 六：复杂控件 | 4 | 阶段一 | 可与阶段五并行 |

**总工期**：串行 ~19 天，并行优化后 ~10-12 天。

## 建议执行顺序

| 时间 | 阶段 | 目标 |
|------|------|------|
| 第 1-3 天 | 阶段一 + 阶段二 并行 | 动画引擎跑通 + 基本交互修复 |
| 第 4-7 天 | 阶段三 | 主题系统贯通，Button 率先 Theme 化 |
| 第 8-10 天 | 阶段四 | 动画扩展 + VSM，所有控件过渡动画 |
| 第 11-13 天 | 阶段五 | TextBlock/TextBox 增强，文本能力达标 |
| 第 14-17 天 | 阶段六 | 复杂控件增强，整体完成 |

## 风险点

| 风险 | 影响 | 缓解 |
|------|------|------|
| IME/TSF 集成复杂度远超预期 | 阶段五延期 | 可先实现基础 IME，延后高级特性 |
| DependencyProperty 体系庞大 | 阶段三膨胀 | 坚决不用 DP，用 ThemeCallback 轻量替代 |
| 动画帧率与脏矩形冲突 | 阶段一动画卡顿 | 动画期间强制全屏重绘，禁用脏矩形优化 |
| 多控件同时动画性能 | CPU 占用高 | Timeline 自动合并同帧动画，Timer 16ms 限帧 |

---

**最关键的第一步**：阶段一的步骤 1.1 + 1.2（Window 集成 Timeline + WM_TIMER 帧驱动），这是所有动画的基础，改动量小（~50 行），但撬动全局。

---

*文档版本: 1.3*
*创建日期: 2026-04-08*
*最后更新: 2026-04-09 — 阶段三全部完成（含 TextBox/ProgressBar）*
