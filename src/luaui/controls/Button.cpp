#include "Button.h"
#include "IRenderContext.h"

namespace luaui {
namespace controls {

Button::Button() {}

void Button::InitializeComponents() {
    // 添加布局组件
    auto* layout = GetComponents().AddComponent<components::LayoutComponent>(this);
    layout->SetWidth(80);
    layout->SetHeight(32);
    
    // 添加渲染组件
    GetComponents().AddComponent<components::RenderComponent>(this);
    
    // 添加输入组件并配置
    auto* input = GetComponents().AddComponent<components::InputComponent>(this);
    input->SetIsFocusable(true);
}

void Button::SetStateColors(const rendering::Color& normal, 
                            const rendering::Color& hover, 
                            const rendering::Color& pressed) {
    m_normalBackground = normal;
    m_hoverBackground = hover;
    m_pressedBackground = pressed;
    
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void Button::OnRender(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    // 根据状态选择颜色
    rendering::Color bgColor = m_normalBackground;
    if (m_isPressed) {
        bgColor = m_pressedBackground;
    } else if (m_isHovered) {
        bgColor = m_hoverBackground;
    }
    
    // 渲染背景（使用本地坐标 0,0）
    rendering::Rect localRect(0, 0, render->GetRenderRect().width, render->GetRenderRect().height);
    
    auto brush = context->CreateSolidColorBrush(bgColor);
    if (brush) {
        context->FillRectangle(localRect, brush.get());
    }
}

void Button::OnMouseDown(MouseEventArgs& args) {
    m_isPressed = true;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void Button::OnMouseUp(MouseEventArgs& args) {
    if (m_isPressed) {
        m_isPressed = false;
        OnClick();
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void Button::OnMouseEnter() {
    m_isHovered = true;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void Button::OnMouseLeave() {
    m_isHovered = false;
    m_isPressed = false;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void Button::OnClick() {
    // 触发 Click 事件
    Click.Invoke(this);
}

rendering::Size Button::OnMeasure(const rendering::Size& availableSize) {
    // 默认按钮大小
    if (auto* layout = GetLayout()) {
        float w = layout->GetWidth();
        float h = layout->GetHeight();
        if (w > 0 && h > 0) {
            return rendering::Size(w, h);
        }
    }
    return rendering::Size(80, 32);
}

} // namespace controls
} // namespace luaui
