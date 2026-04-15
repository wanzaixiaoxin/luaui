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
    float mx = std::max(0.0f, m_extentHeight - m_viewportHeight);
    return std::max(0.0f, std::min(offset, mx));
}

float ScrollViewer::ClampHorizontalOffset(float offset) {
    float mx = std::max(0.0f, m_extentWidth - m_viewportWidth);
    return std::max(0.0f, std::min(offset, mx));
}

void ScrollViewer::ScrollToHorizontalOffset(float offset) {
    m_horizontalOffset = ClampHorizontalOffset(offset);
    if (GetLayout()) GetLayout()->InvalidateArrange();
}

void ScrollViewer::ScrollToVerticalOffset(float offset) {
    m_verticalOffset = ClampVerticalOffset(offset);
    if (GetLayout()) GetLayout()->InvalidateArrange();
}

// ============================================================================
// Layout
// ============================================================================

rendering::Size ScrollViewer::OnMeasureChildren(const rendering::Size& availableSize) {
    m_viewportWidth = availableSize.width;
    m_viewportHeight = availableSize.height;

    for (size_t i = 0; i < m_children.size(); ++i) {
        if (!m_children[i]->GetIsVisible()) continue;
        auto* layoutable = m_children[i]->AsLayoutable();
        if (layoutable) {
            interfaces::LayoutConstraint constraint;
            // 水平方向允许无限大(可以滚动),垂直方向使用实际可用大小
            constraint.available = rendering::Size(99999, availableSize.height);
            layoutable->Measure(constraint);
            auto desired = layoutable->GetDesiredSize();
            m_extentWidth = desired.width;
            m_extentHeight = desired.height;
        }
    }
    return availableSize;
}

rendering::Size ScrollViewer::OnArrangeChildren(const rendering::Size& finalSize) {
    m_viewportWidth = finalSize.width;
    m_viewportHeight = finalSize.height;

    for (size_t i = 0; i < m_children.size(); ++i) {
        if (!m_children[i]->GetIsVisible()) continue;
        auto* layoutable = m_children[i]->AsLayoutable();
        if (layoutable) {
            layoutable->Arrange(rendering::Rect(
                -m_horizontalOffset,
                -m_verticalOffset,
                (m_extentWidth > finalSize.width) ? m_extentWidth : finalSize.width,
                (m_extentHeight > finalSize.height) ? m_extentHeight : finalSize.height
            ));
        }
    }
    return finalSize;
}

// ============================================================================
// Helpers
// ============================================================================

void ScrollViewer::GlobalToLocal(float gx, float gy, float& lx, float& ly) {
    // 将全局窗口坐标转换为ScrollViewer本地坐标
    auto* render = GetRender();
    if (render) {
        auto rect = render->GetRenderRect();
        lx = gx - rect.x;
        ly = gy - rect.y;
    } else {
        lx = gx;
        ly = gy;
    }
}

bool ScrollViewer::NeedVScroll() const {
    return m_extentHeight > m_viewportHeight && m_viewportHeight > 0;
}

void ScrollViewer::CalcThumb(float& y, float& h) {
    auto* render = GetRender();
    if (!render) { y = 0; h = 0; return; }

    float vpH = render->GetRenderRect().height;
    float trackH = vpH - 2 * SB_MARGIN;
    float ratio = m_viewportHeight / m_extentHeight;
    h = std::max(THUMB_MIN, trackH * ratio);
    float maxOff = std::max(1.0f, m_extentHeight - m_viewportHeight);
    y = SB_MARGIN + (trackH - h) * (m_verticalOffset / maxOff);
}

bool ScrollViewer::HitTestThumb(float gx, float gy) {
    if (!NeedVScroll()) return false;
    float lx, ly;
    GlobalToLocal(gx, gy, lx, ly);

    auto* render = GetRender();
    if (!render) return false;
    float vpW = render->GetRenderRect().width;

    float sbW = m_currentSbWidth;
    float trackX = vpW - sbW - SB_MARGIN;
    if (lx < trackX || lx > trackX + sbW) return false;

    float thumbY, thumbH;
    CalcThumb(thumbY, thumbH);
    return ly >= thumbY && ly <= thumbY + thumbH;
}

bool ScrollViewer::HitTestTrack(float gx, float gy) {
    if (!NeedVScroll()) return false;
    float lx, ly;
    GlobalToLocal(gx, gy, lx, ly);

    auto* render = GetRender();
    if (!render) return false;
    float vpW = render->GetRenderRect().width;
    float vpH = render->GetRenderRect().height;

    float sbW = m_currentSbWidth;
    float trackX = vpW - sbW - SB_MARGIN;
    if (lx < trackX || lx > trackX + sbW) return false;
    if (ly < SB_MARGIN || ly > vpH - SB_MARGIN) return false;
    return true;
}

void ScrollViewer::ApplyOffset(float offset) {
    m_verticalOffset = ClampVerticalOffset(offset);
    if (GetLayout()) GetLayout()->InvalidateArrange();
    if (auto* r = GetRender()) r->Invalidate();
}

// ============================================================================
// Render
// ============================================================================

