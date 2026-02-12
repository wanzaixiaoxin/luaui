#pragma once

#include "BindingEngine.h"
#include <memory>
#include <functional>

// 前向声明
namespace luaui {
    class Control;
namespace controls {
    class Button;
    class Slider;
}
namespace xml {
    class IXmlLoader;
}
}

namespace luaui {
namespace mvvm {

// ============================================================================
// 绑定标记扩展 - 解析 {Binding ...} 语法
// ============================================================================
class BindingExtension {
public:
    // 解析绑定表达式
    // 支持格式:
    //   {Binding PropertyName}
    //   {Binding PropertyName, Mode=TwoWay}
    //   {Binding PropertyName, Converter=ConverterName}
    //   {Binding ElementName=slider, Path=Value}
    static BindingExpression Parse(const std::string& expression);
    
    // 判断是否为绑定表达式
    static bool IsBindingExpression(const std::string& value);
    
    // 应用绑定到控件
    static std::shared_ptr<IBinding> ApplyBinding(
        std::shared_ptr<luaui::Control> control,
        const std::string& propertyName,
        const BindingExpression& expression,
        std::shared_ptr<INotifyPropertyChanged> dataContext
    );
};

// ============================================================================
// 绑定助手类 - 简化ViewModel绑定
// ============================================================================
class BindingHelper {
public:
    // 设置DataContext到控件树
    static void SetDataContext(
        std::shared_ptr<luaui::Control> root,
        std::shared_ptr<INotifyPropertyChanged> context
    );
    
    // 绑定控件属性
    static std::shared_ptr<IBinding> BindProperty(
        std::shared_ptr<luaui::Control> control,
        const std::string& propertyName,
        const std::string& bindingExpression,
        std::shared_ptr<INotifyPropertyChanged> dataContext
    );
};

} // namespace mvvm
} // namespace luaui
