# XML 布局系统设计文档

## 概述

XML 布局系统为 LuaUI 提供声明式的 UI 定义能力，类似于 WPF 的 XAML。通过 XML，开发者可以：

1. **分离 UI 和逻辑** - 界面设计由设计师负责，业务逻辑由开发者负责
2. **可视化工具支持** - 为未来的可视化设计器奠定基础
3. **提高可读性** - 层级结构直观，易于理解
4. **支持 MVVM** - 与数据绑定无缝集成

---

## 设计目标

| 目标 | 说明 |
|------|------|
| **简单性** | 学习成本低，快速上手 |
| **完整性** | 覆盖所有控件和属性 |
| **可扩展** | 支持自定义控件和标记扩展 |
| **性能** | 解析快，内存占用低 |
| **调试友好** | 错误信息清晰，定位准确 |

---

## 核心架构

```
┌─────────────────────────────────────────────────────────────┐
│                     XML Layout System                        │
├─────────────────────────────────────────────────────────────┤
│  XML Parser (TinyXML2)                                       │
│  └── 解析 XML 文本 → DOM 树                                  │
├─────────────────────────────────────────────────────────────┤
│  Layout Loader                                               │
│  ├── Element Factory (控件工厂)                              │
│  ├── Property Setter (属性设置)                              │
│  ├── Markup Extension (标记扩展处理)                         │
│  └── Code-Behind Connector (代码后置连接)                    │
├─────────────────────────────────────────────────────────────┤
│  Binding Infrastructure (绑定基础设施)                       │
│  └── {Binding} 表达式解析 (为 MVVM 准备)                     │
└─────────────────────────────────────────────────────────────┘
```

---

## 1. XML 格式规范

### 1.1 基本结构

```xml
<?xml version="1.0" encoding="UTF-8"?>
<Window xmlns="http://luaui.io/schema"
        Title="Hello World"
        Width="800" 
        Height="600">
    
    <StackPanel Orientation="Vertical" Margin="20">
        <TextBlock Text="Welcome to LuaUI" FontSize="24"/>
        <Button Content="Click Me" Click="OnButtonClick"/>
    </StackPanel>
    
</Window>
```

### 1.2 命名空间

```xml
<!-- 默认命名空间 -->
<Window xmlns="http://luaui.io/schema">

<!-- 带前缀命名空间（用于扩展） -->
<Window xmlns:local="clr-namespace:MyApp.Controls"
        xmlns:ext="http://luaui.io/extensions">
    <local:CustomControl/>
    <ext:ExtensionElement/>
</Window>
```

### 1.3 属性语法

```xml
<!-- 属性值类型自动推断 -->
<Button 
    Content="Text"           <!-- string -->
    Width="100"              <!-- float -->
    IsEnabled="true"         <!-- bool -->
    Background="#FF0000"     <!-- Color (Hex) -->
    Opacity="0.5"            <!-- float -->
    Tags="1,2,3"             <!-- vector<int> -->
/>
```

### 1.4 属性元素语法

```xml
<!-- 简单属性：属性语法 -->
<Button Background="Blue"/>

<!-- 复杂属性：元素语法 -->
<Button>
    <Button.Background>
        <LinearGradientBrush>
            <GradientStop Color="Blue" Offset="0"/>
            <GradientStop Color="Red" Offset="1"/>
        </LinearGradientBrush>
    </Button.Background>
</Button>
```

---

## 2. 控件映射

### 2.1 基础控件

| XML 元素 | C++ 类 | 说明 |
|----------|--------|------|
| `Window` | `Window` | 顶层窗口 |
| `StackPanel` | `StackPanel` | 堆叠布局 |
| `Grid` | `Grid` | 网格布局 |
| `Border` | `Border` | 边框装饰器 |
| `Button` | `Button` | 按钮 |
| `TextBlock` | `TextBlock` | 文本块 |
| `TextBox` | `TextBox` | 文本输入框 |
| `CheckBox` | `CheckBox` | 复选框 |
| `RadioButton` | `RadioButton` | 单选按钮 |
| `Slider` | `Slider` | 滑块 |
| `Image` | `Image` | 图片 |
| `Canvas` | `Canvas` | 画布 |

### 2.2 属性映射示例

