# Slider 滑块控件

`Slider` 用于在数值范围内选择，支持水平/垂直方向和精确调整。

## 基本用法

### XML 中使用

```xml
<!-- 简单滑块（水平，0-100） -->
<Slider Value="{Binding Volume, Mode=TwoWay}"/>

<!-- 带范围和步长 -->
<Slider Minimum="0" 
        Maximum="255" 
        Value="128"
        SmallChange="1"
        LargeChange="10"/>

<!-- 垂直滑块 -->
<Slider IsVertical="true" 
        Height="200"
        Minimum="0"
        Maximum="100"/>

<!-- 双向绑定 -->
<Slider Value="{Binding Brightness, Mode=TwoWay}"/>

<!-- 带刻度 -->
<Slider Minimum="0" 
        Maximum="100" 
        IsSnapToTick="true"
        TickFrequency="10"/>
```

### 代码中使用

```cpp
#include "controls/Slider.h"

// 创建滑块
auto slider = std::make_shared<luaui::controls::Slider>();

// 设置范围
slider->SetMinimum(0.0);
slider->SetMaximum(100.0);

// 设置当前值
slider->SetValue(50.0);

// 设置步长
slider->SetSmallChange(1.0);   // 小步长（点击箭头或方向键）
slider->SetLargeChange(10.0);  // 大步长（点击轨道）

// 启用刻度吸附
slider->SetIsSnapToTick(true);

// 设置为垂直方向
slider->SetIsVertical(true);

// 绑定值变更事件
slider->ValueChanged.Add([](luaui::controls::Slider* sender, double newValue) {
    // 处理值变更
    std::cout << "新值: " << newValue << std::endl;
});
```

## 属性

| 属性 | 类型 | 说明 | XML 属性 | 默认值 |
|------|------|------|----------|--------|
| `Value` | double | 当前值 | `Value` | 0.0 |
| `Minimum` | double | 最小值 | `Minimum` | 0.0 |
| `Maximum` | double | 最大值 | `Maximum` | 100.0 |
| `SmallChange` | double | 小调整步长 | `SmallChange` | 1.0 |
| `LargeChange` | double | 大调整步长 | `LargeChange` | 10.0 |
| `IsSnapToTick` | bool | 是否吸附到刻度 | `IsSnapToTick` | false |
| `IsVertical` | bool | 是否为垂直方向 | `IsVertical` | false |

### 范围属性

```xml
<!-- 自定义范围 -->
<Slider Minimum="-50" Maximum="50" Value="0"/>

<!-- 颜色通道（0-255） -->
<Slider Minimum="0" Maximum="255" Value="128"/>

<!-- 百分比（0.0-1.0） -->
<Slider Minimum="0" Maximum="1" Value="0.5" SmallChange="0.01"/>
```

## 事件

| 事件 | 参数 | 说明 |
|------|------|------|
| `ValueChanged` | `Slider*, double` | 值变更时触发 |

### 事件使用示例

```cpp
// 实时更新
slider->ValueChanged.Add([this](luaui::controls::Slider* sender, double value) {
    // 更新显示
    UpdateDisplay(value);
    
    // 应用设置
    ApplySetting(value);
});

// 延迟更新（减少处理频率）
slider->ValueChanged.Add([this](luaui::controls::Slider* sender, double value) {
    // 取消之前的定时器
    if (m_updateTimer) {
        m_updateTimer->Cancel();
    }
    
    // 延迟 100ms 后更新
    m_updateTimer = SetTimeout([this, value]() {
        ApplySetting(value);
    }, 100);
});
```

## 使用场景

### 音量控制

```xml
<StackPanel Orientation="Horizontal" Spacing="12" VerticalAlignment="Center">
    <TextBlock Text="🔊" FontSize="20"/>
    <Slider Width="200" 
            Value="{Binding Volume, Mode=TwoWay}"
            Minimum="0" 
            Maximum="100"/>
    <TextBlock Text="{Binding Volume}" Width="40"/>
</StackPanel>
```

### 亮度调节

```xml
<Grid Columns="Auto,*,Auto" VerticalAlignment="Center">
    <TextBlock Text="🌑" Grid.Column="0" FontSize="16"/>
    <Slider Grid.Column="1" 
            Margin="12,0"
            Value="{Binding Brightness, Mode=TwoWay}"
            Minimum="0" 
            Maximum="255"
            LargeChange="25"/>
    <TextBlock Text="☀️" Grid.Column="2" FontSize="16"/>
</Grid>
```

### RGB 颜色选择

