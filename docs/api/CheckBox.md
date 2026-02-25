# CheckBox 复选框 & RadioButton 单选按钮

`CheckBox` 用于布尔值选择，`RadioButton` 用于单选组。

## CheckBox 复选框

### 基本用法

#### XML 中使用

```xml
<!-- 简单复选框 -->
<CheckBox Content="记住我"/>

<!-- 带数据绑定 -->
<CheckBox Content="启用功能" 
          IsChecked="{Binding IsEnabled, Mode=TwoWay}"/>

<!-- 默认选中 -->
<CheckBox Content="同意条款" IsChecked="true"/>

<!-- 禁用状态 -->
<CheckBox Content="选项" IsEnabled="false"/>
```

#### 代码中使用

```cpp
#include "controls/CheckBox.h"

// 创建复选框
auto checkBox = std::make_shared<luaui::controls::CheckBox>();

// 设置文本
checkBox->SetText(L"记住密码");

// 设置选中状态
checkBox->SetIsChecked(true);

// 启用三态模式
checkBox->SetIsThreeState(true);

// 绑定选中变更事件
checkBox->CheckedChanged.Add([](luaui::controls::CheckBox* sender, bool isChecked) {
    if (isChecked) {
        // 处理选中
    } else {
        // 处理取消选中
    }
});
```

### 属性

| 属性 | 类型 | 说明 | XML 属性 | 默认值 |
|------|------|------|----------|--------|
| `Text` | wstring | 显示的文本 | `Content` | 空字符串 |
| `IsChecked` | bool | 是否选中 | `IsChecked` | `false` |
| `IsThreeState` | bool | 是否启用三态 | `IsThreeState` | `false` |
| `IsIndeterminate` | bool | 是否为不确定状态（只读）| - | `false` |

### 事件

| 事件 | 参数 | 说明 |
|------|------|------|
| `CheckedChanged` | `CheckBox*, bool` | 选中状态变更时触发 |

### 三态复选框

```xml
<!-- 三态复选框：选中/未选中/不确定 -->
<CheckBox IsThreeState="true" 
          IsChecked="{Binding SelectAllState}"/>
```

三态模式用于表示子项部分选中的情况：
- `true` - 全部选中（显示勾选）
- `false` - 全部未选中（显示空框）
- `null` - 部分选中（显示横线）

### 数据绑定

```xml
<!-- 双向绑定 -->
<CheckBox Content="自动登录" 
          IsChecked="{Binding AutoLogin, Mode=TwoWay}"/>

<!-- 绑定到可空布尔值 -->
<CheckBox Content="全选" 
          IsThreeState="true"
          IsChecked="{Binding IsAllSelected}"/>
```

---

## RadioButton 单选按钮

### 基本用法

#### XML 中使用

```xml
<!-- 单选按钮组 -->
<StackPanel Orientation="Vertical" Spacing="8">
    <TextBlock Text="性别："/>
    <RadioButton Content="男" GroupName="Gender" IsChecked="true"/>
    <RadioButton Content="女" GroupName="Gender"/>
    <RadioButton Content="保密" GroupName="Gender"/>
</StackPanel>

<!-- 带数据绑定 -->
<RadioButton Content="选项 A" 
             GroupName="Options"
             IsChecked="{Binding SelectedOption, Converter=OptionConverter}"/>
```

#### 代码中使用

```cpp
#include "controls/CheckBox.h"  // RadioButton 在同文件中定义

// 创建单选按钮
auto radioBtn = std::make_shared<luaui::controls::RadioButton>();

// 设置文本
radioBtn->SetText(L"选项 1");

// 设置分组名称（同组内只能选一个）
radioBtn->SetGroupName("Group1");

// 设置选中
radioBtn->SetIsChecked(true);

// 绑定选中变更事件
radioBtn->CheckedChanged.Add([](luaui::controls::RadioButton* sender, bool isChecked) {
    if (isChecked) {
        // 处理选中
    }
});
```

### 属性

| 属性 | 类型 | 说明 | XML 属性 | 默认值 |
|------|------|------|----------|--------|
| `Text` | wstring | 显示的文本 | `Content` | 空字符串 |
| `IsChecked` | bool | 是否选中 | `IsChecked` | `false` |
| `GroupName` | string | 分组名称 | `GroupName` | 空字符串 |

### 事件

| 事件 | 参数 | 说明 |
|------|------|------|
| `CheckedChanged` | `RadioButton*, bool` | 选中状态变更时触发 |

### 分组机制

```xml
<!-- 第一组：性别 -->
<StackPanel>
    <RadioButton Content="男" GroupName="Gender" IsChecked="true"/>
    <RadioButton Content="女" GroupName="Gender"/>
</StackPanel>

<!-- 第二组：支付方式（独立分组） -->
<StackPanel>
    <RadioButton Content="支付宝" GroupName="Payment" IsChecked="true"/>
    <RadioButton Content="微信支付" GroupName="Payment"/>
    <RadioButton Content="银行卡" GroupName="Payment"/>
</StackPanel>
```