void ScrollViewer::OnRenderChildren(rendering::IRenderContext* context) {
    if (!context) return;
    auto* render = GetRender();
    if (!render) return;

    float vpW = render->GetRenderRect().width;
    float vpH = render->GetRenderRect().height;

    // clip to viewport
    context->PushClip(rendering::Rect(0, 0, vpW, vpH));

    // render children
    for (size_t i = 0; i < m_children.size(); ++i) {
        if (!m_children[i]->GetIsVisible()) continue;
        auto* ctrl = static_cast<Control*>(m_children[i].get());
        if (auto* r = ctrl->AsRenderable()) r->Render(context);
    }

    context->PopClip();

    // draw vertical scrollbar
    if (!NeedVScroll()) return;

    // expand width on hover/drag
    float targetW = (m_sbHovered || m_dragging) ? SB_EXPANDED : SB_COLLAPSED;
    m_currentSbWidth += (targetW - m_currentSbWidth) * 0.3f;
    if (std::abs(m_currentSbWidth - targetW) < 0.5f) m_currentSbWidth = targetW;

    float sbW = m_currentSbWidth;
    float trackX = vpW - sbW - SB_MARGIN;
    float trackY = SB_MARGIN;
    float trackH = vpH - 2 * SB_MARGIN;

    // track background (light gray, mostly transparent)
    auto trackBrush = context->CreateSolidColorBrush(rendering::Color::FromHex(0x0F000000));
    if (trackBrush) {
        context->FillRoundedRectangle(
            rendering::Rect(trackX, trackY, sbW, trackH),
            rendering::CornerRadius(sbW / 2), trackBrush.get());
    }

    // thumb
    float thumbY, thumbH;
    CalcThumb(thumbY, thumbH);

    // thumb color: dark gray with alpha, brightens on hover/press
    rendering::Color thumbColor;
    if (m_dragging) {
        thumbColor = rendering::Color::FromHex(0x99000000);
    } else if (m_sbHovered) {
        thumbColor = rendering::Color::FromHex(0x77000000);
    } else {
        thumbColor = rendering::Color::FromHex(0x55000000);
    }

    auto thumbBrush = context->CreateSolidColorBrush(thumbColor);
    if (thumbBrush) {
        context->FillRoundedRectangle(
            rendering::Rect(trackX, thumbY, sbW, thumbH),
            rendering::CornerRadius(sbW / 2), thumbBrush.get());
    }
}

// ============================================================================
// Mouse Events
// ============================================================================

void ScrollViewer::OnMouseDown(controls::MouseEventArgs& args) {
    if (!NeedVScroll()) return;

    float lx, ly;
    GlobalToLocal(args.x, args.y, lx, ly);

    auto* render = GetRender();
    if (!render) return;
    float vpW = render->GetRenderRect().width;

    float sbW = m_currentSbWidth;
    float trackX = vpW - sbW - SB_MARGIN;

    // check if in track area
    if (lx < trackX || lx > trackX + sbW) return;

    float thumbY, thumbH;
    CalcThumb(thumbY, thumbH);

    if (ly >= thumbY && ly <= thumbY + thumbH) {
        // click on thumb -> start drag
        m_dragging = true;
        m_sbPressed = true;
        m_dragStartY = args.y;
        m_dragStartOffset = m_verticalOffset;
    } else if (ly >= SB_MARGIN && ly <= render->GetRenderRect().height - SB_MARGIN) {
        // click on track gap -> page up/down
        float page = m_viewportHeight * TRACK_CLICK_RATIO;
        if (ly < thumbY) {
            ApplyOffset(m_verticalOffset - page);
        } else {
            ApplyOffset(m_verticalOffset + page);
        }
    }
    args.Handled = true;
}

void ScrollViewer::OnMouseMove(controls::MouseEventArgs& args) {
    if (!NeedVScroll()) return;

    if (m_dragging) {
        auto* render = GetRender();
        if (!render) return;

        float vpH = render->GetRenderRect().height;
        float trackH = vpH - 2 * SB_MARGIN;
        float thumbY = 0, thumbH = 0;
        CalcThumb(thumbY, thumbH);
        float scrollRange = trackH - thumbH;
        if (scrollRange <= 0) return;

        float deltaY = args.y - m_dragStartY;
        float maxOff = std::max(0.0f, m_extentHeight - m_viewportHeight);
        float newOff = m_dragStartOffset + deltaY * (maxOff / scrollRange);
        ApplyOffset(newOff);
        args.Handled = true;
        return;
    }

    bool wasHovered = m_sbHovered;
    m_sbHovered = HitTestTrack(args.x, args.y);
    if (m_sbHovered != wasHovered) {
        if (auto* r = GetRender()) r->Invalidate();
    }
}

void ScrollViewer::OnMouseUp(controls::MouseEventArgs& args) {
    if (m_dragging) {
        m_dragging = false;
        m_sbPressed = false;
        args.Handled = true;
    }
}

void ScrollViewer::OnMouseWheel(controls::MouseEventArgs& args) {
    if (!NeedVScroll()) return;

    // args.button holds WM_MOUSEWHEEL delta (typically +/-120)
    // delta > 0 -> scroll up -> content moves up -> offset decreases
    float factor = static_cast<float>(args.button) / 120.0f;
    float newOff = m_verticalOffset - factor * WHEEL_STEP;
    ApplyOffset(newOff);

    args.Handled = true;
}

} // namespace controls
} // namespace luaui
