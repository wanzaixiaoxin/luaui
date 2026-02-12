#pragma once

#include "IBindable.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>

namespace luaui {
namespace controls {
    class Control;
}
}

namespace luaui {
namespace mvvm {

// ============================================================================
// PropertyBinding - 属性绑定实现
// ============================================================================
class PropertyBinding : public IBinding, public std::enable_shared_from_this<PropertyBinding> {
public:
    PropertyBinding(
        std::shared_ptr<INotifyPropertyChanged> source,
        std::shared_ptr<void> target,
        const BindingExpression& expression,
        std::function<std::any()> getter,
        std::function<void(const std::any&)> setter
    );
    
    ~PropertyBinding();
    
    // IBinding 实现
    const BindingExpression& GetExpression() const override { return m_expression; }
    std::shared_ptr<INotifyPropertyChanged> GetSource() const override { return m_source; }
    std::weak_ptr<void> GetTarget() const override { return m_target; }
    void UpdateTarget() override;
    void UpdateSource() override;
    void Detach() override;
    bool IsAttached() const override { return m_attached; }
    
private:
    void OnSourcePropertyChanged(const PropertyChangedEventArgs& args);
    std::any GetSourceValue();
    void SetSourceValue(const std::any& value);
    
    std::shared_ptr<INotifyPropertyChanged> m_source;
    std::weak_ptr<void> m_target;
    BindingExpression m_expression;
    std::function<std::any()> m_targetGetter;
    std::function<void(const std::any&)> m_targetSetter;
    
    INotifyPropertyChanged::PropertyChangedHandler m_sourceHandler;
    bool m_attached = true;
    bool m_updating = false; // 防止循环更新
};

// ============================================================================
// BindingEngine - 绑定引擎
// 管理所有绑定关系
// ============================================================================
class BindingEngine {
public:
    static BindingEngine& Instance();
    
    // 创建绑定
    std::shared_ptr<IBinding> CreateBinding(
        std::shared_ptr<INotifyPropertyChanged> source,
        std::shared_ptr<void> target,
        const BindingExpression& expression,
        std::function<std::any()> getter,
        std::function<void(const std::any&)> setter
    );
    
    // 解析绑定表达式字符串
    // 示例: "{Binding UserName, Mode=TwoWay, Converter=UpperCaseConverter}"
    BindingExpression ParseExpression(const std::string& expression);
    
    // 注册值转换器
    void RegisterConverter(const std::string& name, std::shared_ptr<IValueConverter> converter);
    
    // 获取值转换器
    std::shared_ptr<IValueConverter> GetConverter(const std::string& name);
    
    // 断开所有绑定
    void ClearBindings();
    
    // 断开目标的绑定
    void ClearBindingsForTarget(void* target);
    
    // 更新所有绑定
    void UpdateAllBindings();
    
private:
    BindingEngine() = default;
    ~BindingEngine() = default;
    
    BindingEngine(const BindingEngine&) = delete;
    BindingEngine& operator=(const BindingEngine&) = delete;
    
    std::vector<std::weak_ptr<IBinding>> m_bindings;
    std::unordered_map<std::string, std::shared_ptr<IValueConverter>> m_converters;
};

// ============================================================================
// 便捷函数
// ============================================================================

// 绑定控件属性到ViewModel
std::shared_ptr<IBinding> Bind(
    std::shared_ptr<luaui::controls::Control> target,
    const std::string& targetProperty,
    std::shared_ptr<INotifyPropertyChanged> source,
    const BindingExpression& expression
);

} // namespace mvvm
} // namespace luaui
