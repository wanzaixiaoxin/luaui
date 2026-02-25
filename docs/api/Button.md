# Button 按钮控件

按钮是最常用的交互控件，用于触发操作或提交表单。

## 基本用法

### XML 中使用

```xml
<!-- 简单按钮 -->
<Button Content="点击我"/>

<!-- 带事件处理的按钮 -->
<Button Content="保存" Click="OnSaveClick"/>

<!-- 样式化按钮 -->
<Button Content="主要按钮" 
        Style="Primary"
        Width="120" 
        Height="40"/>

<!-- 自定义颜色 -->
<Button Content="危险操作"
        SetStateColors="#E0E0E0,#D0D0D0,#C0C0C0"/>
```

### 代码中使用

```cpp
#include "controls/Button.h"

// 创建按钮
auto button = std::make_shared<luaui::controls::Button>();

// 设置文本
button->SetText(L"确认");

// 设置大小
if (auto* layout = button->GetLayout()) {
    layout->SetWidth(120);
    layout->SetHeight(40);
}

// 设置状态颜色（正常、悬停、按下）
button->SetStateColors(
    luaui::rendering::Color::FromHex(0x0078D4),  // 正常
    luaui::rendering::Color::FromHex(0x106EBE),  // 悬停
    luaui::rendering::Color::FromHex(0x005A9E)   // 按下
);

// 绑定点击事件
button->Click.Add([](luaui::Control* sender) {
    // 处理点击
});
```

## 属性

| 属性 | 类型 | 说明 | XML 属性 |
|------|------|------|----------|
| `Text` | wstring | 按钮显示的文本 | `Content` |
| `IsPressed` | bool | 是否处于按下状态（只读）| - |
| `IsHovered` | bool | 是否处于悬停状态（只读）| - |

### 外观属性

| 属性 | 类型 | 说明 | 默认值 |
|------|------|------|--------|
| `NormalBackground` | Color | 正常状态背景色 | `#E0E0E0` |
| `HoverBackground` | Color | 悬停状态背景色 | `#D0D0D0` |
| `PressedBackground` | Color | 按下状态背景色 | `#C0C0C0` |

## 事件

| 事件 | 参数 | 说明 |
|------|------|------|
| `Click` | `Control*` | 按钮被点击时触发 |
| `MouseEnter` | - | 鼠标进入按钮区域 |
| `MouseLeave` | - | 鼠标离开按钮区域 |
| `MouseDown` | `MouseEventArgs` | 鼠标按下 |
| `MouseUp` | `MouseEventArgs` | 鼠标释放 |

### 事件使用示例

```cpp
// 使用 Lambda
button->Click.Add([this](luaui::Control*) {
    OnSubmit();
});

// 使用成员函数（需要包装）
button->Click.Add([this](luaui::Control*) {
    this->HandleButtonClick();
});
```

## 样式

### 预设样式

```xml
<!-- 主要按钮（蓝色） -->
<Button Content="保存" Style="Primary"/>

<!-- 强调按钮（绿色） -->
<Button Content="新建" Style="Accent"/>

<!-- 危险按钮（红色） -->
<Button Content="删除" Style="Danger"/>

<!-- 无边框按钮 -->
<Button Content="链接" Style="Link"/>
```

### 自定义样式

```xml
<Window.Resources>
    <Style x:Key="RoundedButton" TargetType="Button">
        <Setter Property="Background" Value="#0078D4"/>
        <Setter Property="Foreground" Value="White"/>
        <Setter Property="FontSize" Value="14"/>
        <Setter Property="Padding" Value="16,8"/>
    </Style>
</Window.Resources>

<Button Content="圆角按钮" Style="{StaticResource RoundedButton}"/>
```

## 数据绑定

按钮的 `Content` 属性支持数据绑定：

```xml
<!-- 绑定到 ViewModel 的属性 -->
<Button Content="{Binding ButtonText}" Click="SubmitCommand"/>

<!-- 绑定命令 -->
<Button Content="保存" Command="{Binding SaveCommand}"/>
```

## 布局

### 尺寸设置

```xml
<!-- 固定尺寸 -->
<Button Content="固定大小" Width="100" Height="40"/>

<!-- 自动宽度 -->
<Button Content="自适应宽度" Height="40"/>

<!-- 边距 -->
<Button Content="带边距" Margin="10,5"/>
```

### 在容器中使用

```xml
<StackPanel Orientation="Horizontal" Spacing="8">
    <Button Content="确定" Style="Primary"/>
    <Button Content="取消"/>
    <Button Content="帮助" Style="Link"/>
</StackPanel>
```

## 最佳实践

1. **文本清晰**：按钮文本应该清楚地描述操作结果
   ```xml
   <!-- 推荐 -->
   <Button Content="保存文件"/>
   
   <!-- 不推荐 -->
   <Button Content="确定"/>
   ```

2. **主要操作突出**：使用 `Primary` 样式突出主要操作
   ```xml
   <StackPanel Orientation="Horizontal">
       <Button Content="保存" Style="Primary"/>
       <Button Content="取消"/>
   </StackPanel>
   ```

3. **危险操作警示**：删除等危险操作使用 `Danger` 样式
   ```xml
   <Button Content="删除" Style="Danger" Click="OnDeleteClick"/>
   ```

4. **快捷键支持**：为常用按钮添加快捷键
   ```xml
   <Button Content="保存(_S)" Click="OnSaveClick"/>
   ```

## 参见

- [TextBlock](TextBlock.md) - 文本显示控件
- [Panel](Panel.md) - 布局容器
- [样式系统](../architecture/Style_System_Detailed_Design.md)
