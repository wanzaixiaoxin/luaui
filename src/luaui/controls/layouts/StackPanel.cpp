#include "Panel.h"
#include "Interfaces/IControl.h"
#include "Interfaces/ILayoutable.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include <algorithm>
#include <iostream>

namespace luaui {
namespace controls {

StackPanel::StackPanel() : Panel() {
    std::cout << "[StackPanel] Constructor called (after Panel())" << std::endl;
}

rendering::Size StackPanel::OnMeasureChildren(const rendering::Size& availableSize) {
    bool isHorizontal = (m_orientation == Orientation::Horizontal);
    float totalWidth = 0;
    float totalHeight = 0;
    float maxCross = 0;
    
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        if (auto* layoutable = child->AsLayoutable()) {
            // 强制重新测量子控件
            layoutable->InvalidateMeasure();
            
            interfaces::LayoutConstraint constraint;
            constraint.available = availableSize;
            auto measured = layoutable->Measure(constraint);
            
            if (isHorizontal) {
                totalWidth += measured.width;
                maxCross = std::max(maxCross, measured.height);
            } else {
                totalHeight += measured.height;
                maxCross = std::max(maxCross, measured.width);
            }
        }
    }
    
    // Add spacing
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
    float position = 0;
    
    // Get our render rect from renderable interface
    rendering::Rect renderRect;
    if (auto* render = GetRender()) {
        renderRect = render->GetRenderRect();
    }
    
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        if (auto* layoutable = child->AsLayoutable()) {
            auto desired = layoutable->GetDesiredSize();
            
            rendering::Rect childRect;
            if (isHorizontal) {
                childRect = rendering::Rect(renderRect.x + position, renderRect.y,
                                            desired.width, finalSize.height);
                position += desired.width + m_spacing;
            } else {
                childRect = rendering::Rect(renderRect.x, renderRect.y + position,
                                            finalSize.width, desired.height);
                position += desired.height + m_spacing;
            }
            
            layoutable->Arrange(childRect);
        }
    }
    
    return finalSize;
}

} // namespace controls
} // namespace luaui
