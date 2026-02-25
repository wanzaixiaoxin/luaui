# Border 边框容器

`Border` 是一个装饰性容器，可为单个子元素添加边框、背景、圆角和阴影效果。

## 基本用法

### XML 中使用

```xml
<!-- 简单边框 -->
<Border BorderThickness="1" BorderColor="#CCCCCC">
    <TextBlock Text="带边框的内容" Padding="12"/>
</Border>

<!-- 带背景和圆角 -->
<Border Background="#F5F5F5" 
        BorderThickness="1" 
        BorderColor="#E0E0E0"
        CornerRadius="8"
        Padding="16">
    <TextBlock Text="圆角卡片"/>
</Border>

<!-- 阴影效果 -->
<Border Background="White"
        CornerRadius="4"
        Padding="24"
        Shadow="2">
    <TextBlock Text="带阴影的卡片"/>
</Border>

<!-- 作为装饰器 -->
<Border BorderThickness="2" 
        BorderColor="Red"
        IsVisible="{Binding HasError}">
    <TextBox Text="{Binding Input}"/>
</Border>
```

### 代码中使用

```cpp
#include "controls/Border.h"

// 创建边框
auto border = std::make_shared<luaui::controls::Border>();

// 设置边框样式
border->SetBorderThickness(1.0f);
border->SetBorderColor(luaui::Color(0xFF, 0xCC, 0xCC));

// 设置背景和圆角
border->SetBackgroundColor(luaui::Color(0xF5, 0xF5, 0xF5));
border->SetCornerRadius(8.0f);

// 设置内边距
border->SetPadding(luaui::Thickness(16, 12, 16, 12));

// 添加子元素
auto content = std::make_shared<luaui::controls::TextBlock>();
content->SetText(L"带边框的内容");
border->SetChild(content);
```

## 属性

| 属性 | 类型 | 说明 | XML 属性 | 默认值 |
|------|------|------|----------|--------|
| `Child` | Control | 子元素 | 嵌套内容 | null |
| `Background` | Color/Brush | 背景颜色 | `Background` | Transparent |
| `BorderBrush` | Color/Brush | 边框画刷 | `BorderColor` | null |
| `BorderThickness` | Thickness | 边框粗细 | `BorderThickness` | 0 |
| `CornerRadius` | float | 圆角半径 | `CornerRadius` | 0 |
| `Padding` | Thickness | 内边距 | `Padding` | 0 |

### 边框粗细

```xml
<!-- 统一粗细 -->
<Border BorderThickness="1"/>

<!-- 分别设置（左、上、右、下） -->
<Border BorderThickness="1,2,1,2"/>

<!-- 代码中 -->
border->SetBorderThickness(luaui::Thickness(1, 2, 1, 2));
```

### 圆角半径

```xml
<!-- 统一圆角 -->
<Border CornerRadius="8"/>

<!-- 分别设置（左上、右上、右下、左下） -->
<Border CornerRadius="8,8,0,0"/>  <!-- 只有顶部圆角 -->

<!-- 圆形 -->
<Border CornerRadius="999" Width="100" Height="100"/>
```

## 使用场景

### 卡片样式

```xml
<Border Background="White"
        CornerRadius="8"
        BorderThickness="1"
        BorderColor="#E0E0E0"
        Shadow="2"
        Padding="20">
    <StackPanel Orientation="Vertical" Spacing="12">
        <TextBlock Text="卡片标题" FontWeight="Bold" FontSize="18"/>
        <TextBlock Text="卡片内容描述..." Foreground="Gray"/>
        <Button Content="操作按钮" HorizontalAlignment="Right"/>
    </StackPanel>
</Border>
```

### 分组框

```xml
<Border Background="#FAFAFA"
        BorderThickness="1"
        BorderColor="#DDDDDD"
        CornerRadius="4"
        Padding="16">
    <StackPanel Orientation="Vertical" Spacing="8">
        <TextBlock Text="设置" FontWeight="Bold" Margin="0,0,0,8"/>
        
        <CheckBox Content="启用通知"/>
        <CheckBox Content="自动更新"/>
        <CheckBox Content="暗黑模式"/>
    </StackPanel>
</Border>
```

