#include "Panel.h"
#include "Interfaces/ILayoutable.h"
#include "Interfaces/IRenderable.h"
#include <algorithm>

namespace luaui {
namespace controls {

// ============================================================================
// StackPanel
// ============================================================================
StackPanel::StackPanel() {}

rendering::Size StackPanel::OnMeasureChildren(const rendering::Size& availableSize) {
    rendering::Size desired;
    bool isVertical = (m_orientation == Orientation::Vertical);
    size_t visibleCount = 0;
    
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        auto* layoutable = child->AsLayoutable();
        if (!layoutable) continue;
        
        interfaces::LayoutConstraint constraint;
        constraint.available = availableSize;
        layoutable->Measure(constraint);
        auto childSize = layoutable->GetDesiredSize();
        
        if (isVertical) {
            desired.width = std::max(desired.width, childSize.width);
            desired.height += childSize.height;
        } else {
            desired.width += childSize.width;
            desired.height = std::max(desired.height, childSize.height);
        }
        visibleCount++;
    }
    
    // Add spacing
    if (visibleCount > 1) {
        if (isVertical) {
            desired.height += m_spacing * (visibleCount - 1);
        } else {
            desired.width += m_spacing * (visibleCount - 1);
        }
    }
    
    return desired;
}

rendering::Size StackPanel::OnArrangeChildren(const rendering::Size& finalSize) {
    bool isVertical = (m_orientation == Orientation::Vertical);
    float position = 0;
    
    // Get our render rect from renderable interface
    rendering::Rect renderRect;
    if (auto* renderable = AsRenderable()) {
        renderRect = renderable->GetRenderRect();
    }
    
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        auto* layoutable = child->AsLayoutable();
        if (!layoutable) continue;
        
        auto childSize = layoutable->GetDesiredSize();
        
        rendering::Rect childRect;
        if (isVertical) {
            childRect = rendering::Rect(renderRect.x, renderRect.y + position,
                                        childSize.width, 
                                        childSize.height);
            position += childSize.height + m_spacing;
        } else {
            childRect = rendering::Rect(renderRect.x + position, renderRect.y,
                                        childSize.width, 
                                        childSize.height);
            position += childSize.width + m_spacing;
        }
        
        layoutable->Arrange(childRect);
    }
    
    return finalSize;
}

} // namespace controls
} // namespace luaui
