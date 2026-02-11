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
    // 简化实现：测量所有子控件并累加
    float totalWidth = 0;
    float totalHeight = 0;
    
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        if (auto* layoutable = child->AsLayoutable()) {
            interfaces::LayoutConstraint constraint;
            constraint.available = availableSize;
            layoutable->Measure(constraint);
            auto desired = layoutable->GetDesiredSize();
            
            totalWidth += desired.width;
            totalHeight += desired.height;
        }
    }
    
    return rendering::Size(
        std::min(totalWidth, availableSize.width),
        std::min(totalHeight, availableSize.height)
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
