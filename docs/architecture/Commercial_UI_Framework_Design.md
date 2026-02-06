# LuaUI 商业级框架架构设计

> 目标：构建一个商业级的 Windows UI 框架，面向未来10年的桌面应用开发需求

## 一、设计哲学

### 1.1 核心原则

```
┌─────────────────────────────────────────────────────────────────┐
│                     LuaUI 设计哲学                               │
├─────────────────────────────────────────────────────────────────┤
│  🚀 流畅 (Fluency)    → 60fps渲染，零卡顿体验                     │
│  📐 简洁 (Simplicity) → 最少代码，最大表达能力                    │
│  ⚡ 高效 (Efficiency) → 快速开发，热重载支持                      │
│  🔒 安全 (Security)   → 沙箱执行，资源隔离                        │
│  ✨ 现代 (Modern)     →  Fluent Design, 亚克力效果               │
└─────────────────────────────────────────────────────────────────┘
```

### 1.2 技术选型对比

| 维度 | MFC方案 | Direct2D方案 | 推荐方案 |
|------|---------|--------------|----------|
| 现代化UI | ⭐⭐ | ⭐⭐⭐⭐⭐ | Direct2D |
| 性能 | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ | Direct2D |
| 开发效率 | ⭐⭐⭐ | ⭐⭐⭐⭐ | 相当 |
| 兼容性 | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ | Direct2D |
| 动画支持 | ⭐⭐ | ⭐⭐⭐⭐⭐ | Direct2D |
| 可维护性 | ⭐⭐ | ⭐⭐⭐⭐⭐ | Direct2D |

**决策**：采用 **Direct2D + DirectComposition** 作为渲染后端，构建自绘UI框架

---

## 二、整体架构

### 2.1 架构全景图

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           应用层 (Application Layer)                          │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │   XML Layout    │   Lua Script    │   Style Sheet   │   Resources     │  │
│  │   (.xml)        │   (.lua)        │   (.theme)      │   (.png/.ttf)   │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
├─────────────────────────────────────────────────────────────────────────────┤
│                           框架层 (Framework Layer)                            │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐ │
│  │  Layout     │  │  Binding    │  │  Animation  │  │  Event System       │ │
│  │  Engine     │  │  Engine     │  │  Engine     │  │  (Routing/Delegate) │ │
│  └─────────────┘  └─────────────┘  └─────────────┘  └─────────────────────┘ │
├─────────────────────────────────────────────────────────────────────────────┤
│                           核心层 (Core Layer)                                 │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐ │
│  │  Control    │  │  Lua VM     │  │  Render     │  │  Resource           │ │
│  │  System     │  │  (Sandbox)  │  │  Engine     │  │  Manager            │ │
│  └─────────────┘  └─────────────┘  └─────────────┘  └─────────────────────┘ │
├─────────────────────────────────────────────────────────────────────────────┤
│                           渲染层 (Render Layer)                               │
│  ┌─────────────────────────────┐  ┌─────────────────────────────────────────┐│
│  │  Direct2D / DirectWrite     │  │  DirectComposition (Window Effects)     ││
│  └─────────────────────────────┘  └─────────────────────────────────────────┘│
├─────────────────────────────────────────────────────────────────────────────┤
│                           平台层 (Platform Layer)                             │
│  ┌─────────────────────────────────────────────────────────────────────────┐│
│  │                    Windows API / Win32 / COM                            ││
│  └─────────────────────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────────────────────┘
```

### 2.2 模块依赖关系

```
                    ┌─────────────────┐
                    │   Application   │
                    └────────┬────────┘
                             │
        ┌────────────────────┼────────────────────┐
        │                    │                    │
        ▼                    ▼                    ▼
┌───────────────┐    ┌───────────────┐    ┌───────────────┐
│  LayoutEngine │    │  ScriptEngine │    │  StyleEngine  │
└───────┬───────┘    └───────┬───────┘    └───────┬───────┘
        │                    │                    │
        └────────────────────┼────────────────────┘
                             │
                             ▼
                    ┌─────────────────┐
                    │  ControlSystem  │
                    └────────┬────────┘
                             │
        ┌────────────────────┼────────────────────┐
        │                    │                    │
        ▼                    ▼                    ▼
┌───────────────┐    ┌───────────────┐    ┌───────────────┐
│ RenderEngine  │    │  LuaRuntime   │    │ResourceManager│
└───────────────┘    └───────────────┘    └───────────────┘
```

---

## 三、核心子系统设计

### 3.1 渲染引擎 (Render Engine)

#### 3.1.1 架构设计

```cpp
// 渲染引擎核心接口
namespace LuaUI {
namespace Render {

class IRenderEngine {
public:
    // 初始化/销毁
    virtual bool Initialize(HWND hwnd) = 0;
    virtual void Shutdown() = 0;
    
    // 渲染循环
    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
    virtual void Present() = 0;
    
    // 2D绘图API
    virtual void DrawRectangle(const Rect& rect, const Brush& brush, float strokeWidth = 1.0f) = 0;
    virtual void FillRectangle(const Rect& rect, const Brush& brush) = 0;
    virtual void DrawRoundedRectangle(const Rect& rect, float radiusX, float radiusY, const Brush& brush) = 0;
    virtual void FillRoundedRectangle(const Rect& rect, float radiusX, float radiusY, const Brush& brush) = 0;
    virtual void DrawText(const std::wstring& text, const Rect& rect, const TextFormat& format, const Brush& brush) = 0;
    virtual void DrawImage(const Image& image, const Rect& destRect, const Rect& srcRect, float opacity = 1.0f) = 0;
    
    // 路径绘制（支持SVG-like路径）
    virtual void DrawPath(const Path& path, const Brush& brush, float strokeWidth = 1.0f) = 0;
    virtual void FillPath(const Path& path, const Brush& brush) = 0;
    
    // 变换
    virtual void PushTransform(const Transform& transform) = 0;
    virtual void PopTransform() = 0;
    
    // 裁剪
    virtual void PushClip(const Rect& rect) = 0;
    virtual void PushClip(const Path& path) = 0;
    virtual void PopClip() = 0;
    
    // 图层（用于效果）
    virtual void PushLayer(const Rect& bounds, float opacity = 1.0f, IEffect* effect = nullptr) = 0;
    virtual void PopLayer() = 0;
    