```xml
<Button 
    x:Name="submitBtn"           <!-- 命名元素 -->
    Content="Submit"             <!-- 属性映射 -->
    Width="120"                  <!-- 基础类型 -->
    Height="40"
    IsEnabled="true"             <!-- bool -->
    Margin="10,5,10,5"           <!-- Thickness -->
    Background="#0078D4"         <!-- Color (Hex) -->
    Click="OnSubmitClick"        <!-- 事件连接 -->
/>
```

---

## 3. 标记扩展 (Markup Extensions)

### 3.1 {Binding} - 数据绑定

```xml
<!-- 简单绑定 -->
<TextBlock Text="{Binding UserName}"/>

<!-- 带模式 -->
<TextBox Text="{Binding UserName, Mode=TwoWay}"/>

<!-- 带转换器 -->
<TextBlock Text="{Binding Status, Converter={StaticResource StatusConverter}}"/>

<!-- 相对源 -->
<TextBlock Text="{Binding DataContext.Title, RelativeSource={RelativeSource AncestorType=Window}}"/>
```

### 3.2 {StaticResource}

```xml
<Window.Resources>
    <Color x:Key="PrimaryColor">#0078D4</Color>
    <SolidColorBrush x:Key="PrimaryBrush" Color="{StaticResource PrimaryColor}"/>
</Window.Resources>

<Button Background="{StaticResource PrimaryBrush}"/>
```

### 3.3 {x:Name}

```xml
<Button x:Name="okButton" Content="OK"/>
<!-- 代码中访问：okButton->SetText("Confirm"); -->
```

---

## 4. 代码后置 (Code-Behind)

### 4.1 基本结构

**MainWindow.xml:**
```xml
<Window x:Class="MyApp.MainWindow"
        xmlns="http://luaui.io/schema"
        Title="My App">
    <Button x:Name="submitBtn" Content="Submit" Click="OnSubmitClick"/>
</Window>
```

**MainWindow.h:**
```cpp
#pragma once
#include "luaui/xml/XmlWindow.h"

namespace MyApp {

class MainWindow : public luaui::xml::XmlWindow {
public:
    MainWindow();
    
    // 自动生成的初始化
    void InitializeComponent();
    
    // 事件处理
    void OnSubmitClick(luaui::controls::Control* sender, luaui::controls::RoutedEventArgs& e);
    
private:
    // 命名元素（自动生成）
    luaui::controls::ButtonPtr submitBtn;
};

} // namespace MyApp
```

**MainWindow.cpp:**
```cpp
#include "MainWindow.h"

namespace MyApp {

MainWindow::MainWindow() {
    InitializeComponent();
}

void MainWindow::OnSubmitClick(Control* sender, RoutedEventArgs& e) {
    submitBtn->SetContent("Processing...");
    // 业务逻辑
}

} // namespace MyApp
```

### 4.2 部分类生成（可选）

```cpp
// MainWindow.g.h (自动生成)
#pragma once
namespace MyApp {

class MainWindow_base {
protected:
    void InitializeComponent() {
        // 加载 XML
        // 创建控件
        // 连接事件
    }
    
    luaui::controls::ButtonPtr submitBtn;
};

} // namespace MyApp
```

---

## 5. MVVM 准备

### 5.1 View (XML)

```xml
<Window xmlns="http://luaui.io/schema"
        xmlns:vm="clr-namespace:MyApp.ViewModels"
        Title="User Profile">
    
    <Window.DataContext>
        <vm:UserProfileViewModel/>
    </Window.DataContext>
    
    <StackPanel>
        <TextBlock Text="{Binding UserName}" FontSize="20"/>
        <TextBox Text="{Binding Email, Mode=TwoWay}"/>
        <Button Content="Save" Command="{Binding SaveCommand}"/>
    </StackPanel>
</Window>
```

### 5.2 ViewModel (C++)

```cpp
#pragma once
#include "luaui/mvvm/ViewModelBase.h"
#include "luaui/mvvm/RelayCommand.h"

namespace MyApp {

class UserProfileViewModel : public luaui::mvvm::ViewModelBase {
public:
    UserProfileViewModel();
    
    // 属性（自动通知）
    PROPERTY(std::wstring, UserName)
    PROPERTY(std::wstring, Email)
    
    // 命令
    luaui::mvvm::CommandPtr SaveCommand;

private:
    void OnSave();
    bool CanSave() const;
};

} // namespace MyApp
```

