# LuaUI 样式系统详细设计文档

## 1. 设计目标

基于现有 DependencyProperty 系统，构建类似 WPF/UWP 的样式系统，支持：
- **样式复用**：通过 Style 对象定义可复用的属性集合
- **资源管理**：ResourceDictionary 管理颜色、画刷、尺寸等资源
- **主题切换**：支持浅色/深色主题动态切换
- **触发器系统**：基于属性、数据、事件的动态样式变更
- **与现有架构无缝集成**：复用 DependencyProperty 系统

## 2. 架构设计

### 2.1 整体架构

```
┌─────────────────────────────────────────────────────────────────┐
│                        样式系统分层架构                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  Layer 3: Theme System (主题管理层)                      │   │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐     │   │
│  │  │ThemeManager │  │    Theme    │  │ThemeLoader  │     │   │
│  │  │  (单例)      │  │  (主题定义)  │  │ (XML解析)   │     │   │
│  │  └─────────────┘  └─────────────┘  └─────────────┘     │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  Layer 2: Resource System (资源管理层)                   │   │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐     │   │
│  │  │ResourceDict │  │ResourceKey  │  │ResourceRef  │     │   │
│  │  │ (资源字典)   │  │  (资源键)   │  │ (资源引用)   │     │   │
│  │  └─────────────┘  └─────────────┘  └─────────────┘     │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  Layer 1: Style System (样式定义层)                      │   │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐     │   │
│  │  │    Style    │  │    Setter   │  │   Trigger   │     │   │
│  │  │  (样式定义)  │  │ (属性设置器) │  │   (触发器)   │     │   │
│  │  └─────────────┘  └─────────────┘  └─────────────┘     │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  Layer 0: Integration (集成层 - 现有架构)                 │   │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐     │   │
│  │  │   Control   │  │DependencyProp│  │  IRenderCtx │     │   │
│  │  │  (控件基类)  │  │ (依赖属性)   │  │ (渲染上下文)│     │   │
│  │  └─────────────┘  └─────────────┘  └─────────────┘     │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 2.2 与现有架构的集成点

```
现有架构                    样式系统集成
─────────────────────────────────────────────────
Control                     Control (扩展)
    │                           │
    ├── DependencyProperty      ├── Style (新增)
    ├── GetValue/SetValue       ├── GetResource (新增)
    ├── Render                  └── ApplyStyle (新增)
    └── HitTest                     
                                    
新增功能:                              
- Style 属性 (ControlStyle/ImplicitStyle)
- Resources 属性 (局部资源字典)
- StyleChanged 事件
```

## 3. 核心类设计

### 3.1 ResourceDictionary (资源字典)

```cpp
// include/ResourceDictionary.h
#pragma once
#include <unordered_map>
#include <string>
#include <any>
#include <memory>

namespace luaui {
namespace controls {

// 资源引用类型 (支持 {StaticResource} 和 {DynamicResource})
enum class ResourceLookupMode {
    Static,     // 一次性查找，性能更好
    Dynamic     // 动态查找，支持运行时变更
};

// 资源键
struct ResourceKey {
    std::string name;
    std::string typeHint;  // 类型提示，用于类型安全检查
    
    bool operator==(const ResourceKey& other) const {
        return name == other.name;
    }
};

// 资源字典
class ResourceDictionary : public std::enable_shared_from_this<ResourceDictionary> {
public:
    using Ptr = std::shared_ptr<ResourceDictionary>;
    using ConstPtr = std::shared_ptr<const ResourceDictionary>;
    
    ResourceDictionary() = default;
    ~ResourceDictionary() = default;
    
    // 资源管理
    void Add(const std::string& key, const std::any& value);
    void Remove(const std::string& key);
    bool Contains(const std::string& key) const;
    std::any Get(const std::string& key) const;
    
    // 类型安全获取
    template<typename T>
    T Get(const std::string& key) const {
        return std::any_cast<T>(Get(key));
    }
    
    template<typename T>
    T GetOrDefault(const std::string& key, const T& defaultValue) const {
        auto it = m_resources.find(key);
        if (it != m_resources.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (...) {
                return defaultValue;
            }
        }
        return defaultValue;
    }
    
    // 合并另一个字典（用于主题继承）
    void Merge(const ResourceDictionary& other);
    