    // 效果
    virtual void ApplyBlur(float radius) = 0;
    virtual void ApplyDropShadow(const Vector2& offset, float blurRadius, const Color& color) = 0;
    virtual void ApplyColorMatrix(const ColorMatrix& matrix) = 0;
};

}} // namespace LuaUI::Render
```

#### 3.1.2 渲染流程

```
┌─────────────────────────────────────────────────────────────────┐
│                        渲染流程                                  │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐  │
│  │  Update  │───▶│  Layout  │───▶│  Render  │───▶│  Present │  │
│  │  (16ms)  │    │  (Pass)  │    │  (Pass)  │    │  (VSync) │  │
│  └──────────┘    └──────────┘    └──────────┘    └──────────┘  │
│       │                               │                         │
│       ▼                               ▼                         │
│  ┌──────────┐                   ┌──────────────┐               │
│  │  Animation│                   │  Draw Calls  │               │
│  │  System   │                   │  Batching    │               │
│  └──────────┘                   └──────────────┘               │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 3.2 控件系统 (Control System)

#### 3.2.1 控件继承体系

```
┌─────────────────────────────────────────────────────────────────┐
│                        控件继承体系                              │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│                         UIElement                               │
│                    (基类，基础属性)                              │
│                            │                                    │
│         ┌──────────────────┼──────────────────┐                │
│         │                  │                  │                │
│         ▼                  ▼                  ▼                │
│    FrameworkElement    Shape            VisualLayer             │
│    (布局/样式支持)    (几何图形)        (特效层)                │
│         │                                                      │
│    ┌────┴────┬────────┬────────┬────────┬────────┐            │
│    │         │        │        │        │        │            │
│    ▼         ▼        ▼        ▼        ▼        ▼            │
│ Control   Panel    TextBlock  Image   Border   ItemsControl   │
│ (交互)   (容器)    (文本)    (图片)   (边框)   (集合)          │
│    │         │                                      │          │
│    │         ▼                                      ▼          │
│    │    ┌────────┬────────┬────────┐          ListBox         │
│    │    │        │        │        │          ListView        │
│    │    ▼        ▼        ▼        ▼          TreeView        │
│    │  StackPanel Grid   Canvas   DockPanel   DataGrid         │
│    │   (堆叠)   (网格)   (绝对)   (停靠)      (表格)           │
│    │                                                           │
│    └────┬────────┬────────┬────────┬────────┐                 │
│         │        │        │        │        │                 │
│         ▼        ▼        ▼        ▼        ▼                 │
│      Button   TextBox  CheckBox  Slider  ProgressBar          │
│      (按钮)   (输入框) (复选框)  (滑块)   (进度条)             │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

#### 3.2.2 控件核心类设计

```cpp
namespace LuaUI {
namespace UI {

// ==================== 基础元素 ====================
class UIElement {
public:
    // 视觉属性
    PROPERTY(float, Opacity, 1.0f);
    PROPERTY(bool, IsVisible, true);
    PROPERTY(bool, IsEnabled, true);
    PROPERTY(Transform, RenderTransform, Transform::Identity());
    PROPERTY(Point, RenderTransformOrigin, Point(0.5, 0.5));
    PROPERTY(Effect*, Effect, nullptr);
    
    // 命中测试
    virtual bool HitTest(const Point& point) const;
    
    // 渲染
    virtual void OnRender(IRenderContext* context);
    
    // 动画支持
    void BeginAnimation(const DependencyProperty& property, Animation* animation);
    void StopAnimation(const DependencyProperty& property);
};

// ==================== 框架元素（支持布局）====================
class FrameworkElement : public UIElement {
public:
    // 尺寸属性
    PROPERTY(float, Width, NAN);           // NAN = Auto
    PROPERTY(float, Height, NAN);          // NAN = Auto
    PROPERTY(float, MinWidth, 0);
    PROPERTY(float, MinHeight, 0);
    PROPERTY(float, MaxWidth, FLT_MAX);
    PROPERTY(float, MaxHeight, FLT_MAX);
    PROPERTY(Thickness, Margin, Thickness(0));
    PROPERTY(HorizontalAlignment, HorizontalAlignment, HorizontalAlignment::Stretch);
    PROPERTY(VerticalAlignment, VerticalAlignment, VerticalAlignment::Stretch);
    
    // 布局回调
    virtual Size MeasureOverride(const Size& availableSize);
    virtual Size ArrangeOverride(const Size& finalSize);
    
    // 数据绑定
    void SetBinding(const DependencyProperty& property, const Binding& binding);
    void ClearBinding(const DependencyProperty& property);
};

// ==================== 控件基类 ====================
class Control : public FrameworkElement {
public:
    // 样式属性
    PROPERTY(Style*, Style, nullptr);
    PROPERTY(Brush*, Background, nullptr);
    PROPERTY(Brush*, Foreground, nullptr);
    PROPERTY(Brush*, BorderBrush, nullptr);
    PROPERTY(float, BorderThickness, 0);
    PROPERTY(float, CornerRadius, 0);
    PROPERTY(FontFamily, FontFamily, FontFamily(L"Segoe UI"));
    PROPERTY(float, FontSize, 14);
    PROPERTY(FontWeight, FontWeight, FontWeight::Normal);
    
    // 模板
    PROPERTY(ControlTemplate*, Template, nullptr);
    
    // 焦点
    PROPERTY(bool, IsFocused, false);
    PROPERTY(bool, IsTabStop, true);
    PROPERTY(int, TabIndex, 0);
    
    // 事件
    Event<RoutedEventArgs> Click;
    Event<RoutedEventArgs> GotFocus;
    Event<RoutedEventArgs> LostFocus;
    Event<KeyEventArgs> KeyDown;
    Event<KeyEventArgs> KeyUp;
    
    // 模板应用
    virtual void OnApplyTemplate();
};

}} // namespace LuaUI::UI
```

### 3.3 布局系统 (Layout System)

#### 3.3.1 布局面板类型

```xml
<!-- StackPanel: 线性堆叠布局 -->
<StackPanel Orientation="Vertical" Spacing="10">
    <Button Content="Button 1"/>
    <Button Content="Button 2"/>
    <Button Content="Button 3"/>
</StackPanel>

<!-- Grid: 网格布局 -->
<Grid Columns="100,*,2*" Rows="Auto,*,50">
    <!-- Columns: 固定100px, 剩余1/3, 剩余2/3 -->
    <!-- Rows: 自动高度, 填充剩余, 固定50px -->
    <Button Grid.Row="0" Grid.Column="0" Content="Top Left"/>
    <Button Grid.Row="1" Grid.Column="1" Grid.ColumnSpan="2" Content="Center"/>
    <Button Grid.Row="2" Grid.Column="0" Grid.ColumnSpan="3" Content="Bottom"/>
</Grid>

