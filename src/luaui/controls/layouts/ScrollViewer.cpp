#include "layouts/ScrollViewer.h"
#include "Panel.h"
#include "Control.h"
#include "IRenderContext.h"
#include "Logger.h"
#include <algorithm>

namespace luaui {
namespace controls {

ScrollViewer::ScrollViewer() {}

void ScrollViewer::InitializeComponents() {
    Panel::InitializeComponents();
}

void ScrollViewer::SetHorizontalScrollBarVisibility(ScrollBarVisibility visibility) {
    if (m_horizontalScrollBarVisibility != visibility) {
        m_horizontalScrollBarVisibility = visibility;
        if (GetLayout()) GetLayout()->InvalidateMeasure();
    }
}

void ScrollViewer::SetVerticalScrollBarVisibility(ScrollBarVisibility visibility) {
    if (m_verticalScrollBarVisibility != visibility) {
        m_verticalScrollBarVisibility = visibility;
        if (GetLayout()) GetLayout()->InvalidateMeasure();
    }
}

float ScrollViewer::ClampVerticalOffset(float offset) {
    float maxOffset = std::max(0.0f, m_extentHeight - m_viewportHeight);
    return std::max(0.0f, std::min(offset, maxOffset));
}

float ScrollViewer::ClampHorizontalOffset(float offset) {
    float maxOffset = std::max(0.0f, m_extentWidth - m_viewportWidth);
    return std::max(0.0f, std::min(offset, maxOffset));
}

void ScrollViewer::ScrollToHorizontalOffset(float offset) {
    m_horizontalOffset = ClampHorizontalOffset(offset);
    if (GetLayout()) GetLayout()->InvalidateArrange();
}

void ScrollViewer::ScrollToVerticalOffset(float offset) {
    m_verticalOffset = ClampVerticalOffset(offset);
    if (GetLayout()) GetLayout()->InvalidateArrange();
}

rendering::Size ScrollViewer::OnMeasureChildren(const rendering::Size& availableSize) {
    m_viewportWidth = availableSize.width;
    m_viewportHeight = availableSize.height;

    for (size_t i = 0; i < m_children.size(); ++i) {
        std::shared_ptr<interfaces::IControl> child = m_children[i];
        if (!child->GetIsVisible()) continue;

        interfaces::ILayoutable* layoutable = child->AsLayoutable();
        if (layoutable) {
            interfaces::LayoutConstraint constraint;
            constraint.available = rendering::Size(99999, 99999);
            layoutable->Measure(constraint);

            rendering::Size desired = layoutable->GetDesiredSize();
            m_extentWidth = desired.width;
            m_extentHeight = desired.height;
        }
    }

    return availableSize;
}

rendering::Size ScrollViewer::OnArrangeChildren(const rendering::Size& finalSize) {
    components::RenderComponent* render = GetRender();
    if (!render) return finalSize;

    rendering::Rect contentRect = render->GetRenderRect();

    m_viewportWidth = finalSize.width;
    m_viewportHeight = finalSize.height;

    for (size_t i = 0; i < m_children.size(); ++i) {
        std::shared_ptr<interfaces::IControl> child = m_children[i];
        if (!child->GetIsVisible()) continue;

        interfaces::ILayoutable* layoutable = child->AsLayoutable();
        if (layoutable) {
            float x = contentRect.x - m_horizontalOffset;
            float y = contentRect.y - m_verticalOffset;

            layoutable->Arrange(rendering::Rect(x, y, m_extentWidth, m_extentHeight));
        }
    }

    return finalSize;
}

void ScrollViewer::OnRenderChildren(rendering::IRenderContext* context) {
    if (!context) return;

    components::RenderComponent* render = GetRender();
    if (!render) return;

    float vpW = render->GetRenderRect().width;
    float vpH = render->GetRenderRect().height;

    // clip to viewport
    rendering::Rect clip(0, 0, vpW, vpH);
    context->PushClip(clip);

    // render children
    for (size_t i = 0; i < m_children.size(); ++i) {
        std::shared_ptr<interfaces::IControl> child = m_children[i];
        if (!child->GetIsVisible()) continue;
        Control* ctrl = static_cast<Control*>(child.get());
        interfaces::IRenderable* renderable = ctrl->AsRenderable();
        if (renderable) {
            renderable->Render(context);
        }
    }

    context->PopClip();

    // draw vertical scrollbar
    bool needVertical = m_extentHeight > m_viewportHeight && m_viewportHeight > 0;
    if (needVertical) {
        float sbWidth = m_sbWidth;
        float trackX = vpW - sbWidth - 2;
        float trackY = 2;
        float trackH = vpH - 4;

        rendering::Rect trackRect(trackX, trackY, sbWidth, trackH);
        rendering::ISolidColorBrushPtr trackBrush(context->CreateSolidColorBrush(m_scrollbarTrackColor));
        if (trackBrush) {
            context->FillRoundedRectangle(trackRect, rendering::CornerRadius(sbWidth / 2), trackBrush.get());
        }

        float thumbRatio = m_viewportHeight / m_extentHeight;
        float thumbH = std::max(m_scrollbarMinThumb, trackH * thumbRatio);
        float maxScroll = std::max(1.0f, m_extentHeight - m_viewportHeight);
        float thumbY = trackY + (trackH - thumbH) * (m_verticalOffset / maxScroll);

        rendering::Rect thumbRect(trackX, thumbY, sbWidth, thumbH);
        rendering::ISolidColorBrushPtr thumbBrush(context->CreateSolidColorBrush(m_scrollbarThumbColor));
        if (thumbBrush) {
            context->FillRoundedRectangle(thumbRect, rendering::CornerRadius(sbWidth / 2), thumbBrush.get());
        }
    }
}

void ScrollViewer::OnMouseWheel(controls::MouseEventArgs& args) {
    float step = 48.0f;
    float newOffset = m_verticalOffset - step;

    float maxOffset = std::max(0.0f, m_extentHeight - m_viewportHeight);
    m_verticalOffset = std::max(0.0f, std::min(newOffset, maxOffset));

    if (GetLayout()) {
        GetLayout()->InvalidateArrange();
    }
    components::RenderComponent* r = GetRender();
    if (r) {
        r->Invalidate();
    }

    args.Handled = true;
}

} // namespace controls
} // namespace luaui
