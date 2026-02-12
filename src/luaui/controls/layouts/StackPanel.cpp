#include "Panel.h"
#include "Interfaces/IControl.h"
#include "Interfaces/ILayoutable.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include "Logger.h"
#include <algorithm>

namespace luaui {
namespace controls {

StackPanel::StackPanel() : Panel() {}

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
    
    luaui::utils::Logger::TraceF("[StackPanel::OnArrangeChildren] finalSize=%.1fx%.1f", finalSize.width, finalSize.height);
    
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        if (auto* layoutable = child->AsLayoutable()) {
            auto desired = layoutable->GetDesiredSize();
            
            rendering::Rect childRect;
            if (isHorizontal) {
                // Horizontal: each child gets its desired width, full height
                childRect = rendering::Rect(position, 0,
                                            desired.width, finalSize.height);
                position += desired.width + m_spacing;
            } else {
                // Vertical: each child gets full width, its desired height
                childRect = rendering::Rect(0, position,
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
