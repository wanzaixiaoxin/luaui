#include "Shapes.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"

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

// ============================================================================
// Ellipse
// ============================================================================
Ellipse::Ellipse() {}

void Ellipse::InitializeComponents() {
    GetComponents().AddComponent<components::LayoutComponent>(this);
    GetComponents().AddComponent<components::RenderComponent>(this);
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

} // namespace controls
} // namespace luaui
