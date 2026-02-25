# ProgressBar 进度条控件

`ProgressBar` 用于显示操作进度或状态指示，支持确定性和不确定性两种模式。

## 基本用法

### XML 中使用

```xml
<!-- 确定性进度（显示具体进度） -->
<ProgressBar Value="75" Maximum="100"/>

<!-- 不确定进度（动画循环） -->
<ProgressBar IsIndeterminate="true"/>

<!-- 垂直进度条 -->
<ProgressBar IsVertical="true" Height="200" Value="60"/>

<!-- 绑定到数据 -->
<ProgressBar Value="{Binding DownloadProgress}"
             Maximum="{Binding TotalSize}"/>
```

### 代码中使用

```cpp
#include "controls/ProgressBar.h"

// 创建进度条
auto progressBar = std::make_shared<luaui::controls::ProgressBar>();

// 设置确定性模式
progressBar->SetIsIndeterminate(false);
progressBar->SetMinimum(0.0);
progressBar->SetMaximum(100.0);
progressBar->SetValue(0.0);

// 更新进度
progressBar->SetValue(50.0);

// 设置为不确定模式
progressBar->SetIsIndeterminate(true);

// 设置为垂直
progressBar->SetIsVertical(true);
```

## 属性

| 属性 | 类型 | 说明 | XML 属性 | 默认值 |
|------|------|------|----------|--------|
| `Value` | double | 当前进度值 | `Value` | 0.0 |
| `Minimum` | double | 最小值 | `Minimum` | 0.0 |
| `Maximum` | double | 最大值 | `Maximum` | 100.0 |
| `IsIndeterminate` | bool | 是否为不确定模式 | `IsIndeterminate` | false |
| `IsVertical` | bool | 是否为垂直方向 | `IsVertical` | false |

### 确定性模式

```xml
<!-- 显示具体进度 -->
<ProgressBar Value="75" Maximum="100"/>

<!-- 自定义范围 -->
<ProgressBar Minimum="0" Maximum="1024" Value="512"/>
```

### 不确定模式

```xml
<!-- 无限循环动画（等待中） -->
<ProgressBar IsIndeterminate="true"/>
```

## 事件

| 事件 | 参数 | 说明 |
|------|------|------|
| `ValueChanged` | `ProgressBar*, double` | 值变更时触发 |

## 使用场景

### 文件下载

```xml
<Grid Columns="*,Auto" VerticalAlignment="Center" Spacing="12">
    <!-- 进度条 -->
    <ProgressBar Grid.Column="0" 
                 Value="{Binding DownloadedBytes}"
                 Maximum="{Binding TotalBytes}"
                 Height="8"/>
    
    <!-- 百分比文本 -->
    <TextBlock Grid.Column="1" 
               Text="{Binding ProgressPercent, StringFormat='{}{0:F0}%' }"
               Width="40"/>
</Grid>

<!-- 下载信息 -->
<StackPanel Orientation="Horizontal" Spacing="16">
    <TextBlock Text="{Binding DownloadSpeed}" Foreground="Gray"/>
    <TextBlock Text="{Binding TimeRemaining}" Foreground="Gray"/>
</StackPanel>
```

### 安装向导

```xml
<StackPanel Orientation="Vertical" Spacing="16" Margin="24">
    <TextBlock Text="正在安装..." FontSize="16"/>
    
    <!-- 总体进度 -->
    <ProgressBar Value="{Binding OverallProgress}" Height="16"/>
    
    <!-- 当前文件 -->
    <TextBlock Text="{Binding CurrentFile}" 
               FontSize="12" 
               Foreground="Gray"
               TextTrimming="CharacterEllipsis"/>
    
    <!-- 文件进度（嵌套） -->
    <ProgressBar Value="{Binding FileProgress}" 
                 Height="8"
                 Foreground="LightBlue"/>
</StackPanel>
```

### 批量处理

