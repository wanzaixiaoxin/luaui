# 样式与主题系统设计

## 1. 概述

样式系统提供类似CSS的样式定义能力，支持资源复用、主题切换和动态样式。

## 2. 架构

```
┌─────────────────────────────────────────────────────────────────┐
│                      样式系统架构                                │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │                    Theme System                          │   │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐     │   │
│  │  │   Light     │  │    Dark     │  │   Custom    │     │   │
│  │  │   Theme     │  │    Theme    │  │   Theme     │     │   │
│  │  └─────────────┘  └─────────────┘  └─────────────┘     │   │
│  │                                                           │   │
│  │  Theme inheritance: Dark -> Light (base)                │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │                    Resource Dictionary                    │   │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐     │   │
│  │  │   Colors    │  │   Brushes   │  │   Styles    │     │   │
│  │  │   Sizes     │  │   Fonts     │  │ Templates   │     │   │
│  │  └─────────────┘  └─────────────┘  └─────────────┘     │   │
│  │                                                           │   │
│  │  Resource lookup order:                                  │   │
│  │  1. Element Resources -> 2. Window Resources ->         │   │
│  │  3. Application Resources -> 4. Theme Resources          │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │                    Style System                          │   │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐     │   │
│  │  │  Property   │  │  Trigger    │  │  Template   │     │   │
│  │  │   Setters   │  │  System     │  │  System     │     │   │
│  │  └─────────────┘  └─────────────┘  └─────────────┘     │   │
│  │                                                           │   │
│  │  Style application: Inline -> Element Style ->           │   │
│  │  Type Style -> BasedOn Style                             │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## 3. 主题文件格式

```xml
<!-- themes/default.theme -->
<Theme Name="Default" Version="1.0.0">
    
    <!-- 元数据 -->
    <Metadata>
        <Author>LuaUI Team</Author>
        <Description>Default light theme</Description>
        <MinVersion>1.0</MinVersion>
    </Metadata>
    
    <!-- 颜色调色板 -->
    <Colors>
        <!-- 主色调 -->
        <Color x:Key="PrimaryColor">#0078D4</Color>
        <Color x:Key="PrimaryLightColor">#5C9BD1</Color>
        <Color x:Key="PrimaryDarkColor">#005A9E</Color>
        
        <!-- 强调色 -->
        <Color x:Key="AccentColor">#107C10</Color>
        <Color x:Key="AccentLightColor">#4CAF50</Color>
        <Color x:Key="AccentDarkColor">#0B5C0B</Color>
        
        <!-- 语义色 -->
        <Color x:Key="ErrorColor">#D83B01</Color>
        <Color x:Key="WarningColor">#FFB900</Color>
        <Color x:Key="SuccessColor">#107C10</Color>
        <Color x:Key="InfoColor">#0078D4</Color>
        
        <!-- 中性色 -->
        <Color x:Key="Gray100">#F3F3F3</Color>
        <Color x:Key="Gray200">#E5E5E5</Color>
        <Color x:Key="Gray300">#CCCCCC</Color>
        <Color x:Key="Gray400">#999999</Color>
        <Color x:Key="Gray500">#666666</Color>
        <Color x:Key="Gray600">#333333</Color>
        <Color x:Key="Gray700">#1F1F1F</Color>
        <Color x:Key="Gray800">#1A1A1A</Color>
        <Color x:Key="Gray900">#111111</Color>
    </Colors>
    
    <!-- 画刷资源 -->
    <Brushes>
        <!-- 背景画刷 -->
        <SolidColorBrush x:Key="WindowBackgroundBrush" Color="{StaticResource Gray100}"/>
        <SolidColorBrush x:Key="SurfaceBrush" Color="#FFFFFF"/>
        <SolidColorBrush x:Key="SurfaceElevatedBrush" Color="#FFFFFF"/>
        
        <!-- 前景画刷 -->
        <SolidColorBrush x:Key="PrimaryTextBrush" Color="{StaticResource Gray900}"/>
        <SolidColorBrush x:Key="SecondaryTextBrush" Color="{StaticResource Gray600}"/>
        <SolidColorBrush x:Key="DisabledTextBrush" Color="{StaticResource Gray400}"/>
        <SolidColorBrush x:Key="PlaceholderTextBrush" Color="{StaticResource Gray400}"/>
        
        <!-- 强调画刷 -->
        <SolidColorBrush x:Key="PrimaryBrush" Color="{StaticResource PrimaryColor}"/>
        <SolidColorBrush x:Key="PrimaryHoverBrush" Color="{StaticResource PrimaryLightColor}"/>
        <SolidColorBrush x:Key="PrimaryPressedBrush" Color="{StaticResource PrimaryDarkColor}"/>
        <SolidColorBrush x:Key="AccentBrush" Color="{StaticResource AccentColor}"/>
        
        <!-- 边框画刷 -->
        <SolidColorBrush x:Key="BorderBrush" Color="{StaticResource Gray300}"/>
        <SolidColorBrush x:Key="BorderFocusedBrush" Color="{StaticResource PrimaryColor}"/>
        <SolidColorBrush x:Key="BorderErrorBrush" Color="{StaticResource ErrorColor}"/>
        
        <!-- 渐变画刷 -->
        <LinearGradientBrush x:Key="HeaderBrush" StartPoint="0,0" EndPoint="0,1">
            <GradientStop Offset="0" Color="{StaticResource PrimaryColor}"/>
            <GradientStop Offset="1" Color="{StaticResource PrimaryDarkColor}"/>
        </LinearGradientBrush>
        
        <LinearGradientBrush x:Key="OverlayBrush" StartPoint="0,0" EndPoint="1,1">
            <GradientStop Offset="0" Color="#000000" Opacity="0"/>
            <GradientStop Offset="1" Color="#000000" Opacity="0.2"/>
        </LinearGradientBrush>
    </Brushes>
    
    <!-- 字体资源 -->
    <Fonts>
        <FontFamily x:Key="DefaultFontFamily">Segoe UI, Microsoft YaHei, sans-serif</FontFamily>
        <FontFamily x:Key="MonospaceFontFamily">Consolas, Courier New, monospace</FontFamily>
        
        <!-- 字体大小 -->
        <x:Double x:Key="FontSizeXS">10</x:Double>
        <x:Double x:Key="FontSizeSM">12</x:Double>
        <x:Double x:Key="FontSizeMD">14</x:Double>
        <x:Double x:Key="FontSizeLG">16</x:Double>
        <x:Double x:Key="FontSizeXL">20</x:Double>
        <x:Double x:Key="FontSize2XL">24</x:Double>
        <x:Double x:Key="FontSize3XL">32</x:Double>
    </Fonts>
    
    <!-- 尺寸资源 -->
    <Sizes>
        <x:Double x:Key="BorderRadiusSM">2</x:Double>
        <x:Double x:Key="BorderRadiusMD">4</x:Double>
        <x:Double x:Key="BorderRadiusLG">8</x:Double>
        <x:Double x:Key="BorderRadiusXL">12</x:Double>
        <x:Double x:Key="BorderRadiusFull">9999</x:Double>
        
        <Thickness x:Key="PaddingXS">4</Thickness>
        <Thickness x:Key="PaddingSM">8</Thickness>
        <Thickness x:Key="PaddingMD">12</Thickness>
        <Thickness x:Key="PaddingLG">16</Thickness>
        <Thickness x:Key="PaddingXL">24</Thickness>
        
        <Thickness x:Key="MarginXS">4</Thickness>
        <Thickness x:Key="MarginSM">8</Thickness>
        <Thickness x:Key="MarginMD">12</Thickness>
        <Thickness x:Key="MarginLG">16</Thickness>
        <Thickness x:Key="MarginXL">24</Thickness>
        
        <x:Double x:Key="ShadowSM">2</x:Double>
        <x:Double x:Key="ShadowMD">4</x:Double>
        <x:Double x:Key="ShadowLG">8</x:Double>
        <x:Double x:Key="ShadowXL">16</x:Double>
    </Sizes>
    
    <!-- 控件样式 -->
    <Styles>
        <!-- 基础控件样式 -->
        <Style x:Key="BaseControlStyle" TargetType="Control">
            <Setter Property="FontFamily" Value="{StaticResource DefaultFontFamily}"/>
            <Setter Property="FontSize" Value="{StaticResource FontSizeMD}"/>
            <Setter Property="Foreground" Value="{StaticResource PrimaryTextBrush}"/>
        </Style>
        
        <!-- 按钮样式 -->
        <Style x:Key="ButtonBaseStyle" TargetType="Button" BasedOn="{StaticResource BaseControlStyle}">
            <Setter Property="Background" Value="{StaticResource PrimaryBrush}"/>
            <Setter Property="Foreground" Value="#FFFFFF"/>
            <Setter Property="BorderThickness" Value="0"/>
            <Setter Property="CornerRadius" Value="{StaticResource BorderRadiusMD}"/>
            <Setter Property="Padding" Value="{StaticResource PaddingMD}"/>
            <Setter Property="MinWidth" Value="80"/>
            <Setter Property="MinHeight" Value="32"/>
            <Setter Property="HorizontalContentAlignment" Value="Center"/>
            <Setter Property="VerticalContentAlignment" Value="Center"/>
            <Setter Property="Template">
                <Setter.Value>
                    <ControlTemplate TargetType="Button">
                        <Border x:Name="border"
                                Background="{TemplateBinding Background}"
                                BorderBrush="{TemplateBinding BorderBrush}"
                                BorderThickness="{TemplateBinding BorderThickness}"
                                CornerRadius="{TemplateBinding CornerRadius}"
                                Padding="{TemplateBinding Padding}">
                            <ContentPresenter x:Name="contentPresenter"
                                            Content="{TemplateBinding Content}"
                                            ContentTemplate="{TemplateBinding ContentTemplate}"
                                            HorizontalAlignment="{TemplateBinding HorizontalContentAlignment}"
                                            VerticalAlignment="{TemplateBinding VerticalContentAlignment}"/>
                        </Border>
                        <ControlTemplate.Triggers>
                            <Trigger Property="IsMouseOver" Value="True">
                                <Setter TargetName="border" Property="Background" Value="{StaticResource PrimaryHoverBrush}"/>
                                <Setter TargetName="border" Property="Cursor" Value="Hand"/>
                            </Trigger>
                            <Trigger Property="IsPressed" Value="True">
                                <Setter TargetName="border" Property="Background" Value="{StaticResource PrimaryPressedBrush}"/>
                                <Setter TargetName="contentPresenter" Property="RenderTransform">
                                    <Setter.Value>
                                        <ScaleTransform ScaleX="0.98" ScaleY="0.98"/>
                                    </Setter.Value>
                                </Setter>
                            </Trigger>
                            <Trigger Property="IsEnabled" Value="False">
                                <Setter TargetName="border" Property="Opacity" Value="0.5"/>
                                <Setter TargetName="border" Property="Cursor" Value="NotAllowed"/>
                            </Trigger>
                            <Trigger Property="IsFocused" Value="True">
                                <Setter TargetName="border" Property="BorderBrush" Value="{StaticResource PrimaryBrush}"/>
                                <Setter TargetName="border" Property="BorderThickness" Value="2"/>
                            </Trigger>
                        </ControlTemplate.Triggers>
                    </ControlTemplate>
                </Setter.Value>
            </Setter>
        </Style>
        
        <!-- 次要按钮 -->
        <Style x:Key="SecondaryButtonStyle" TargetType="Button" BasedOn="{StaticResource ButtonBaseStyle}">
            <Setter Property="Background" Value="transparent"/>
            <Setter Property="Foreground" Value="{StaticResource PrimaryBrush}"/>
            <Setter Property="BorderBrush" Value="{StaticResource PrimaryBrush}"/>
            <Setter Property="BorderThickness" Value="1"/>
        </Style>
        
        <!-- 幽灵按钮 -->
        <Style x:Key="GhostButtonStyle" TargetType="Button" BasedOn="{StaticResource ButtonBaseStyle}">
            <Setter Property="Background" Value="transparent"/>
            <Setter Property="Foreground" Value="{StaticResource PrimaryTextBrush}"/>
            <Setter Property="BorderThickness" Value="0"/>
        </Style>
        
        <!-- 文本框样式 -->
        <Style x:Key="TextBoxBaseStyle" TargetType="TextBox" BasedOn="{StaticResource BaseControlStyle}">
            <Setter Property="Background" Value="{StaticResource SurfaceBrush}"/>
            <Setter Property="BorderBrush" Value="{StaticResource BorderBrush}"/>
            <Setter Property="BorderThickness" Value="1"/>
            <Setter Property="CornerRadius" Value="{StaticResource BorderRadiusMD}"/>
            <Setter Property="Padding" Value="{StaticResource PaddingSM}"/>
            <Setter Property="MinHeight" Value="32"/>
            <Setter Property="CaretBrush" Value="{StaticResource PrimaryBrush}"/>
            <Setter Property="SelectionBrush" Value="{StaticResource PrimaryLightColor}"/>
            <Setter Property="PlaceholderForeground" Value="{StaticResource PlaceholderTextBrush}"/>
            <Setter Property="Template">
                <Setter.Value>
                    <ControlTemplate TargetType="TextBox">
                        <Border x:Name="border"
                                Background="{TemplateBinding Background}"
                                BorderBrush="{TemplateBinding BorderBrush}"
                                BorderThickness="{TemplateBinding BorderThickness}"
                                CornerRadius="{TemplateBinding CornerRadius}">
                            <Grid>
                                <ScrollViewer x:Name="PART_ContentHost"
                                            Margin="{TemplateBinding Padding}"/>
                                <TextBlock x:Name="placeholder"
                                         Text="{TemplateBinding PlaceholderText}"
                                         Foreground="{TemplateBinding PlaceholderForeground}"
                                         Margin="{TemplateBinding Padding}"
                                         IsVisible="{TemplateBinding Text, Converter={StaticResource StringEmptyToVisibilityConverter}}"/>
                            </Grid>
                        </Border>
                        <ControlTemplate.Triggers>
                            <Trigger Property="IsMouseOver" Value="True">
                                <Setter TargetName="border" Property="BorderBrush" Value="{StaticResource BorderFocusedBrush}"/>
                            </Trigger>
                            <Trigger Property="IsFocused" Value="True">
                                <Setter TargetName="border" Property="BorderBrush" Value="{StaticResource PrimaryBrush}"/>
                                <Setter TargetName="border" Property="BorderThickness" Value="2"/>
                            </Trigger>
                            <Trigger Property="IsEnabled" Value="False">
                                <Setter TargetName="border" Property="Background" Value="{StaticResource Gray200}"/>
                                <Setter TargetName="border" Property="Opacity" Value="0.6"/>
                            </Trigger>
                            <Trigger Property="Validation.HasError" Value="True">
                                <Setter TargetName="border" Property="BorderBrush" Value="{StaticResource ErrorColor}"/>
                            </Trigger>
                        </ControlTemplate.Triggers>
                    </ControlTemplate>
                </Setter.Value>
            </Setter>
        </Style>
        
        <!-- 更多控件样式... -->
    </Styles>
    
    <!-- 动画资源 -->
    <Animations>
        <Storyboard x:Key="FadeIn">
            <DoubleAnimation Storyboard.TargetProperty="Opacity"
                           From="0" To="1"
                           Duration="0:0:0.2"
                           EasingFunction="QuadraticEaseOut"/>
        </Storyboard>
        
        <Storyboard x:Key="FadeOut">
            <DoubleAnimation Storyboard.TargetProperty="Opacity"
                           From="1" To="0"
                           Duration="0:0:0.15"
                           EasingFunction="QuadraticEaseIn"/>
        </Storyboard>
        
        <Storyboard x:Key="ScaleIn">
            <DoubleAnimation Storyboard.TargetProperty="ScaleX"
                           From="0.9" To="1"
                           Duration="0:0:0.2"
                           EasingFunction="BackEaseOut"/>
            <DoubleAnimation Storyboard.TargetProperty="ScaleY"
                           From="0.9" To="1"
                           Duration="0:0:0.2"
                           EasingFunction="BackEaseOut"/>
            <DoubleAnimation Storyboard.TargetProperty="Opacity"
                           From="0" To="1"
                           Duration="0:0:0.2"/>
        </Storyboard>
    </Animations>
    
    <!-- 阴影效果 -->
    <Effects>
        <DropShadowEffect x:Key="ShadowSM" OffsetX="0" OffsetY="1" BlurRadius="2" Color="#000000" Opacity="0.1"/>
        <DropShadowEffect x:Key="ShadowMD" OffsetX="0" OffsetY="2" BlurRadius="4" Color="#000000" Opacity="0.15"/>
        <DropShadowEffect x:Key="ShadowLG" OffsetX="0" OffsetY="4" BlurRadius="8" Color="#000000" Opacity="0.2"/>
        <DropShadowEffect x:Key="ShadowXL" OffsetX="0" OffsetY="8" BlurRadius="16" Color="#000000" Opacity="0.25"/>
    </Effects>
    
