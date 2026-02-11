#include "layouts/Viewbox.h"
#include "Interfaces/IControl.h"
#include "Interfaces/ILayoutable.h"
#include "Components/RenderComponent.h"
#include <algorithm>

namespace luaui {
namespace controls {

Viewbox::Viewbox() {
    m_stretch = Stretch::Uniform;
}

void Viewbox::SetStretch(Stretch stretch) {
    if (m_stretch != stretch) {
        m_stretch = stretch;
        if (auto* layout = GetLayout()) layout->InvalidateMeasure();
    }
}

rendering::Size Viewbox::OnMeasureChildren(const rendering::Size& availableSize) {
    // Viewbox 只能有一个子控件
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        if (auto* layoutable = child->AsLayoutable()) {
            // 给子控件无限空间来测量其自然大小
            interfaces::LayoutConstraint constraint;
            constraint.available = rendering::Size(99999, 99999);
            layoutable->Measure(constraint);
            
            m_childSize = layoutable->GetDesiredSize();
            
            // Viewbox 本身希望占用可用空间，但不超过可用空间
            return rendering::Size(
                std::min(m_childSize.width, availableSize.width),
                std::min(m_childSize.height, availableSize.height)
            );
        }
    }
    
    return rendering::Size(0, 0);
}

rendering::Size Viewbox::OnArrangeChildren(const rendering::Size& finalSize) {
    auto* render = GetRender();
    if (!render) return finalSize;
    
    auto contentRect = render->GetRenderRect();
    
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        if (auto* layoutable = child->AsLayoutable()) {
            float scaleX = 1.0f;
            float scaleY = 1.0f;
            
            if (m_childSize.width > 0 && m_childSize.height > 0) {
                scaleX = finalSize.width / m_childSize.width;
                scaleY = finalSize.height / m_childSize.height;
                
                switch (m_stretch) {
                    case Stretch::None:
                        scaleX = scaleY = 1.0f;
                        break;
                    case Stretch::Fill:
                        // 使用上面计算的 scaleX 和 scaleY
                        break;
                    case Stretch::Uniform:
                        scaleX = scaleY = std::min(scaleX, scaleY);
                        break;
                    case Stretch::UniformToFill:
                        scaleX = scaleY = std::max(scaleX, scaleY);
                        break;
                }
            }
            
            // 计算缩放后的大小
            float scaledWidth = m_childSize.width * scaleX;
            float scaledHeight = m_childSize.height * scaleY;
            
            // 居中显示
            float x = contentRect.x + (finalSize.width - scaledWidth) / 2;
            float y = contentRect.y + (finalSize.height - scaledHeight) / 2;
            
            layoutable->Arrange(rendering::Rect(x, y, scaledWidth, scaledHeight));
            
            // TODO: 应用缩放变换到子控件
            // child->SetRenderTransform(...);
        }
    }
    
    return finalSize;
}

} // namespace controls
} // namespace luaui