### 输入框高亮

```xml
<!-- 错误状态 -->
<Border BorderThickness="2"
        BorderColor="Red"
        CornerRadius="4"
        IsVisible="{Binding HasError}">
    <TextBox Text="{Binding Input}"/>
</Border>

<!-- 焦点状态 -->
<Border BorderThickness="2"
        BorderColor="{Binding IsFocused, Converter={StaticResource FocusColorConverter}}">
    <TextBox Text="{Binding Input}"/>
</Border>
```

### 图片圆角

```xml
<Border CornerRadius="8" ClipToBounds="true">
    <Image Source="avatar.png" Stretch="Uniform"/>
</Border>
```

### 提示框

```xml
<Border Background="#FFF9E6"
        BorderThickness="1"
        BorderColor="#FFE066"
        CornerRadius="4"
        Padding="12,8">
    <StackPanel Orientation="Horizontal" Spacing="8">
        <TextBlock Text="⚠️" FontSize="16"/>
        <TextBlock Text="注意：此操作不可撤销" Foreground="#B7791F"/>
    </StackPanel>
</Border>
```

### 分隔线

```xml
<!-- 水平分隔线 -->
<Border Background="#E0E0E0" Height="1"/>

<!-- 垂直分隔线 -->
<Border Background="#E0E0E0" Width="1"/>

<!-- 带边距的分隔 -->
<Border Background="#E0E0E0" Height="1" Margin="16,8"/>
```

### 徽章/标签

```xml
<Border Background="#0078D4"
        CornerRadius="12"
        Padding="8,4">
    <TextBlock Text="NEW" 
               Foreground="White" 
               FontSize="10" 
               FontWeight="Bold"/>
</Border>

<Border Background="#E81123"
        CornerRadius="10"
        Width="20" Height="20"
        HorizontalContentAlignment="Center"
        VerticalContentAlignment="Center">
    <TextBlock Text="5" Foreground="White" FontSize="12"/>
</Border>
```

## 布局组合

### 嵌套边框

```xml
<!-- 多层边框效果 -->
<Border Background="#FFFFFF" Padding="16">
    <Border Background="#F5F5F5" 
            BorderThickness="1"
            BorderColor="#DDDDDD"
            CornerRadius="8"
            Padding="12">
        <TextBlock Text="嵌套边框内容"/>
    </Border>
</Border>
```

### 与布局容器结合

```xml
<!-- Grid 中的边框 -->
<Grid Columns="*,*,*" Spacing="16">
    <Border Grid.Column="0" 
            Background="White"
            CornerRadius="8"
            Padding="16">
        <StackPanel>
            <TextBlock Text="卡片 1" FontWeight="Bold"/>
            <TextBlock Text="内容..." Foreground="Gray"/>
        </StackPanel>
    </Border>
    
    <Border Grid.Column="1" 
            Background="White"
            CornerRadius="8"
            Padding="16">
        <StackPanel>
            <TextBlock Text="卡片 2" FontWeight="Bold"/>
            <TextBlock Text="内容..." Foreground="Gray"/>
        </StackPanel>
    </Border>
    
    <Border Grid.Column="2" 
            Background="White"
            CornerRadius="8"
            Padding="16">
        <StackPanel>
            <TextBlock Text="卡片 3" FontWeight="Bold"/>
            <TextBlock Text="内容..." Foreground="Gray"/>
        </StackPanel>
    </Border>
</Grid>
```

## 动态样式

### 根据状态改变

```cpp
// 设置错误状态样式
void SetErrorState(bool hasError) {
    if (hasError) {
        m_border->SetBorderColor(luaui::Color(0xE8, 0x11, 0x23));  // 红色
        m_border->SetBorderThickness(2.0f);
        m_border->SetBackgroundColor(luaui::Color(0xFD, 0xF2, 0xF2));  // 浅红背景
    } else {
        m_border->SetBorderColor(luaui::Color(0xCC, 0xCC, 0xCC));  // 灰色
        m_border->SetBorderThickness(1.0f);
        m_border->SetBackgroundColor(luaui::Color(0xFF, 0xFF, 0xFF));  // 白色
    }
}
```

