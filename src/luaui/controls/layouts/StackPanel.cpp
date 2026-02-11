#include "Panel.h"
#include "Interfaces/IControl.h"
#include "Interfaces/ILayoutable.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include <algorithm>

namespace luaui {
namespace controls {

StackPanel::StackPanel() {}

rendering::Size StackPanel::OnMeasureChildren(const rendering::Size& availableSize) {
    bool isHorizontal = (m_orientation == Orientation::Horizontal);
    float totalWidth = 0;
    float totalHeight = 0;
    float maxCross = 0;
    
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        if (auto* layoutable = child->AsLayoutable()) {
            interfaces::LayoutConstraint constraint;
            constraint.available = availableSize;
            layoutable->Measure(constraint);
            auto desired = layoutable->GetDesiredSize();
            
            if (isHorizontal) {
                totalWidth += desired.width;
                maxCross = std::max(maxCross, desired.height);
            } else {
                totalHeight += desired.height;
                maxCross = std::max(maxCross, desired.width);
            }
        }
    }
    
    // 添加间距
    if (!m_children.empty()) {
        if (isHorizontal) {
            totalWidth += m_spacing * (m_children.size() - 1);
        } else {
            totalHeight += m_spacing * (m_children.size() - 1);
        }
    }
    
    if (isHorizontal) {
        return rendering::Size(totalWidth, maxCross);
    } else {
        return rendering::Size(maxCross, totalHeight);
    }
}

rendering::Size StackPanel::OnArrangeChildren(const rendering::Size& finalSize) {
    bool isHorizontal = (m_orientation == Orientation::Horizontal);
    
    auto* render = GetRender();
    if (!render) return finalSize;
    
    auto contentRect = render->GetRenderRect();
    float pos = isHorizontal ? contentRect.x : contentRect.y;
    
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        if (auto* layoutable = child->AsLayoutable()) {
            auto desired = layoutable->GetDesiredSize();
            
            rendering::Rect rect;
            if (isHorizontal) {
                rect = rendering::Rect(pos, contentRect.y, desired.width, desired.height);
                pos += desired.width + m_spacing;
            } else {
                rect = rendering::Rect(contentRect.x, pos, desired.width, desired.height);
                pos += desired.height + m_spacing;
            }
            
            layoutable->Arrange(rect);
        }
    }
    
    return finalSize;
}

} // namespace controls
} // namespace luaui