```xml
<Border Background="#F5F5F5" Padding="16">
    <StackPanel Orientation="Vertical" Spacing="12">
        <Grid Columns="*,Auto">
            <TextBlock Grid.Column="0" Text="处理进度"/>
            <TextBlock Grid.Column="1" 
                       Text="{Binding CompletedCount, StringFormat='{}{0}/{1}', StringFormatParameters={Binding TotalCount}}"/>
        </Grid>
        
        <ProgressBar Value="{Binding CompletedCount}"
                     Maximum="{Binding TotalCount}"
                     Height="12"/>
        
        <TextBlock Text="{Binding CurrentItemName}" 
                   FontSize="12" 
                   Foreground="Gray"/>
    </StackPanel>
</Border>
```

### 电池电量

```xml
<StackPanel Orientation="Horizontal" Spacing="8" VerticalAlignment="Center">
    <!-- 电池图标 -->
    <Border Width="48" Height="24" BorderThickness="2" CornerRadius="4">
        <ProgressBar Value="{Binding BatteryLevel}"
                     Maximum="100"
                     Background="Transparent"
                     Foreground="{Binding BatteryColor}"/>
    </Border>
    <TextBlock Text="{Binding BatteryPercent}"/>
</StackPanel>
```

### 系统初始化

```xml
<Grid VerticalAlignment="Center" HorizontalAlignment="Center">
    <StackPanel Orientation="Vertical" Spacing="24">
        <TextBlock Text="正在启动..." 
                   FontSize="24" 
                   HorizontalAlignment="Center"/>
        
        <!-- 加载动画 -->
        <ProgressBar IsIndeterminate="true" 
                     Width="200" 
                     Height="4"/>
        
        <TextBlock Text="{Binding LoadingStep}" 
                   FontSize="14" 
                   Foreground="Gray"
                   HorizontalAlignment="Center"/>
    </StackPanel>
</Grid>
```

### 容量指示

```xml
<StackPanel Orientation="Vertical" Spacing="8">
    <!-- 存储空间 -->
    <Grid Columns="*,Auto">
        <TextBlock Grid.Column="0" Text="存储空间"/>
        <TextBlock Grid.Column="1" 
                   Text="{Binding UsedSpace, StringFormat='{}{0}GB / {1}GB', StringFormatParameters={Binding TotalSpace}}"/>
    </Grid>
    <ProgressBar Value="{Binding UsedSpace}"
                 Maximum="{Binding TotalSpace}"
                 Foreground="{Binding StorageColor}"/>
    
    <!-- 内存使用 -->
    <Grid Columns="*,Auto">
        <TextBlock Grid.Column="0" Text="内存使用"/>
        <TextBlock Grid.Column="1" 
                   Text="{Binding MemoryPercent, StringFormat='{}{0:F1}%'}"/>
    </Grid>
    <ProgressBar Value="{Binding UsedMemory}"
                 Maximum="{Binding TotalMemory}"
                 Foreground="{Binding MemoryColor}"/>
</StackPanel>
```

## 样式定制

### 颜色状态

```xml
<Window.Resources>
    <!-- 正常 -->
    <Style x:Key="NormalProgressBar" TargetType="ProgressBar">
        <Setter Property="Foreground" Value="#0078D4"/>
        <Setter Property="Background" Value="#E0E0E0"/>
    </Style>
    
    <!-- 警告 -->
    <Style x:Key="WarningProgressBar" TargetType="ProgressBar">
        <Setter Property="Foreground" Value="#FFB900"/>
    </Style>
    
    <!-- 危险 -->
    <Style x:Key="DangerProgressBar" TargetType="ProgressBar">
        <Setter Property="Foreground" Value="#E81123"/>
    </Style>
    
    <!-- 成功 -->
    <Style x:Key="SuccessProgressBar" TargetType="ProgressBar">
        <Setter Property="Foreground" Value="#107C10"/>
    </Style>
</Window.Resources>

<!-- 使用 -->
<ProgressBar Style="{StaticResource NormalProgressBar}" Value="50"/>
<ProgressBar Style="{StaticResource WarningProgressBar}" Value="75"/>
```