</Theme>
```

## 4. 暗色主题示例

```xml
<!-- themes/dark.theme -->
<Theme Name="Dark" BaseTheme="Default" Version="1.0.0">
    
    <Metadata>
        <Author>LuaUI Team</Author>
        <Description>Dark theme for low-light environments</Description>
    </Metadata>
    
    <Colors>
        <!-- 重新定义颜色 -->
        <Color x:Key="Gray100">#1F1F1F</Color>
        <Color x:Key="Gray200">#2D2D2D</Color>
        <Color x:Key="Gray300">#3D3D3D</Color>
        <Color x:Key="Gray400">#555555</Color>
        <Color x:Key="Gray500">#888888</Color>
        <Color x:Key="Gray600">#AAAAAA</Color>
        <Color x:Key="Gray700">#CCCCCC</Color>
        <Color x:Key="Gray800">#E5E5E5</Color>
        <Color x:Key="Gray900">#FFFFFF</Color>
    </Colors>
    
    <Brushes>
        <!-- 重新定义画刷 -->
        <SolidColorBrush x:Key="WindowBackgroundBrush" Color="{StaticResource Gray100}"/>
        <SolidColorBrush x:Key="SurfaceBrush" Color="{StaticResource Gray200}"/>
        <SolidColorBrush x:Key="SurfaceElevatedBrush" Color="{StaticResource Gray300}"/>
        <SolidColorBrush x:Key="PrimaryTextBrush" Color="{StaticResource Gray900}"/>
        <SolidColorBrush x:Key="SecondaryTextBrush" Color="{StaticResource Gray600}"/>
        <SolidColorBrush x:Key="BorderBrush" Color="{StaticResource Gray400}"/>
    </Brushes>
    
    <!-- 覆盖特定样式 -->
    <Styles>
        <Style TargetType="TextBox" BasedOn="{StaticResource TextBoxBaseStyle}">
            <Setter Property="Background" Value="{StaticResource Gray300}"/>
            <Setter Property="Foreground" Value="{StaticResource Gray900}"/>
        </Style>
    </Styles>
    