<!-- DockPanel: 停靠布局 -->
<DockPanel>
    <Menu DockPanel.Dock="Top"/>
    <StatusBar DockPanel.Dock="Bottom"/>
    <TreeView DockPanel.Dock="Left" Width="200"/>
    <ContentControl Content="{Binding MainContent}"/>
</DockPanel>

<!-- FlexPanel: 弹性布局 (类似CSS Flexbox) -->
<FlexPanel Direction="Row" JustifyContent="Center" AlignItems="Center" Wrap="Wrap">
    <Button Content="Item 1" Flex="1"/>
    <Button Content="Item 2" Flex="2"/>
    <Button Content="Item 3" Flex="1"/>
</FlexPanel>

<!-- WrapPanel: 自动换行布局 -->
<WrapPanel ItemWidth="100" ItemHeight="100" Orientation="Horizontal">
    <!-- 子控件自动换行排列 -->
</WrapPanel>

<!-- UniformGrid: 均匀网格 -->
<UniformGrid Columns="3" Rows="3">
    <!-- 3x3 均匀网格 -->
</UniformGrid>

<!-- Canvas: 绝对定位 -->
<Canvas>
    <Button Canvas.Left="10" Canvas.Top="20" Content="Absolute"/>
</Canvas>
```

#### 3.3.2 布局流程

```cpp
class LayoutEngine {
public:
    // 两阶段布局算法
    void Measure(UIElement* root, const Size& availableSize) {
        // 自底向上测量所需尺寸
        for (auto child : root->GetChildren()) {
            Measure(child, CalculateAvailableSize(child));
        }
        root->Measure(availableSize);
    }
    
    void Arrange(UIElement* root, const Rect& finalRect) {
        // 自顶向下排列位置
        root->Arrange(finalRect);
        for (auto child : root->GetChildren()) {
            Arrange(child, CalculateChildRect(child, finalRect));
        }
    }
    
    // 增量布局优化
    void InvalidateMeasure(UIElement* element);
    void InvalidateArrange(UIElement* element);
    void ProcessLayoutQueue();  // 批量处理布局请求
};
```

### 3.4 数据绑定系统 (Data Binding)

#### 3.4.1 MVVM 架构

```
┌─────────────────────────────────────────────────────────────────┐
│                        MVVM 架构                                │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   ┌─────────────┐         ┌─────────────┐         ┌───────────┐ │
│   │    View     │ ◄─────► │   ViewModel │ ◄─────► │   Model   │ │
│   │  (XML定义)   │ 绑定    │  (Lua实现)   │ 调用    │ (数据/业务)│ │
│   └─────────────┘         └─────────────┘         └───────────┘ │
│          │                         │                            │
│          │                         │                            │
│          ▼                         ▼                            │
│   ┌─────────────────────────────────────────────────────────┐  │
│   │                 Binding Engine                          │  │
│   │  ┌──────────┐  ┌──────────┐  ┌──────────┐              │  │
│   │  │  OneWay  │  │  TwoWay  │  │ OneTime  │              │  │
│   │  │ (只读)    │  │ (双向)    │  │ (一次性)  │              │  │
│   │  └──────────┘  └──────────┘  └──────────┘              │  │
│   │                                                          │  │
│   │  功能：属性变更通知、值转换器、验证规则、集合同步          │  │
│   └─────────────────────────────────────────────────────────┘  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

#### 3.4.2 绑定语法

```xml
<!-- View (main_view.xml) -->
<Window xmlns="http://schemas.luaui.org/2025/xaml"
        xmlns:x="http://schemas.luaui.org/2025/xaml/x"
        Title="{Binding WindowTitle}"
        Width="800" Height="600">
    
    <Window.DataContext>
        <x:LuaSource Path="viewmodels/main_vm.lua" Type="MainViewModel"/>
    </Window.DataContext>
    
    <DockPanel>
        <!-- Menu -->
        <Menu DockPanel.Dock="Top">
            <MenuItem Header="File">
                <MenuItem Header="New" Command="{Binding NewFileCommand}"/>
                <MenuItem Header="Open" Command="{Binding OpenFileCommand}"/>
                <Separator/>
                <MenuItem Header="Exit" Command="{Binding ExitCommand}"/>
            </MenuItem>
        </Menu>
        
        <!-- Sidebar -->
        <Border DockPanel.Dock="Left" Width="200" Background="{ThemeResource SidebarBackground}">
            <TreeView ItemsSource="{Binding ProjectItems}"
                      SelectedItem="{Binding SelectedItem, Mode=TwoWay}"
                      x:Name="projectTree">
                <TreeView.ItemTemplate>
                    <DataTemplate>
                        <StackPanel Orientation="Horizontal">
                            <Image Source="{Binding Icon}" Width="16" Height="16"/>
                            <TextBlock Text="{Binding Name}" Margin="5,0"/>
                        </StackPanel>
                    </DataTemplate>
                </TreeView.ItemTemplate>
            </TreeView>
        </Border>
        
        <!-- Main Content -->
        <Grid>
            <Grid.RowDefinitions>
                <RowDefinition Height="Auto"/>
                <RowDefinition Height="*"/>
                <RowDefinition Height="Auto"/>
            </Grid.RowDefinitions>
            
            <!-- Toolbar -->
            <ToolBar Grid.Row="0" ItemsSource="{Binding ToolbarItems}"/>
            
            <!-- Content -->
            <ContentControl Grid.Row="1" 
                          Content="{Binding CurrentView}"
                          ContentTemplateSelector="{Binding ViewSelector}"/>
            
            <!-- Status Bar -->
            <StatusBar Grid.Row="2">
                <TextBlock Text="{Binding StatusMessage}"/>
                <ProgressBar Value="{Binding ProgressValue}" 
                           Maximum="{Binding ProgressMaximum}"
                           IsVisible="{Binding IsProgressVisible}"/>
            </StatusBar>
        </Grid>
    </DockPanel>
</Window>
```