### 尺寸样式

```xml
<Style x:Key="ThinProgressBar" TargetType="ProgressBar">
    <Setter Property="Height" Value="4"/>
    <Setter Property="CornerRadius" Value="2"/>
</Style>

<Style x:Key="ThickProgressBar" TargetType="ProgressBar">
    <Setter Property="Height" Value="16"/>
    <Setter Property="CornerRadius" Value="8"/>
</Style>
```

## 代码控制进度

### 模拟进度

```cpp
// 模拟文件下载进度
void DownloadFile(const std::wstring& url) {
    auto progressBar = GetProgressBar();
    progressBar->SetIsIndeterminate(false);
    
    // 模拟下载
    for (int i = 0; i <= 100; ++i) {
        progressBar->SetValue(i);
        
        // 更新 UI
        Application::Current()->ProcessMessages();
        
        // 模拟延迟
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    // 下载完成
    progressBar->SetValue(100);
}
```

### 异步操作

```cpp
// 异步任务进度
class AsyncTask : public std::enable_shared_from_this<AsyncTask> {
public:
    void Start(std::shared_ptr<luaui::controls::ProgressBar> progressBar) {
        m_progressBar = progressBar;
        m_progressBar->SetIsIndeterminate(false);
        m_progressBar->SetValue(0);
        
        // 在后台线程执行
        std::thread([self = shared_from_this()]() {
            for (int i = 0; i <= 100; ++i) {
                self->ReportProgress(i);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }).detach();
    }
    
private:
    void ReportProgress(int value) {
        // 切换到 UI 线程更新
        Dispatcher::Current()->Invoke([this, value]() {
            m_progressBar->SetValue(value);
        });
    }
    
    std::shared_ptr<luaui::controls::ProgressBar> m_progressBar;
};
```

## 最佳实践

### 1. 确定性 vs 不确定模式

```xml
<!-- 知道进度时用确定性 -->
<ProgressBar Value="{Binding CurrentProgress}" Maximum="{Binding TotalProgress}"/>

<!-- 不知道进度时用不确定模式 -->
<ProgressBar IsIndeterminate="{Binding IsWaiting}"/>
```

### 2. 显示百分比

```xml
<Grid Columns="*,60">
    <ProgressBar Grid.Column="0" Value="{Binding Progress}"/>
    <TextBlock Grid.Column="1" 
               Text="{Binding ProgressPercent}" 
               HorizontalAlignment="Right"/>
</Grid>
```

### 3. 及时更新

```cpp
// 推荐：频繁更新保持响应
for (size_t i = 0; i < items.size(); ++i) {
    ProcessItem(items[i]);
    progressBar->SetValue(static_cast<double>(i + 1) / items.size() * 100);
    Application::Current()->ProcessMessages();
}
```

### 4. 完成状态

```xml
<!-- 完成时改变样式 -->
<ProgressBar Value="{Binding Progress}"
             Foreground="{Binding IsComplete, Converter={StaticResource CompleteColorConverter}}"/>
```

### 5. 多阶段进度

```xml
<StackPanel Spacing="8">
    <TextBlock Text="阶段 {Binding CurrentStage}/{Binding TotalStage}"/>
    <ProgressBar Value="{Binding StageProgress}"/>
    <ProgressBar Value="{Binding OverallProgress}" Height="4" Foreground="Gray"/>
</StackPanel>
```

## 性能优化

1. **批量更新**：避免过于频繁的 SetValue 调用
2. **动画优化**：不确定模式使用硬件加速
3. **可见性控制**：不需要时设置 IsVisible="false"

## 参见

- [Slider](Slider.md) - 滑块控件
- [TextBlock](TextBlock.md) - 文本显示
- [数据绑定](../architecture/MVVM_DataBinding_Design.md)
