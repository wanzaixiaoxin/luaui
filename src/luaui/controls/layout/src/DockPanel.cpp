#include "DockPanel.h"
#include <algorithm>
#include <unordered_map>

namespace luaui {
namespace controls {

static std::unordered_map<Control*, Dock> s_dockMap;

void DockPanel::SetDock(Control* control, Dock dock) {
    if (control) s_dockMap[control] = dock;
}

Dock DockPanel::GetDock(Control* control) {
    if (!control) return Dock::Left;
    auto it = s_dockMap.find(control);
    return (it != s_dockMap.end()) ? it->second : Dock::Left;
}

Size DockPanel::MeasureOverride(const Size& availableSize) {
    Size desired;
    float accumulatedWidth = 0;
    float accumulatedHeight = 0;
    
    for (size_t i = 0; i < m_children.size(); ++i) {
        auto& child = m_children[i];
        if (!child->GetIsVisible()) continue;
        
        bool isLastChild = (i == m_children.size() - 1);
        
        if (isLastChild && m_lastChildFill) {
            Size remaining(
                std::max(0.0f, availableSize.width - accumulatedWidth),
                std::max(0.0f, availableSize.height - accumulatedHeight)
            );
            child->Measure(remaining);
        } else {
            child->Measure(availableSize);
        }
        
        Dock dock = GetDock(child.get());
        Size childSize = child->GetDesiredSize();
        
        switch (dock) {
            case Dock::Left:
            case Dock::Right:
                accumulatedWidth += childSize.width;
                desired.height = std::max(desired.height, accumulatedHeight + childSize.height);
                desired.width = std::max(desired.width, accumulatedWidth);
                break;
            case Dock::Top:
            case Dock::Bottom:
                accumulatedHeight += childSize.height;
                desired.width = std::max(desired.width, accumulatedWidth + childSize.width);
                desired.height = std::max(desired.height, accumulatedHeight);
                break;
        }
    }
    
    if (m_lastChildFill && !m_children.empty()) {
        auto& lastChild = m_children.back();
        if (lastChild->GetIsVisible()) {
            Size lastSize = lastChild->GetDesiredSize();
            desired.width = std::max(desired.width, accumulatedWidth + lastSize.width);
            desired.height = std::max(desired.height, accumulatedHeight + lastSize.height);
        }
    }
    
    return desired;
}

Size DockPanel::ArrangeOverride(const Size& finalSize) {
    float left = 0, top = 0;
    float right = finalSize.width;
    float bottom = finalSize.height;
    
    for (size_t i = 0; i < m_children.size(); ++i) {
        auto& child = m_children[i];
        if (!child->GetIsVisible()) continue;
        
        bool isLastChild = (i == m_children.size() - 1);
        Rect childRect;
        
        if (isLastChild && m_lastChildFill) {
            childRect = Rect(left, top, right - left, bottom - top);
        } else {
            Dock dock = GetDock(child.get());
            Size childSize = child->GetDesiredSize();
            
            switch (dock) {
                case Dock::Left:
                    childRect = Rect(left, top, childSize.width, bottom - top);
                    left += childSize.width;
                    break;
                case Dock::Top:
                    childRect = Rect(left, top, right - left, childSize.height);
                    top += childSize.height;
                    break;
                case Dock::Right:
                    childRect = Rect(right - childSize.width, top, childSize.width, bottom - top);
                    right -= childSize.width;
                    break;
                case Dock::Bottom:
                    childRect = Rect(left, bottom - childSize.height, right - left, childSize.height);
                    bottom -= childSize.height;
                    break;
            }
        }
        
        child->Arrange(childRect);
    }
    
    return finalSize;
}

} // namespace controls
} // namespace luaui