```lua
-- ViewModel (viewmodels/main_vm.lua)
local MainViewModel = {}
MainViewModel.__index = MainViewModel

-- 属性定义（自动生成变更通知）
MainViewModel.Properties = {
    WindowTitle = { type = "string", default = "LuaUI Application" },
    StatusMessage = { type = "string", default = "Ready" },
    IsProgressVisible = { type = "boolean", default = false },
    ProgressValue = { type = "number", default = 0 },
    ProgressMaximum = { type = "number", default = 100 },
    SelectedItem = { type = "object", mode = "twoWay" },
    ProjectItems = { type = "ObservableCollection", itemType = "TreeItem" },
}

function MainViewModel.new()
    local self = setmetatable({}, MainViewModel)
    
    -- 初始化属性
    self.WindowTitle = "My Application"
    self.ProjectItems = ObservableCollection.new()
    
    -- 初始化命令
    self.NewFileCommand = Command.new(function() self:onNewFile() end)
    self.OpenFileCommand = Command.new(function() self:onOpenFile() end)
    self.ExitCommand = Command.new(function() self:onExit() end)
    
    return self
end

function MainViewModel:onNewFile()
    -- 业务逻辑
    self.StatusMessage = "Creating new file..."
    -- 异步操作示例
    Task.run(function()
        -- 耗时操作
        Thread.sleep(1000)
        self.StatusMessage = "New file created"
    end)
end

function MainViewModel:onOpenFile()
    -- 使用系统对话框
    local dialog = OpenFileDialog.new()
    dialog.Filter = "Lua files (*.lua)|*.lua|All files (*.*)|*.*"
    if dialog:ShowDialog() then
        self:loadFile(dialog.FileName)
    end
end

-- 属性变更时自动通知UI
function MainViewModel:setStatusMessage(value)
    if self._statusMessage ~= value then
        self._statusMessage = value
        self:notifyPropertyChanged("StatusMessage")
    end
end

return MainViewModel
```

---

## 四、XML布局系统

### 4.1 完整的XML Schema设计

```xml
<?xml version="1.0" encoding="UTF-8"?>
<xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema"
           xmlns:ui="http://schemas.luaui.org/2025/xaml"
           xmlns:x="http://schemas.luaui.org/2025/xaml/x"
           targetNamespace="http://schemas.luaui.org/2025/xaml"
           elementFormDefault="qualified">

    <!-- ==================== 基础类型 ==================== -->
    
    <!-- 尺寸类型：数字或 "Auto" -->
    <xs:simpleType name="LengthType">
        <xs:union>
            <xs:simpleType>
                <xs:restriction base="xs:float">
                    <xs:minInclusive value="0"/>
                </xs:restriction>
            </xs:simpleType>
            <xs:simpleType>
                <xs:restriction base="xs:string">
                    <xs:enumeration value="Auto"/>
                    <xs:enumeration value="*"/>
                </xs:restriction>
            </xs:simpleType>
        </xs:union>
    </xs:simpleType>
    
    <!-- 网格长度：数字、"Auto" 或 "*" 或 "2*" -->
    <xs:simpleType name="GridLengthType">
        <xs:restriction base="xs:string">
            <xs:pattern value="(\d+(\.\d+)?)|(Auto)|(\d*\*)"/>
        </xs:restriction>
    </xs:simpleType>
    
    <!-- 颜色类型 -->
    <xs:simpleType name="ColorType">
        <xs:restriction base="xs:string">
            <!-- #RGB, #RGBA, #RRGGBB, #RRGGBBAA -->
            <xs:pattern value="#([0-9A-Fa-f]{3,4}|[0-9A-Fa-f]{6}|[0-9A-Fa-f]{8})"/>
        </xs:restriction>
    </xs:simpleType>
    
    <!-- 边距/厚度类型 -->
    <xs:simpleType name="ThicknessType">
        <xs:restriction base="xs:string">
            <!-- 1, 2, or 4 values -->
            <xs:pattern value="\d+(\.\d+)?( \d+(\.\d+)?){0,1}( \d+(\.\d+)?){0,1}( \d+(\.\d+)?){0,1}"/>
        </xs:restriction>
    </xs:simpleType>
    
    <!-- 绑定表达式 -->
    <xs:simpleType name="BindingType">
        <xs:restriction base="xs:string">
            <xs:pattern value="\{Binding .*\}|\{StaticResource .*\}|\{ThemeResource .*\}|\{x:.*\}"/>
        </xs:restriction>
    </xs:simpleType>
    
    <!-- ==================== 核心属性组 ==================== -->
    
    <xs:attributeGroup name="FrameworkElementAttributes">
        <xs:attribute name="x:Name" type="xs:ID"/>
        <xs:attribute name="Width" type="LengthType"/>
        <xs:attribute name="Height" type="LengthType"/>
        <xs:attribute name="MinWidth" type="xs:float"/>
        <xs:attribute name="MinHeight" type="xs:float"/>
        <xs:attribute name="MaxWidth" type="xs:float"/>
        <xs:attribute name="MaxHeight" type="xs:float"/>
        <xs:attribute name="Margin" type="ThicknessType"/>
        <xs:attribute name="HorizontalAlignment" type="xs:string"/>
        <xs:attribute name="VerticalAlignment" type="xs:string"/>
        <xs:attribute name="Opacity" type="xs:float"/>
        <xs:attribute name="IsVisible" type="xs:boolean"/>
        <xs:attribute name="IsEnabled" type="xs:boolean"/>
        <xs:attribute name="ToolTip" type="xs:string"/>
        <xs:attribute name="Tag" type="xs:string"/>
    </xs:attributeGroup>
    
    <xs:attributeGroup name="ControlAttributes">
        <xs:attributeGroup ref="FrameworkElementAttributes"/>
        <xs:attribute name="Background" type="ColorType"/>
        <xs:attribute name="Foreground" type="ColorType"/>
        <xs:attribute name="BorderBrush" type="ColorType"/>
        <xs:attribute name="BorderThickness" type="ThicknessType"/>
        <xs:attribute name="CornerRadius" type="xs:float"/>
        <xs:attribute name="FontFamily" type="xs:string"/>
        <xs:attribute name="FontSize" type="xs:float"/>
        <xs:attribute name="FontWeight" type="xs:string"/>
    </xs:attributeGroup>
    
    <!-- ==================== 控件定义 ==================== -->
    
    <!-- Window -->
    <xs:element name="Window">
        <xs:complexType>
            <xs:sequence>
                <xs:element name="Window.DataContext" minOccurs="0">
                    <xs:complexType>
                        <xs:attribute name="Path" type="xs:string" use="required"/>
                        <xs:attribute name="Type" type="xs:string" use="required"/>
                    </xs:complexType>
                </xs:element>
                <xs:element name="Window.Resources" minOccurs="0">
                    <xs:complexType>
                        <xs:sequence>
                            <xs:any maxOccurs="unbounded"/>
                        </xs:sequence>
                    </xs:complexType>
                </xs:element>
                <xs:element ref="ui:FrameworkElement" minOccurs="0" maxOccurs="1"/>
            </xs:sequence>
            <xs:attribute name="Title" type="xs:string"/>
            <xs:attribute name="Icon" type="xs:string"/>
            <xs:attribute name="Width" type="xs:float"/>
            <xs:attribute name="Height" type="xs:float"/>
            <xs:attribute name="MinWidth" type="xs:float"/>
            <xs:attribute name="MinHeight" type="xs:float"/>
            <xs:attribute name="MaxWidth" type="xs:float"/>
            <xs:attribute name="MaxHeight" type="xs:float"/>
            <xs:attribute name="WindowState" type="xs:string"/>
            <xs:attribute name="WindowStyle" type="xs:string"/>
            <xs:attribute name="ResizeMode" type="xs:string"/>
            <xs:attribute name="Background" type="ColorType"/>
        </xs:complexType>
    </xs:element>
    
    <!-- Grid -->
    <xs:element name="Grid">
        <xs:complexType>
            <xs:sequence>
                <xs:element name="Grid.ColumnDefinitions" minOccurs="0">
                    <xs:complexType>
                        <xs:sequence>
                            <xs:element name="ColumnDefinition" maxOccurs="unbounded">
                                <xs:complexType>
                                    <xs:attribute name="Width" type="GridLengthType"/>
                                    <xs:attribute name="MinWidth" type="xs:float"/>
                                    <xs:attribute name="MaxWidth" type="xs:float"/>
                                </xs:complexType>
                            </xs:element>
                        </xs:sequence>
                    </xs:complexType>
                </xs:element>
                <xs:element name="Grid.RowDefinitions" minOccurs="0">
                    <xs:complexType>
                        <xs:sequence>
                            <xs:element name="RowDefinition" maxOccurs="unbounded">
                                <xs:complexType>
                                    <xs:attribute name="Height" type="GridLengthType"/>
                                    <xs:attribute name="MinHeight" type="xs:float"/>
                                    <xs:attribute name="MaxHeight" type="xs:float"/>
                                </xs:complexType>
                            </xs:element>
                        </xs:sequence>
                    </xs:complexType>
                </xs:element>
                <xs:group ref="ui:FrameworkElementGroup" minOccurs="0" maxOccurs="unbounded"/>
            </xs:sequence>
            <xs:attributeGroup ref="PanelAttributes"/>
        </xs:complexType>
    </xs:element>
    
    <!-- Button -->
    <xs:element name="Button">
        <xs:complexType mixed="true">
            <xs:choice minOccurs="0" maxOccurs="unbounded">
                <xs:group ref="ui:FrameworkElementGroup"/>
            </xs:choice>
            <xs:attributeGroup ref="ControlAttributes"/>
            <xs:attribute name="Content" type="xs:string"/>
            <xs:attribute name="Command" type="BindingType"/>
            <xs:attribute name="CommandParameter" type="xs:string"/>
            <xs:attribute name="IsDefault" type="xs:boolean"/>
            <xs:attribute name="IsCancel" type="xs:boolean"/>
        </xs:complexType>
    </xs:element>
    
    <!-- 更多控件... -->
    
</xs:schema>
```

