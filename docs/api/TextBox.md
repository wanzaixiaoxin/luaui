# TextBox 文本输入控件

`TextBox` 是可编辑的文本输入控件，支持键盘输入、选择和光标控制。

## 基本用法

### XML 中使用

```xml
<!-- 简单输入框 -->
<TextBox Text="默认文本"/>

<!-- 带占位符 -->
<TextBox Placeholder="请输入用户名"/>

<!-- 密码输入 -->
<TextBox IsPassword="true" Placeholder="请输入密码"/>

<!-- 数据绑定 -->
<TextBox Text="{Binding UserName, Mode=TwoWay}"/>

<!-- 只读 -->
<TextBox Text="{Binding DisplayText}" IsReadOnly="true"/>

<!-- 限制长度 -->
<TextBox Text="{Binding Phone}" MaxLength="11"/>
```

### 代码中使用

```cpp
#include "controls/TextBox.h"

// 创建输入框
auto textBox = std::make_shared<luaui::controls::TextBox>();

// 设置文本
textBox->SetText(L"默认内容");

// 设置占位符
textBox->SetPlaceholder(L"请输入...");

// 设置为密码模式
textBox->SetIsPassword(true);

// 设置为只读
textBox->SetIsReadOnly(true);

// 设置最大长度
textBox->SetMaxLength(100);

// 绑定文本变更事件
textBox->TextChanged.Add([](luaui::controls::TextBox* sender, const std::wstring& newText) {
    // 处理文本变更
});
```

## 属性

| 属性 | 类型 | 说明 | XML 属性 | 默认值 |
|------|------|------|----------|--------|
| `Text` | wstring | 当前文本内容 | `Text` | 空字符串 |
| `Placeholder` | wstring | 占位提示文本 | `Placeholder` | 空字符串 |
| `IsReadOnly` | bool | 是否只读 | `IsReadOnly` | `false` |
| `IsPassword` | bool | 是否为密码模式 | `IsPassword` | `false` |
| `MaxLength` | int | 最大字符数（0=无限制）| `MaxLength` | 0 |
| `CaretPosition` | int | 光标位置 | - | 0 |

### 选择相关属性

| 属性 | 类型 | 说明 |
|------|------|------|
| `HasSelection` | bool | 是否有选中的文本（只读）|
| `SelectionStart` | int | 选区起始位置 |
| `SelectionEnd` | int | 选区结束位置 |

## 事件

| 事件 | 参数 | 说明 |
|------|------|------|
| `TextChanged` | `TextBox*, const wstring&` | 文本内容变更时触发 |
| `GotFocus` | - | 获得输入焦点时触发 |
| `LostFocus` | - | 失去输入焦点时触发 |
| `KeyDown` | `KeyEventArgs` | 键盘按键按下时触发 |

### 事件使用示例

```cpp
// 文本变更事件
textBox->TextChanged.Add([this](luaui::controls::TextBox* sender, const std::wstring& text) {
    // 验证输入
    if (text.length() < 3) {
        // 显示错误提示
    }
});

// 焦点事件
textBox->GotFocus.Add([](luaui::Control*) {
    // 高亮显示
});

textBox->LostFocus.Add([this](luaui::Control*) {
    // 验证输入
    ValidateInput();
});
```

## XML 配置

```xml
<!-- 基础配置 -->
<TextBox Width="200" Height="32"/>

<!-- 占位符 -->
<TextBox Placeholder="请输入邮箱地址"/>

<!-- 密码输入 -->
<TextBox IsPassword="true" Placeholder="密码"/>

<!-- 数字输入 -->
<TextBox Text="{Binding Age}" MaxLength="3"/>

<!-- 多行输入 -->
<TextBox Text="{Binding Description}" 
         AcceptsReturn="true"
         Height="100"
         VerticalScrollBarVisibility="Auto"/>

<!-- 自动换行 -->
<TextBox TextWrapping="Wrap" Width="300"/>
```

## 数据绑定

```xml
<!-- 双向绑定（输入控件的默认模式） -->
<TextBox Text="{Binding UserName, Mode=TwoWay}"/>

<!-- 延迟更新（失去焦点后才更新 ViewModel） -->
<TextBox Text="{Binding SearchText, UpdateSourceTrigger=LostFocus}"/>

<!-- 实时更新（每次输入都更新） -->
<TextBox Text="{Binding SearchText, UpdateSourceTrigger=PropertyChanged}"/>

<!-- 带验证 -->
<TextBox Text="{Binding Email, ValidatesOnDataErrors=True}"/>
```

## 输入模式

### 密码输入

```xml
<!-- 密码输入框 -->
<TextBox IsPassword="true" Placeholder="请输入密码"/>

<!-- 带确认密码 -->
<StackPanel Orientation="Vertical" Spacing="8">
    <TextBlock Text="密码"/>
    <TextBox IsPassword="true" Text="{Binding Password}"/>
    <TextBlock Text="确认密码"/>
    <TextBox IsPassword="true" Text="{Binding ConfirmPassword}"/>
</StackPanel>
```

