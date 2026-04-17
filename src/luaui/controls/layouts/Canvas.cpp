#include "layouts/Canvas.h"
#include "Control.h"
#include "IRenderContext.h"
#include "Interfaces/IControl.h"
#include "Interfaces/ILayoutable.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include <algorithm>
#include <vector>

namespace luaui {
namespace controls {

// Static member definition
std::unordered_map<ControlID, Canvas::PositionInfo> Canvas::s_positionInfo;

Canvas::Canvas() {}

void Canvas::SetLeft(const std::shared_ptr<IControl>& control, float left) {
    if (!control) return;
    s_positionInfo[control->GetID()].left = left;
}

void Canvas::SetTop(const std::shared_ptr<IControl>& control, float top) {
    if (!control) return;
    s_positionInfo[control->GetID()].top = top;
}

void Canvas::SetRight(const std::shared_ptr<IControl>& control, float right) {
    if (!control) return;
    s_positionInfo[control->GetID()].right = right;
}

void Canvas::SetBottom(const std::shared_ptr<IControl>& control, float bottom) {
    if (!control) return;
    s_positionInfo[control->GetID()].bottom = bottom;
}

void Canvas::SetZIndex(const std::shared_ptr<IControl>& control, int zIndex) {
    if (!control) return;
    s_positionInfo[control->GetID()].zIndex = zIndex;
}

float Canvas::GetLeft(const std::shared_ptr<IControl>& control) {
    if (!control) return 0;
    auto it = s_positionInfo.find(control->GetID());
    return (it != s_positionInfo.end()) ? it->second.left : 0;
}

float Canvas::GetTop(const std::shared_ptr<IControl>& control) {
    if (!control) return 0;
    auto it = s_positionInfo.find(control->GetID());
    return (it != s_positionInfo.end()) ? it->second.top : 0;
}

float Canvas::GetRight(const std::shared_ptr<IControl>& control) {
    if (!control) return -1;
    auto it = s_positionInfo.find(control->GetID());
    return (it != s_positionInfo.end()) ? it->second.right : -1;
}

float Canvas::GetBottom(const std::shared_ptr<IControl>& control) {
    if (!control) return -1;
    auto it = s_positionInfo.find(control->GetID());
    return (it != s_positionInfo.end()) ? it->second.bottom : -1;
}

int Canvas::GetZIndex(const std::shared_ptr<IControl>& control) {
    if (!control) return 0;
    auto it = s_positionInfo.find(control->GetID());
    return (it != s_positionInfo.end()) ? it->second.zIndex : 0;
}

rendering::Size Canvas::OnMeasureChildren(const rendering::Size& availableSize) {
    (void)availableSize;
    // Canvas gives children infinite space for measurement
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        if (auto* layoutable = child->AsLayoutable()) {
            interfaces::LayoutConstraint constraint;
            constraint.available = rendering::Size(99999, 99999);
            layoutable->Measure(constraint);
        }
    }
    
    // Calculate maximum bounds
    float maxWidth = 0;
    float maxHeight = 0;
    
    auto* render = GetRender();
    auto contentRect = render ? render->GetRenderRect() : rendering::Rect(0, 0, 0, 0);
    
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        float left = GetLeft(child);
        float top = GetTop(child);
        
        if (auto* layoutable = child->AsLayoutable()) {
            auto desired = layoutable->GetDesiredSize();
            maxWidth = std::max(maxWidth, left + desired.width);
            maxHeight = std::max(maxHeight, top + desired.height);
        }
    }
    
    return rendering::Size(maxWidth, maxHeight);
}

rendering::Size Canvas::OnArrangeChildren(const rendering::Size& finalSize) {
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;

        if (auto* layoutable = child->AsLayoutable()) {
            float left = GetLeft(child);
            float top = GetTop(child);
            float right = GetRight(child);
            float bottom = GetBottom(child);
            auto desired = layoutable->GetDesiredSize();

            // Calculate position based on Left/Top or Right/Bottom
            float x, y;

            if (right >= 0) {
                // Right is set: position from right edge
                x = finalSize.width - right - desired.width;
            } else {
                // Use Left
                x = left;
            }

            if (bottom >= 0) {
                // Bottom is set: position from bottom edge
                y = finalSize.height - bottom - desired.height;
            } else {
                // Use Top
                y = top;
            }

            layoutable->Arrange(rendering::Rect(x, y, desired.width, desired.height));
        }
    }

    return finalSize;
}

void Canvas::OnRenderChildren(rendering::IRenderContext* context) {
    if (!context) return;
    
    // Sort children by ZIndex for rendering
    std::vector<std::pair<int, std::shared_ptr<IControl>>> sortedChildren;
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        sortedChildren.emplace_back(GetZIndex(child), child);
    }
    
    // Sort by ZIndex (lower values render first)
    std::sort(sortedChildren.begin(), sortedChildren.end(),
        [](const auto& a, const auto& b) { return a.first < b.first; });
    
    // Render in ZIndex order
    for (auto& [zIndex, child] : sortedChildren) {
        auto* ctrl = static_cast<Control*>(child.get());
        if (auto* r = ctrl->AsRenderable()) {
            r->Render(context);
        }
    }
}

} // namespace controls
} // namespace luaui