    // 父字典（支持资源查找链）
    void SetParent(ResourceDictionary* parent) { m_parent = parent; }
    ResourceDictionary* GetParent() const { return m_parent; }
    
    // 资源变更通知
    using ResourceChangedCallback = std::function<void(const std::string& key)>;
    void SubscribeResourceChanged(ResourceChangedCallback callback);
    void NotifyResourceChanged(const std::string& key);
    
private:
    std::unordered_map<std::string, std::any> m_resources;
    ResourceDictionary* m_parent = nullptr;
    std::vector<ResourceChangedCallback> m_callbacks;
};

// 资源引用（支持延迟解析）
class ResourceReference {
public:
    ResourceReference(const std::string& key, ResourceLookupMode mode = ResourceLookupMode::Static)
        : m_key(key), m_mode(mode) {}
    
    const std::string& GetKey() const { return m_key; }
    ResourceLookupMode GetMode() const { return m_mode; }
    
    // 解析引用为实际值
    std::any Resolve(ResourceDictionary* localResources, 
                     ResourceDictionary* themeResources) const;
    
private:
    std::string m_key;
    ResourceLookupMode m_mode;
};

} // namespace controls
} // namespace luaui
```

### 3.2 Setter (属性设置器)

```cpp
// include/Setter.h
#pragma once
#include "Control.h"
#include "ResourceDictionary.h"
#include <variant>

namespace luaui {
namespace controls {

// Setter 值可以是直接值或资源引用
using SetterValue = std::variant<std::any, ResourceReference>;

// 属性设置器
class Setter {
public:
    Setter() = default;
    Setter(DependencyProperty::Id propertyId, const std::any& value)
        : m_propertyId(propertyId), m_value(value) {}
    Setter(DependencyProperty::Id propertyId, const ResourceReference& ref)
        : m_propertyId(propertyId), m_value(ref) {}
    
    // 应用 Setter 到控件
    void Apply(Control* target, ResourceDictionary* resources) const;
    
    // 获取属性ID
    DependencyProperty::Id GetPropertyId() const { return m_propertyId; }
    
    // 获取值（解析资源引用）
    std::any GetValue(ResourceDictionary* resources) const;
    
private:
    DependencyProperty::Id m_propertyId;
    SetterValue m_value;
};

} // namespace controls
} // namespace luaui
```

### 3.3 Style (样式)

```cpp
// include/Style.h
#pragma once
#include "Setter.h"
#include "Trigger.h"
#include <vector>
#include <string>
#include <typeindex>

namespace luaui {
namespace controls {

// 样式类
class Style : public std::enable_shared_from_this<Style> {
public:
    using Ptr = std::shared_ptr<Style>;
    using ConstPtr = std::shared_ptr<const Style>;
    
    // 构造时指定目标类型
    explicit Style(const std::type_index& targetType);
    
    // 目标类型
    const std::type_index& GetTargetType() const { return m_targetType; }
    
    // BasedOn 样式继承
    void SetBasedOn(Style::Ptr basedOn) { m_basedOn = basedOn; }
    Style::Ptr GetBasedOn() const { return m_basedOn; }
    
    // Setter 管理
    void AddSetter(const Setter& setter);
    void AddSetter(DependencyProperty::Id propertyId, const std::any& value);
    void ClearSetters() { m_setters.clear(); }
    const std::vector<Setter>& GetSetters() const { return m_setters; }
    
    // Trigger 管理
    void AddTrigger(Trigger::Ptr trigger);
    void ClearTriggers() { m_triggers.clear(); }
    const std::vector<Trigger::Ptr>& GetTriggers() const { return m_triggers; }
    
    // 应用/取消应用样式
    void Apply(Control* target, ResourceDictionary* resources);
    void Unapply(Control* target);
    
    // 检查样式是否适用于指定类型
    bool CanApplyTo(const std::type_index& type) const;
    
private:
    std::type_index m_targetType;
    Style::Ptr m_basedOn;
    std::vector<Setter> m_setters;
    std::vector<Trigger::Ptr> m_triggers;
    
    // 记录已应用的 Setter 值，用于取消应用
    struct AppliedValue {
        DependencyProperty::Id propertyId;
        std::any originalValue;
    };
    std::unordered_map<Control*, std::vector<AppliedValue>> m_appliedValues;
};

} // namespace controls
} // namespace luaui
```

### 3.4 Trigger (触发器)

```cpp
// include/Trigger.h
#pragma once
#include "Setter.h"
#include "Control.h"
#include <functional>
#include <vector>

