#include "Shapes.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include "IRenderContext.h"
#include <iostream>

namespace luaui {
namespace controls {

// ============================================================================
// Rectangle
// ============================================================================
Rectangle::Rectangle() {}

void Rectangle::InitializeComponents() {
    GetComponents().AddComponent<components::LayoutComponent>(this);
    GetComponents().AddComponent<components::RenderComponent>(this);
}

rendering::Size Rectangle::OnMeasure(const rendering::Size& availableSize) {
    if (auto* layout = GetLayout()) {
        float w = layout->GetWidth();
        float h = layout->GetHeight();
        if (w > 0 && h > 0) {
            return rendering::Size(w, h);
        }
    }
    return rendering::Size(60, 60);
}

void Rectangle::SetFill(const rendering::Color& color) {
    m_fill = color;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void Rectangle::SetStroke(const rendering::Color& color) {
    m_stroke = color;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void Rectangle::SetStrokeThickness(float thickness) {
    m_strokeThickness = thickness;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void Rectangle::SetRadiusX(float radius) {
    m_radiusX = radius;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void Rectangle::SetRadiusY(float radius) {
    m_radiusY = radius;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void Rectangle::OnRender(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    // 使用本地坐标
    rendering::Rect localRect(0, 0, render->GetRenderRect().width, render->GetRenderRect().height);
    
    // 绘制填充
    if (m_fill.a > 0) {
        auto fillBrush = context->CreateSolidColorBrush(m_fill);
        if (fillBrush) {
            context->FillRectangle(localRect, fillBrush.get());
        }
    }
    
    // 绘制描边
    if (m_strokeThickness > 0 && m_stroke.a > 0) {
        auto strokeBrush = context->CreateSolidColorBrush(m_stroke);
        if (strokeBrush) {
            context->DrawRectangle(localRect, strokeBrush.get(), m_strokeThickness);
        }
    }
}

// ============================================================================
// Ellipse
// ============================================================================
Ellipse::Ellipse() {}

void Ellipse::InitializeComponents() {
    GetComponents().AddComponent<components::LayoutComponent>(this);
    GetComponents().AddComponent<components::RenderComponent>(this);
}

rendering::Size Ellipse::OnMeasure(const rendering::Size& availableSize) {
    if (auto* layout = GetLayout()) {
        float w = layout->GetWidth();
        float h = layout->GetHeight();
        if (w > 0 && h > 0) {
            return rendering::Size(w, h);
        }
    }
    return rendering::Size(60, 60);
}

void Ellipse::OnRender(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    // 使用本地坐标
    float w = render->GetRenderRect().width;
    float h = render->GetRenderRect().height;
    rendering::Point center(w / 2, h / 2);
    
    // 绘制填充
    if (m_fill.a > 0) {
        auto fillBrush = context->CreateSolidColorBrush(m_fill);
        if (fillBrush) {
            context->FillEllipse(center, w / 2, h / 2, fillBrush.get());
        }
    }
    
    // 绘制描边
    if (m_strokeThickness > 0 && m_stroke.a > 0) {
        auto strokeBrush = context->CreateSolidColorBrush(m_stroke);
        if (strokeBrush) {
            context->DrawEllipse(center, w / 2, h / 2, strokeBrush.get(), m_strokeThickness);
        }
    }
}

void Ellipse::SetFill(const rendering::Color& color) {
    m_fill = color;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void Ellipse::SetStroke(const rendering::Color& color) {
    m_stroke = color;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void Ellipse::SetStrokeThickness(float thickness) {
    m_strokeThickness = thickness;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

// ============================================================================
// Line
// ============================================================================
Line::Line() {}

void Line::InitializeComponents() {
    GetComponents().AddComponent<components::LayoutComponent>(this);
    GetComponents().AddComponent<components::RenderComponent>(this);
    
    if (auto* layout = GetLayout()) {
        layout->SetWidth(100);
        layout->SetHeight(100);
    }
}

void Line::SetX1(float x) {
    m_x1 = x;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void Line::SetY1(float y) {
    m_y1 = y;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void Line::SetX2(float x) {
    m_x2 = x;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void Line::SetY2(float y) {
    m_y2 = y;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void Line::SetStroke(const rendering::Color& color) {
    m_stroke = color;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void Line::SetStrokeThickness(float thickness) {
    m_strokeThickness = thickness;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void Line::OnRender(rendering::IRenderContext* context) {
    if (!context) return;
    
    if (m_strokeThickness <= 0 || m_stroke.a <= 0) return;
    
    auto strokeBrush = context->CreateSolidColorBrush(m_stroke);
    if (strokeBrush) {
        context->DrawLine(
            rendering::Point(m_x1, m_y1),
            rendering::Point(m_x2, m_y2),
            strokeBrush.get(),
            m_strokeThickness
        );
    }
}

} // namespace controls
} // namespace luaui
