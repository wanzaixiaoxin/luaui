#include "CheckBox.h"
#include "IRenderContext.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include "Components/InputComponent.h"

namespace luaui {
namespace controls {

// ============================================================================
// CheckBox
// ============================================================================
CheckBox::CheckBox() {}

void CheckBox::InitializeComponents() {
    auto* layout = GetComponents().AddComponent<components::LayoutComponent>(this);
    layout->SetWidth(m_boxSize + m_spacing + 80);
    layout->SetHeight(m_boxSize);
    
    GetComponents().AddComponent<components::RenderComponent>(this);
    
    auto* input = GetComponents().AddComponent<components::InputComponent>(this);
    input->SetIsFocusable(true);
}

rendering::Size CheckBox::OnMeasure(const rendering::Size& availableSize) {
    return rendering::Size(m_boxSize + m_spacing + 80, m_boxSize);
}

void CheckBox::SetText(const std::wstring& text) {
    if (m_text != text) {
        m_text = text;
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void CheckBox::SetIsChecked(bool checked) {
    if (m_isChecked != checked || m_isIndeterminate) {
        m_isChecked = checked;
        m_isIndeterminate = false;
        UpdateVisualState();
        CheckedChanged.Invoke(this, m_isChecked);
    }
}

void CheckBox::Toggle() {
    if (m_isThreeState) {
        if (!m_isChecked && !m_isIndeterminate) {
            SetIsChecked(true);
        } else if (m_isChecked && !m_isIndeterminate) {
            m_isChecked = false;
            m_isIndeterminate = true;
            UpdateVisualState();
            CheckedChanged.Invoke(this, m_isChecked);
        } else {
            SetIsChecked(false);
        }
    } else {
        SetIsChecked(!m_isChecked);
    }
}

void CheckBox::OnClick() {
    Toggle();
}

void CheckBox::OnMouseEnter() {
    m_isHovered = true;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void CheckBox::OnMouseLeave() {
    m_isHovered = false;
    m_isPressed = false;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void CheckBox::OnMouseDown(MouseEventArgs& args) {
    m_isPressed = true;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void CheckBox::OnMouseUp(MouseEventArgs& args) {
    m_isPressed = false;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void CheckBox::OnRender(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    // 选择边框颜色
    rendering::Color borderColor = m_normalBorder;
    if (m_isPressed) {
        borderColor = m_pressedBorder;
    } else if (m_isHovered) {
        borderColor = m_hoverBorder;
    }
    
    // 绘制复选框
    rendering::Rect boxRect(0, 0, m_boxSize, m_boxSize);
    
    // 填充背景
    auto bgBrush = context->CreateSolidColorBrush(m_boxBackground);
    if (bgBrush) {
        context->FillRectangle(boxRect, bgBrush.get());
    }
    
    // 绘制边框
    auto borderBrush = context->CreateSolidColorBrush(borderColor);
    if (borderBrush) {
        context->DrawRectangle(boxRect, borderBrush.get(), 1.5f);
    }
    
    // 绘制勾选标记
    if (m_isChecked && !m_isIndeterminate) {
        auto checkBrush = context->CreateSolidColorBrush(m_checkColor);
        if (checkBrush) {
            // 绘制简单的勾选形状（两条线）
            float padding = m_boxSize * 0.2f;
            float x1 = boxRect.x + padding;
            float y1 = boxRect.y + m_boxSize * 0.5f;
            float x2 = boxRect.x + m_boxSize * 0.4f;
            float y2 = boxRect.y + m_boxSize - padding;
            float x3 = boxRect.x + m_boxSize - padding;
            float y3 = boxRect.y + padding;
            
            context->DrawLine(rendering::Point(x1, y1), rendering::Point(x2, y2), checkBrush.get(), 2.0f);
            context->DrawLine(rendering::Point(x2, y2), rendering::Point(x3, y3), checkBrush.get(), 2.0f);
        }
    } else if (m_isIndeterminate) {
        // 绘制不确定状态（横线）
        auto checkBrush = context->CreateSolidColorBrush(m_checkColor);
        if (checkBrush) {
            float padding = m_boxSize * 0.25f;
            float y = boxRect.y + m_boxSize * 0.5f;
            context->DrawLine(
                rendering::Point(boxRect.x + padding, y),
                rendering::Point(boxRect.x + m_boxSize - padding, y),
                checkBrush.get(), 2.0f);
        }
    }
    
    // 绘制文本
    if (!m_text.empty()) {
        auto textBrush = context->CreateSolidColorBrush(rendering::Color::Black());
        auto textFormat = context->CreateTextFormat(L"Microsoft YaHei", m_fontSize);
        if (textBrush && textFormat) {
            rendering::Point textPos(m_boxSize + m_spacing, (m_boxSize - m_fontSize) / 2);
            context->DrawTextString(m_text, textFormat.get(), textPos, textBrush.get());
        }
    }
}

void CheckBox::UpdateVisualState() {
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

// ============================================================================
// RadioButton
// ============================================================================
RadioButton::RadioButton() {}

void RadioButton::InitializeComponents() {
    auto* layout = GetComponents().AddComponent<components::LayoutComponent>(this);
    layout->SetMinWidth(m_circleSize + m_spacing + 60);
    layout->SetMinHeight(m_circleSize);
    
    GetComponents().AddComponent<components::RenderComponent>(this);
    
    auto* input = GetComponents().AddComponent<components::InputComponent>(this);
    input->SetIsFocusable(true);
}

void RadioButton::SetText(const std::wstring& text) {
    if (m_text != text) {
        m_text = text;
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void RadioButton::SetIsChecked(bool checked) {
    if (m_isChecked != checked) {
        m_isChecked = checked;
        if (m_isChecked) {
            OnChecked();
        }
        UpdateVisualState();
        CheckedChanged.Invoke(this, m_isChecked);
    }
}

void RadioButton::OnChecked() {
    // TODO: 取消同组其他RadioButton的选中状态
}

void RadioButton::OnRender(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    // 选择边框颜色
    rendering::Color borderColor = m_normalBorder;
    if (m_isPressed) {
        borderColor = m_pressedBorder;
    } else if (m_isHovered) {
        borderColor = m_hoverBorder;
    }
    
    // 绘制圆形外框
    rendering::Point center(m_circleSize / 2, m_circleSize / 2);
    float radiusX = m_circleSize / 2;
    float radiusY = m_circleSize / 2;
    
    // 填充背景
    auto bgBrush = context->CreateSolidColorBrush(m_circleBackground);
    if (bgBrush) {
        context->FillEllipse(center, radiusX, radiusY, bgBrush.get());
    }
    
    // 绘制边框
    auto borderBrush = context->CreateSolidColorBrush(borderColor);
    if (borderBrush) {
        context->DrawEllipse(center, radiusX, radiusY, borderBrush.get(), 1.5f);
    }
    
    // 绘制选中标记（内部圆点）
    if (m_isChecked) {
        auto checkBrush = context->CreateSolidColorBrush(m_checkColor);
        if (checkBrush) {
            float dotRadius = m_dotSize / 2;
            context->FillEllipse(center, dotRadius, dotRadius, checkBrush.get());
        }
    }
    
    // 绘制文本
    if (!m_text.empty()) {
        auto textBrush = context->CreateSolidColorBrush(rendering::Color::Black());
        auto textFormat = context->CreateTextFormat(L"Microsoft YaHei", m_fontSize);
        if (textBrush && textFormat) {
            rendering::Point textPos(m_circleSize + m_spacing, (m_circleSize - m_fontSize) / 2);
            context->DrawTextString(m_text, textFormat.get(), textPos, textBrush.get());
        }
    }
}

void RadioButton::UpdateVisualState() {
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

} // namespace controls
} // namespace luaui