namespace luaui {
namespace controls {

// 触发器基类
class Trigger : public std::enable_shared_from_this<Trigger> {
public:
    using Ptr = std::shared_ptr<Trigger>;
    
    virtual ~Trigger() = default;
    
    // 附加/分离到控件
    virtual void Attach(Control* target, ResourceDictionary* resources) = 0;
    virtual void Detach(Control* target) = 0;
    
    // 获取触发器是否处于激活状态
    virtual bool IsActive() const = 0;
};

// 属性触发器 (Property Trigger)
class PropertyTrigger : public Trigger {
public:
    PropertyTrigger(DependencyProperty::Id propertyId, const std::any& value);
    
    void AddSetter(const Setter& setter);
    void AddSetter(DependencyProperty::Id propertyId, const std::any& value);
    
    void Attach(Control* target, ResourceDictionary* resources) override;
    void Detach(Control* target) override;
    bool IsActive() const override { return m_isActive; }
    
private:
    DependencyProperty::Id m_propertyId;
    std::any m_triggerValue;
    std::vector<Setter> m_setters;
    
    Control* m_attachedTarget = nullptr;
    ResourceDictionary* m_resources = nullptr;
    bool m_isActive = false;
    std::vector<Setter> m_originalSetters;  // 记录原始值用于恢复
    
    void OnPropertyChanged(Control* control, DependencyProperty::Id propertyId);
    void Evaluate();
    void Enter();
    void Exit();
};

// 数据触发器 (Data Trigger) - 基于绑定表达式
class DataTrigger : public Trigger {
public:
    // TODO: 实现基于数据绑定的触发器
};

// 事件触发器 (Event Trigger)
class EventTrigger : public Trigger {
public:
    explicit EventTrigger(const std::string& eventName);
    
    void AddAction(std::function<void(Control*)> action);
    
    void Attach(Control* target, ResourceDictionary* resources) override;
    void Detach(Control* target) override;
    bool IsActive() const override { return false; } // 事件触发器无持续状态
    
private:
    std::string m_eventName;
    std::vector<std::function<void(Control*)>> m_actions;
    Control* m_attachedTarget = nullptr;
};

// 多条件触发器 (MultiTrigger)
class MultiTrigger : public Trigger {
public:
    // 添加条件
    void AddCondition(DependencyProperty::Id propertyId, const std::any& value);
    void AddSetter(const Setter& setter);
    
    void Attach(Control* target, ResourceDictionary* resources) override;
    void Detach(Control* target) override;
    bool IsActive() const override;
    
private:
    struct Condition {
        DependencyProperty::Id propertyId;
        std::any value;
    };
    std::vector<Condition> m_conditions;
    std::vector<Setter> m_setters;
    Control* m_attachedTarget = nullptr;
    bool m_isActive = false;
};

} // namespace controls
} // namespace luaui
```

### 3.5 Theme 和 ThemeManager

```cpp
// include/Theme.h
#pragma once
#include "ResourceDictionary.h"
#include "Style.h"
#include <unordered_map>
#include <string>

namespace luaui {
namespace controls {

// 主题类
class Theme {
public:
    Theme() = default;
    ~Theme() = default;
    
    // 主题信息
    const std::string& GetName() const { return m_name; }
    void SetName(const std::string& name) { m_name = name; }
    
    const std::string& GetBaseTheme() const { return m_baseTheme; }
    void SetBaseTheme(const std::string& base) { m_baseTheme = base; }
    
    const std::string& GetVersion() const { return m_version; }
    void SetVersion(const std::string& version) { m_version = version; }
    
    // 资源字典
    ResourceDictionary& GetResources() { return m_resources; }
    const ResourceDictionary& GetResources() const { return m_resources; }
    
    // 样式管理
    void AddStyle(const std::string& key, Style::Ptr style);
    void RemoveStyle(const std::string& key);
    Style::Ptr GetStyle(const std::string& key) const;
    Style::Ptr GetStyleForType(const std::type_index& type) const;
    
