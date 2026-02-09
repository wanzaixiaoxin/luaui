# LuaUI 布局系统设计文档

## 概述

LuaUI 布局系统提供了一套灵活、强大的容器布局机制，支持多种布局模式，类似于 WPF/UWP 的布局系统。

## 架构设计

### 核心原则

1. **布局即面板**: 布局容器继承自 `Panel`，与普通控件统一体系
2. **Measure/Arrange 模式**: 两阶段布局计算
   - **Measure**: 计算期望大小
   - **Arrange**: 确定最终位置和大小
3. **绝对坐标**: 所有子元素使用绝对窗口坐标渲染

### 类层次结构

```
Control (控件基类)
    └── Panel (容器基类)
            ├── StackPanel      (堆叠布局)
            ├── Grid            (网格布局)
            ├── Canvas          (绝对定位)
            ├── DockPanel       (停靠布局)
            └── WrapPanel       (自动换行)
```

## 布局容器详解

### 1. StackPanel - 堆叠布局

```cpp
auto panel = std::make_shared<StackPanel>();
panel->SetOrientation(StackPanel::Orientation::Vertical);  // 或 Horizontal
panel->SetSpacing(10);  // 子元素间距
panel->SetMargin(20, 20, 20, 20);  // 边距
```

**特点:**
- 子元素沿单一方向排列
- 支持垂直或水平方向
- 可设置固定间距

### 2. Grid - 网格布局

```cpp
auto grid = std::make_shared<Grid>();

// 定义列 (Pixel, Auto, Star)
grid->AddColumn(GridLength::Pixel(200));    // 固定200px
grid->AddColumn(GridLength::Star(1));       // 比例填充
grid->AddColumn(GridLength::Auto());        // 自动大小

// 定义行
grid->AddRow(GridLength::Star(1));

// 放置子元素
auto child = std::make_shared<Control>();
Grid::SetRow(child.get(), 0);
Grid::SetColumn(child.get(), 1);
Grid::SetRowSpan(child.get(), 2);  // 跨行
grid->AddChild(child);
```

**特点:**
- 类似 HTML Table
- 支持 Star 比例分配
- 支持跨行跨列

### 3. Canvas - 绝对定位

```cpp
auto canvas = std::make_shared<Canvas>();

auto child = std::make_shared<Control>();
Canvas::SetLeft(child.get(), 100);   // X坐标
Canvas::SetTop(child.get(), 50);     // Y坐标
// 或 SetRight/SetBottom 实现右对齐/底对齐

canvas->AddChild(child);
```

**特点:**
- 精确像素定位
- 支持 Left/Top 或 Right/Bottom 定位
- 适合图形编辑器、游戏UI等场景

### 4. DockPanel - 停靠布局

```cpp
auto dock = std::make_shared<DockPanel>();
dock->SetLastChildFill(true);  // 最后一个子元素填充剩余空间

auto top = std::make_shared<Control>();
top->SetHeight(60);
DockPanel::SetDock(top.get(), Dock::Top);
dock->AddChild(top);

auto fill = std::make_shared<Control>();  // 自动填充中间
dock->AddChild(fill);
```

**特点:**
- 子元素停靠到边缘
- 最后子元素填充剩余空间
- 适合 IDE 布局（工具栏、状态栏、主区域）

### 5. WrapPanel - 自动换行

```cpp
auto wrap = std::make_shared<WrapPanel>();
wrap->SetOrientation(WrapPanel::Orientation::Horizontal);
wrap->SetItemWidth(120);  // 统一项目宽度

for (int i = 0; i < 10; i++) {
    wrap->AddChild(std::make_shared<Control>());
}
```

**特点:**
- 空间不足时自动换行
- 类似 CSS Flexbox wrap
- 适合工具栏、图标网格

## 使用示例

### 复合布局示例

```cpp
// 根布局：垂直 StackPanel
auto root = std::make_shared<StackPanel>();
root->SetOrientation(StackPanel::Orientation::Vertical);

// 标题栏（Border）
auto header = std::make_shared<Border>();
header->SetHeight(60);
header->SetBackground(Color::FromHex(0x2196F3));
root->AddChild(header);

// 主区域：Grid 左右分栏
auto mainGrid = std::make_shared<Grid>();
mainGrid->AddColumn(GridLength::Pixel(250));  // 左侧固定
mainGrid->AddColumn(GridLength::Star(1));     // 右侧填充
mainGrid->AddRow(GridLength::Star(1));

// 左侧：垂直 StackPanel
auto leftPanel = std::make_shared<StackPanel>();
Grid::SetColumn(leftPanel.get(), 0);
Grid::SetRow(leftPanel.get(), 0);
mainGrid->AddChild(leftPanel);

// 右侧：Canvas 自由绘图
auto canvas = std::make_shared<Canvas>();
Grid::SetColumn(canvas.get(), 1);
mainGrid->AddChild(canvas);

root->AddChild(mainGrid);

// 状态栏：DockPanel
auto statusBar = std::make_shared<DockPanel>();
statusBar->SetHeight(30);
root->AddChild(statusBar);
```

## 最佳实践

### 1. 选择合适的布局容器

| 场景 | 推荐布局 |
|------|----------|
| 垂直/水平列表 | StackPanel |
| 表单、表格 | Grid |
| 图形编辑器、游戏UI | Canvas |
| IDE 界面 | DockPanel |
| 工具栏、标签云 | WrapPanel |

### 2. 避免过度嵌套

```cpp
// 不推荐：嵌套过深
auto root = std::make_shared<StackPanel>();
auto panel1 = std::make_shared<StackPanel>();
auto panel2 = std::make_shared<StackPanel>();
auto panel3 = std::make_shared<StackPanel>();
panel2->AddChild(panel3);
panel1->AddChild(panel2);
root->AddChild(panel1);

// 推荐：扁平化结构
auto root = std::make_shared<Grid>();
// 直接放置子元素
```

### 3. 使用 Star 尺寸实现响应式

```cpp
// 响应式：右侧始终占剩余空间的 2/3
grid->AddColumn(GridLength::Pixel(250));   // 固定侧边栏
grid->AddColumn(GridLength::Star(2));      // 主区域占 2 份
grid->AddColumn(GridLength::Star(1));      // 辅助区域占 1 份
```

## 与渲染系统集成

布局系统计算完成后，所有子元素的 `RenderRect` 都包含绝对窗口坐标：

```cpp
// 测量和排列
root->Measure(Size(width, height));
root->Arrange(Rect(0, 0, width, height));

// 渲染（使用绝对坐标）
root->Render(context);
```

## 性能优化

1. **脏区域检测**: 仅重新布局变化的区域
2. **缓存布局结果**: 尺寸未变时跳过重新计算
3. **批量更新**: 避免频繁单个元素更新
