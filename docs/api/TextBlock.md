# TextBlock 文本显示控件

`TextBlock` 是用于显示只读文本的控件，支持字体样式、颜色和大小设置。

## 基本用法

### XML 中使用

```xml
<!-- 简单文本 -->
<TextBlock Text="Hello World"/>

<!-- 样式化文本 -->
<TextBlock Text="标题文本" 
           FontSize="24" 
           FontWeight="Bold"
           Foreground="#0078D4"/>

<!-- 多行文本 -->
<TextBlock Text="第一行&#x0a;第二行&#x0a;第三行"/>

<!-- 数据绑定 -->
<TextBlock Text="{Binding UserName}"/>
```

### 代码中使用

```cpp
#include "controls/TextBlock.h"

// 创建文本控件
auto textBlock = std::make_shared<luaui::controls::TextBlock>();

// 设置文本
textBlock->SetText(L"欢迎使用 LuaUI");

// 设置字体大小
textBlock->SetFontSize(18.0f);

// 设置文本颜色
textBlock->SetForeground(luaui::rendering::Color::FromHex(0x333333));

// 设置字体粗细
textBlock->SetFontWeight(luaui::rendering::FontWeight::Bold);

// 设置字体样式（斜体）
textBlock->SetFontStyle(luaui::rendering::FontStyle::Italic);
```

## 属性

| 属性 | 类型 | 说明 | XML 属性 | 默认值 |
|------|------|------|----------|--------|
| `Text` | wstring | 显示的文本内容 | `Text` | 空字符串 |
| `FontSize` | float | 字体大小（磅）| `FontSize` | 14.0 |
| `Foreground` | Color | 文本颜色 | `Foreground` | `Black()` |
| `FontWeight` | FontWeight | 字体粗细 | `FontWeight` | `Regular` |
| `FontStyle` | FontStyle | 字体样式 | `FontStyle` | `Normal` |

### 字体粗细 (FontWeight)

| 值 | 说明 |
|------|------|
| `Thin` | 细体 (100) |
| `ExtraLight` | 超轻 (200) |
| `Light` | 轻体 (300) |
| `Regular` | 常规 (400) |
| `Medium` | 中等 (500) |
| `SemiBold` | 半粗 (600) |
| `Bold` | 粗体 (700) |
| `ExtraBold` | 超粗 (800) |
| `Black` | 黑体 (900) |

### 字体样式 (FontStyle)

| 值 | 说明 |
|------|------|
| `Normal` | 正常 |
| `Italic` | 斜体 |
| `Oblique` | 倾斜体 |

## XML 属性

```xml
<!-- 文本内容 -->
<TextBlock Text="显示文本"/>

<!-- 字体大小 -->
<TextBlock Text="大号文本" FontSize="24"/>

<!-- 文本颜色（支持多种格式） -->
<TextBlock Text="蓝色文本" Foreground="#0078D4"/>
<TextBlock Text="红色文本" Foreground="Red"/>
<TextBlock Text="RGB颜色" Foreground="rgb(255,0,0)"/>

<!-- 字体粗细 -->
<TextBlock Text="粗体" FontWeight="Bold"/>
<TextBlock Text="细体" FontWeight="Light"/>

<!-- 字体样式 -->
<TextBlock Text="斜体" FontStyle="Italic"/>

<!-- 垂直对齐 -->
<TextBlock Text="居中" VerticalAlignment="Center"/>
```

## 数据绑定

`TextBlock` 最常用的场景是显示绑定的数据：

```xml
<!-- 简单绑定 -->
<TextBlock Text="{Binding UserName}"/>

<!-- 带字符串格式化 -->
<TextBlock Text="{Binding Price, StringFormat='{}{0:C}'}"/>

<!-- 多绑定 -->
<TextBlock Text="{Binding FirstName} {Binding LastName}"/>

<!-- 带值转换器 -->
<TextBlock Text="{Binding Status, Converter=StatusToTextConverter}"/>
```

## 文本换行

```xml
<!-- 自动换行（在容器边界处） -->
<TextBlock Text="这是一段很长的文本..." TextWrapping="Wrap"/>

<!-- 不换行 -->
<TextBlock Text="单行文本" TextWrapping="NoWrap"/>

<!-- 整词换行 -->
<TextBlock Text="整词换行文本..." TextWrapping="WrapWholeWords"/>
```

## 文本裁剪

```xml
<!-- 文本溢出时裁剪 -->
<TextBlock Text="很长的文本..." TextTrimming="Clip"/>

<!-- 显示省略号 -->
<TextBlock Text="很长的文本..." TextTrimming="CharacterEllipsis"/>

<!-- 词尾省略号 -->
<TextBlock Text="很长的文本..." TextTrimming="WordEllipsis"/>
```

## 使用场景

### 标题

```xml
<StackPanel Orientation="Vertical" Spacing="8">
    <TextBlock Text="主标题" FontSize="24" FontWeight="Bold"/>
    <TextBlock Text="副标题描述" FontSize="14" Foreground="#666666"/>
</StackPanel>
```

### 标签

```xml
<StackPanel Orientation="Horizontal" Spacing="8">
    <TextBlock Text="用户名：" VerticalAlignment="Center"/>
    <TextBox Text="{Binding UserName}"/>
</StackPanel>
```

### 状态显示

```xml
<TextBlock Text="{Binding StatusText}" 
           Foreground="{Binding StatusColor}"
           FontStyle="Italic"/>
```

### 帮助提示

```xml
<TextBlock Text="提示：点击保存按钮提交表单" 
           FontSize="12"
           Foreground="#888888"
           FontStyle="Italic"/>
```

## 与 Label 的区别

`TextBlock` 和 `Label` 都用于显示文本，但有以下区别：

| 特性 | TextBlock | Label |
|------|-----------|-------|
| 用途 | 纯文本显示 | 表单标签 |
| 快捷键 | 不支持 | 支持访问键 |
| 性能 | 更轻量 | 略重 |
| 换行 | 支持 | 支持 |

推荐使用 `TextBlock` 作为默认的文本显示控件。

## 性能优化

1. **避免频繁更新**：大量文本更新应考虑使用虚拟化
2. **固定尺寸**：设置明确宽度可避免测量开销
3. **缓存文本**：静态文本可预先渲染

```xml
<!-- 推荐：固定宽度 -->
<TextBlock Text="内容" Width="200"/>

<!-- 不推荐：频繁绑定的长文本 -->
<TextBlock Text="{Binding LongTextThatChangesFrequently}"/>
```

## 参见

- [TextBox](TextBox.md) - 文本输入控件
- [Button](Button.md) - 按钮控件
- [样式系统](../architecture/Style_System_Detailed_Design.md)