    // 加载主题
    bool LoadFromFile(const std::string& path);
    bool LoadFromString(const std::string& xml);
    
private:
    std::string m_name;
    std::string m_baseTheme;
    std::string m_version;
    ResourceDictionary m_resources;
    std::unordered_map<std::string, Style::Ptr> m_styles;
    std::unordered_map<std::type_index, Style::Ptr> m_implicitStyles;
};

// 主题管理器（单例）
class ThemeManager {
public:
    static ThemeManager& GetInstance();
    
    // 加载/卸载主题
    bool LoadTheme(const std::string& name, const std::string& path);
    void UnloadTheme(const std::string& name);
    
    // 切换主题
    void SetCurrentTheme(const std::string& name);
    const std::string& GetCurrentThemeName() const { return m_currentThemeName; }
    Theme* GetCurrentTheme() { return m_currentTheme.get(); }
    
    // 获取资源
    std::any GetResource(const std::string& key) const;
    template<typename T>
    T GetResource(const std::string& key) const {
        return std::any_cast<T>(GetResource(key));
    }
    
    // 获取样式
    Style::Ptr GetStyle(const std::string& key) const;
    Style::Ptr GetImplicitStyle(const std::type_index& type) const;
    
    // 主题变更事件
    using ThemeChangedCallback = std::function<void(const std::string& newTheme)>;
    void SubscribeThemeChanged(ThemeChangedCallback callback);
    
    // 刷新所有控件的样式
    void InvalidateAllStyles();
    
private:
    ThemeManager() = default;
    ~ThemeManager() = default;
    
    std::unordered_map<std::string, std::unique_ptr<Theme>> m_themes;
    std::unique_ptr<Theme> m_currentTheme;
    std::string m_currentThemeName;
    std::vector<ThemeChangedCallback> m_callbacks;
};

} // namespace controls
} // namespace luaui
```

### 3.6 Control 集成

```cpp
// 现有 Control 类需要扩展的属性（添加到 Control.h）

class Control : public std::enable_shared_from_this<Control> {
public:
    // ... 现有代码 ...
    
    // ============ 样式系统集成 ============
    
    // 样式属性
    Style::Ptr GetStyle() const { return m_style; }
    void SetStyle(Style::Ptr style);
    
    // 内联 Setter（类似于内联样式）
    void SetInlineSetter(DependencyProperty::Id propertyId, const std::any& value);
    void ClearInlineSetter(DependencyProperty::Id propertyId);
    
    // 资源字典（控件级局部资源）
    ResourceDictionary& GetResources();
    void SetResources(ResourceDictionary::Ptr resources);
    
    // 样式重新应用
    void InvalidateStyle();
    void UpdateStyle();
    
    // 资源查找
    std::any FindResource(const std::string& key) const;
    template<typename T>
    T FindResource(const std::string& key) const {
        return std::any_cast<T>(FindResource(key));
    }
    
protected:
    // ... 现有成员 ...
    
    // 样式相关
    Style::Ptr m_style;
    Style::Ptr m_appliedStyle;  // 当前已应用的样式
    ResourceDictionary::Ptr m_resources;
    std::unordered_map<DependencyProperty::Id, std::any> m_inlineSetters;
    
    // 样式更新
    void ApplyStyle();
    void ClearAppliedStyle();
    Style::Ptr FindApplicableStyle() const;
};
```

## 4. 样式优先级规则

```
样式应用优先级（从高到低）：

1. 内联 Setter (SetInlineSetter)
   └── 最高优先级，直接通过代码设置

2. 内联 Style (Style 属性直接设置)
   └── <Button Style="{StaticResource CustomStyle}"/>

3. 隐式 Style (基于控件类型的默认样式)
   └── <Style TargetType="Button"> (无 x:Key)

4. BasedOn 继承样式
   └── Style 继承链的优先级传递

5. 默认值 (DependencyProperty 默认值)
   └── 最低优先级

资源查找顺序（从高到低）：

1. 元素 Resources
2. 父元素 Resources（递归向上）
3. Window/Page Resources
4. Application Resources
5. Theme Resources
```

## 5. 使用示例

### 5.1 C++ API 使用

```cpp
// 创建资源字典
auto resources = std::make_shared<ResourceDictionary>();
resources->Add("PrimaryColor", Color::FromHex(0x0078D4));
resources->Add("PrimaryBrush", std::make_shared<SolidColorBrush>(Color::FromHex(0x0078D4)));