### 数据绑定样式

```xml
<Border BorderColor="{Binding IsValid, Converter={StaticResource ValidColorConverter}}"
        BorderThickness="{Binding IsValid, Converter={StaticResource ValidThicknessConverter}}"
        CornerRadius="4">
    <TextBox Text="{Binding Input, Mode=TwoWay}"/>
</Border>
```

## 预定义样式

```xml
<Window.Resources>
    <!-- 卡片样式 -->
    <Style x:Key="CardStyle" TargetType="Border">
        <Setter Property="Background" Value="White"/>
        <Setter Property="CornerRadius" Value="8"/>
        <Setter Property="BorderThickness" Value="1"/>
        <Setter Property="BorderColor" Value="#E0E0E0"/>
        <Setter Property="Padding" Value="20"/>
    </Style>
    
    <!-- 提示框样式 -->
    <Style x:Key="AlertStyle" TargetType="Border">
        <Setter Property="Background" Value="#FFF9E6"/>
        <Setter Property="BorderColor" Value="#FFE066"/>
        <Setter Property="CornerRadius" Value="4"/>
        <Setter Property="Padding" Value="12,8"/>
    </Style>
    
    <!-- 错误样式 -->
    <Style x:Key="ErrorStyle" TargetType="Border">
        <Setter Property="BorderColor" Value="#E81123"/>
        <Setter Property="BorderThickness" Value="2"/>
        <Setter Property="CornerRadius" Value="4"/>
    </Style>
    
    <!-- 标签样式 -->
    <Style x:Key="TagStyle" TargetType="Border">
        <Setter Property="Background" Value="#E3F2FD"/>
        <Setter Property="CornerRadius" Value="12"/>
        <Setter Property="Padding" Value="8,4"/>
    </Style>
</Window.Resources>
```

## 最佳实践

### 1. 合理使用 Padding

```xml
<!-- 推荐：内边距让内容有呼吸空间 -->
<Border Padding="16">
    <TextBlock Text="内容"/>
</Border>

<!-- 不推荐：内容贴边 -->
<Border>
    <TextBlock Text="内容" Margin="16"/>
</Border>
```

### 2. 统一圆角

```xml
<!-- 推荐：使用一致的圆角半径 -->
<Border CornerRadius="8"/>  <!-- 小卡片 -->
<Border CornerRadius="16"/> <!-- 大卡片 -->
<Border CornerRadius="999"/> <!-- 圆形 -->
```

### 3. 避免过度嵌套

```xml
<!-- 不推荐：多层嵌套 -->
<Border>
    <Border>
        <Border>
            <TextBlock Text="内容"/>
        </Border>
    </Border>
</Border>

<!-- 推荐：直接样式 -->
<Border Background="#F5F5F5" 
        BorderThickness="1"
        Padding="16">
    <TextBlock Text="内容"/>
</Border>
```

### 4. 配合 ClipToBounds

```xml
<!-- 确保子元素不超出圆角 -->
<Border CornerRadius="8" ClipToBounds="true">
    <Image Source="photo.jpg"/>
</Border>
```

### 5. 可访问性

```xml
<!-- 提供足够的颜色对比度 -->
<Border Background="#FFFFFF" BorderColor="#666666">
    <TextBlock Text="高对比度文本" Foreground="#000000"/>
</Border>
```

## 性能提示

1. **避免频繁重绘**：静态边框性能开销小
2. **阴影效果**：可能增加渲染开销，按需使用
3. **嵌套层级**：过多嵌套影响布局性能

## 参见

- [Panel](Panel.md) - 面板基类
- [Grid](Grid.md) - 网格布局
- [StackPanel](Panel.md#stackpanel-堆栈面板) - 堆栈面板
