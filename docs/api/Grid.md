# Grid 网格布局

`Grid` 是最强大的布局容器，支持按行和列排列子控件，可实现复杂的表格布局。

## 基本用法

### XML 中使用

```xml
<!-- 简单网格：2行2列 -->
<Grid>
    <Grid.RowDefinitions>
        <RowDefinition Height="Auto"/>
        <RowDefinition Height="*"/>
    </Grid.RowDefinitions>
    <Grid.ColumnDefinitions>
        <ColumnDefinition Width="Auto"/>
        <ColumnDefinition Width="*"/>
    </Grid.ColumnDefinitions>
    
    <!-- 子控件 -->
    <TextBlock Text="标签1" Grid.Row="0" Grid.Column="0"/>
    <TextBox Grid.Row="0" Grid.Column="1"/>
    <TextBlock Text="标签2" Grid.Row="1" Grid.Column="0"/>
    <TextBox Grid.Row="1" Grid.Column="1"/>
</Grid>
```

### 代码中使用

```cpp
#include "controls/layouts/Grid.h"

// 创建网格
auto grid = std::make_shared<luaui::controls::Grid>();

// 添加列
column->AddColumn(luaui::controls::GridLength::Auto());      // 自动宽度
column->AddColumn(luaui::controls::GridLength::Star(1.0f));  // 比例宽度
column->AddColumn(luaui::controls::GridLength::Pixel(100));  // 固定宽度

// 添加行
grid->AddRow(luaui::controls::GridLength::Auto());
grid->AddRow(luaui::controls::GridLength::Star(1.0f));

// 添加子控件并设置位置
auto textBlock = std::make_shared<luaui::controls::TextBlock>();
textBlock->SetText(L"标签");
grid->AddChild(textBlock);
grid->SetRow(textBlock, 0);
grid->SetColumn(textBlock, 0);

auto textBox = std::make_shared<luaui::controls::TextBox>();
grid->AddChild(textBox);
grid->SetRow(textBox, 0);
grid->SetColumn(textBox, 1);
```

## 行列定义

### 列宽类型 (ColumnDefinition)

| 类型 | 说明 | 示例 |
|------|------|------|
| `Auto` | 根据内容自动调整 | `<ColumnDefinition Width="Auto"/>` |
| `Pixel` | 固定像素宽度 | `<ColumnDefinition Width="100"/>` |
| `Star` | 按比例分配剩余空间 | `<ColumnDefinition Width="*"/>` 或 `<ColumnDefinition Width="2*"/>` |

### 行高类型 (RowDefinition)

| 类型 | 说明 | 示例 |
|------|------|------|
| `Auto` | 根据内容自动调整 | `<RowDefinition Height="Auto"/>` |
| `Pixel` | 固定像素高度 | `<RowDefinition Height="50"/>` |
| `Star` | 按比例分配剩余空间 | `<RowDefinition Height="*"/>` |

### Star 比例计算

```xml
<!-- 两列，第二列是第一列的两倍宽 -->
<Grid>
    <Grid.ColumnDefinitions>
        <ColumnDefinition Width="*"/>    <!-- 1/3 -->
        <ColumnDefinition Width="2*"/>   <!-- 2/3 -->
    </Grid.ColumnDefinitions>
</Grid>
```

## 附加属性

| 属性 | 类型 | 说明 | 默认值 |
|------|------|------|--------|
| `Grid.Row` | int | 所在行索引 | 0 |
| `Grid.Column` | int | 所在列索引 | 0 |
| `Grid.RowSpan` | int | 跨行数 | 1 |
| `Grid.ColumnSpan` | int | 跨列数 | 1 |

### 跨行跨列

```xml
<Grid>
    <Grid.RowDefinitions>
        <RowDefinition Height="Auto"/>
        <RowDefinition Height="Auto"/>
        <RowDefinition Height="*"/>
    </Grid.RowDefinitions>
    <Grid.ColumnDefinitions>
        <ColumnDefinition Width="Auto"/>
        <ColumnDefinition Width="*"/>
    </Grid.ColumnDefinitions>
    
    <!-- 跨两列的标题 -->
    <TextBlock Text="标题" 
               Grid.Row="0" Grid.Column="0"
               Grid.ColumnSpan="2"
               FontSize="18" FontWeight="Bold"/>
    
    <!-- 跨两行的侧边栏 -->
    <Border Grid.Row="1" Grid.Column="0"
            Grid.RowSpan="2"
            Background="#F5F5F5"/>
    
    <!-- 主内容区 -->
    <TextBox Grid.Row="1" Grid.Column="1"/>
    <Button Grid.Row="2" Grid.Column="1" Content="提交"/>
</Grid>
```