// 创建样式
auto buttonStyle = std::make_shared<Style>(typeid(Button));
buttonStyle->AddSetter(Control::BackgroundProperty, ResourceReference("PrimaryBrush"));
buttonStyle->AddSetter(Control::ForegroundProperty, Color::White());
buttonStyle->AddSetter(Control::CornerRadiusProperty, CornerRadius(4));

// 添加触发器
auto hoverTrigger = std::make_shared<PropertyTrigger>(
    Control::IsMouseOverProperty, true
);
hoverTrigger->AddSetter(Control::BackgroundProperty, Color::FromHex(0x005A9E));
buttonStyle->AddTrigger(hoverTrigger);

// 应用样式
auto button = std::make_shared<Button>();
button->SetStyle(buttonStyle);

// 主题切换
ThemeManager::GetInstance().LoadTheme("dark", "themes/dark.theme");
ThemeManager::GetInstance().SetCurrentTheme("dark");
```

### 5.2 XML 定义

```xml
<!-- themes/default.theme -->
<Theme Name="Default" Version="1.0.0">
    <Resources>
        <!-- 颜色 -->
        <Color x:Key="PrimaryColor">#0078D4</Color>
        <Color x:Key="SecondaryColor">#107C10</Color>
        
        <!-- 画刷 -->
        <SolidColorBrush x:Key="PrimaryBrush" Color="{StaticResource PrimaryColor}"/>
        <SolidColorBrush x:Key="WindowBackground" Color="#F3F3F3"/>
        
        <!-- 尺寸 -->
        <x:Double x:Key="BorderRadius">4</x:Double>
        <Thickness x:Key="StandardPadding">12,8</Thickness>
    </Resources>
    
    <Styles>
        <!-- 基础控件样式 -->
        <Style x:Key="BaseControlStyle" TargetType="Control">
            <Setter Property="FontSize" Value="14"/>
            <Setter Property="Foreground" Value="#333333"/>
        </Style>
        
        <!-- 按钮样式 -->
        <Style x:Key="PrimaryButtonStyle" TargetType="Button" 
               BasedOn="{StaticResource BaseControlStyle}">
            <Setter Property="Background" Value="{StaticResource PrimaryBrush}"/>
            <Setter Property="Foreground" Value="#FFFFFF"/>
            <Setter Property="CornerRadius" Value="{StaticResource BorderRadius}"/>
            <Setter Property="Padding" Value="{StaticResource StandardPadding}"/>
            
            <Triggers>
                <Trigger Property="IsMouseOver" Value="True">
                    <Setter Property="Background" Value="#005A9E"/>
                </Trigger>
                <Trigger Property="IsPressed" Value="True">
                    <Setter Property="Background" Value="#004578"/>
                </Trigger>
            </Triggers>
        </Style>
    </Styles>
</Theme>
```

## 6. 实现路线图

### Phase 7.1: 基础资源系统 (1周)
- ResourceDictionary 实现
- 资源查找链
- 资源变更通知

### Phase 7.2: Setter 和 Style (1周)
- Setter 实现
- Style 类实现
- BasedOn 继承

### Phase 7.3: Trigger 系统 (1周)
- PropertyTrigger 实现
- EventTrigger 实现
- MultiTrigger 实现

### Phase 7.4: 主题管理 (1周)
- Theme 类实现
- ThemeManager 单例
- XML 主题加载

### Phase 7.5: 集成与测试 (1周)
- Control 集成
- 样式优先级验证
- 主题切换测试
- 示例程序

## 7. 文件结构

```
src/luaui/style/
├── include/
│   ├── ResourceDictionary.h    # 资源字典
│   ├── Setter.h                # 属性设置器
│   ├── Style.h                 # 样式定义
│   ├── Trigger.h               # 触发器系统
│   ├── Theme.h                 # 主题管理
│   └── StyleSystem.h           # 统一头文件
├── src/
│   ├── ResourceDictionary.cpp
│   ├── Setter.cpp
│   ├── Style.cpp
│   ├── Trigger.cpp
│   └── Theme.cpp
└── CMakeLists.txt
```

## 8. 与现有代码的兼容性

- **向后兼容**: 不使用样式的控件继续以默认方式工作
- **渐进式采用**: 可以逐步为控件添加样式支持
- **无侵入**: 样式系统作为独立模块，不影响现有架构
