#pragma once
#include "Setter.h"
#include "Control.h"
#include <functional>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

namespace luaui {
namespace controls {

// 触发器基类
class Trigger : public std::enable_shared_from_this<Trigger> {
public:
    using Ptr = std::shared_ptr<Trigger>;
    
    virtual ~Trigger() = default;
    
    // 附加/分离到控件
    virtual void Attach(Control* target, const ResourceDictionary* resources) = 0;
    virtual void Detach(Control* target) = 0;
    
    // 获取触发器是否处于激活状态
    virtual bool IsActive() const = 0;
    
    // 获取触发器名称（用于调试）
    virtual std::string GetName() const = 0;
    
    // 属性变化通知
    virtual void OnPropertyChanged(Control* control, DependencyProperty::Id propertyId) {}
};

// 属性触发器 (Property Trigger)
// 当指定属性等于特定值时激活
class PropertyTrigger : public Trigger {
public:
    PropertyTrigger(DependencyProperty::Id propertyId, const std::any& value);
    
    // Setter 管理
    void AddSetter(const Setter& setter);
    void AddSetter(DependencyProperty::Id propertyId, const std::any& value);
    const std::vector<Setter>& GetSetters() const { return m_setters; }
    void ClearSetters() { m_setters.clear(); }
    
    // 获取触发器属性信息（用于复制）
    DependencyProperty::Id GetPropertyId() const { return m_propertyId; }
    const std::any& GetTriggerValue() const { return m_triggerValue; }
    
    // Trigger 接口实现
    void Attach(Control* target, const ResourceDictionary* resources) override;
    void Detach(Control* target) override;
    bool IsActive() const override { return m_isActive; }
    std::string GetName() const override { return "PropertyTrigger"; }
    
    // 手动触发评估（用于初始化时检查当前状态）
    void Evaluate();
    
    // 属性变化通知
    void OnPropertyChanged(Control* control, DependencyProperty::Id propertyId) override;
    
private:
    DependencyProperty::Id m_propertyId;
    std::any m_triggerValue;
    std::vector<Setter> m_setters;
    
    Control* m_attachedTarget = nullptr;
    const ResourceDictionary* m_resources = nullptr;
    bool m_isActive = false;
    
    // 记录原始值，用于恢复
    std::unordered_map<DependencyProperty::Id, std::any> m_originalValues;
    
    void Enter();
    void Exit();
    
    // 比较两个值是否相等
    bool ValuesEqual(const std::any& a, const std::any& b) const;
};

// 数据触发器 (Data Trigger) - 基于绑定表达式
// TODO: 在数据绑定系统完成后实现
class DataTrigger : public Trigger {
public:
    DataTrigger() = default;
    
    void Attach(Control* target, const ResourceDictionary* resources) override {}
    void Detach(Control* target) override {}
    bool IsActive() const override { return false; }
    std::string GetName() const override { return "DataTrigger"; }
};

// 事件触发器 (Event Trigger)
// 当指定事件发生时触发动作
class EventTrigger : public Trigger {
public:
    explicit EventTrigger(const std::string& eventName);
    
    // 动作管理
    void AddAction(std::function<void(Control*)> action);
    void ClearActions() { m_actions.clear(); }
    
    // Trigger 接口实现
    void Attach(Control* target, const ResourceDictionary* resources) override;
    void Detach(Control* target) override;
    bool IsActive() const override { return false; } // 事件触发器无持续状态
    std::string GetName() const override { return "EventTrigger:" + m_eventName; }
    
private:
    std::string m_eventName;
    std::vector<std::function<void(Control*)>> m_actions;
    Control* m_attachedTarget = nullptr;
    
    void OnEvent(Control* sender);
};

// 多条件触发器 (MultiTrigger)
// 多个条件同时满足时激活
class MultiTrigger : public Trigger {
public:
    struct Condition {
        DependencyProperty::Id propertyId;
        std::any value;
        
        Condition(DependencyProperty::Id id, const std::any& val) 
            : propertyId(id), value(val) {}
    };
    
    // 条件管理
    void AddCondition(DependencyProperty::Id propertyId, const std::any& value);
    void AddSetter(const Setter& setter);
    void AddSetter(DependencyProperty::Id propertyId, const std::any& value);
    
    // Trigger 接口实现
    void Attach(Control* target, const ResourceDictionary* resources) override;
    void Detach(Control* target) override;
    bool IsActive() const override { return m_isActive; }
    std::string GetName() const override { return "MultiTrigger"; }
    
    // 评估所有条件
    void Evaluate();
    
    // 属性变化通知
    void OnPropertyChanged(Control* control, DependencyProperty::Id propertyId) override;
    
private:
    std::vector<Condition> m_conditions;
    std::vector<Setter> m_setters;
    
    Control* m_attachedTarget = nullptr;
    const ResourceDictionary* m_resources = nullptr;
    bool m_isActive = false;
    
    std::unordered_map<DependencyProperty::Id, std::any> m_originalValues;
    
    void Enter();
    void Exit();
    bool CheckAllConditions() const;
    bool ValuesEqual(const std::any& a, const std::any& b) const;
};

} // namespace controls
} // namespace luaui
