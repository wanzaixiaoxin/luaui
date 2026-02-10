#include "StackPanel.h"
#include <algorithm>

namespace luaui {
namespace controls {

Size StackPanel::MeasureOverride(const Size& availableSize) {
    Size desired;
    bool isVertical = (m_orientation == Orientation::Vertical);
    size_t visibleCount = 0;
    
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        child->Measure(availableSize);
        Size childSize = child->GetDesiredSize();
        
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

Size StackPanel::ArrangeOverride(const Size& finalSize) {
    bool isVertical = (m_orientation == Orientation::Vertical);
    float position = 0;
    
    // Get our position from render rect
    float baseX = m_renderRect.x;
    float baseY = m_renderRect.y;
    
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        Size childSize = child->GetDesiredSize();
        Rect childRect;
        
        if (isVertical) {
            // 使用子控件的实际宽度，而不是 StackPanel 的完整宽度
            childRect = Rect(baseX, baseY + position, childSize.width, childSize.height);
            position += childSize.height + m_spacing;
        } else {
            childRect = Rect(baseX + position, baseY, childSize.width, childSize.height);
            position += childSize.width + m_spacing;
        }
        
        child->Arrange(childRect);
    }
    
    return finalSize;
}

} // namespace controls
} // namespace luaui
