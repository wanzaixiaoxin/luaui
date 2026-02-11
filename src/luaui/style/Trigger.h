#pragma once

#include "Style.h"
#include <functional>
#include <string>
#include <memory>

namespace luaui {
namespace controls {

/**
 * @brief 属性触发器（简化版）
 */
class Trigger : public std::enable_shared_from_this<Trigger> {
public:
    using Ptr = std::shared_ptr<Trigger>;
    
    // 条件检查函数
    using ConditionCheck = std::function<bool(Control*)>;
    
    Trigger() = default;
    Trigger(ConditionCheck condition) : m_condition(condition) {}
    
    // 设置条件
    void SetCondition(ConditionCheck condition) { m_condition = condition; }
    
    // 添加 Setter
    void AddSetter(const Setter& setter) { m_setters.push_back(setter); }
    
    // 检查条件并应用
    void Update(Control* target) {
        if (!target || !m_condition) return;
        
        bool shouldApply = m_condition(target);
        if (shouldApply != m_wasApplied) {
            if (shouldApply) {
                // 应用 Setter
                for (const auto& setter : m_setters) {
                    setter.Apply(target);
                }
            }
            m_wasApplied = shouldApply;
        }
    }

private:
    ConditionCheck m_condition;
    std::vector<Setter> m_setters;
    bool m_wasApplied = false;
};

} // namespace controls
} // namespace luaui