### 只读显示

```xml
<!-- 只读文本显示（可复制） -->
<TextBox Text="{Binding OrderId}" IsReadOnly="true"/>

<!-- 样式化只读文本 -->
<TextBox Text="{Binding Status}" 
         IsReadOnly="true"
         Background="Transparent"
         BorderThickness="0"/>
```

### 数字输入

```xml
<!-- 整数输入 -->
<TextBox Text="{Binding Quantity}" MaxLength="5"/>

<!-- 带值转换器 -->
<TextBox Text="{Binding Price, Converter=DecimalConverter}"/>
```

## 方法

| 方法 | 说明 |
|------|------|
| `SelectAll()` | 全选文本 |
| `ClearSelection()` | 清除选择 |
| `SetCaretPosition(int)` | 设置光标位置 |
| `InsertText(wstring)` | 在光标位置插入文本 |
| `DeleteSelection()` | 删除选中的文本 |

```cpp
// 全选
textBox->SelectAll();

// 设置光标到末尾
textBox->SetCaretPosition(textBox->GetText().length());

// 在光标位置插入文本
textBox->InsertText(L"插入的内容");

// 删除选中的内容
textBox->DeleteSelection();
```

## 常见使用场景

### 登录表单

```xml
<StackPanel Orientation="Vertical" Spacing="12" Width="300">
    <TextBlock Text="用户名"/>
    <TextBox Text="{Binding UserName, Mode=TwoWay}" 
             Placeholder="请输入用户名"/>
    
    <TextBlock Text="密码"/>
    <TextBox IsPassword="true"
             Text="{Binding Password, Mode=TwoWay}"
             Placeholder="请输入密码"/>
    
    <Button Content="登录" Click="LoginCommand" Style="Primary"/>
</StackPanel>
```

### 搜索框

```xml
<StackPanel Orientation="Horizontal" Spacing="8">
    <TextBox Text="{Binding SearchText, Mode=TwoWay, UpdateSourceTrigger=PropertyChanged}"
             Placeholder="搜索..."
             Width="250"/>
    <Button Content="搜索" Click="SearchCommand"/>
</StackPanel>
```

### 个人资料编辑

```xml
<Grid Columns="Auto,*" Rows="Auto,Auto,Auto,Auto" Spacing="8,12">
    <TextBlock Text="昵称：" Grid.Row="0" Grid.Column="0" VerticalAlignment="Center"/>
    <TextBox Text="{Binding Nickname}" Grid.Row="0" Grid.Column="1"/>
    
    <TextBlock Text="邮箱：" Grid.Row="1" Grid.Column="0" VerticalAlignment="Center"/>
    <TextBox Text="{Binding Email}" Grid.Row="1" Grid.Column="1"/>
    
    <TextBlock Text="简介：" Grid.Row="2" Grid.Column="0" VerticalAlignment="Top"/>
    <TextBox Text="{Binding Bio}" 
             Grid.Row="2" Grid.Column="1"
             Height="80"
             AcceptsReturn="true"
             TextWrapping="Wrap"/>
</Grid>
```

## 样式定制

```xml
<Window.Resources>
    <Style TargetType="TextBox">
        <Setter Property="FontSize" Value="14"/>
        <Setter Property="Padding" Value="8,4"/>
        <Setter Property="BorderColor" Value="#CCCCCC"/>
        <Setter Property="FocusedBorderColor" Value="#0078D4"/>
    </Style>
    
    <Style x:Key="SearchBox" TargetType="TextBox">
        <Setter Property="Background" Value="#F5F5F5"/>
        <Setter Property="CornerRadius" Value="16"/>
        <Setter Property="Padding" Value="12,6"/>
    </Style>
</Window.Resources>
```

## 最佳实践

1. **使用占位符**：为空输入框提供提示
   ```xml
   <TextBox Placeholder="请输入用户名"/>
   ```

2. **合理设置宽度**：避免输入框过宽或过窄
   ```xml
   <!-- 用户名：适中宽度 -->
   <TextBox Width="200"/>
   
   <!-- 邮箱：较宽 -->
   <TextBox Width="280"/>
   
   <!-- 验证码：较窄 -->
   <TextBox Width="100"/>
   ```

3. **密码必须加密显示**
   ```xml
   <TextBox IsPassword="true" Text="{Binding Password}"/>
   ```

4. **限制输入长度**
   ```xml
   <TextBox Text="{Binding Phone}" MaxLength="11"/>
   ```

5. **使用 TwoWay 绑定**
   ```xml
   <TextBox Text="{Binding Value, Mode=TwoWay}"/>
   ```

## 参见

- [TextBlock](TextBlock.md) - 只读文本显示
- [CheckBox](CheckBox.md) - 复选框控件
- [数据绑定](../architecture/MVVM_DataBinding_Design.md)