</Theme>
```

## 5. 样式系统C++设计

```cpp
namespace LuaUI {
namespace Style {

// 资源字典
class ResourceDictionary {
public:
    void Add(const std::string& key, const std::any& value);
    void Remove(const std::string& key);
    bool Contains(const std::string& key) const;
    std::any Get(const std::string& key) const;
    
    template<typename T>
    T Get(const std::string& key) const {
        return std::any_cast<T>(Get(key));
    }
    
    // 合并另一个字典
    void Merge(const ResourceDictionary& other);
    
    // 设置父字典（用于资源查找链）
    void SetParent(ResourceDictionary* parent);
    
private:
    std::unordered_map<std::string, std::any> m_resources;
    ResourceDictionary* m_parent = nullptr;
};

// 样式
class Style : public RefCounted {
public:
    explicit Style(const std::type_info& targetType);
    
    void SetBasedOn(Style* basedOn);
    Style* GetBasedOn() const { return m_basedOn; }
    
    void SetTargetType(const std::type_info& type) { m_targetType = &type; }
    const std::type_info* GetTargetType() const { return m_targetType; }
    
    // 添加Setter
    void AddSetter(const Setter& setter);
    void AddSetter(const DependencyPropertyBase& property, const std::any& value);
    
    // 添加Trigger
    void AddTrigger(TriggerBase* trigger);
    
