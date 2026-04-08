#include "Button.h"
#include "IRenderContext.h"
#include "Logger.h"
#include "Window.h"

namespace luaui {
namespace controls {

Button::Button() {}

void Button::InitializeComponents() {
    auto* layout = GetComponents().AddComponent<components::LayoutComponent>(this);
    layout->SetWidth(80);
    layout->SetHeight(32);

    GetComponents().AddComponent<components::RenderComponent>(this);

    auto* input = GetComponents().AddComponent<components::InputComponent>(this);
    input->SetIsFocusable(true);
}

// ============================================================================
// 属性 setter
// ============================================================================

void Button::SetText(const std::wstring& text) {
    if (m_text != text) {
        m_text = text;
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void Button::SetForeground(const rendering::Color& color) {
    if (m_foreground.r != color.r || m_foreground.g != color.g ||
        m_foreground.b != color.b || m_foreground.a != color.a) {
        m_foreground = color;
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void Button::SetBorderBrush(const rendering::Color& color) {
    if (m_borderBrush.r != color.r || m_borderBrush.g != color.g ||
        m_borderBrush.b != color.b || m_borderBrush.a != color.a) {
        m_borderBrush = color;
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void Button::SetBorderThickness(float thickness) {
    if (m_borderThickness != thickness) {
        m_borderThickness = thickness;
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void Button::SetCornerRadius(const rendering::CornerRadius& radius) {
    if (m_cornerRadius.topLeft != radius.topLeft ||
        m_cornerRadius.topRight != radius.topRight ||
        m_cornerRadius.bottomRight != radius.bottomRight ||
        m_cornerRadius.bottomLeft != radius.bottomLeft) {
        m_cornerRadius = radius;
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void Button::SetPadding(const rendering::Thickness& padding) {
    if (m_padding.left != padding.left || m_padding.top != padding.top ||
        m_padding.right != padding.right || m_padding.bottom != padding.bottom) {
        m_padding = padding;
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void Button::SetFontSize(float size) {
    if (m_fontSize != size) {
        m_fontSize = size;
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void Button::SetFontFamily(const std::wstring& family) {
    if (m_fontFamily != family) {
        m_fontFamily = family;
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void Button::SetStateColors(const rendering::Color& normal,
                            const rendering::Color& hover,
                            const rendering::Color& pressed) {
    m_normalBackground = normal;
    m_hoverBackground = hover;
    m_pressedBackground = pressed;
    m_animBg = normal;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void Button::SetDisabledColors(const rendering::Color& bg,
                               const rendering::Color& fg,
                               const rendering::Color& border) {
    m_disabledBackground = bg;
    m_disabledForeground = fg;
    m_disabledBorderBrush = border;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

// ============================================================================
// 动画辅助
// ============================================================================

rendering::Color Button::GetTargetBgColor() const {
    if (!GetIsEnabled()) return m_disabledBackground;
    if (m_isPressed) return m_pressedBackground;
    if (m_isHovered) return m_hoverBackground;
    return m_normalBackground;
}

void Button::AnimateBgTo(const rendering::Color& target, float durationMs) {
    auto* wnd = GetWindow();
    if (!wnd || !wnd->GetTimeline()) {
        m_animBg = target;
        return;
    }

    auto anim = wnd->GetTimeline()->CreateAnimation();
    anim->SetDuration(durationMs);
    anim->SetEasing(rendering::Easing::CubicOut);
    anim->SetFillMode(rendering::FillMode::Forwards);

    rendering::Color start = m_animBg;
    anim->SetStartValue(rendering::AnimationValue(start));
    anim->SetEndValue(rendering::AnimationValue(target));

    anim->SetUpdateCallback([this](const rendering::AnimationValue& val) {
        m_animBg = val.AsColor();
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    });

    anim->Play();
    wnd->GetTimeline()->Add(std::move(anim));
    wnd->StartAnimTimer();
}

// ============================================================================
// 渲染
// ============================================================================

void Button::OnRender(rendering::IRenderContext* context) {
    if (!context) return;

    auto* render = GetRender();
    if (!render) return;

    rendering::Rect localRect(0, 0, render->GetRenderRect().width, render->GetRenderRect().height);

    rendering::Color bgColor = m_animBg;
    rendering::Color fgColor = GetIsEnabled() ? m_foreground : m_disabledForeground;
    rendering::Color borderColor = GetIsEnabled() ? m_borderBrush : m_disabledBorderBrush;

    // 1. 背景（圆角矩形）
    auto bgBrush = context->CreateSolidColorBrush(bgColor);
    if (bgBrush) {
        context->FillRoundedRectangle(localRect, m_cornerRadius, bgBrush.get());
    }

    // 2. 边框（透明时跳过）
    if (borderColor.a > 0 && m_borderThickness > 0) {
        auto borderBrush = context->CreateSolidColorBrush(borderColor);
        if (borderBrush) {
            context->DrawRoundedRectangle(localRect, m_cornerRadius, borderBrush.get(), m_borderThickness);
        }
    }

    // 3. 文本
    if (!m_text.empty()) {
        float bt = m_borderThickness;
        rendering::Rect contentRect(
            m_padding.left + bt,
            m_padding.top + bt,
            localRect.width - m_padding.left - m_padding.right - 2 * bt,
            localRect.height - m_padding.top - m_padding.bottom - 2 * bt
        );

        auto textBrush = context->CreateSolidColorBrush(fgColor);
        auto textFormat = context->CreateTextFormat(m_fontFamily, m_fontSize);
        if (textBrush && textFormat) {
            textFormat->SetTextAlignment(rendering::TextAlignment::Center);
            textFormat->SetParagraphAlignment(rendering::ParagraphAlignment::Center);
            context->DrawTextString(m_text, textFormat.get(), contentRect, textBrush.get());
        }
    }

    // 4. 焦点指示（虚线内框）
    if (auto* input = GetInput()) {
        if (input->GetIsFocused()) {
            rendering::Rect focusRect(2, 2, localRect.width - 4, localRect.height - 4);
            auto focusBrush = context->CreateSolidColorBrush(rendering::Color::FromHex(0x0078D4));
            if (focusBrush) {
                rendering::StrokeStyle dashStyle;
                dashStyle.dashes = {2.0f, 2.0f};
                float focusRadius = (std::max)(0.0f, m_cornerRadius.topLeft - 1);
                context->DrawRoundedRectangle(focusRect, rendering::CornerRadius(focusRadius),
                    focusBrush.get(), 1.0f, &dashStyle);
            }
        }
    }
}

// ============================================================================
// 输入处理
// ============================================================================

void Button::OnMouseDown(MouseEventArgs& args) {
    (void)args;
    if (!GetIsEnabled()) return;
    m_isPressed = true;
    AnimateBgTo(GetTargetBgColor(), 100.0f);
}

void Button::OnMouseUp(MouseEventArgs& args) {
    (void)args;
    if (m_isPressed) {
        m_isPressed = false;
        AnimateBgTo(GetTargetBgColor(), 150.0f);
    }
}

void Button::OnMouseEnter() {
    if (!GetIsEnabled()) return;
    m_isHovered = true;
    AnimateBgTo(GetTargetBgColor(), 150.0f);
}

void Button::OnMouseLeave() {
    m_isHovered = false;
    m_isPressed = false;
    AnimateBgTo(GetTargetBgColor(), 150.0f);
}

void Button::OnClick() {
    if (!GetIsEnabled()) return;
    Click.Invoke(this);
}

// ============================================================================
// 布局
// ============================================================================

rendering::Size Button::OnMeasure(const rendering::Size& availableSize) {
    (void)availableSize;
    if (auto* layout = GetLayout()) {
        float w = layout->GetWidth();
        float h = layout->GetHeight();
        if (w > 0 && h > 0) {
            return rendering::Size(w, h);
        }
    }
    float defaultW = 80 + m_padding.left + m_padding.right + 2 * m_borderThickness;
    float defaultH = 32 + m_padding.top + m_padding.bottom + 2 * m_borderThickness;
    return rendering::Size(defaultW, defaultH);
}

} // namespace controls
} // namespace luaui
