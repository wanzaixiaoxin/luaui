#pragma once
#include "Control.h"
#include "ResourceDictionary.h"
#include <variant>
#include <any>

namespace luaui {
namespace controls {

// Setter 值可以是直接值或资源引用
using SetterValue = std::variant<std::any, ResourceReference>;

// 属性设置器 - 用于 Style 中定义属性值
class Setter {
public:
    Setter() : m_propertyId(0) {}
    Setter(DependencyProperty::Id propertyId, const std::any& value)
        : m_propertyId(propertyId), m_value(value) {}
    Setter(DependencyProperty::Id propertyId, const ResourceReference& ref)
        : m_propertyId(propertyId), m_value(ref) {}
    
    // 获取属性ID
    DependencyProperty::Id GetPropertyId() const { return m_propertyId; }
    void SetPropertyId(DependencyProperty::Id id) { m_propertyId = id; }
    
    // 获取原始值（可能是 ResourceReference）
    const SetterValue& GetValue() const { return m_value; }
    void SetValue(const std::any& value) { m_value = value; }
    void SetValue(const ResourceReference& ref) { m_value = ref; }
    
    // 应用 Setter 到控件
    void Apply(Control* target, const ResourceDictionary* resources) const;
    
    // 获取解析后的值
    std::any GetResolvedValue(const ResourceDictionary* resources) const;
    
    // 检查是否有值
    bool HasValue() const;
    
private:
    DependencyProperty::Id m_propertyId;
    SetterValue m_value;
};

} // namespace controls
} // namespace luaui