    // 应用样式到元素
    void Apply(UIElement* element);
    void Unapply(UIElement* element);
    
private:
    const std::type_info* m_targetType;
    Style* m_basedOn = nullptr;
    std::vector<Setter> m_setters;
    std::vector<RefPtr<TriggerBase>> m_triggers;
};

// Setter
struct Setter {
    const DependencyPropertyBase* Property;
    std::any Value;
};

// Trigger基类
class TriggerBase : public RefCounted {
public:
    virtual ~TriggerBase() = default;
    virtual void Attach(UIElement* element) = 0;
    virtual void Detach(UIElement* element) = 0;
};

// 属性Trigger
class Trigger : public TriggerBase {
public:
    Trigger(const DependencyPropertyBase& property, const std::any& value);
    
    void AddSetter(const Setter& setter);
    void AddSetter(const DependencyPropertyBase& property, const std::any& value);
    
    void Attach(UIElement* element) override;
    void Detach(UIElement* element) override;
    
private:
    const DependencyPropertyBase* m_property;
    std::any m_value;
    std::vector<Setter> m_setters;
    UIElement* m_attachedElement = nullptr;
    Subscription m_propertyChangedSub;
};

// 数据Trigger
class DataTrigger : public TriggerBase {
public:
    DataTrigger(const BindingBase& binding, const std::any& value);
    // ...
};

// 事件Trigger
class EventTrigger : public TriggerBase {
public:
    explicit EventTrigger(const std::string& eventName);
    
