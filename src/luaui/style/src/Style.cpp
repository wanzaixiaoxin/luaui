#include "Style.h"

namespace luaui {
namespace controls {

Style::Style(const std::type_index& targetType)
    : m_targetType(targetType) {
}

void Style::AddSetter(const Setter& setter) {
    m_setters.push_back(setter);
}

void Style::AddSetter(DependencyProperty::Id propertyId, const std::any& value) {
    m_setters.emplace_back(propertyId, value);
}

void Style::AddTrigger(Trigger::Ptr trigger) {
    if (trigger) {
        m_triggers.push_back(trigger);
    }
}

void Style::Apply(Control* target, const ResourceDictionary* resources) {
    if (!target) {
        return;
    }
    
    // 检查类型匹配
    if (!CanApplyTo(typeid(*target))) {
        return;
    }
    
    // 如果已经应用，先取消应用
    if (IsAppliedTo(target)) {
        Unapply(target);
    }
    
    AppliedTargetInfo targetInfo;
    
    // 获取所有 Setter（包括 BasedOn 的）
    auto allSetters = GetAllSetters();
    
    // 应用所有 Setter
    targetInfo.setterInfos.reserve(allSetters.size());
    for (const auto& setter : allSetters) {
        auto info = ApplySetter(target, setter, resources);
        targetInfo.setterInfos.push_back(info);
    }
    
    // 为每个 Trigger 创建副本并附加到目标
    // 这样每个控件都有独立的 Trigger 实例，避免状态冲突
    auto allTriggers = GetAllTriggers();
    for (auto& trigger : allTriggers) {
        // 创建 Trigger 的副本（如果是 PropertyTrigger）
        if (auto propTrigger = std::dynamic_pointer_cast<PropertyTrigger>(trigger)) {
            auto triggerCopy = std::make_shared<PropertyTrigger>(
                propTrigger->GetPropertyId(), 
                propTrigger->GetTriggerValue()
            );
            // 复制 Setters
            for (const auto& setter : propTrigger->GetSetters()) {
                triggerCopy->AddSetter(setter);
            }
            triggerCopy->Attach(target, resources);
            targetInfo.triggerInstances.push_back(triggerCopy);
        } else {
            // 其他类型的 Trigger 直接使用原实例（假设它们支持多目标）
            trigger->Attach(target, resources);
            targetInfo.triggerInstances.push_back(trigger);
        }
    }
    
    // 保存应用记录
    m_appliedTargets[target] = std::move(targetInfo);
}

void Style::Unapply(Control* target) {
    if (!target) {
        return;
    }
    
    auto it = m_appliedTargets.find(target);
    if (it == m_appliedTargets.end()) {
        return;
    }
    
    // 分离所有 Trigger
    for (auto& trigger : it->second.triggerInstances) {
        trigger->Detach(target);
    }
    
    // 恢复原始值
    for (const auto& info : it->second.setterInfos) {
        if (info.wasSetLocally) {
            // 恢复原始值
            target->SetValue(info.propertyId, info.originalValue);
        } else {
            // 清除本地值，回退到默认值
            target->ClearValue(info.propertyId);
        }
    }
    
    m_appliedTargets.erase(it);
}

bool Style::CanApplyTo(const std::type_index& type) const {
    // 简单类型检查，实际应该使用 is_base_of
    return m_targetType == type;
}

bool Style::IsAppliedTo(const Control* target) const {
    return m_appliedTargets.find(target) != m_appliedTargets.end();
}

std::vector<Setter> Style::GetAllSetters() const {
    std::vector<Setter> result;
    CollectSetters(result);
    return result;
}

std::vector<Trigger::Ptr> Style::GetAllTriggers() const {
    std::vector<Trigger::Ptr> result;
    CollectTriggers(result);
    return result;
}

Style::AppliedSetterInfo Style::ApplySetter(Control* target, const Setter& setter, 
                                            const ResourceDictionary* resources) {
    AppliedSetterInfo info;
    info.propertyId = setter.GetPropertyId();
    
    // 保存原始值和设置状态
    info.originalValue = target->GetValue(info.propertyId);
    info.wasSetLocally = target->HasLocalValue(info.propertyId);
    
    // 应用 Setter
    setter.Apply(target, resources);
    
    return info;
}

void Style::CollectSetters(std::vector<Setter>& result) const {
    // 先收集 BasedOn 的 Setter（优先级较低）
    if (m_basedOn) {
        m_basedOn->CollectSetters(result);
    }
    
    // 再添加自己的 Setter（可能覆盖 BasedOn 的）
    result.insert(result.end(), m_setters.begin(), m_setters.end());
}

void Style::CollectTriggers(std::vector<Trigger::Ptr>& result) const {
    // 先收集 BasedOn 的 Trigger
    if (m_basedOn) {
        m_basedOn->CollectTriggers(result);
    }
    
    // 再添加自己的 Trigger
    result.insert(result.end(), m_triggers.begin(), m_triggers.end());
}

void Style::NotifyTriggersOfPropertyChange(Control* target, DependencyProperty::Id propertyId) {
    auto it = m_appliedTargets.find(target);
    if (it != m_appliedTargets.end()) {
        for (auto& trigger : it->second.triggerInstances) {
            trigger->OnPropertyChanged(target, propertyId);
        }
    }
}

} // namespace controls
} // namespace luaui
