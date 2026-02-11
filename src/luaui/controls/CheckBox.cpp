#include "CheckBox.h"
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
    layout->SetMinWidth(m_boxSize + m_spacing + 60);
    layout->SetMinHeight(m_boxSize);
    
    GetComponents().AddComponent<components::RenderComponent>(this);
    
    auto* input = GetComponents().AddComponent<components::InputComponent>(this);
    input->SetIsFocusable(true);
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

void RadioButton::UpdateVisualState() {
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

} // namespace controls
} // namespace luaui