    void AddAction(TriggerAction* action);
    void Attach(UIElement* element) override;
    void Detach(UIElement* element) override;
    
private:
    std::string m_eventName;
    std::vector<RefPtr<TriggerAction>> m_actions;
};

// Trigger动作
class TriggerAction : public RefCounted {
public:
    virtual ~TriggerAction() = default;
    virtual void Invoke(UIElement* target) = 0;
};

// 开始动画动作
class BeginStoryboardAction : public TriggerAction {
public:
    explicit BeginStoryboardAction(Storyboard* storyboard);
    void Invoke(UIElement* target) override;
private:
    Storyboard* m_storyboard;
};

// 主题管理器
class ThemeManager {
public:
    static ThemeManager& GetInstance();
    
    // 加载主题
    bool LoadTheme(const std::string& themeName);
    bool LoadThemeFromFile(const std::string& path);
    
    // 获取当前主题
    Theme* GetCurrentTheme() const;
    std::string GetCurrentThemeName() const;
    
    // 切换主题
    void SetTheme(const std::string& themeName);
    
    // 获取可用主题列表
    std::vector<std::string> GetAvailableThemes() const;
    
    // 主题变更事件
    Event<std::string> ThemeChanged;
    
    // 获取主题资源
    std::any GetResource(const std::string& key) const;
    template<typename T>
    T GetResource(const std::string& key) const {
        return std::any_cast<T>(GetResource(key));
    }
    
