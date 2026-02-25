# Panel 面板容器 & StackPanel 堆栈面板

`Panel` 是容器控件的基类，`StackPanel` 按水平或垂直方向排列子控件。

## Panel 基类

`Panel` 是所有容器控件的基类，提供子控件管理功能。

### 属性

| 属性 | 类型 | 说明 |
|------|------|------|
| `Children` | vector<IControl> | 子控件集合（只读）|
| `ChildCount` | size_t | 子控件数量（只读）|

### 方法

| 方法 | 说明 |
|------|------|
| `AddChild(control)` | 添加子控件 |
| `RemoveChild(control)` | 移除子控件 |
| `RemoveChildAt(index)` | 按索引移除子控件 |
| `ClearChildren()` | 清空所有子控件 |
| `InsertChild(index, control)` | 在指定位置插入子控件 |
| `GetChild(index)` | 获取指定索引的子控件 |

### 代码中使用

```cpp
#include "controls/Panel.h"

// 创建面板
auto panel = std::make_shared<luaui::controls::Panel>();

// 添加子控件
auto button = std::make_shared<luaui::controls::Button>();
button->SetText(L"按钮");
panel->AddChild(button);

// 添加多个子控件
for (int i = 0; i < 5; i++) {
    auto btn = std::make_shared<luaui::controls::Button>();
    btn->SetText(L"按钮 " + std::to_wstring(i));
    panel->AddChild(btn);
}

// 获取子控件数量
size_t count = panel->GetChildCount();

// 清空子控件
panel->ClearChildren();
```

---

## StackPanel 堆栈面板

`StackPanel` 按水平或垂直方向依次排列子控件。

### 基本用法

#### XML 中使用

```xml
<!-- 垂直排列（默认） -->
<StackPanel Orientation="Vertical" Spacing="8">
    <TextBlock Text="标题"/>
    <TextBox Placeholder="输入内容"/>
    <Button Content="提交"/>
</StackPanel>

<!-- 水平排列 -->
<StackPanel Orientation="Horizontal" Spacing="12">
    <Button Content="保存" Style="Primary"/>
    <Button Content="取消"/>
    <Button Content="帮助" Style="Link"/>
</StackPanel>
```

#### 代码中使用

```cpp
#include "controls/Panel.h"

// 创建垂直堆栈面板
auto vStack = std::make_shared<luaui::controls::StackPanel>();
vStack->SetOrientation(luaui::controls::StackPanel::Orientation::Vertical);
vStack->SetSpacing(8.0f);

// 创建水平堆栈面板
auto hStack = std::make_shared<luaui::controls::StackPanel>();
hStack->SetOrientation(luaui::controls::StackPanel::Orientation::Horizontal);
hStack->SetSpacing(12.0f);
```

### 属性

| 属性 | 类型 | 说明 | XML 属性 | 默认值 |
|------|------|------|----------|--------|
| `Orientation` | Orientation | 排列方向 | `Orientation` | `Vertical` |
| `Spacing` | float | 子控件间距 | `Spacing` | 0 |

### Orientation 枚举

| 值 | 说明 |
|------|------|
| `Vertical` | 垂直排列（从上到下）|
| `Horizontal` | 水平排列（从左到右）|

## 使用场景

### 表单布局

```xml
<StackPanel Orientation="Vertical" Spacing="12" Margin="24">
    <TextBlock Text="用户名" FontWeight="SemiBold"/>
    <TextBox Text="{Binding UserName}" Placeholder="请输入用户名"/>
    
    <TextBlock Text="密码" FontWeight="SemiBold"/>
    <TextBox IsPassword="true" Text="{Binding Password}"/>
    
    <CheckBox Content="记住我" IsChecked="{Binding RememberMe}"/>
    
    <Button Content="登录" Style="Primary"/>
</StackPanel>
```

### 工具栏

```xml
<StackPanel Orientation="Horizontal" Spacing="8" Padding="8">
    <Button Content="新建" Style="Accent"/>
    <Button Content="打开"/>
    <Button Content="保存"/>
    <Separator/>
    <Button Content="剪切"/>
    <Button Content="复制"/>
    <Button Content="粘贴"/>
</StackPanel>
```

### 列表项

```xml
<ScrollViewer Height="400">
    <StackPanel Orientation="Vertical" Spacing="4">
        <Border Background="#F5F5F5" Padding="12">
            <TextBlock Text="项目 1"/>
        </Border>
        <Border Background="#F5F5F5" Padding="12">
            <TextBlock Text="项目 2"/>
        </Border>
        <Border Background="#F5F5F5" Padding="12">
            <TextBlock Text="项目 3"/>
        </Border>
    </StackPanel>
</ScrollViewer>
```