### 4.2 热重载支持

```cpp
class HotReloadManager {
public:
    void WatchFile(const std::string& path);
    void OnFileChanged(const std::string& path);
    
    // 热重载策略
    enum class ReloadStrategy {
        FullRestart,      // 完全重启应用
        ViewOnly,         // 仅重载视图
        PreserveState,    // 保留状态重载
    };
    
private:
    void reloadView(const std::string& xmlPath);
    void preserveStateReload(const std::string& xmlPath);
};
```

---

## 五、Lua脚本系统

### 5.1 Lua绑定架构

```cpp
namespace LuaUI {
namespace Lua {

// Lua VM 封装（沙箱化）
class LuaSandbox {
public:
    bool Initialize();
    void Shutdown();
    
    // 安全执行
    bool ExecuteSafe(const std::string& code, 
                     std::chrono::milliseconds timeout = std::chrono::seconds(5));
    
    // 内存限制
    void SetMemoryLimit(size_t bytes);
    
    // API白名单
    void WhitelistAPI(const std::vector<std::string>& apis);
    
    // 绑定C++类到Lua
    template<typename T>
    void RegisterClass(const std::string& name);
    
    template<typename T, typename Func>
    void RegisterMethod(const std::string& className, 
                        const std::string& methodName, 
                        Func func);
    
    // 属性绑定（支持数据绑定）
    template<typename T>
    void RegisterProperty(const std::string& className,
                          const std::string& propName,
                          std::function<void(T*, lua_State*)> getter,
                          std::function<void(T*, lua_State*)> setter);
};

// 视图模型基类
class ViewModelBase {
public:
    // 属性变更通知
    void NotifyPropertyChanged(const std::string& propertyName);
    
    // 批量属性变更（优化性能）
    void BeginBatchUpdate();
    void EndBatchUpdate();
    
    // 命令
    void RegisterCommand(const std::string& name, std::function<void()> handler);
    
    // 生命周期
    virtual void OnCreated();
    virtual void OnActivated();
    virtual void OnDeactivated();
    virtual void OnDestroy();
};

}} // namespace LuaUI::Lua
```

### 5.2 Lua API设计