    // 注册主题变更回调
    using ThemeChangedCallback = std::function<void(const std::string&)>;
    Subscription SubscribeThemeChanged(ThemeChangedCallback callback);
    
private:
    ThemeManager();
    ~ThemeManager();
    
    std::unordered_map<std::string, std::unique_ptr<Theme>> m_themes;
    Theme* m_currentTheme = nullptr;
    std::string m_currentThemeName;
};

// 主题类
class Theme {
public:
    bool LoadFromFile(const std::string& path);
    bool LoadFromString(const std::string& xml);
    
    const std::string& GetName() const { return m_name; }
    const std::string& GetBaseTheme() const { return m_baseTheme; }
    
    ResourceDictionary& GetResources() { return m_resources; }
    
    // 获取样式
    Style* GetStyle(const std::type_info& targetType);
    Style* GetStyle(const std::string& key);
    
private:
    std::string m_name;
    std::string m_baseTheme;
    std::string m_version;
    ResourceDictionary m_resources;
    std::unordered_map<std::string, RefPtr<Style>> m_styles;
};

}} // namespace LuaUI::Style
```

## 6. 使用示例

```xml
<!-- 在XML中使用样式 -->
<Window>
    <Window.Resources>
        <!-- 局部资源 -->
        <SolidColorBrush x:Key="CustomBackground" Color="#FF5722"/>
        
        <!-- 局部样式 -->
        <Style x:Key="AccentButton" TargetType="Button" BasedOn="{StaticResource ButtonBaseStyle}">
            <Setter Property="Background" Value="{StaticResource AccentBrush}"/>
            <Setter Property="Template">
                <Setter.Value>
                    <ControlTemplate TargetType="Button">
                        <Border Background="{TemplateBinding Background}"
                                CornerRadius="20">
                            <ContentPresenter HorizontalAlignment="Center" 
                                            VerticalAlignment="Center"/>
                        </Border>
                    </ControlTemplate>
                </Setter.Value>
            </Setter>
        </Style>
    </Window.Resources>
    
    <StackPanel>
        <!-- 使用主题样式 -->
        <Button Content="Primary" Style="{StaticResource ButtonBaseStyle}"/>
        <Button Content="Secondary" Style="{StaticResource SecondaryButtonStyle}"/>
        
        <!-- 使用内联样式覆盖 -->
        <Button Content="Custom">
            <Button.Style>
                <Style TargetType="Button" BasedOn="{StaticResource ButtonBaseStyle}">
                    <Setter Property="Background" Value="{StaticResource CustomBackground}"/>
                </Style>
            </Button.Style>
        </Button>
        
        <!-- 隐式样式（自动应用到所有TextBox） -->
        <TextBox Text="Default styled textbox"/>
    </StackPanel>
