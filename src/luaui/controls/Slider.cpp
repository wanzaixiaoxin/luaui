#include "Slider.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include "Components/InputComponent.h"
#include <algorithm>
#include <cmath>

namespace luaui {
namespace controls {

// ============================================================================
// Slider
// ============================================================================
Slider::Slider() {}

void Slider::InitializeComponents() {
    auto* layout = GetComponents().AddComponent<components::LayoutComponent>(this);
    GetComponents().AddComponent<components::RenderComponent>(this);
    
    auto* input = GetComponents().AddComponent<components::InputComponent>(this);
    input->SetIsFocusable(true);
    
    if (m_isVertical) {
        layout->SetMinWidth(16);
        layout->SetMinHeight(100);
    } else {
        layout->SetMinWidth(100);
        layout->SetMinHeight(16);
    }
}

void Slider::SetValue(double value) {
    double oldValue = m_value;
    m_value = value;
    ClampValue();
    
    if (m_value != oldValue) {
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
        ValueChanged.Invoke(this, m_value);
    }
}

void Slider::SetMinimum(double min) {
    if (m_minimum != min) {
        m_minimum = min;
        ClampValue();
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void Slider::SetMaximum(double max) {
    if (m_maximum != max) {
        m_maximum = max;
        ClampValue();
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void Slider::SetIsVertical(bool vertical) {
    if (m_isVertical != vertical) {
        m_isVertical = vertical;
        if (auto* layout = GetLayout()) {
            if (m_isVertical) {
                layout->SetMinWidth(16);
                layout->SetMinHeight(100);
            } else {
                layout->SetMinWidth(100);
                layout->SetMinHeight(16);
            }
        }
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void Slider::ClampValue() {
    if (m_value < m_minimum) m_value = m_minimum;
    if (m_value > m_maximum) m_value = m_maximum;
}

// ============================================================================
// ProgressBar
// ============================================================================
ProgressBar::ProgressBar() {}

void ProgressBar::InitializeComponents() {
    auto* layout = GetComponents().AddComponent<components::LayoutComponent>(this);
    GetComponents().AddComponent<components::RenderComponent>(this);
    
    if (m_isVertical) {
        layout->SetWidth(8);
        layout->SetMinHeight(100);
    } else {
        layout->SetMinWidth(100);
        layout->SetHeight(8);
    }
}

void ProgressBar::SetValue(double value) {
    if (m_value != value && !m_isIndeterminate) {
        m_value = value;
        if (m_value < m_minimum) m_value = m_minimum;
        if (m_value > m_maximum) m_value = m_maximum;
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void ProgressBar::SetMinimum(double min) {
    if (m_minimum != min) {
        m_minimum = min;
        if (m_value < m_minimum) {
            m_value = m_minimum;
        }
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void ProgressBar::SetMaximum(double max) {
    if (m_maximum != max) {
        m_maximum = max;
        if (m_value > m_maximum) {
            m_value = m_maximum;
        }
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void ProgressBar::SetIsIndeterminate(bool indeterminate) {
    if (m_isIndeterminate != indeterminate) {
        m_isIndeterminate = indeterminate;
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void ProgressBar::SetIsVertical(bool vertical) {
    if (m_isVertical != vertical) {
        m_isVertical = vertical;
        if (auto* layout = GetLayout()) {
            if (m_isVertical) {
                layout->SetWidth(8);
                layout->SetHeight(0);
                layout->SetMinHeight(100);
            } else {
                layout->SetWidth(0);
                layout->SetHeight(8);
                layout->SetMinWidth(100);
            }
        }
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

} // namespace controls
} // namespace luaui
