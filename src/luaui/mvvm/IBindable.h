#pragma once

#include <memory>
#include <string>
#include <functional>
#include <any>

namespace luaui {
namespace mvvm {

// 前向声明
class IBinding;
class IValueConverter;

// ============================================================================
// 属性变更事件参数
// ============================================================================
struct PropertyChangedEventArgs {
    std::string propertyName;
    std::any oldValue;
    std::any newValue;
};

// ============================================================================
// INotifyPropertyChanged - 属性变更通知接口
// ============================================================================
class INotifyPropertyChanged {
public:
    virtual ~INotifyPropertyChanged() = default;
    
    // 属性变更事件
    using PropertyChangedHandler = std::function<void(const PropertyChangedEventArgs&)>;
    
    // 订阅属性变更
    virtual void SubscribePropertyChanged(PropertyChangedHandler handler) = 0;
    
    // 取消订阅
    virtual void UnsubscribePropertyChanged(PropertyChangedHandler handler) = 0;
    
    // 通知属性变更（由ViewModel调用）
    virtual void NotifyPropertyChanged(const std::string& propertyName) = 0;
    
    // 获取属性值（由绑定引擎调用）
    virtual std::any GetPropertyValue(const std::string& propertyName) const = 0;
    
    // 设置属性值（由TwoWay绑定调用）
    virtual void SetPropertyValue(const std::string& propertyName, const std::any& value) = 0;
};

// ============================================================================
// IValueConverter - 值转换器接口
// ============================================================================
class IValueConverter {
public:
    virtual ~IValueConverter() = default;
    
    // 转换源值到目标值（ViewModel -> View）
    virtual std::any Convert(const std::any& value, const std::string& parameter = "") = 0;
    
    // 转换目标值回源值（View -> ViewModel，仅TwoWay模式需要）
    virtual std::any ConvertBack(const std::any& value, const std::string& parameter = "") = 0;
};

// ============================================================================
// 绑定模式
// ============================================================================
enum class BindingMode {
    OneWay,           // 单向绑定 (VM -> View)
    TwoWay,           // 双向绑定 (VM <-> View)
    OneWayToSource,   // 单向到源 (View -> VM)
    OneTime,          // 一次性绑定
    Default           // 使用控件的默认模式
};

// ============================================================================
// 绑定表达式
// ============================================================================
struct BindingExpression {
    std::string path;                    // 属性路径，如 "User.Name"
    BindingMode mode = BindingMode::OneWay;
    std::shared_ptr<IValueConverter> converter;
    std::string converterParameter;
    std::string elementName;             // 绑定到命名元素
    std::string sourceType;              // 源类型（如 "Self", "Ancestor"）
    int ancestorLevel = 1;               // 祖先级别
    std::string updateSourceTrigger = "PropertyChanged"; // 更新触发时机
    
    bool isValid() const { return !path.empty() || !elementName.empty(); }
};

// ============================================================================
// IBinding - 绑定接口
// ============================================================================
class IBinding {
public:
    virtual ~IBinding() = default;
    
    // 获取绑定表达式
    virtual const BindingExpression& GetExpression() const = 0;
    
    // 获取源对象
    virtual std::shared_ptr<INotifyPropertyChanged> GetSource() const = 0;
    
    // 获取目标对象
    virtual std::weak_ptr<void> GetTarget() const = 0;
    
    // 更新绑定（从源到目标）
    virtual void UpdateTarget() = 0;
    
    // 更新源（从目标到源，用于TwoWay）
    virtual void UpdateSource() = 0;
    
    // 断开绑定
    virtual void Detach() = 0;
    
    // 是否有效
    virtual bool IsAttached() const = 0;
};

// ============================================================================
// IBindable - 可绑定对象接口（由View实现）
// ============================================================================
class IBindable {
public:
    virtual ~IBindable() = default;
    
    // 获取/设置 DataContext
    virtual std::shared_ptr<INotifyPropertyChanged> GetDataContext() const = 0;
    virtual void SetDataContext(std::shared_ptr<INotifyPropertyChanged> context) = 0;
    
    // 应用绑定
    virtual void ApplyBinding(const std::string& propertyName, std::shared_ptr<IBinding> binding) = 0;
    
    // 获取属性值
    virtual std::any GetPropertyValue(const std::string& propertyName) = 0;
    
    // 设置属性值
    virtual void SetPropertyValue(const std::string& propertyName, const std::any& value) = 0;
};

} // namespace mvvm
} // namespace luaui
