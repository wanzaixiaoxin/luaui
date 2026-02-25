# LuaUI API 文档

本文档提供 LuaUI 框架中所有控件和类的完整 API 参考。

## 控件文档

### 基础控件

| 控件 | 说明 | 文档 |
|------|------|------|
| [Button](Button.md) | 按钮控件，支持点击事件和命令绑定 | [查看](Button.md) |
| [TextBlock](TextBlock.md) | 文本显示控件 | [查看](TextBlock.md) |
| [TextBox](TextBox.md) | 文本输入框 | [查看](TextBox.md) |
| [CheckBox](CheckBox.md) | 复选框 | [查看](CheckBox.md) |
| [RadioButton](RadioButton.md) | 单选按钮 | [查看](RadioButton.md) |

### 选择控件

| 控件 | 说明 | 文档 |
|------|------|------|
| [Slider](Slider.md) | 滑块控件，用于数值范围选择 | [查看](Slider.md) |
| [ProgressBar](ProgressBar.md) | 进度条，显示操作进度 | [查看](ProgressBar.md) |

### 布局容器

| 控件 | 说明 | 文档 |
|------|------|------|
| [Panel](Panel.md) | 面板基类，包含 StackPanel | [查看](Panel.md) |
| [Grid](Grid.md) | 网格布局，支持行列排列 | [查看](Grid.md) |

### 装饰控件

| 控件 | 说明 | 文档 |
|------|------|------|
| [Border](Border.md) | 边框容器，支持圆角和背景 | [查看](Border.md) |
| [Image](Image.md) | 图片显示控件 | [查看](Image.md) |

## 控件继承层次

```
Control (基类)
├── ContentControl
│   └── Button
│   └── Border
│   └── Image
├── Panel
│   └── StackPanel
│   └── Grid
├── TextBlock
├── TextBox
├── CheckBox
├── RadioButton
├── Slider
└── ProgressBar
```

## 快速参考

### 事件处理

```cpp
// 点击事件
button->Click.Add([](luaui::controls::Button* sender) {
    // 处理点击
});

// 属性变更
slider->ValueChanged.Add([](luaui::controls::Slider* sender, double newValue) {
    // 处理值变更
});
```

### 数据绑定

```xml
<!-- 单向绑定 -->
<TextBlock Text="{Binding UserName}"/>

<!-- 双向绑定 -->
<TextBox Text="{Binding UserName, Mode=TwoWay}"/>
<CheckBox IsChecked="{Binding IsEnabled, Mode=TwoWay}"/>

<!-- 绑定到命令 -->
<Button Content="保存" Command="{Binding SaveCommand}"/>
```

### 布局原则

1. **选择合适的容器**
   - 线性排列 → StackPanel
   - 表格布局 → Grid
   - 单元素装饰 → Border

2. **使用 Margin 和 Padding**
   - Margin：控件与其他控件的距离
   - Padding：内容与边框的距离

3. **合理设置尺寸**
   - 固定尺寸 → Width/Height
   - 自适应 → MinWidth/MaxWidth
   - 比例分配 → Grid 的 Star 单位

## 示例索引

| 示例 | 说明 | 位置 |
|------|------|------|
| 基础控件演示 | Button、TextBox 等控件使用 | `examples/01_controls_demo/` |
| MVVM 演示 | 数据绑定和命令 | `examples/02_mvvm_demo/` |
| Lua MVVM 演示 | Lua 集成与 MVVM | `examples/03_lua_mvvm_demo/` |

## 更多资源

- [架构文档](../architecture/) - 深入了解框架设计
- [README](../../README.md) - 项目概述
- [LICENSE](../../LICENSE) - 许可证信息