### 5.3 绑定连接

```cpp
// XML 加载时自动建立绑定
// TextBox.Text <-> ViewModel.Email
// 变更自动同步
```

---

## 6. 实现细节

### 6.1 类图

```
IXmlLoader                    IMarkupExtension
    │                              │
    ▼                              ▼
LayoutLoader              BindingExtension
    │                              │
    ├── ElementFactory             ├── StaticResourceExtension
    ├── PropertySetter             └── RelativeSourceExtension
    └── MarkupExtensionParser
```

### 6.2 关键接口

```cpp
// XML 加载器
class IXmlLoader {
public:
    virtual ControlPtr Load(const std::string& xmlPath) = 0;
    virtual ControlPtr LoadFromString(const std::string& xml) = 0;
};

// 元素工厂
class IElementFactory {
public:
    using CreateFunc = std::function<ControlPtr()>;
    
    virtual void Register(const std::string& tag, CreateFunc func) = 0;
    virtual ControlPtr Create(const std::string& tag) = 0;
};

// 标记扩展
class IMarkupExtension {
public:
    virtual std::any ProvideValue(IProvideValueTarget* target) = 0;
};

// 属性设置器
class IPropertySetter {
public:
    virtual void Set(Control* control, const std::string& property, const std::any& value) = 0;
};
```

### 6.3 解析流程

```
1. 读取 XML 文件
        ↓
2. TinyXML2 解析为 DOM
        ↓
3. 遍历 DOM 树
        ↓
4. 对每个元素：
   a. 通过 Factory 创建控件
   b. 解析属性（处理 MarkupExtension）
   c. 设置属性值
   d. 建立父子关系
        ↓
5. 返回根控件
```

---

## 7. 错误处理

### 7.1 错误类型

| 错误 | 示例 | 处理 |
|------|------|------|
| XML 语法错误 | 未闭合标签 | TinyXML2 报告行号 |
| 未知元素 | `<UnknownControl/>` | 警告，跳过或抛异常 |
| 未知属性 | `UnknownAttr="value"` | 警告，忽略 |
| 类型转换失败 | `Width="abc"` | 错误，提示期望值类型 |
| 缺少必需属性 | `<Button/>` (无 Content) | 警告或使用默认值 |
| 循环引用 | A 引用 B，B 引用 A | 检测并抛异常 |

### 7.2 错误报告示例

```
Error: MainWindow.xml(12,23)
  Element: <Button>
  Attribute: Width
  Value: "abc"
  Expected: float (e.g., "100", "100.5")
  
Error: MainWindow.xml(15,5)
  Unknown element: <CustomControl>
  Did you mean: <Button>, <Border>?
  Hint: Register custom control with XmlLoader::RegisterElement()
```

---

## 8. 示例程序

### 8.1 基础布局 (02_mvvm_demo)

**layout_basic.xml:**
```xml
<?xml version="1.0" encoding="UTF-8"?>
<Window xmlns="http://luaui.io/schema"
        Title="XML Layout Demo"
        Width="800" Height="600">
    
    <Grid>
        <Grid.RowDefinitions>
            <RowDefinition Height="Auto"/>
            <RowDefinition Height="*"/>
            <RowDefinition Height="Auto"/>
        </Grid.RowDefinitions>
        
        <!-- Header -->
        <Border Grid.Row="0" Background="#333333" Padding="20">
            <TextBlock Text="XML Layout System" 
                       Foreground="White" 
                       FontSize="24"/>
        </Border>
        
        <!-- Content -->
        <StackPanel Grid.Row="1" Margin="20" Spacing="10">
            <TextBlock Text="Welcome to the XML layout demo!" 
                       FontSize="16"/>
            <Button Content="Blue Theme" 
                    SetStateColors="#0078D4,#005A9E,#004578"
                    Padding="15,8"/>
            <Button Content="Red Theme"
                    SetStateColors="#D83B01,#A52C00,#7A1F00"
                    Padding="15,8"/>
        </StackPanel>
        
        <!-- Footer -->
        <StackPanel Grid.Row="2" 
                    Orientation="Horizontal" 
                    HorizontalAlignment="Right"
                    Margin="20">
            <Button Content="OK" Width="80"/>
            <Button Content="Cancel" Width="80" Margin="10,0,0,0"/>
        </StackPanel>
    </Grid>
</Window>
```

