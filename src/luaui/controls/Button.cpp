#include "Button.h"
#include "IRenderContext.h"
#include "Logger.h"

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

// ============================================================================
// 属性 setter
// ============================================================================

void Button::SetText(const std::wstring& text) {
    if (m_text != text) {
        m_text = text;
        std::string narrow(text.begin(), text.end());
        luaui::utils::Logger::DebugF("[Button] SetText: '%s'", narrow.c_str());
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
// 渲染
// ============================================================================

void Button::OnRender(rendering::IRenderContext* context) {
    if (!context) return;

    auto* render = GetRender();
    if (!render) return;

    rendering::Rect localRect(0, 0, render->GetRenderRect().width, render->GetRenderRect().height);

    // 根据状态选择颜色
    rendering::Color bgColor, fgColor, borderColor;
    if (!GetIsEnabled()) {
        bgColor = m_disabledBackground;
        fgColor = m_disabledForeground;
        borderColor = m_disabledBorderBrush;
    } else if (m_isPressed) {
        bgColor = m_pressedBackground;
        fgColor = m_foreground;
        borderColor = m_borderBrush;
    } else if (m_isHovered) {
        bgColor = m_hoverBackground;
        fgColor = m_foreground;
        borderColor = m_borderBrush;
    } else {
        bgColor = m_normalBackground;
        fgColor = m_foreground;
        borderColor = m_borderBrush;
    }

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

    // 3. 文本（内容区域 = 控件矩形 - Padding - BorderThickness）
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
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void Button::OnMouseUp(MouseEventArgs& args) {
    (void)args;
    if (m_isPressed) {
        m_isPressed = false;
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void Button::OnMouseEnter() {
    if (!GetIsEnabled()) return;
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
    // 默认按钮大小（考虑 padding 和 border）
    float defaultW = 80 + m_padding.left + m_padding.right + 2 * m_borderThickness;
    float defaultH = 32 + m_padding.top + m_padding.bottom + 2 * m_borderThickness;
    return rendering::Size(defaultW, defaultH);
}

} // namespace controls
} // namespace luaui
