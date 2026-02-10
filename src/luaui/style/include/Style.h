#pragma once
#include "Setter.h"
#include "Trigger.h"
#include <vector>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <memory>

namespace luaui {
namespace controls {

// 样式类 - 定义一组 Setter 和 Trigger
class Style : public std::enable_shared_from_this<Style> {
public:
    using Ptr = std::shared_ptr<Style>;
    using ConstPtr = std::shared_ptr<const Style>;
    
    // 构造时指定目标类型
    explicit Style(const std::type_index& targetType);
    
    // 目标类型
    const std::type_index& GetTargetType() const { return m_targetType; }
    void SetTargetType(const std::type_index& type) { m_targetType = type; }
    
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
    void Apply(Control* target, const ResourceDictionary* resources);
    void Unapply(Control* target);
    
    // 检查样式是否适用于指定类型
    bool CanApplyTo(const std::type_index& type) const;
    
    // 检查是否已经应用到指定控件
    bool IsAppliedTo(const Control* target) const;
    
    // 获取所有 Setter（包括 BasedOn 继承的）
    std::vector<Setter> GetAllSetters() const;
    
    // 获取所有 Trigger（包括 BasedOn 继承的）
    std::vector<Trigger::Ptr> GetAllTriggers() const;
    
    // 通知所有 Trigger 属性变化
    void NotifyTriggersOfPropertyChange(Control* target, DependencyProperty::Id propertyId);
    
private:
    // 应用 Setter 时记录的信息
    struct AppliedSetterInfo {
        DependencyProperty::Id propertyId;
        std::any originalValue;
        bool wasSetLocally;  // 原来是否是本地设置的值
    };
    
    // 为每个控件存储的应用信息（包括独立的 Trigger 实例）
    struct AppliedTargetInfo {
        std::vector<AppliedSetterInfo> setterInfos;
        std::vector<Trigger::Ptr> triggerInstances;  // 每个控件独立的 Trigger 副本
    };
    
    std::type_index m_targetType;
    Style::Ptr m_basedOn;
    std::vector<Setter> m_setters;
    std::vector<Trigger::Ptr> m_triggers;
    
    // 记录已应用的控件
    std::unordered_map<const Control*, AppliedTargetInfo> m_appliedTargets;
    
    // 应用单个 Setter，返回应用信息
    AppliedSetterInfo ApplySetter(Control* target, const Setter& setter, 
                                   const ResourceDictionary* resources);
    
    // 收集所有 Setter（递归 BasedOn）
    void CollectSetters(std::vector<Setter>& result) const;
    
    // 收集所有 Trigger（递归 BasedOn）
    void CollectTriggers(std::vector<Trigger::Ptr>& result) const;
};

} // namespace controls
} // namespace luaui