**main.cpp:**
```cpp
#include "luaui/xml/XmlLoader.h"

int main() {
    auto loader = luaui::xml::CreateXmlLoader();
    auto window = loader->Load("layout_basic.xml");
    
    // 显示窗口
    window->Show();
    return 0;
}
```

### 8.2 带代码后置

**MainWindow.xml:**
```xml
<Window x:Class="MyApp.MainWindow"
        xmlns="http://luaui.io/schema"
        Title="Code-Behind Demo">
    <Button x:Name="toggleBtn" 
            Content="Toggle Color"
            Click="OnToggleClick"/>
</Window>
```

**MainWindow.cpp:**
```cpp
#include "MainWindow.h"

void MainWindow::OnToggleClick(Control* sender, RoutedEventArgs& e) {
    static bool isBlue = true;
    toggleBtn->SetStateColors(
        isBlue ? Color::Purple() : Color::Blue(),
        isBlue ? Color::DarkPurple() : Color::DarkBlue(),
        isBlue ? Color::DarkerPurple() : Color::DarkerBlue()
    );
    isBlue = !isBlue;
}
```

---

## 9. 性能考虑

### 9.1 优化策略

| 策略 | 说明 |
|------|------|
| 延迟解析 | 只在需要时解析复杂属性 |
| 对象池 | 复用常用控件实例 |
| 属性缓存 | 缓存反射结果 |
| 二进制缓存 | 可选：编译为二进制格式 |

### 9.2 性能目标

- 解析 100KB XML：< 10ms
- 创建 100 个控件：< 5ms
- 内存开销：< 20% 比等效 C++ 代码

---

## 10. 扩展指南

### 10.1 注册自定义控件

```cpp
// 注册自定义控件
loader->RegisterElement("CustomButton", []() {
    return std::make_shared<CustomButton>();
});

// 注册自定义属性
loader->RegisterProperty<CustomButton, Color>(
    "GlowColor",
    &CustomButton::GetGlowColor,
    &CustomButton::SetGlowColor
);
```

### 10.2 自定义标记扩展

```cpp
class TranslateExtension : public IMarkupExtension {
public:
    TranslateExtension(const std::string& key) : m_key(key) {}
    
    std::any ProvideValue(IProvideValueTarget* target) override {
        return Localization::GetString(m_key);
    }
    
private:
    std::string m_key;
};

// 注册
loader->RegisterMarkupExtension("Translate", [](const std::string& param) {
    return std::make_unique<TranslateExtension>(param);
});
```

**使用：**
```xml
<TextBlock Text="{Translate WelcomeMessage}"/>
```

---

## 11. 与 MVVM 的集成路线图

### Phase 8 (当前): XML 布局基础
- ✅ XML 解析
- ✅ 控件创建
- ✅ 属性设置
- ✅ 基础标记扩展 ({StaticResource})

### Phase 9 (下一步): MVVM 核心
- 🔄 {Binding} 标记扩展
- 🔄 INotifyPropertyChanged
- 🔄 ICommand / RelayCommand
- 🔄 数据上下文 (DataContext)

### Phase 10: Lua 集成
- 🔄 Lua 绑定 XML 属性
- 🔄 Lua ViewModel
- 🔄 热重载支持

---

## 附录

### A. XML Schema 参考

```xml
<xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema"
           targetNamespace="http://luaui.io/schema"
           xmlns="http://luaui.io/schema">
    
    <xs:element name="Window" type="WindowType"/>
    <xs:element name="Button" type="ButtonType"/>
    <!-- ... -->
    
</xs:schema>
```

### B. 相关文档

- [MVVM_DataBinding_Design.md](./MVVM_DataBinding_Design.md)
- [Commercial_UI_Framework_Design.md](./Commercial_UI_Framework_Design.md)
- [Lua_Binding_Design.md](./Lua_Binding_Design.md)

---

*文档版本: 1.0*  
*创建日期: 2026-02-10*  
*状态: 设计阶段*