## 常见布局模式

### 表单布局

```xml
<Grid Margin="24">
    <Grid.RowDefinitions>
        <RowDefinition Height="Auto"/>
        <RowDefinition Height="Auto"/>
        <RowDefinition Height="Auto"/>
        <RowDefinition Height="Auto"/>
        <RowDefinition Height="*"/>
    </Grid.RowDefinitions>
    <Grid.ColumnDefinitions>
        <ColumnDefinition Width="Auto" MinWidth="80"/>
        <ColumnDefinition Width="*"/>
    </Grid.ColumnDefinitions>
    
    <!-- 用户名 -->
    <TextBlock Text="用户名：" Grid.Row="0" Grid.Column="0" VerticalAlignment="Center"/>
    <TextBox Grid.Row="0" Grid.Column="1" Text="{Binding UserName}"/>
    
    <!-- 邮箱 -->
    <TextBlock Text="邮箱：" Grid.Row="1" Grid.Column="0" VerticalAlignment="Center"/>
    <TextBox Grid.Row="1" Grid.Column="1" Text="{Binding Email}"/>
    
    <!-- 电话 -->
    <TextBlock Text="电话：" Grid.Row="2" Grid.Column="0" VerticalAlignment="Center"/>
    <TextBox Grid.Row="2" Grid.Column="1" Text="{Binding Phone}"/>
    
    <!-- 地址 -->
    <TextBlock Text="地址：" Grid.Row="3" Grid.Column="0" VerticalAlignment="Top"/>
    <TextBox Grid.Row="3" Grid.Column="1" 
             Height="60" AcceptsReturn="true"
             Text="{Binding Address}"/>
    
    <!-- 按钮 -->
    <StackPanel Grid.Row="4" Grid.Column="1" 
                Orientation="Horizontal" HorizontalAlignment="Right" Spacing="8">
        <Button Content="取消"/>
        <Button Content="保存" Style="Primary"/>
    </StackPanel>
</Grid>
```

### 三栏布局

```xml
<Grid>
    <Grid.ColumnDefinitions>
        <ColumnDefinition Width="200"/>    <!-- 左侧边栏（固定宽度） -->
        <ColumnDefinition Width="*"/>      <!-- 主内容区（自适应） -->
        <ColumnDefinition Width="250"/>    <!-- 右侧边栏（固定宽度） -->
    </Grid.ColumnDefinitions>
    
    <!-- 左侧导航 -->
    <Border Grid.Column="0" Background="#2C3E50">
        <StackPanel Orientation="Vertical">
            <Button Content="首页" Style="NavButton"/>
            <Button Content="设置" Style="NavButton"/>
            <Button Content="关于" Style="NavButton"/>
        </StackPanel>
    </Border>
    
    <!-- 主内容 -->
    <ScrollViewer Grid.Column="1">
        <StackPanel Margin="24">
            <!-- 内容 -->
        </StackPanel>
    </ScrollViewer>
    
    <!-- 右侧信息面板 -->
    <Border Grid.Column="2" Background="#F5F5F5">
        <StackPanel Margin="16">
            <TextBlock Text="详细信息" FontWeight="Bold"/>
            <!-- 详情内容 -->
        </StackPanel>
    </Border>
</Grid>
```

### 响应式布局