```xml
<StackPanel Orientation="Vertical" Spacing="12">
    <TextBlock Text="颜色选择" FontWeight="Bold"/>
    
    <!-- 红色通道 -->
    <StackPanel Orientation="Horizontal" Spacing="8">
        <TextBlock Text="R" Foreground="Red" Width="20"/>
        <Slider Width="200" 
                Value="{Binding Red, Mode=TwoWay}"
                Minimum="0" Maximum="255" 
                SmallChange="1"
                Foreground="Red"/>
        <TextBlock Text="{Binding Red}" Width="40"/>
    </StackPanel>
    
    <!-- 绿色通道 -->
    <StackPanel Orientation="Horizontal" Spacing="8">
        <TextBlock Text="G" Foreground="Green" Width="20"/>
        <Slider Width="200" 
                Value="{Binding Green, Mode=TwoWay}"
                Minimum="0" Maximum="255"
                Foreground="Green"/>
        <TextBlock Text="{Binding Green}" Width="40"/>
    </StackPanel>
    
    <!-- 蓝色通道 -->
    <StackPanel Orientation="Horizontal" Spacing="8">
        <TextBlock Text="B" Foreground="Blue" Width="20"/>
        <Slider Width="200" 
                Value="{Binding Blue, Mode=TwoWay}"
                Minimum="0" Maximum="255"
                Foreground="Blue"/>
        <TextBlock Text="{Binding Blue}" Width="40"/>
    </StackPanel>
    
    <!-- 颜色预览 -->
    <Border Width="100" Height="40" 
            Background="{Binding SelectedColor}"/>
</StackPanel>
```

### 缩放控制

```xml
<StackPanel Orientation="Horizontal" Spacing="8" VerticalAlignment="Center">
    <Button Content="-" Width="32" Click="ZoomOutCommand"/>
    <Slider Width="150" 
            Value="{Binding ZoomLevel, Mode=TwoWay}"
            Minimum="25" 
            Maximum="400"
            LargeChange="25"
            IsSnapToTick="true"
            TickFrequency="25"/>
    <Button Content="+" Width="32" Click="ZoomInCommand"/>
    <TextBlock Text="{Binding ZoomText}" Width="50"/>
</StackPanel>
```

### 垂直滚动条样式

```xml
<Grid Columns="*,Auto">
    <!-- 内容区域 -->
    <ScrollViewer Grid.Column="0">
        <TextBlock Text="{Binding LongContent}" TextWrapping="Wrap"/>
    </ScrollViewer>
    
    <!-- 自定义滚动条 -->
    <Slider Grid.Column="1"
            IsVertical="true"
            Width="20"
            Height="400"
            Value="{Binding ScrollPosition, Mode=TwoWay}"
            Minimum="0"
            Maximum="{Binding MaxScrollPosition}"/>
</Grid>
```

## 数据绑定

### TwoWay 绑定

```xml
<!-- 值变更会同步更新 ViewModel -->
<Slider Value="{Binding Volume, Mode=TwoWay}"/>

<!-- 带延迟更新（失去焦点后更新） -->
<Slider Value="{Binding Setting, Mode=TwoWay, UpdateSourceTrigger=LostFocus}"/>
```

### 带值转换器

```xml
<!-- 百分比显示（0-1 转换为 0-100%） -->
<Slider Value="{Binding Percentage, Converter=PercentageConverter}"/>

<!-- 对数刻度 -->
<Slider Value="{Binding Frequency, Converter=LogarithmicConverter}"
        Minimum="20" Maximum="20000"/>
```

## 交互方式

1. **拖动滑块**：直接设置值
2. **点击轨道**：按 LargeChange 增减
3. **方向键**：按 SmallChange 增减
4. **Home/End**：跳到最小/最大值

## 样式定制

```xml
<Window.Resources>
    <Style TargetType="Slider">
        <Setter Property="Height" Value="24"/>
        <Setter Property="ThumbColor" Value="#0078D4"/>
        <Setter Property="TrackColor" Value="#E0E0E0"/>
        <Setter Property="ActiveTrackColor" Value="#0078D4"/>
    </Style>
    
    <Style x:Key="VerticalSlider" TargetType="Slider">
        <Setter Property="IsVertical" Value="true"/>
        <Setter Property="Width" Value="24"/>
        <Setter Property="ThumbWidth" Value="16"/>
        <Setter Property="ThumbHeight" Value="32"/>
    </Style>
</Window.Resources>
```

## 最佳实践

### 1. 设置合理的范围

```xml
<!-- 推荐：明确的范围 -->
<Slider Minimum="0" Maximum="100" Value="50"/>

<!-- 不推荐：范围过大或过小 -->
<Slider Minimum="-999999" Maximum="999999"/>
```

### 2. 使用 SmallChange 和 LargeChange

```xml
<!-- 小步长用于精细调整 -->
<Slider SmallChange="1" LargeChange="10"/>

<!-- 大步长用于快速跳转 -->
<Slider SmallChange="0.1" LargeChange="1"/>
```

### 3. 显示当前值

```xml
<StackPanel Orientation="Horizontal" Spacing="8">
    <Slider Width="200" Value="{Binding Value, Mode=TwoWay}"/>
    <TextBlock Text="{Binding Value, StringFormat='{}{0:F1}'}" Width="50"/>
</StackPanel>
```

### 4. 使用 TwoWay 绑定

```xml
<!-- 推荐 -->
<Slider Value="{Binding Volume, Mode=TwoWay}"/>

<!-- 不推荐（单向绑定无法更新 ViewModel） -->
<Slider Value="{Binding Volume}"/>
```

### 5. 提供视觉反馈

```xml
<Grid>
    <!-- 背景轨道 -->
    <Border Background="#E0E0E0" Height="4"/>
    <!-- 滑块 -->
    <Slider Height="24"/>
</Grid>
```

## 参见

- [ProgressBar](ProgressBar.md) - 进度条控件
- [TextBlock](TextBlock.md) - 文本显示
- [数据绑定](../architecture/MVVM_DataBinding_Design.md)
