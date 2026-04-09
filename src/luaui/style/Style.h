#pragma once

#include <functional>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace luaui {
namespace controls {

class Control;

/**
 * @brief Style 属性设置器（新架构简化版）
 * 
 * 使用回调函数而非依赖属性系统
 */
class Setter {
public:
    using PropertyApplier = std::function<void(Control*)>;
    
    Setter() = default;
    Setter(PropertyApplier applier) : m_applier(applier) {}
    
    void Apply(Control* target) const {
        if (m_applier && target) {
            m_applier(target);
        }
    }
    
    bool HasApplier() const { return m_applier != nullptr; }

private:
    PropertyApplier m_applier;
};

/**
 * @brief Style 类（新架构简化版）
 * 
 * 用于批量设置控件属性
 */
class Style : public std::enable_shared_from_this<Style> {
public:
    using Ptr = std::shared_ptr<Style>;
    using ConstPtr = std::shared_ptr<const Style>;
    
    Style() = default;
    
    // Setter 管理
    void AddSetter(const Setter& setter) { m_setters.push_back(setter); }
    void ClearSetters() { m_setters.clear(); }
    const std::vector<Setter>& GetSetters() const { return m_setters; }
    
    // 应用样式到控件
    void Apply(Control* target) const {
        if (!target) return;
        for (const auto& setter : m_setters) {
            setter.Apply(target);
        }
    }

private:
    std::vector<Setter> m_setters;
};

} // namespace controls
} // namespace luaui
