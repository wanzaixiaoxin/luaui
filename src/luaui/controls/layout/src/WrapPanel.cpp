#include "WrapPanel.h"
#include <algorithm>
#include <vector>

namespace luaui {
namespace controls {

Size WrapPanel::MeasureOverride(const Size& availableSize) {
    bool isHorizontal = (m_orientation == Orientation::Horizontal);
    Size desired;
    Size lineSize;
    
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        child->Measure(availableSize);
        
        Size childSize = child->GetDesiredSize();
        if (m_itemWidth > 0) childSize.width = m_itemWidth;
        if (m_itemHeight > 0) childSize.height = m_itemHeight;
        
        if (isHorizontal) {
            if (lineSize.width + childSize.width > availableSize.width && lineSize.width > 0) {
                desired.width = std::max(desired.width, lineSize.width);
                desired.height += lineSize.height;
                lineSize = Size();
            }
            
            lineSize.width += childSize.width;
            lineSize.height = std::max(lineSize.height, childSize.height);
        } else {
            if (lineSize.height + childSize.height > availableSize.height && lineSize.height > 0) {
                desired.width += lineSize.width;
                desired.height = std::max(desired.height, lineSize.height);
                lineSize = Size();
            }
            
            lineSize.height += childSize.height;
            lineSize.width = std::max(lineSize.width, childSize.width);
        }
    }
    
    if (isHorizontal) {
        desired.width = std::max(desired.width, lineSize.width);
        desired.height += lineSize.height;
    } else {
        desired.width += lineSize.width;
        desired.height = std::max(desired.height, lineSize.height);
    }
    
    return desired;
}

Size WrapPanel::ArrangeOverride(const Size& finalSize) {
    bool isHorizontal = (m_orientation == Orientation::Horizontal);
    
    float x = 0, y = 0;
    float lineSize = 0;
    float currentLineCross = 0;
    
    // Calculate line sizes for alignment
    std::vector<float> lineSizes;
    float curLineSize = 0, curLineCross = 0;
    
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        Size childSize = child->GetDesiredSize();
        if (m_itemWidth > 0) childSize.width = m_itemWidth;
        if (m_itemHeight > 0) childSize.height = m_itemHeight;
        
        if (isHorizontal) {
            if (curLineSize + childSize.width > finalSize.width && curLineSize > 0) {
                lineSizes.push_back(curLineCross);
                curLineSize = 0;
                curLineCross = 0;
            }
            curLineSize += childSize.width;
            curLineCross = std::max(curLineCross, childSize.height);
        } else {
            if (curLineSize + childSize.height > finalSize.height && curLineSize > 0) {
                lineSizes.push_back(curLineCross);
                curLineSize = 0;
                curLineCross = 0;
            }
            curLineSize += childSize.height;
            curLineCross = std::max(curLineCross, childSize.width);
        }
    }
    lineSizes.push_back(curLineCross);
    
    // Arrange children
    size_t currentLine = 0;
    x = 0; y = 0;
    lineSize = 0;
    currentLineCross = lineSizes.empty() ? 0 : lineSizes[0];
    
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        Size childSize = child->GetDesiredSize();
        if (m_itemWidth > 0) childSize.width = m_itemWidth;
        if (m_itemHeight > 0) childSize.height = m_itemHeight;
        
        if (isHorizontal) {
            if (lineSize + childSize.width > finalSize.width && lineSize > 0) {
                x = 0;
                y += currentLineCross;
                currentLine++;
                currentLineCross = (currentLine < lineSizes.size()) ? lineSizes[currentLine] : 0;
                lineSize = 0;
            }
            
            child->Arrange(Rect(x, y, childSize.width, childSize.height));
            x += childSize.width;
            lineSize += childSize.width;
        } else {
            if (lineSize + childSize.height > finalSize.height && lineSize > 0) {
                x += currentLineCross;
                y = 0;
                currentLine++;
                currentLineCross = (currentLine < lineSizes.size()) ? lineSizes[currentLine] : 0;
                lineSize = 0;
            }
            
            child->Arrange(Rect(x, y, childSize.width, childSize.height));
            y += childSize.height;
            lineSize += childSize.height;
        }
    }
    
    return finalSize;
}

} // namespace controls
} // namespace luaui