```xml
<Grid>
    <Grid.RowDefinitions>
        <RowDefinition Height="Auto"/>     <!-- 头部 -->
        <RowDefinition Height="*"/>        <!-- 内容 -->
        <RowDefinition Height="Auto"/>     <!-- 底部 -->
    </Grid.RowDefinitions>
    
    <!-- 头部工具栏 -->
    <Border Grid.Row="0" Height="48" Background="#0078D4">
        <StackPanel Orientation="Horizontal" Spacing="8">
            <Button Content="新建" Style="ToolbarButton"/>
            <Button Content="打开" Style="ToolbarButton"/>
        </StackPanel>
    </Border>
    
    <!-- 主内容区 -->
    <Grid Grid.Row="1">
        <Grid.ColumnDefinitions>
            <ColumnDefinition Width="Auto" MinWidth="150" MaxWidth="300"/>
            <ColumnDefinition Width="*"/>
        </Grid.ColumnDefinitions>
        
        <!-- 可折叠侧边栏 -->
        <Border Grid.Column="0" Background="#F0F0F0">
            <!-- 树形导航 -->
        </Border>
        
        <!-- 工作区 -->
        <Grid Grid.Column="1">
            <!-- ... -->
        </Grid>
    </Grid>
    
    <!-- 状态栏 -->
    <Border Grid.Row="2" Height="24" Background="#E0E0E0">
        <TextBlock Text="就绪" Margin="8,0"/>
    </Border>
</Grid>
```

## 代码操作

### 动态添加行列

```cpp
auto grid = std::make_shared<luaui::controls::Grid>();

// 添加三列
grid->AddColumn(luaui::controls::GridLength::Auto());
grid->AddColumn(luaui::controls::GridLength::Star(1.0f));
grid->AddColumn(luaui::controls::GridLength::Pixel(100));

// 添加两行
grid->AddRow(luaui::controls::GridLength::Auto());
grid->AddRow(luaui::controls::GridLength::Star(1.0f));
```

### 动态设置位置

```cpp
// 添加控件并设置行列
auto control = std::make_shared<luaui::controls::Button>();
control->SetText(L"按钮");
grid->AddChild(control);
grid->SetRow(control, 1);
grid->SetColumn(control, 2);

// 设置跨行列
grid->SetRowSpan(control, 2);
grid->SetColumnSpan(control, 2);

// 获取位置信息
int row = grid->GetRow(control);
int col = grid->GetColumn(control);
```

### 清空行列定义

```cpp
// 清空列定义
grid->ClearColumns();

// 清空行定义
grid->ClearRows();
```

## 最佳实践

### 1. 使用 Auto 适应内容

```xml
<!-- 推荐：标签列用 Auto，输入框列用 Star -->
<Grid>
    <Grid.ColumnDefinitions>
        <ColumnDefinition Width="Auto"/>   <!-- 标签宽度自适应 -->
        <ColumnDefinition Width="*"/>      <!-- 输入框占据剩余空间 -->
    </Grid.ColumnDefinitions>
</Grid>
```

### 2. 设置 MinWidth/MaxWidth

```xml
<!-- 限制侧边栏宽度 -->
<ColumnDefinition Width="Auto" MinWidth="150" MaxWidth="300"/>
```

### 3. 避免过多嵌套

```xml
<!-- 不推荐：Grid 内嵌套多层 StackPanel -->
<Grid>
    <StackPanel>
        <StackPanel>
            <!-- ... -->
        </StackPanel>
    </StackPanel>
</Grid>

<!-- 推荐：直接使用 Grid 的行列 -->
<Grid>
    <Grid.RowDefinitions><!-- ... --></Grid.RowDefinitions>
    <Grid.ColumnDefinitions><!-- ... --></Grid.ColumnDefinitions>
    <!-- 子控件 -->
</Grid>
```

### 4. 使用 Grid.RowSpan/Grid.ColumnSpan

```xml
<!-- 推荐：使用跨行列避免嵌套 -->
<Grid>
    <TextBlock Grid.Row="0" Grid.Column="0" Grid.ColumnSpan="2" Text="标题"/>
</Grid>
```

### 5. 保持行列定义简洁

```xml
<!-- 推荐：简单的行列定义 -->
<Grid Columns="Auto,*" Rows="Auto,*">
    <!-- 子控件 -->
</Grid>

<!-- 或简写形式（如果支持） -->
<Grid Columns="Auto,*" Rows="Auto,*">
```

## 性能优化

1. **避免频繁增删行列**：影响整体布局计算
2. **使用固定尺寸**：减少测量开销
3. **合理使用 Star**：过多的 Star 列会增加计算复杂度

## 参见

- [Panel](Panel.md) - 面板基类
- [StackPanel](Panel.md#stackpanel-堆栈面板) - 堆栈面板
- [Border](Border.md) - 边框容器
- [布局系统](../architecture/Layout_System.md)