### 嵌套使用

```xml
<StackPanel Orientation="Vertical" Spacing="16">
    <!-- 头部：水平排列 -->
    <StackPanel Orientation="Horizontal" Spacing="8">
        <TextBlock Text="标题" FontSize="20" FontWeight="Bold"/>
        <Button Content="设置" Style="Link"/>
    </StackPanel>
    
    <!-- 内容：垂直排列 -->
    <StackPanel Orientation="Vertical" Spacing="8">
        <TextBlock Text="内容区域"/>
        <TextBox Height="100"/>
    </StackPanel>
    
    <!-- 底部：水平排列 -->
    <StackPanel Orientation="Horizontal" Spacing="8" HorizontalAlignment="Right">
        <Button Content="取消"/>
        <Button Content="确定" Style="Primary"/>
    </StackPanel>
</StackPanel>
```

## 布局行为

### 垂直布局

```xml
<StackPanel Orientation="Vertical" Spacing="8" Width="300">
    <!-- 子控件按垂直方向堆叠 -->
    <!-- 宽度自动填充容器（除非子控件有明确宽度） -->
    <!-- 高度由子控件自身决定 -->
</StackPanel>
```

### 水平布局

```xml
<StackPanel Orientation="Horizontal" Spacing="8" Height="40">
    <!-- 子控件按水平方向排列 -->
    <!-- 高度自动填充容器（除非子控件有明确高度） -->
    <!-- 宽度由子控件自身决定 -->
</StackPanel>
```

### 与 ScrollViewer 配合

```xml
<!-- 垂直滚动 -->
<ScrollViewer VerticalScrollBarVisibility="Auto" MaxHeight="400">
    <StackPanel Orientation="Vertical" Spacing="4">
        <!-- 大量子控件 -->
    </StackPanel>
</ScrollViewer>

<!-- 水平滚动 -->
<ScrollViewer HorizontalScrollBarVisibility="Auto">
    <StackPanel Orientation="Horizontal" Spacing="8">
        <!-- 大量子控件 -->
    </StackPanel>
</ScrollViewer>
```

## 最佳实践

### 1. 合理使用 Spacing

```xml
<!-- 推荐：一致的间距 -->
<StackPanel Spacing="12">
    <TextBlock Text="标题"/>
    <TextBox/>
    <Button/>
</StackPanel>

<!-- 不推荐：过多的小间距 -->
<StackPanel Spacing="2">
    <!-- ... -->
</StackPanel>
```

### 2. 控制嵌套层级

```xml
<!-- 推荐：最多 3-4 层嵌套 -->
<StackPanel>
    <StackPanel>
        <!-- 内容 -->
    </StackPanel>
</StackPanel>

<!-- 不推荐：过深的嵌套影响性能 -->
<StackPanel>
    <StackPanel>
        <StackPanel>
            <StackPanel>
                <!-- 避免这种结构 -->
            </StackPanel>
        </StackPanel>
    </StackPanel>
</StackPanel>
```

### 3. 为复杂布局使用 Grid

```xml
<!-- 简单布局用 StackPanel -->
<StackPanel>
    <TextBlock/>
    <TextBox/>
    <Button/>
</StackPanel>

<!-- 复杂表格布局用 Grid -->
<Grid Columns="Auto,*" Rows="Auto,Auto">
    <!-- ... -->
</Grid>
```

### 4. 设置明确的尺寸

```xml
<!-- 推荐：设置明确尺寸 -->
<StackPanel Width="400" Spacing="12">
    <TextBox Height="32"/>
</StackPanel>

<!-- 不推荐：完全依赖自动尺寸 -->
<StackPanel>
    <!-- 可能导致布局问题 -->
</StackPanel>
```

## 性能考虑

1. **大量子控件**：超过 100 项建议使用虚拟化
   ```xml
   <ScrollViewer>
       <VirtualizingStackPanel>
           <!-- 大量子控件 -->
       </VirtualizingStackPanel>
   </ScrollViewer>
   ```

2. **避免频繁增删子控件**：批量操作前先暂停布局更新

3. **使用缓存**：静态内容可设置 `CacheMode`

## 样式

```xml
<Window.Resources>
    <Style TargetType="StackPanel">
        <Setter Property="Spacing" Value="8"/>
    </Style>
    
    <Style x:Key="FormStackPanel" TargetType="StackPanel">
        <Setter Property="Orientation" Value="Vertical"/>
        <Setter Property="Spacing" Value="12"/>
        <Setter Property="Margin" Value="24"/>
    </Style>
</Window.Resources>
```

## 参见

- [Grid](Grid.md) - 网格布局
- [Border](Border.md) - 边框容器
- [布局系统](../architecture/Layout_System.md)
