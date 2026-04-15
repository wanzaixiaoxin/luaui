#include "layouts/DockPanel.h"
#include "Interfaces/IControl.h"
#include "Interfaces/ILayoutable.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include <algorithm>

namespace luaui {
namespace controls {

// 静态成员定义
std::unordered_map<ControlID, Dock> DockPanel::s_dockInfo;

DockPanel::DockPanel() {}

void DockPanel::SetDock(const std::shared_ptr<IControl>& control, Dock dock) {
    if (!control) return;
    s_dockInfo[control->GetID()] = dock;
}

Dock DockPanel::GetDock(const std::shared_ptr<IControl>& control) {
    if (!control) return Dock::Left;
    auto it = s_dockInfo.find(control->GetID());
    return (it != s_dockInfo.end()) ? it->second : Dock::Left;
}

rendering::Size DockPanel::OnMeasureChildren(const rendering::Size& availableSize) {
    // Correct implementation: measure children with proper constraints based on Dock direction
    float remainingWidth = availableSize.width;
    float remainingHeight = availableSize.height;
    
    for (size_t i = 0; i < m_children.size(); ++i) {
        auto& child = m_children[i];
        if (!child->GetIsVisible()) continue;
        
        if (auto* layoutable = child->AsLayoutable()) {
            Dock dock = GetDock(child);
            bool isLastChild = (i == m_children.size() - 1);
            
            interfaces::LayoutConstraint constraint;
            
            if (isLastChild && m_lastChildFill) {
                // Last child fills remaining space
                constraint.available = rendering::Size(remainingWidth, remainingHeight);
            } else {
                // Constrain based on dock direction
                switch (dock) {
                    case Dock::Left:
                    case Dock::Right:
                        // Left/Right docked children get full remaining height
                        constraint.available = rendering::Size(remainingWidth, remainingHeight);
                        break;
                    case Dock::Top:
                    case Dock::Bottom:
                        // Top/Bottom docked children get full remaining width
                        constraint.available = rendering::Size(remainingWidth, remainingHeight);
                        break;
                }
            }
            
            layoutable->Measure(constraint);
            auto desired = layoutable->GetDesiredSize();
            
            // Update remaining space
            if (!(isLastChild && m_lastChildFill)) {
                switch (dock) {
                    case Dock::Left:
                    case Dock::Right:
                        remainingWidth -= desired.width;
                        break;
                    case Dock::Top:
                    case Dock::Bottom:
                        remainingHeight -= desired.height;
                        break;
                }
            }
        }
    }
    
    // Return the size we actually need
    return rendering::Size(
        availableSize.width - std::max(0.0f, remainingWidth),
        availableSize.height - std::max(0.0f, remainingHeight)
    );
}

rendering::Size DockPanel::OnArrangeChildren(const rendering::Size& finalSize) {
    auto* render = GetRender();
    if (!render) return finalSize;
    
    auto contentRect = render->GetRenderRect();
    
    float left = contentRect.x;
    float top = contentRect.y;
    float right = contentRect.x + finalSize.width;
    float bottom = contentRect.y + finalSize.height;
    
    for (size_t i = 0; i < m_children.size(); ++i) {
        auto& child = m_children[i];
        if (!child->GetIsVisible()) continue;
        
        if (auto* layoutable = child->AsLayoutable()) {
            auto desired = layoutable->GetDesiredSize();
            
            bool isLastChild = (i == m_children.size() - 1);
            
            if (isLastChild && m_lastChildFill) {
                // 最后一个子控件填充剩余空间
                layoutable->Arrange(rendering::Rect(left, top, right - left, bottom - top));
            } else {
                Dock dock = GetDock(child);
                rendering::Rect rect;
                
                switch (dock) {
                    case Dock::Left:
                        rect = rendering::Rect(left, top, desired.width, bottom - top);
                        left += desired.width;
                        break;
                    case Dock::Top:
                        rect = rendering::Rect(left, top, right - left, desired.height);
                        top += desired.height;
                        break;
                    case Dock::Right:
                        rect = rendering::Rect(right - desired.width, top, desired.width, bottom - top);
                        right -= desired.width;
                        break;
                    case Dock::Bottom:
                        rect = rendering::Rect(left, bottom - desired.height, right - left, desired.height);
                        bottom -= desired.height;
                        break;
                }
                
                layoutable->Arrange(rect);
            }
        }
    }
    
    return finalSize;
}

} // namespace controls
} // namespace luaui
