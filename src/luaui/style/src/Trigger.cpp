#include "Trigger.h"
#include "Logger.h"
#include <typeinfo>

namespace luaui {
namespace controls {

// PropertyTrigger 实现
PropertyTrigger::PropertyTrigger(DependencyProperty::Id propertyId, const std::any& value)
    : m_propertyId(propertyId), m_triggerValue(value) {
}

void PropertyTrigger::AddSetter(const Setter& setter) {
    m_setters.push_back(setter);
}

void PropertyTrigger::AddSetter(DependencyProperty::Id propertyId, const std::any& value) {
    m_setters.emplace_back(propertyId, value);
}

void PropertyTrigger::Attach(Control* target, const ResourceDictionary* resources) {
    if (!target) {
        return;
    }
    
    // Support attaching to multiple targets (style shared by multiple controls)
    if (!m_attachedTarget) {
        m_attachedTarget = target;
        m_resources = resources;
    } else if (m_attachedTarget != target) {
        // Already attached to a different target, cannot attach to another
        // Each control should have its own trigger instance when style is shared
        return;
    }
    
    // 订阅属性变更通知
    auto* prop = DependencyProperty::GetById(m_propertyId);
    if (prop && prop->GetMetadata().changedCallback) {
        // 注意：这里需要控件支持属性变更回调
        // 目前通过 Evaluate 在初始化时检查
    }
    
    // 初始评估
    Evaluate();
}

void PropertyTrigger::Detach(Control* target) {
    if (m_attachedTarget != target) {
        return;
    }
    
    // 如果处于激活状态，先退出
    if (m_isActive) {
        Exit();
    }
    
    m_attachedTarget = nullptr;
    m_resources = nullptr;
}

void PropertyTrigger::Evaluate() {
    if (!m_attachedTarget) {
        return;
    }
    
    // 获取当前属性值
    std::any currentValue = m_attachedTarget->GetValue(m_propertyId);
    
    // 检查是否满足条件
    bool shouldBeActive = ValuesEqual(currentValue, m_triggerValue);
    
    if (shouldBeActive && !m_isActive) {
        Enter();
    } else if (!shouldBeActive && m_isActive) {
        Exit();
    }
}

void PropertyTrigger::OnPropertyChanged(Control* control, DependencyProperty::Id propertyId) {
    if (propertyId != m_propertyId) {
        return;
    }
    Evaluate();
}

void PropertyTrigger::Enter() {
    if (!m_attachedTarget || m_isActive) {
        return;
    }
    
    m_isActive = true;
    
    // 保存原始值并应用 Setter
    for (const auto& setter : m_setters) {
        DependencyProperty::Id propId = setter.GetPropertyId();
        
        // 保存原始值（如果还没保存）
        if (m_originalValues.find(propId) == m_originalValues.end()) {
            m_originalValues[propId] = m_attachedTarget->GetValue(propId);
        }
        
        // 应用 Setter
        setter.Apply(m_attachedTarget, m_resources);
    }
}

void PropertyTrigger::Exit() {
    if (!m_attachedTarget || !m_isActive) {
        return;
    }
    
    m_isActive = false;
    
    // 恢复原始值
    for (const auto& [propId, originalValue] : m_originalValues) {
        m_attachedTarget->SetValue(propId, originalValue);
    }
    m_originalValues.clear();
}

bool PropertyTrigger::ValuesEqual(const std::any& a, const std::any& b) const {
    if (!a.has_value() || !b.has_value()) {
        return !a.has_value() && !b.has_value();
    }
    
    // 尝试比较常见类型
    try {
        if (a.type() == b.type()) {
            if (a.type() == typeid(bool)) {
                return std::any_cast<bool>(a) == std::any_cast<bool>(b);
            }
            if (a.type() == typeid(int)) {
                return std::any_cast<int>(a) == std::any_cast<int>(b);
            }
            if (a.type() == typeid(float)) {
                return std::any_cast<float>(a) == std::any_cast<float>(b);
            }
            if (a.type() == typeid(double)) {
                return std::any_cast<double>(a) == std::any_cast<double>(b);
            }
            if (a.type() == typeid(std::string)) {
                return std::any_cast<std::string>(a) == std::any_cast<std::string>(b);
            }
        }
    } catch (...) {
    }
    
    // 默认使用 type_info 比较（不安全，但对于大多数 UI 场景足够）
    return false;
}

// EventTrigger 实现
EventTrigger::EventTrigger(const std::string& eventName)
    : m_eventName(eventName) {
}

void EventTrigger::AddAction(std::function<void(Control*)> action) {
    m_actions.push_back(std::move(action));
}

void EventTrigger::Attach(Control* target, const ResourceDictionary* resources) {
    if (!target || m_attachedTarget) {
        return;
    }
    
    m_attachedTarget = target;
    
    // TODO: 订阅事件
    // 需要在 Control 中添加事件订阅机制
}

void EventTrigger::Detach(Control* target) {
    if (m_attachedTarget != target) {
        return;
    }
    
    // TODO: 取消订阅事件
    
    m_attachedTarget = nullptr;
}

void EventTrigger::OnEvent(Control* sender) {
    for (auto& action : m_actions) {
        if (action) {
            action(sender);
        }
    }
}

// MultiTrigger 实现
void MultiTrigger::AddCondition(DependencyProperty::Id propertyId, const std::any& value) {
    m_conditions.emplace_back(propertyId, value);
}

void MultiTrigger::AddSetter(const Setter& setter) {
    m_setters.push_back(setter);
}

void MultiTrigger::AddSetter(DependencyProperty::Id propertyId, const std::any& value) {
    m_setters.emplace_back(propertyId, value);
}

void MultiTrigger::Attach(Control* target, const ResourceDictionary* resources) {
    if (!target || m_attachedTarget) {
        return;
    }
    
    m_attachedTarget = target;
    m_resources = resources;
    
    // 初始评估
    Evaluate();
}

void MultiTrigger::Detach(Control* target) {
    if (m_attachedTarget != target) {
        return;
    }
    
    if (m_isActive) {
        Exit();
    }
    
    m_attachedTarget = nullptr;
    m_resources = nullptr;
}

void MultiTrigger::Evaluate() {
    if (!m_attachedTarget) {
        return;
    }
    
    bool shouldBeActive = CheckAllConditions();
    
    if (shouldBeActive && !m_isActive) {
        Enter();
    } else if (!shouldBeActive && m_isActive) {
        Exit();
    }
}

void MultiTrigger::OnPropertyChanged(Control* control, DependencyProperty::Id propertyId) {
    // 检查是否是关注的属性
    bool isRelevant = false;
    for (const auto& cond : m_conditions) {
        if (cond.propertyId == propertyId) {
            isRelevant = true;
            break;
        }
    }
    
    if (isRelevant) {
        Evaluate();
    }
}

void MultiTrigger::Enter() {
    if (!m_attachedTarget || m_isActive) {
        return;
    }
    
    m_isActive = true;
    
    for (const auto& setter : m_setters) {
        DependencyProperty::Id propId = setter.GetPropertyId();
        
        if (m_originalValues.find(propId) == m_originalValues.end()) {
            m_originalValues[propId] = m_attachedTarget->GetValue(propId);
        }
        
        setter.Apply(m_attachedTarget, m_resources);
    }
}

void MultiTrigger::Exit() {
    if (!m_attachedTarget || !m_isActive) {
        return;
    }
    
    m_isActive = false;
    
    for (const auto& [propId, originalValue] : m_originalValues) {
        m_attachedTarget->SetValue(propId, originalValue);
    }
    m_originalValues.clear();
}

bool MultiTrigger::CheckAllConditions() const {
    for (const auto& cond : m_conditions) {
        std::any currentValue = m_attachedTarget->GetValue(cond.propertyId);
        if (!ValuesEqual(currentValue, cond.value)) {
            return false;
        }
    }
    return true;
}

bool MultiTrigger::ValuesEqual(const std::any& a, const std::any& b) const {
    if (!a.has_value() || !b.has_value()) {
        return !a.has_value() && !b.has_value();
    }
    
    try {
        if (a.type() == b.type()) {
            if (a.type() == typeid(bool)) {
                return std::any_cast<bool>(a) == std::any_cast<bool>(b);
            }
            if (a.type() == typeid(int)) {
                return std::any_cast<int>(a) == std::any_cast<int>(b);
            }
            if (a.type() == typeid(float)) {
                return std::any_cast<float>(a) == std::any_cast<float>(b);
            }
            if (a.type() == typeid(double)) {
                return std::any_cast<double>(a) == std::any_cast<double>(b);
            }
            if (a.type() == typeid(std::string)) {
                return std::any_cast<std::string>(a) == std::any_cast<std::string>(b);
            }
        }
    } catch (...) {
    }
    
    return false;
}

} // namespace controls
} // namespace luaui