```lua
-- 核心命名空间
local UI = require("UI")
local Binding = require("UI.Binding")
local Animation = require("UI.Animation")
local Storage = require("UI.Storage")

-- ==================== 控件操作 ====================

-- 获取控件
local button = UI.getControl("myButton")           -- 通过ID获取
local controls = UI.findControls(".button-class")  -- 通过样式类获取
local container = UI.getControl("panel")
local child = container:findChild("childId")       -- 在容器内查找

-- 属性设置
button:setProperty("text", "Click Me")
button:setProperty("isEnabled", false)
button:setProperty("background", "#FF5722")

-- 批量设置（原子操作）
button:setProperties({
    text = "New Text",
    width = 120,
    height = 40,
    opacity = 0.8
})

-- 数据绑定（ViewModel -> View）
button:setBinding("text", "ViewModel.ButtonText")
button:setBinding("isEnabled", "ViewModel.CanClick", Binding.Mode.OneWay)

-- 双向绑定（View <-> ViewModel）
local textBox = UI.getControl("usernameInput")
textBox:setBinding("text", "ViewModel.Username", Binding.Mode.TwoWay)

-- ==================== 事件处理 ====================

-- 传统事件绑定
button:onClick(function(sender, args)
    print("Button clicked!")
end)

-- 命令绑定（MVVM推荐）
button:setCommand("ViewModel.SubmitCommand")

-- 多事件监听
local subscription = button:subscribe({
    onClick = function(sender, args) end,
    onMouseEnter = function(sender, args) end,
    onMouseLeave = function(sender, args) end,
})

-- 取消订阅
subscription:unsubscribe()

-- ==================== 动画 ====================

-- 简单动画
button:animate({
    property = "opacity",
    from = 1.0,
    to = 0.5,
    duration = 300,
    easing = Animation.Easing.EaseInOut
})

-- 复合动画
local storyboard = Animation.Storyboard.new()
storyboard:addAnimation(button, {
    property = "width",
    to = 200,
    duration = 500,
    easing = Animation.Easing.Bounce
})
storyboard:addAnimation(button, {
    property = "background",
    to = "#4CAF50",
    duration = 300
}, 200)  -- 延迟200ms开始
storyboard:start()

-- 关键帧动画
local animation = Animation.Keyframe.new(button)
animation:addFrame(0, { x = 0, opacity = 0 })
animation:addFrame(0.3, { x = 100, opacity = 0.5 })
animation:addFrame(1.0, { x = 200, opacity = 1 })
animation:play()

-- ==================== 本地存储 ====================

-- 应用设置
local settings = Storage.Settings.new()
settings:set("username", "John")
settings:set("window.width", 1024)
settings:save()

-- 读取
local username = settings:get("username", "default")

-- 本地数据库（SQLite）
local db = Storage.Database.open("app_data.db")
db:execute([[CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name TEXT)]])
db:execute("INSERT INTO users (name) VALUES (?)", {"Alice"})
local users = db:query("SELECT * FROM users WHERE name = ?", {"Alice"})

-- ==================== 网络请求 ====================

local Http = require("UI.Http")

-- GET请求
Http.get("https://api.example.com/data", {
    headers = { Authorization = "Bearer token123" },
    onSuccess = function(response)
        local data = response:json()
        ViewModel:setData(data)
    end,
    onError = function(error)
        UI.showError(error.message)
    end
})

-- POST请求
Http.post("https://api.example.com/users", {
    body = { name = "John", age = 30 },
    contentType = "application/json"
})

-- ==================== 对话框 ====================

-- 消息框
UI.showMessage("操作成功", UI.MessageIcon.Info)
UI.showConfirm("确定删除？", function(result)
    if result == UI.DialogResult.Yes then
        ViewModel:delete()
    end
end)

-- 文件对话框
local dialog = UI.FileDialog.new()
dialog.title = "选择文件"
dialog.filter = "图片文件|*.png;*.jpg|所有文件|*.*"
if dialog:showDialog() then
    local path = dialog.fileName
    ViewModel:loadImage(path)
end

-- ==================== 多线程 ====================

local Task = require("UI.Task")

-- 异步任务
Task.run(function()
    -- 在后台线程执行
    local result = heavyComputation()
    return result
end):continueWith(function(result)
    -- 回到主线程
    ViewModel:updateResult(result)
end)

-- 进度报告
Task.run(function(progress)
    for i = 1, 100 do
        Thread.sleep(50)
        progress:report(i / 100)
    end
end, {
    onProgress = function(value)
        ViewModel.ProgressValue = value * 100
    end,
    onComplete = function()
        ViewModel.StatusMessage = "完成"
    end
})

-- ==================== 日志 ====================

local Logger = require("UI.Logger")

Logger.info("应用启动")
Logger.debug("调试信息: {}", { data = "value" })
Logger.warning("警告信息")
Logger.error("错误信息")

-- ==================== 性能监控 ====================

local Profiler = require("UI.Profiler")

-- 性能分析
Profiler.begin("HeavyOperation")
-- ... 耗时操作
Profiler.end("HeavyOperation")

-- 内存监控
local memory = Profiler.getMemoryUsage()
Logger.info("当前内存使用: {} MB", memory.usedMB)
```

---

## 六、样式与主题系统

### 6.1 样式系统架构

```xml
<!-- themes/default.theme -->
<Theme Name="Default" Version="1.0">
    
    <!-- 颜色资源 -->
    <Color x:Key="PrimaryColor">#0078D4</Color>
    <Color x:Key="SecondaryColor">#005A9E</Color>
    <Color x:Key="AccentColor">#107C10</Color>
    <Color x:Key="ErrorColor">#D83B01</Color>
    <Color x:Key="WarningColor">#FFB900</Color>
    
    <!-- 画刷资源 -->
    <SolidColorBrush x:Key="BackgroundBrush" Color="#F3F3F3"/>
    <SolidColorBrush x:Key="SurfaceBrush" Color="#FFFFFF"/>
    <SolidColorBrush x:Key="PrimaryBrush" Color="{StaticResource PrimaryColor}"/>
    <LinearGradientBrush x:Key="HeaderBrush" StartPoint="0,0" EndPoint="0,1">
        <GradientStop Offset="0" Color="#0078D4"/>
        <GradientStop Offset="1" Color="#005A9E"/>
    </LinearGradientBrush>
    
    <!-- 控件样式 -->
    <Style TargetType="Button">
        <Setter Property="Background" Value="{StaticResource PrimaryBrush}"/>
        <Setter Property="Foreground" Value="#FFFFFF"/>
        <Setter Property="BorderThickness" Value="0"/>
        <Setter Property="CornerRadius" Value="4"/>
        <Setter Property="Padding" Value="12 6"/>
        <Setter Property="FontSize" Value="14"/>
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="Button">
                    <Border Background="{TemplateBinding Background}"
                            BorderBrush="{TemplateBinding BorderBrush}"
                            BorderThickness="{TemplateBinding BorderThickness}"
                            CornerRadius="{TemplateBinding CornerRadius}"
                            x:Name="border">
                        <ContentPresenter Content="{TemplateBinding Content}"
                                        HorizontalAlignment="Center"
                                        VerticalAlignment="Center"
                                        Margin="{TemplateBinding Padding}"/>
                    </Border>
                    <ControlTemplate.Triggers>
                        <!-- 悬停效果 -->
                        <Trigger Property="IsMouseOver" Value="True">
                            <Setter TargetName="border" Property="Background" Value="{StaticResource SecondaryColor}"/>
                        </Trigger>
                        <!-- 按下效果 -->
                        <Trigger Property="IsPressed" Value="True">
                            <Setter TargetName="border" Property="Background" Value="#004578"/>
                            <Setter TargetName="border" Property="RenderTransform">
                                <Setter.Value>
                                    <ScaleTransform ScaleX="0.98" ScaleY="0.98"/>
                                </Setter.Value>
                            </Setter>
                        </Trigger>
                        <!-- 禁用效果 -->
                        <Trigger Property="IsEnabled" Value="False">
                            <Setter TargetName="border" Property="Opacity" Value="0.5"/>
                        </Trigger>
                    </ControlTemplate.Triggers>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
    
    <!-- 文本框样式 -->
    <Style TargetType="TextBox">
        <Setter Property="Background" Value="{StaticResource SurfaceBrush}"/>
        <Setter Property="BorderBrush" Value="#CCCCCC"/>
        <Setter Property="BorderThickness" Value="1"/>
        <Setter Property="CornerRadius" Value="2"/>
        <Setter Property="Padding" Value="8 4"/>
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="TextBox">
                    <Border Background="{TemplateBinding Background}"
                            BorderBrush="{TemplateBinding BorderBrush}"
                            BorderThickness="{TemplateBinding BorderThickness}"
                            CornerRadius="{TemplateBinding CornerRadius}"
                            x:Name="border">
                        <ScrollViewer x:Name="PART_ContentHost"/>
                    </Border>
                    <ControlTemplate.Triggers>
                        <Trigger Property="IsFocused" Value="True">
                            <Setter TargetName="border" Property="BorderBrush" Value="{StaticResource PrimaryColor}"/>
                        </Trigger>
                    </ControlTemplate.Triggers>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
    
</Theme>
```

