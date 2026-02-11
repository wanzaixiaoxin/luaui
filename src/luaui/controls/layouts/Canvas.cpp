#include "layouts/Canvas.h"
#include "Interfaces/IControl.h"
#include "Interfaces/ILayoutable.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include <algorithm>

namespace luaui {
namespace controls {

// 静态成员定义
std::unordered_map<ControlID, rendering::Point> Canvas::s_positions;

Canvas::Canvas() {}

void Canvas::SetLeft(const std::shared_ptr<IControl>& control, float left) {
    if (!control) return;
    s_positions[control->GetID()].x = left;
}

void Canvas::SetTop(const std::shared_ptr<IControl>& control, float top) {
    if (!control) return;
    s_positions[control->GetID()].y = top;
}

void Canvas::SetRight(const std::shared_ptr<IControl>& control, float right) {
    // Canvas 不直接支持 Right，可以通过计算实现
    // 简化：忽略此属性
}

void Canvas::SetBottom(const std::shared_ptr<IControl>& control, float bottom) {
    // Canvas 不直接支持 Bottom，可以通过计算实现
    // 简化：忽略此属性
}

float Canvas::GetLeft(const std::shared_ptr<IControl>& control) {
    if (!control) return 0;
    auto it = s_positions.find(control->GetID());
    return (it != s_positions.end()) ? it->second.x : 0;
}

float Canvas::GetTop(const std::shared_ptr<IControl>& control) {
    if (!control) return 0;
    auto it = s_positions.find(control->GetID());
    return (it != s_positions.end()) ? it->second.y : 0;
}

float Canvas::GetRight(const std::shared_ptr<IControl>& control) {
    // 简化实现
    return 0;
}

float Canvas::GetBottom(const std::shared_ptr<IControl>& control) {
    // 简化实现
    return 0;
}

rendering::Size Canvas::OnMeasureChildren(const rendering::Size& availableSize) {
    // Canvas 给子控件无限空间进行测量
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        if (auto* layoutable = child->AsLayoutable()) {
            interfaces::LayoutConstraint constraint;
            constraint.available = rendering::Size(99999, 99999);
            layoutable->Measure(constraint);
        }
    }
    
    // 计算最大边界
    float maxWidth = 0;
    float maxHeight = 0;
    
    auto* render = GetRender();
    auto contentRect = render ? render->GetRenderRect() : rendering::Rect(0, 0, 0, 0);
    
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        float left = GetLeft(child);
        float top = GetTop(child);
        
        if (auto* layoutable = child->AsLayoutable()) {
            auto desired = layoutable->GetDesiredSize();
            maxWidth = std::max(maxWidth, left + desired.width);
            maxHeight = std::max(maxHeight, top + desired.height);
        }
    }
    
    return rendering::Size(maxWidth, maxHeight);
}

rendering::Size Canvas::OnArrangeChildren(const rendering::Size& finalSize) {
    auto* render = GetRender();
    if (!render) return finalSize;
    
    auto contentRect = render->GetRenderRect();
    
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        if (auto* layoutable = child->AsLayoutable()) {
            float left = GetLeft(child);
            float top = GetTop(child);
            auto desired = layoutable->GetDesiredSize();
            
            layoutable->Arrange(rendering::Rect(
                contentRect.x + left,
                contentRect.y + top,
                desired.width,
                desired.height
            ));
        }
    }
    
    return finalSize;
}

} // namespace controls
} // namespace luaui
