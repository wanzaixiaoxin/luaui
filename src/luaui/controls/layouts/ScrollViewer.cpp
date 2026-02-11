#include "layouts/ScrollViewer.h"
#include "Interfaces/IControl.h"
#include "Interfaces/ILayoutable.h"
#include "Components/RenderComponent.h"
#include <algorithm>

namespace luaui {
namespace controls {

ScrollViewer::ScrollViewer() {
    m_horizontalScrollBarVisibility = ScrollBarVisibility::Auto;
    m_verticalScrollBarVisibility = ScrollBarVisibility::Auto;
    m_horizontalOffset = 0;
    m_verticalOffset = 0;
    m_extentWidth = 0;
    m_extentHeight = 0;
    m_viewportWidth = 0;
    m_viewportHeight = 0;
}

void ScrollViewer::SetHorizontalScrollBarVisibility(ScrollBarVisibility visibility) {
    if (m_horizontalScrollBarVisibility != visibility) {
        m_horizontalScrollBarVisibility = visibility;
        if (auto* layout = GetLayout()) layout->InvalidateMeasure();
    }
}

void ScrollViewer::SetVerticalScrollBarVisibility(ScrollBarVisibility visibility) {
    if (m_verticalScrollBarVisibility != visibility) {
        m_verticalScrollBarVisibility = visibility;
        if (auto* layout = GetLayout()) layout->InvalidateMeasure();
    }
}

void ScrollViewer::ScrollToHorizontalOffset(float offset) {
    float maxOffset = std::max(0.0f, m_extentWidth - m_viewportWidth);
    m_horizontalOffset = std::max(0.0f, std::min(offset, maxOffset));
    if (auto* layout = GetLayout()) layout->InvalidateArrange();
}

void ScrollViewer::ScrollToVerticalOffset(float offset) {
    float maxOffset = std::max(0.0f, m_extentHeight - m_viewportHeight);
    m_verticalOffset = std::max(0.0f, std::min(offset, maxOffset));
    if (auto* layout = GetLayout()) layout->InvalidateArrange();
}

rendering::Size ScrollViewer::OnMeasureChildren(const rendering::Size& availableSize) {
    // 计算视口大小（减去滚动条空间）
    m_viewportWidth = availableSize.width;
    m_viewportHeight = availableSize.height;
    
    // 测量子控件，给无限空间以获取其完整大小
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        if (auto* layoutable = child->AsLayoutable()) {
            interfaces::LayoutConstraint constraint;
            // 水平滚动时给无限宽度，垂直滚动时给无限高度
            constraint.available = rendering::Size(99999, 99999);
            layoutable->Measure(constraint);
            
            auto desired = layoutable->GetDesiredSize();
            m_extentWidth = desired.width;
            m_extentHeight = desired.height;
        }
    }
    
    // 返回可用空间（ScrollViewer 填满可用空间）
    return availableSize;
}

rendering::Size ScrollViewer::OnArrangeChildren(const rendering::Size& finalSize) {
    auto* render = GetRender();
    if (!render) return finalSize;
    
    auto contentRect = render->GetRenderRect();
    
    m_viewportWidth = finalSize.width;
    m_viewportHeight = finalSize.height;
    
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        if (auto* layoutable = child->AsLayoutable()) {
            // 子控件使用其完整大小，通过偏移实现滚动
            float x = contentRect.x - m_horizontalOffset;
            float y = contentRect.y - m_verticalOffset;
            
            layoutable->Arrange(rendering::Rect(x, y, m_extentWidth, m_extentHeight));
        }
    }
    
    return finalSize;
}

} // namespace controls
} // namespace luaui