</Window>
```

```lua
-- 在Lua中切换主题
local UI = require("UI")

-- 切换暗色主题
UI.setTheme("dark")

-- 监听主题变化
UI.onThemeChanged(function(newTheme)
    print("Theme switched to: " .. newTheme)
end)

-- 获取主题颜色
local primaryColor = UI.getThemeResource("PrimaryColor")
local bgBrush = UI.getThemeResource("WindowBackgroundBrush")
```

## 7. 动态样式支持

```cpp
// 运行时样式切换
void ThemeManager::SetTheme(const std::string& themeName) {
    auto it = m_themes.find(themeName);
    if (it == m_themes.end()) {
        if (!LoadTheme(themeName)) {
            LogError("Failed to load theme: {}", themeName);
            return;
        }
    }
    
    m_currentTheme = m_themes[themeName].get();
    m_currentThemeName = themeName;
    
    // 通知所有UI元素重新应用样式
    Application::GetInstance().InvalidateStyles();
    
    // 触发事件
    ThemeChanged.Fire(themeName);
}

// 元素重新应用样式
void UIElement::InvalidateStyle() {
    // 清除当前样式
    if (m_appliedStyle) {
        m_appliedStyle->Unapply(this);
    }
    
    // 查找新样式
    m_appliedStyle = FindStyle();
    
    // 应用新样式
    if (m_appliedStyle) {
        m_appliedStyle->Apply(this);
    }
    
    // 递归处理子元素
    for (auto child : m_children) {
        child->InvalidateStyle();
    }
}
```