### 6.2 暗色主题示例

```xml
<!-- themes/dark.theme -->
<Theme Name="Dark" BaseTheme="Default">
    <Color x:Key="BackgroundColor">#1E1E1E</Color>
    <Color x:Key="SurfaceColor">#252526</Color>
    <Color x:Key="TextColor">#CCCCCC</Color>
    
    <SolidColorBrush x:Key="BackgroundBrush" Color="{StaticResource BackgroundColor}"/>
    <SolidColorBrush x:Key="SurfaceBrush" Color="{StaticResource SurfaceColor}"/>
    <SolidColorBrush x:Key="ForegroundBrush" Color="{StaticResource TextColor}"/>
    
    <!-- 覆盖默认样式 -->
    <Style TargetType="Window">
        <Setter Property="Background" Value="{StaticResource BackgroundBrush}"/>
    </Style>
    
    <Style TargetType="TextBlock" BasedOn="{StaticResource {x:Type TextBlock}}">
        <Setter Property="Foreground" Value="{StaticResource ForegroundBrush}"/>
    </Style>
</Theme>
```

---

## 七、动画系统

### 7.1 动画架构

```cpp
namespace LuaUI {
namespace Animation {

// 缓动函数
enum class EasingFunction {
    Linear,
    QuadraticEaseIn, QuadraticEaseOut, QuadraticEaseInOut,
    CubicEaseIn, CubicEaseOut, CubicEaseInOut,
    QuarticEaseIn, QuarticEaseOut, QuarticEaseInOut,
    QuinticEaseIn, QuinticEaseOut, QuinticEaseInOut,
    SineEaseIn, SineEaseOut, SineEaseInOut,
    ExponentialEaseIn, ExponentialEaseOut, ExponentialEaseInOut,
    CircularEaseIn, CircularEaseOut, CircularEaseInOut,
    ElasticEaseIn, ElasticEaseOut, ElasticEaseInOut,
    BackEaseIn, BackEaseOut, BackEaseInOut,
    BounceEaseIn, BounceEaseOut, BounceEaseInOut,
    // 自定义贝塞尔曲线
    Bezier
};

// 动画基类
class Animation : public RefCounted {
public:
    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual void Pause() = 0;
    virtual void Resume() = 0;
    virtual bool IsRunning() const = 0;
    
    // 事件
    Event<void> Completed;
    Event<float> ProgressChanged;
};

// 属性动画
class PropertyAnimation : public Animation {
public:
    void SetTarget(UIElement* target);
    void SetProperty(const DependencyProperty& property);
    void SetFrom(const Value& value);
    void SetTo(const Value& value);
    void SetDuration(std::chrono::milliseconds duration);
    void SetEasing(EasingFunction easing);
    void SetDelay(std::chrono::milliseconds delay);
    void SetRepeatCount(int count);  // -1 = 无限
    void SetAutoReverse(bool autoReverse);
};

// 关键帧动画
class KeyframeAnimation : public Animation {
public:
    void AddKeyframe(float time, const Value& value, EasingFunction easing = EasingFunction::Linear);
};

// 故事板（组合动画）
class Storyboard : public Animation {
public:
    void AddAnimation(Animation* animation);
    void AddAnimation(Animation* animation, std::chrono::milliseconds delay);
    void SetFillBehavior(FillBehavior behavior);  // HoldEnd, Stop
};

// 动画管理器
class AnimationManager {
public:
    void Update(float deltaTime);  // 每帧调用
    void RegisterAnimation(Animation* animation);
    void UnregisterAnimation(Animation* animation);
};

}} // namespace LuaUI::Animation
```

### 7.2 动画效果示例

```xml
<!-- 入场动画示例 -->
<Window.Resources>
    <Storyboard x:Key="FadeInAnimation">
        <DoubleAnimation Storyboard.TargetProperty="Opacity"
                        From="0" To="1"
                        Duration="0:0:0.3"
                        EasingFunction="QuadraticEaseOut"/>
        <DoubleAnimation Storyboard.TargetProperty="(UIElement.RenderTransform).(ScaleTransform.ScaleX)"
                        From="0.9" To="1"
                        Duration="0:0:0.3"
                        EasingFunction="BackEaseOut"/>
        <DoubleAnimation Storyboard.TargetProperty="(UIElement.RenderTransform).(ScaleTransform.ScaleY)"
                        From="0.9" To="1"
                        Duration="0:0:0.3"
                        EasingFunction="BackEaseOut"/>
    </Storyboard>
    
    <Storyboard x:Key="SlideInFromBottom">
        <DoubleAnimation Storyboard.TargetProperty="(UIElement.RenderTransform).(TranslateTransform.Y)"
                        From="50" To="0"
                        Duration="0:0:0.4"
                        EasingFunction="QuadraticEaseOut"/>
        <DoubleAnimation Storyboard.TargetProperty="Opacity"
                        From="0" To="1"
                        Duration="0:0:0.3"/>
    </Storyboard>
</Window.Resources>

<Grid>
    <Border Loaded="StartAnimation">
        <Border.Triggers>
            <EventTrigger RoutedEvent="Loaded">
                <BeginStoryboard Storyboard="{StaticResource FadeInAnimation}"/>
            </EventTrigger>
        </Border.Triggers>
        <!-- 内容 -->
    </Border>
</Grid>
```