---

## 使用场景

### 设置页面

```xml
<StackPanel Orientation="Vertical" Spacing="12" Margin="24">
    <TextBlock Text="通知设置" FontSize="18" FontWeight="Bold"/>
    
    <CheckBox Content="启用推送通知" 
              IsChecked="{Binding PushEnabled, Mode=TwoWay}"/>
    <CheckBox Content="启用邮件通知" 
              IsChecked="{Binding EmailEnabled, Mode=TwoWay}"/>
    <CheckBox Content="启用短信通知" 
              IsChecked="{Binding SmsEnabled, Mode=TwoWay}"/>
    
    <Separator/>
    
    <TextBlock Text="通知频率" FontSize="16"/>
    <RadioButton Content="实时" GroupName="Frequency" IsChecked="true"/>
    <RadioButton Content="每小时汇总" GroupName="Frequency"/>
    <RadioButton Content="每日汇总" GroupName="Frequency"/>
</StackPanel>
```

### 表单选项

```xml
<StackPanel Spacing="8">
    <!-- 复选框列表 -->
    <TextBlock Text="兴趣爱好（可多选）："/>
    <CheckBox Content="阅读" IsChecked="{Binding HobbyReading}"/>
    <CheckBox Content="音乐" IsChecked="{Binding HobbyMusic}"/>
    <CheckBox Content="运动" IsChecked="{Binding HobbySports}"/>
    <CheckBox Content="旅游" IsChecked="{Binding HobbyTravel}"/>
    
    <Separator/>
    
    <!-- 单选 -->
    <TextBlock Text="学历："/>
    <RadioButton Content="高中及以下" GroupName="Education"/>
    <RadioButton Content="本科" GroupName="Education" IsChecked="true"/>
    <RadioButton Content="硕士" GroupName="Education"/>
    <RadioButton Content="博士及以上" GroupName="Education"/>
</StackPanel>
```

### 列表选择

```xml
<!-- 带全选功能的多选列表 -->
<StackPanel>
    <CheckBox Content="全选" 
              IsThreeState="true"
              IsChecked="{Binding IsAllSelected}"/>
    
    <Border Background="#F5F5F5" Padding="8">
        <StackPanel>
            <CheckBox Content="项目 1" IsChecked="{Binding Item1Selected}"/>
            <CheckBox Content="项目 2" IsChecked="{Binding Item2Selected}"/>
            <CheckBox Content="项目 3" IsChecked="{Binding Item3Selected}"/>
        </StackPanel>
    </Border>
</StackPanel>
```

## 最佳实践

### 1. 复选框用于多选，单选按钮用于互斥选择

```xml
<!-- 正确：使用复选框（可多选） -->
<CheckBox Content="选项 A"/>
<CheckBox Content="选项 B"/>
<CheckBox Content="选项 C"/>

<!-- 正确：使用单选按钮（互斥） -->
<RadioButton Content="选项 1" GroupName="Group"/>
<RadioButton Content="选项 2" GroupName="Group"/>
<RadioButton Content="选项 3" GroupName="Group"/>
```

### 2. 使用 TwoWay 绑定

```xml
<!-- 推荐 -->
<CheckBox IsChecked="{Binding IsEnabled, Mode=TwoWay}"/>

<!-- 不推荐（单向绑定无法更新 ViewModel） -->
<CheckBox IsChecked="{Binding IsEnabled}"/>
```

### 3. 提供明确的标签文本

```xml
<!-- 推荐 -->
<CheckBox Content="我同意用户协议和隐私政策"/>

<!-- 不推荐 -->
<CheckBox Content="同意"/>
```

### 4. 单选按钮必须有默认选中项

```xml
<!-- 推荐 -->
<RadioButton Content="选项 A" IsChecked="true"/>
<RadioButton Content="选项 B"/>

<!-- 不推荐 -->
<RadioButton Content="选项 A"/>
<RadioButton Content="选项 B"/>
```

### 5. 分组名称要有意义

```xml
<!-- 推荐 -->
<RadioButton GroupName="PaymentMethod"/>

<!-- 不推荐 -->
<RadioButton GroupName="Group1"/>
```

## 样式定制

```xml
<Window.Resources>
    <!-- 复选框样式 -->
    <Style TargetType="CheckBox">
        <Setter Property="FontSize" Value="14"/>
        <Setter Property="Spacing" Value="8"/>
    </Style>
    
    <!-- 单选按钮样式 -->
    <Style TargetType="RadioButton">
        <Setter Property="FontSize" Value="14"/>
        <Setter Property="CircleSize" Value="16"/>
    </Style>
</Window.Resources>
```

## 参见

- [Button](Button.md) - 按钮控件
- [TextBlock](TextBlock.md) - 文本显示
- [MVVM 数据绑定](../architecture/MVVM_DataBinding_Design.md)
