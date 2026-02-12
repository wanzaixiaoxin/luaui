#include "XmlBindingExtension.h"
#include "Logger.h"
#include "Control.h"
#include <algorithm>

namespace luaui {
namespace mvvm {

// ============================================================================
// BindingExtension 实现
// ============================================================================
BindingExpression BindingExtension::Parse(const std::string& expression) {
    return BindingEngine::Instance().ParseExpression(expression);
}

bool BindingExtension::IsBindingExpression(const std::string& value) {
    if (value.length() < 3) return false;
    return value.front() == '{' && value.back() == '}';
}

std::shared_ptr<IBinding> BindingExtension::ApplyBinding(
    std::shared_ptr<luaui::Control> control,
    const std::string& propertyName,
    const BindingExpression& expression,
    std::shared_ptr<INotifyPropertyChanged> dataContext
) {
    if (!control || !dataContext || !expression.isValid()) {
        return nullptr;
    }
    
    // 获取属性的getter/setter
    std::function<std::any()> getter;
    std::function<void(const std::any&)> setter;
    
    // 这里简化处理，实际应该通过反射获取
    (void)propertyName;
    
    return BindingEngine::Instance().CreateBinding(
        dataContext, control, expression, getter, setter
    );
}

// ============================================================================
// BindingHelper 实现
// ============================================================================
void BindingHelper::SetDataContext(
    std::shared_ptr<luaui::Control> root,
    std::shared_ptr<INotifyPropertyChanged> context
) {
    // 这里可以遍历控件树设置DataContext
    (void)root;
    (void)context;
}

std::shared_ptr<IBinding> BindingHelper::BindProperty(
    std::shared_ptr<luaui::Control> control,
    const std::string& propertyName,
    const std::string& bindingExpression,
    std::shared_ptr<INotifyPropertyChanged> dataContext
) {
    auto expr = BindingExtension::Parse(bindingExpression);
    return BindingExtension::ApplyBinding(control, propertyName, expr, dataContext);
}

} // namespace mvvm
} // namespace luaui