---

## 八、安全架构

### 8.1 Lua沙箱安全

```cpp
class LuaSandbox {
public:
    void InitializeSecure() {
        // 1. 移除危险函数
        lua_pushnil(L);
        lua_setglobal(L, "dofile");
        lua_pushnil(L);
        lua_setglobal(L, "loadfile");
        
        // 2. 限制标准库
        luaL_requiref(L, "io", ioLibRestricted, 1);
        luaL_requiref(L, "os", osLibRestricted, 1);
        luaL_requiref(L, "debug", debugLibRestricted, 1);
        
        // 3. 设置内存限制
        lua_sethook(L, memoryCheckHook, LUA_MASKCOUNT, 1000);
        
        // 4. 设置执行时间限制
        lua_sethook(L, timeoutHook, LUA_MASKLINE, 100000);
    }
    
private:
    static void timeoutHook(lua_State* L, lua_Debug* ar) {
        if (executionTime > maxExecutionTime) {
            luaL_error(L, "Script execution timeout");
        }
    }
    
    static void memoryCheckHook(lua_State* L, lua_Debug* ar) {
        if (lua_gc(L, LUA_GCCOUNT, 0) > maxMemoryKB) {
            luaL_error(L, "Memory limit exceeded");
        }
    }
};
```

### 8.2 网络安全

```cpp
class SecureHttpClient {
public:
    // 仅允许HTTPS
    bool AllowHttp = false;
    
    // 域名白名单
    std::vector<std::string> WhitelistDomains;
    
    // 请求签名验证
    bool VerifySignatures = true;
    
    // 请求频率限制
    RateLimiter RequestRateLimiter;
    
    // 请求审计日志
    void LogRequest(const HttpRequest& request);
};
```

---

## 九、性能优化策略

### 9.1 渲染优化

```cpp
class RenderOptimizer {
public:
    // 脏矩形渲染
    void InvalidateRect(const Rect& rect);
    void RenderDirtyRegions();
    
    // 离屏缓存
    void CacheElement(UIElement* element);
    void InvalidateCache(UIElement* element);
    
    // 绘制调用合并
    void BatchDrawCalls();
    
    // 自适应帧率
    void SetTargetFrameRate(int fps);  // 30, 60, 120
    void AdaptiveQuality(float load);
};
```

### 9.2 内存管理

```cpp
class MemoryPool {
public:
    // 对象池
    template<typename T>
    T* Acquire();
    
    template<typename T>
    void Release(T* obj);
    
    // 纹理图集
    void PackTextures();
    
    // 字体缓存
    void CacheFontGlyphs(const Font& font, const std::wstring& text);
};
```

---

## 十、开发工具链

### 10.1 可视化设计器

```
┌─────────────────────────────────────────────────────────────────┐
│                    LuaUI Studio 设计器                          │
├─────────────────────────────────────────────────────────────────┤
│  ┌──────────┐  ┌─────────────────────────────────┐  ┌────────┐ │
│  │ 工具箱    │  │                                 │  │ 属性   │ │
│  │          │  │                                 │  │        │ │
│  │ □ Button │  │      可视化设计画布              │  │ Name   │ │
│  │ □ TextBox│  │                                 │  │ Width  │ │
│  │ □ Label  │  │      ┌─────────────────┐       │  │ Height │ │
│  │ □ Panel  │  │      │    控件预览      │       │  │ ...    │ │
│  │ ...      │  │      └─────────────────┘       │  │        │ │
│  │          │  │                                 │  │        │ │
│  └──────────┘  └─────────────────────────────────┘  └────────┘ │
├─────────────────────────────────────────────────────────────────┤
│  ┌───────────────────────────────────────────────────────────┐ │
│  │  XAML编辑器 │ Lua编辑器 │ 预览 │ 调试                      │ │
│  └───────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────┘
```

### 10.2 调试支持

```lua
-- 调试API
local Debug = require("UI.Debug")

-- 元素检查
Debug.inspect(UI.getControl("myButton"))

-- 性能分析
Debug.profile(function()
    -- 代码
end)

-- 内存快照
Debug.dumpMemory("memory_snapshot.txt")

-- UI树可视化
Debug.visualizeTree()
```

---

## 十一、部署与分发

### 11.1 打包结构

```
MyApp/
├── MyApp.exe              # 主程序（C++运行时）
├── LuaUI.dll              # 框架核心
├── lua54.dll              # Lua运行时
├── d2d1.dll               # Direct2D (Windows 10+内置)
├── resources/
│   ├── app.xaml           # 应用级资源
│   ├── themes/
│   │   ├── default.theme
│   │   └── dark.theme
│   └── images/
├── views/
│   ├── main_window.xml
│   └── dialogs/
├── viewmodels/
│   ├── main_vm.lua
│   └── dialogs/
├── models/
│   └── data_models.lua
└── config.json            # 应用配置
```

### 11.2 单文件发布

```cpp
// 资源打包到EXE
class ResourcePacker {
public:
    // 将所有资源打包到单个EXE
    void PackToSingleExecutable(const std::string& outputPath);
    
    // 运行时解包到内存
    std::vector<uint8_t> ExtractResource(const std::string& path);
};
```

---

## 十二、路线图

### Phase 1: 基础框架 (3个月)
- [ ] Direct2D渲染引擎
- [ ] 基础控件集（Button, TextBox, Label, Panel）
- [ ] XML布局解析器
- [ ] Lua绑定基础

### Phase 2: 核心功能 (3个月)
- [ ] 完整布局系统（Grid, StackPanel, DockPanel, FlexPanel）
- [ ] 数据绑定引擎
- [ ] 样式系统
- [ ] 事件路由系统

### Phase 3: 高级特性 (3个月)
- [ ] 动画系统
- [ ] 现代化视觉效果（亚克力、阴影、圆角）
- [ ] MVVM完整支持
- [ ] 热重载

### Phase 4: 工具与优化 (3个月)
- [ ] 可视化设计器
- [ ] 性能优化
- [ ] 安全加固
- [ ] 文档与示例

---

## 总结

本架构设计旨在构建一个**商业级**的Windows UI框架，具有以下核心优势：

1. **现代化UI**: Direct2D渲染，支持Fluent Design
2. **高效开发**: XML+Lua，MVVM架构，热重载
3. **流畅体验**: 60fps渲染，优化布局，流畅动画
4. **安全可靠**: Lua沙箱，网络安全，内存管理
5. **商业就绪**: 完善的工具链，文档，示例

这是一个长期的项目，建议采用迭代开发方式，逐步完善各个子系统。
