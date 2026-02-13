#include "layouts/WrapPanel.h"
#include "Interfaces/IControl.h"
#include "Interfaces/ILayoutable.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include <algorithm>

namespace luaui {
namespace controls {

WrapPanel::WrapPanel() {}

rendering::Size WrapPanel::OnMeasureChildren(const rendering::Size& availableSize) {
    bool isHorizontal = (m_orientation == Orientation::Horizontal);
    float lineSize = 0;
    (void)lineSize;
    float lineOffset = 0;
    float maxOtherSize = 0;
    
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        if (auto* layoutable = child->AsLayoutable()) {
            interfaces::LayoutConstraint constraint;
            constraint.available = availableSize;
            layoutable->Measure(constraint);
            auto desired = layoutable->GetDesiredSize();
            
            float childWidth = m_itemWidth > 0 ? m_itemWidth : desired.width;
            float childHeight = m_itemHeight > 0 ? m_itemHeight : desired.height;
            
            if (isHorizontal) {
                if (lineSize + childWidth > availableSize.width && lineSize > 0) {
                    // 换行
                    lineOffset += maxOtherSize;
                    maxOtherSize = 0;
                    lineSize = childWidth;
                } else {
                    lineSize += childWidth;
                }
                maxOtherSize = std::max(maxOtherSize, childHeight);
            } else {
                if (lineSize + childHeight > availableSize.height && lineSize > 0) {
                    // 换列
                    lineOffset += maxOtherSize;
                    maxOtherSize = 0;
                    lineSize = childHeight;
                } else {
                    lineSize += childHeight;
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
    float lineSize = 0;
    (void)lineSize;
    float maxOtherSize = 0;
    
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        if (auto* layoutable = child->AsLayoutable()) {
            auto desired = layoutable->GetDesiredSize();
            float childWidth = m_itemWidth > 0 ? m_itemWidth : desired.width;
            float childHeight = m_itemHeight > 0 ? m_itemHeight : desired.height;
            
            if (isHorizontal) {
                if (itemPos + childWidth > contentRect.x + finalSize.width && itemPos > contentRect.x) {
                    // 换行
                    linePos += maxOtherSize;
                    itemPos = contentRect.x;
                    maxOtherSize = 0;
                }
                
                layoutable->Arrange(rendering::Rect(itemPos, linePos, childWidth, childHeight));
                itemPos += childWidth;
                maxOtherSize = std::max(maxOtherSize, childHeight);
            } else {
                if (itemPos + childHeight > contentRect.y + finalSize.height && itemPos > contentRect.y) {
                    // 换列
                    linePos += maxOtherSize;
                    itemPos = contentRect.y;
                    maxOtherSize = 0;
                }
                
                layoutable->Arrange(rendering::Rect(linePos, itemPos, childWidth, childHeight));
                itemPos += childHeight;
                maxOtherSize = std::max(maxOtherSize, childWidth);
            }
        }
    }
    
    return finalSize;
}

} // namespace controls
} // namespace luaui
