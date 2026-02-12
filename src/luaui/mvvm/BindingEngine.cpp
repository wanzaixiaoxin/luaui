#include "BindingEngine.h"
#include "Control.h"
#include "Logger.h"
#include <regex>
#include <sstream>
#include <algorithm>

// 辅助函数：清理字符串
static std::string Trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

namespace luaui {
namespace mvvm {

// ============================================================================
// PropertyBinding 实现
// ============================================================================
PropertyBinding::PropertyBinding(
    std::shared_ptr<INotifyPropertyChanged> source,
    std::shared_ptr<void> target,
    const BindingExpression& expression,
    std::function<std::any()> getter,
    std::function<void(const std::any&)> setter
)
    : m_source(source)
    , m_target(target)
    , m_expression(expression)
    , m_targetGetter(getter)
    , m_targetSetter(setter)
{
    if (!m_source) {
        throw std::invalid_argument("Source cannot be null");
    }
    
    // 订阅源属性变更
    m_sourceHandler = [this](const PropertyChangedEventArgs& args) {
        OnSourcePropertyChanged(args);
    };
    m_source->SubscribePropertyChanged(m_sourceHandler);
    
    // 初始更新
    UpdateTarget();
}

PropertyBinding::~PropertyBinding() {
    Detach();
}

void PropertyBinding::Detach() {
    if (!m_attached) return;
    
    m_attached = false;
    
    if (m_source) {
        m_source->UnsubscribePropertyChanged(m_sourceHandler);
    }
    
    m_target.reset();
}

void PropertyBinding::UpdateTarget() {
    if (!m_attached || m_updating) return;
    
    auto target = m_target.lock();
    if (!target) {
        Detach();
        return;
    }
    
    m_updating = true;
    
    try {
        std::any value = GetSourceValue();
        
        // 应用转换器
        if (m_expression.converter) {
            value = m_expression.converter->Convert(value, m_expression.converterParameter);
        }
        
        m_targetSetter(value);
    } catch (const std::exception& e) {
        utils::Logger::ErrorF("[Binding] UpdateTarget failed: %s", e.what());
    }
    
    m_updating = false;
}

void PropertyBinding::UpdateSource() {
    if (!m_attached || m_updating) return;
    
    if (m_expression.mode != BindingMode::TwoWay && 
        m_expression.mode != BindingMode::OneWayToSource) {
        return;
    }
    
    m_updating = true;
    
    try {
        std::any value = m_targetGetter();
        
        // 应用转换器（反向）
        if (m_expression.converter) {
            value = m_expression.converter->ConvertBack(value, m_expression.converterParameter);
        }
        
        SetSourceValue(value);
    } catch (const std::exception& e) {
        utils::Logger::ErrorF("[Binding] UpdateSource failed: %s", e.what());
    }
    
    m_updating = false;
}

void PropertyBinding::OnSourcePropertyChanged(const PropertyChangedEventArgs& args) {
    if (!m_attached) return;
    
    // 检查是否是绑定的属性
    if (args.propertyName.empty() || args.propertyName == m_expression.path) {
        UpdateTarget();
    }
}

std::any PropertyBinding::GetSourceValue() {
    // 这里简化处理，实际应该通过反射获取属性值
    // 暂时返回空值，由具体实现提供
    return std::any{};
}

void PropertyBinding::SetSourceValue(const std::any& value) {
    // 这里简化处理，实际应该通过反射设置属性值
    (void)value;
}

// ============================================================================
// BindingEngine 实现
// ============================================================================
BindingEngine& BindingEngine::Instance() {
    static BindingEngine instance;
    return instance;
}

std::shared_ptr<IBinding> BindingEngine::CreateBinding(
    std::shared_ptr<INotifyPropertyChanged> source,
    std::shared_ptr<void> target,
    const BindingExpression& expression,
    std::function<std::any()> getter,
    std::function<void(const std::any&)> setter
) {
    auto binding = std::make_shared<PropertyBinding>(source, target, expression, getter, setter);
    m_bindings.push_back(binding);
    return binding;
}

BindingExpression BindingEngine::ParseExpression(const std::string& expression) {
    BindingExpression result;
    
    // 去除花括号
    std::string expr = expression;
    if (expr.length() >= 2 && expr.front() == '{' && expr.back() == '}') {
        expr = expr.substr(1, expr.length() - 2);
    }
    
    // 去除前缀 "Binding "
    const std::string prefix = "Binding ";
    if (expr.length() >= prefix.length() && expr.substr(0, prefix.length()) == prefix) {
        expr = expr.substr(prefix.length());
    }
    
    // 解析路径和参数
    // 格式: Path, Mode=XXX, Converter=XXX, ...
    std::istringstream iss(expr);
    std::string token;
    
    // 第一个token是路径
    if (std::getline(iss, token, ',')) {
        token = Trim(token);
        // 可能是 "Path=XXX" 或直接的 "XXX"
        if (token.substr(0, 5) == "Path=") {
            result.path = token.substr(5);
        } else {
            result.path = token;
        }
    }
    
    // 解析其他参数
    while (std::getline(iss, token, ',')) {
        token = Trim(token);
        
        auto pos = token.find('=');
        if (pos == std::string::npos) continue;
        
        std::string key = Trim(token.substr(0, pos));
        std::string value = Trim(token.substr(pos + 1));
        
        if (key == "Mode") {
            if (value == "OneWay") result.mode = BindingMode::OneWay;
            else if (value == "TwoWay") result.mode = BindingMode::TwoWay;
            else if (value == "OneWayToSource") result.mode = BindingMode::OneWayToSource;
            else if (value == "OneTime") result.mode = BindingMode::OneTime;
        } else if (key == "Converter") {
            result.converter = GetConverter(value);
        } else if (key == "ConverterParameter") {
            // 去除可能存在的引号（XML属性值可能用单引号或双引号包裹）
            if (value.length() >= 2) {
                if ((value.front() == '\'' && value.back() == '\'') ||
                    (value.front() == '"' && value.back() == '"')) {
                    value = value.substr(1, value.length() - 2);
                }
            }
            result.converterParameter = value;
        } else if (key == "ElementName") {
            result.elementName = value;
        } else if (key == "UpdateSourceTrigger") {
            result.updateSourceTrigger = value;
        }
    }
    
    return result;
}

void BindingEngine::RegisterConverter(const std::string& name, std::shared_ptr<IValueConverter> converter) {
    m_converters[name] = converter;
}

std::shared_ptr<IValueConverter> BindingEngine::GetConverter(const std::string& name) {
    auto it = m_converters.find(name);
    return (it != m_converters.end()) ? it->second : nullptr;
}

void BindingEngine::ClearBindings() {
    for (auto& weakBinding : m_bindings) {
        if (auto binding = weakBinding.lock()) {
            binding->Detach();
        }
    }
    m_bindings.clear();
}

void BindingEngine::ClearBindingsForTarget(void* target) {
    for (auto& weakBinding : m_bindings) {
        if (auto binding = weakBinding.lock()) {
            if (binding->GetTarget().lock().get() == target) {
                binding->Detach();
            }
        }
    }
}

void BindingEngine::UpdateAllBindings() {
    for (auto& weakBinding : m_bindings) {
        if (auto binding = weakBinding.lock()) {
            binding->UpdateTarget();
        }
    }
}

// ============================================================================
// 便捷函数
// ============================================================================
std::shared_ptr<IBinding> Bind(
    std::shared_ptr<Control> target,
    const std::string& targetProperty,
    std::shared_ptr<INotifyPropertyChanged> source,
    const BindingExpression& expression
) {
    // 这里需要获取控件属性的getter/setter
    // 暂时返回nullptr，由具体控件实现
    (void)target;
    (void)targetProperty;
    (void)source;
    (void)expression;
    return nullptr;
}

} // namespace mvvm
} // namespace luaui
