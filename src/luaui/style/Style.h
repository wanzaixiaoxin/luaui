#pragma once

#include "Control.h"
#include "Components/RenderComponent.h"
#include "Components/LayoutComponent.h"
#include <functional>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace luaui {
namespace controls {

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
    
    // 静态辅助方法：创建常用 Setter
    static Setter BackgroundSetter(const rendering::Color& color) {
        return Setter([color](Control* ctrl) {
            if (auto* render = ctrl->GetRender()) {
                render->SetBackground(color);
            }
        });
    }
    
    static Setter WidthSetter(float width) {
        return Setter([width](Control* ctrl) {
            if (auto* layout = ctrl->GetLayout()) {
                layout->SetWidth(width);
            }
        });
    }
    
    static Setter HeightSetter(float height) {
        return Setter([height](Control* ctrl) {
            if (auto* layout = ctrl->GetLayout()) {
                layout->SetHeight(height);
            }
        });
    }

private:
    std::vector<Setter> m_setters;
};

/**
 * @brief 主题类（简化版）
 */
class Theme {
public:
    using Ptr = std::shared_ptr<Theme>;
    
    // 获取/设置默认样式
    void SetDefaultStyle(const std::string& controlType, Style::Ptr style) {
        m_defaultStyles[controlType] = style;
    }
    
    Style::Ptr GetDefaultStyle(const std::string& controlType) const {
        auto it = m_defaultStyles.find(controlType);
        return (it != m_defaultStyles.end()) ? it->second : nullptr;
    }
    
    // 应用默认样式到控件
    void ApplyDefaultStyle(Control* control) const {
        if (!control) return;
        auto style = GetDefaultStyle(control->GetTypeName());
        if (style) {
            style->Apply(control);
        }
    }
    
    // 获取全局主题实例
    static Theme& GetCurrent() {
        static Theme instance;
        return instance;
    }

private:
    std::unordered_map<std::string, Style::Ptr> m_defaultStyles;
};

} // namespace controls
} // namespace luaui
