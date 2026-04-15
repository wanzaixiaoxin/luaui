#include "layouts/WrapPanel.h"
#include "Interfaces/IControl.h"
#include "Interfaces/ILayoutable.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include <algorithm>

namespace luaui {
namespace controls {

WrapPanel::WrapPanel() {}

void WrapPanel::SetOrientation(Orientation orient) {
    if (m_orientation != orient) {
        m_orientation = orient;
        if (auto* layout = GetLayout()) layout->InvalidateMeasure();
    }
}

void WrapPanel::SetItemWidth(float width) {
    if (m_itemWidth != width) {
        m_itemWidth = width;
        if (auto* layout = GetLayout()) layout->InvalidateMeasure();
    }
}

void WrapPanel::SetItemHeight(float height) {
    if (m_itemHeight != height) {
        m_itemHeight = height;
        if (auto* layout = GetLayout()) layout->InvalidateMeasure();
    }
}

void WrapPanel::SetSpacing(float spacing) {
    if (m_spacing != spacing) {
        m_spacing = spacing;
        if (auto* layout = GetLayout()) layout->InvalidateMeasure();
    }
}

rendering::Size WrapPanel::OnMeasureChildren(const rendering::Size& availableSize) {
    bool isHorizontal = (m_orientation == Orientation::Horizontal);
    float lineSize = 0;
    float lineOffset = 0;
    float maxOtherSize = 0;
    bool firstInLine = true;
    
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        if (auto* layoutable = child->AsLayoutable()) {
            interfaces::LayoutConstraint constraint;
            constraint.available = availableSize;
            layoutable->Measure(constraint);
            auto desired = layoutable->GetDesiredSize();
            
            float childWidth = m_itemWidth > 0 ? m_itemWidth : desired.width;
            float childHeight = m_itemHeight > 0 ? m_itemHeight : desired.height;
            
            // Add spacing between items (not before first item in line)
            float spacingWidth = firstInLine ? 0 : m_spacing;
            
            if (isHorizontal) {
                if (lineSize + spacingWidth + childWidth > availableSize.width && lineSize > 0) {
                    // Wrap to new line
                    lineOffset += maxOtherSize + m_spacing;
                    maxOtherSize = 0;
                    lineSize = childWidth;
                    firstInLine = true;
                } else {
                    lineSize += spacingWidth + childWidth;
                    firstInLine = false;
                }
                maxOtherSize = std::max(maxOtherSize, childHeight);
            } else {
                if (lineSize + spacingWidth + childHeight > availableSize.height && lineSize > 0) {
                    // Wrap to new column
                    lineOffset += maxOtherSize + m_spacing;
                    maxOtherSize = 0;
                    lineSize = childHeight;
                    firstInLine = true;
                } else {
                    lineSize += spacingWidth + childHeight;
                    firstInLine = false;
                }
                maxOtherSize = std::max(maxOtherSize, childWidth);
            }
        }
    }
    
    if (isHorizontal) {
        return rendering::Size(availableSize.width, lineOffset + maxOtherSize);
    } else {
        return rendering::Size(lineOffset + maxOtherSize, availableSize.height);
    }
}

rendering::Size WrapPanel::OnArrangeChildren(const rendering::Size& finalSize) {
    bool isHorizontal = (m_orientation == Orientation::Horizontal);
    
    auto* render = GetRender();
    if (!render) return finalSize;
    
    auto contentRect = render->GetRenderRect();
    
    float linePos = isHorizontal ? contentRect.y : contentRect.x;
    float itemPos = isHorizontal ? contentRect.x : contentRect.y;
    float maxOtherSize = 0;
    bool firstInLine = true;
    
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        if (auto* layoutable = child->AsLayoutable()) {
            auto desired = layoutable->GetDesiredSize();
            float childWidth = m_itemWidth > 0 ? m_itemWidth : desired.width;
            float childHeight = m_itemHeight > 0 ? m_itemHeight : desired.height;
            
            // Add spacing between items
            float spacing = firstInLine ? 0 : m_spacing;
            
            if (isHorizontal) {
                if (itemPos + spacing + childWidth > contentRect.x + finalSize.width && itemPos > contentRect.x) {
                    // Wrap to new line
                    linePos += maxOtherSize + m_spacing;
                    itemPos = contentRect.x;
                    maxOtherSize = 0;
                    firstInLine = true;
                    spacing = 0;
                }
                
                layoutable->Arrange(rendering::Rect(itemPos + spacing, linePos, childWidth, childHeight));
                itemPos += spacing + childWidth;
                maxOtherSize = std::max(maxOtherSize, childHeight);
                firstInLine = false;
            } else {
                if (itemPos + spacing + childHeight > contentRect.y + finalSize.height && itemPos > contentRect.y) {
                    // Wrap to new column
                    linePos += maxOtherSize + m_spacing;
                    itemPos = contentRect.y;
                    maxOtherSize = 0;
                    firstInLine = true;
                    spacing = 0;
                }
                
                layoutable->Arrange(rendering::Rect(linePos, itemPos + spacing, childWidth, childHeight));
                itemPos += spacing + childHeight;
                maxOtherSize = std::max(maxOtherSize, childWidth);
                firstInLine = false;
            }
        }
    }
    
    return finalSize;
}

} // namespace controls
} // namespace luaui
