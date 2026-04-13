#include "Splitter.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include "Components/InputComponent.h"
#include "IRenderContext.h"
#include "Theme.h"
#include "Window.h"

namespace luaui {
namespace controls {

Splitter::Splitter() {}

void Splitter::InitializeComponents() {
    GetComponents().AddComponent<components::LayoutComponent>(this);
    GetComponents().AddComponent<components::RenderComponent>(this);
    GetComponents().AddComponent<components::InputComponent>(this);
}

void Splitter::SetIsVertical(bool vertical) {
    if (m_isVertical != vertical) {
        m_isVertical = vertical;
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
    }
}

void Splitter::SetThickness(float t) {
    if (m_thickness != t) {
        m_thickness = t;
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
    }
}

void Splitter::ApplyTheme() {
    auto& t = Theme::GetCurrent();
    using namespace theme;
    m_bgColor = t.GetColor(kSplitterBg);
    m_hoverBg = t.GetColor(kSplitterHoverBg);
    m_activeBg = t.GetColor(kSplitterActiveBg);
    m_gripColor = t.GetColor(kSplitterGrip);
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

rendering::Size Splitter::OnMeasure(const rendering::Size& availableSize) {
    if (m_isVertical) {
        return rendering::Size(m_thickness, availableSize.height);
    } else {
        return rendering::Size(availableSize.width, m_thickness);
    }
}

void Splitter::OnRender(rendering::IRenderContext* context) {
    if (!context) return;

    auto* render = GetRender();
    if (!render) return;

    auto rect = render->GetRenderRect();

    // 绘制背景
    auto bgBrush = context->CreateSolidColorBrush(GetCurrentBgColor());
    if (bgBrush) {
        context->FillRectangle(rect, bgBrush.get());
    }

    // 绘制抓取点
    DrawGripDots(context, rect);
}

void Splitter::DrawGripDots(rendering::IRenderContext* context, const rendering::Rect& rect) {
    auto brush = context->CreateSolidColorBrush(m_gripColor);
    if (!brush) return;

    float centerX = rect.x + rect.width / 2;
    float centerY = rect.y + rect.height / 2;

    if (m_isVertical) {
        for (int i = -1; i <= 1; ++i) {
            float y = centerY + static_cast<float>(i) * 4;
            context->FillRectangle(
                rendering::Rect(centerX - 1, y - 1, 2, 2),
                brush.get());
        }
    } else {
        for (int i = -1; i <= 1; ++i) {
            float x = centerX + static_cast<float>(i) * 4;
            context->FillRectangle(
                rendering::Rect(x - 1, centerY - 1, 2, 2),
                brush.get());
        }
    }
}

rendering::Color Splitter::GetCurrentBgColor() const {
    if (m_isDragging) return m_activeBg;
    if (m_isHovered) return m_hoverBg;
    return m_bgColor;
}

void Splitter::OnMouseEnter() {
    m_isHovered = true;
    if (auto* render = GetRender()) render->Invalidate();
}

void Splitter::OnMouseLeave() {
    m_isHovered = false;
    if (auto* render = GetRender()) render->Invalidate();
}

void Splitter::OnMouseDown(MouseEventArgs& args) {
    m_isDragging = true;
    m_dragStartPos = m_isVertical ? args.x : args.y;
    if (auto* render = GetRender()) render->Invalidate();
    args.Handled = true;
}

void Splitter::OnMouseMove(MouseEventArgs& args) {
    if (m_isDragging) {
        float currentPos = m_isVertical ? args.x : args.y;
        float delta = currentPos - m_dragStartPos;
        m_dragStartPos = currentPos;
        PositionChanged.Invoke(this, delta);
    }
    args.Handled = true;
}

void Splitter::OnMouseUp(MouseEventArgs& args) {
    (void)args;
    m_isDragging = false;
    if (auto* render = GetRender()) render->Invalidate();
}

} // namespace controls
} // namespace luaui
