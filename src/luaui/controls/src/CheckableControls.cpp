#include "luaui/controls/CheckableControls.h"
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <algorithm>
#include <map>

namespace luaui {
namespace controls {

// ==================== RadioButtonGroup 管理器 ====================
class RadioButtonGroupManager {
public:
    static RadioButtonGroupManager& GetInstance() {
        static RadioButtonGroupManager instance;
        return instance;
    }
    
    void Register(RadioButton* button) {
        if (!button) return;
        m_groups[button->GetGroupName()].push_back(button);
    }
    
    void Unregister(RadioButton* button) {
        if (!button) return;
        auto& buttons = m_groups[button->GetGroupName()];
        auto it = std::find(buttons.begin(), buttons.end(), button);
        if (it != buttons.end()) {
            buttons.erase(it);
        }
    }
    
    void SelectButton(RadioButton* button) {
        if (!button) return;
        
        const std::wstring& groupName = button->GetGroupName();
        for (auto* btn : m_groups[groupName]) {
            if (btn != button && btn->GetIsSelected()) {
                btn->SetIsSelected(false);
            }
        }
    }

private:
    std::map<std::wstring, std::vector<RadioButton*>> m_groups;
};

// ==================== CheckBox ====================
CheckBox::CheckBox() {
    SetIsFocusable(true);
    SetText(L"CheckBox");
}

void CheckBox::SetIsChecked(bool checked) {
    if (m_isChecked != checked) {
        m_isChecked = checked;
        Invalidate();
        if (m_checkChangedHandler) {
            m_checkChangedHandler(this, m_isChecked);
        }
    }
}

void CheckBox::SetText(const std::wstring& text) {
    if (m_text != text) {
        m_text = text;
        InvalidateMeasure();
    }
}

Size CheckBox::MeasureOverride(const Size& availableSize) {
    // 估算文本宽度（每个字符约 8 像素）
    float textWidth = m_text.empty() ? 0 : m_text.length() * 8.0f;
    float textHeight = 16.0f;
    
    return Size(BoxSize + TextSpacing + textWidth, 
                (std::max)(BoxSize, textHeight));
}

void CheckBox::Render(IRenderContext* context) {
    // 绘制复选框
    Rect boxRect(m_renderRect.x, m_renderRect.y + (m_actualHeight - BoxSize) / 2, 
                 BoxSize, BoxSize);
    
    // 背景
    auto bgBrush = context->CreateSolidColorBrush(Color::White());
    context->FillRectangle(boxRect, bgBrush.get());
    
    // 边框
    bool hasFocus = GetIsFocused();
    Color borderColor = hasFocus ? Color::FromHex(0x0078D4) : Color::FromHex(0x808080);
    auto borderBrush = context->CreateSolidColorBrush(borderColor);
    context->DrawRectangle(boxRect, borderBrush.get(), 2.0f);
    
    // 选中标记
    if (m_isChecked) {
        auto checkBrush = context->CreateSolidColorBrush(Color::FromHex(0x0078D4));
        float padding = 4.0f;
        Rect checkRect(boxRect.x + padding, boxRect.y + padding,
                      BoxSize - padding * 2, BoxSize - padding * 2);
        context->FillRectangle(checkRect, checkBrush.get());
    }
    
    // 绘制文本
    if (!m_text.empty()) {
        auto textBrush = context->CreateSolidColorBrush(Color::Black());
        auto textFormat = context->CreateTextFormat(L"Segoe UI", 14.0f);
        Point textPos(m_renderRect.x + BoxSize + TextSpacing, 
                     m_renderRect.y + (m_actualHeight - 16) / 2);
        context->DrawTextString(m_text, textFormat.get(), textPos, textBrush.get());
    }
}

void CheckBox::OnMouseDown(MouseEventArgs& args) {
    SetIsChecked(!m_isChecked);
    args.Handled = true;
}

void CheckBox::OnKeyDown(KeyEventArgs& args) {
    if (args.KeyCode == VK_SPACE) {
        SetIsChecked(!m_isChecked);
        args.Handled = true;
    }
}

// ==================== RadioButton ====================
RadioButton::RadioButton() {
    SetIsFocusable(true);
    SetText(L"RadioButton");
    RadioButtonGroupManager::GetInstance().Register(this);
}

RadioButton::~RadioButton() {
    RadioButtonGroupManager::GetInstance().Unregister(this);
}

void RadioButton::SetIsSelected(bool selected) {
    if (m_isSelected != selected) {
        m_isSelected = selected;
        Invalidate();
        
        // 如果选中，取消同组其他按钮的选中状态
        if (m_isSelected) {
            RadioButtonGroupManager::GetInstance().SelectButton(this);
        }
        
        if (m_isSelected && m_selectedHandler) {
            m_selectedHandler(this);
        }
    }
}

void RadioButton::SetText(const std::wstring& text) {
    if (m_text != text) {
        m_text = text;
        InvalidateMeasure();
    }
}

Size RadioButton::MeasureOverride(const Size& availableSize) {
    float textWidth = m_text.empty() ? 0 : m_text.length() * 8.0f;
    float textHeight = 16.0f;
    
    return Size(CircleSize + TextSpacing + textWidth, 
                (std::max)(CircleSize, textHeight));
}

void RadioButton::Render(IRenderContext* context) {
    // 计算圆形中心
    float centerX = m_renderRect.x + CircleSize / 2;
    float centerY = m_renderRect.y + m_actualHeight / 2;
    float radius = CircleSize / 2 - 1;
    
    // 绘制外圆
    bool hasFocus = GetIsFocused();
    Color borderColor = hasFocus ? Color::FromHex(0x0078D4) : Color::FromHex(0x808080);
    auto borderBrush = context->CreateSolidColorBrush(borderColor);
    context->DrawCircle(Point(centerX, centerY), radius, borderBrush.get(), 2.0f);
    
    // 绘制选中点
    if (m_isSelected) {
        auto dotBrush = context->CreateSolidColorBrush(Color::FromHex(0x0078D4));
        context->FillCircle(Point(centerX, centerY), radius - 5, dotBrush.get());
    }
    
    // 绘制文本
    if (!m_text.empty()) {
        auto textBrush = context->CreateSolidColorBrush(Color::Black());
        auto textFormat = context->CreateTextFormat(L"Segoe UI", 14.0f);
        Point textPos(m_renderRect.x + CircleSize + TextSpacing, 
                     m_renderRect.y + (m_actualHeight - 16) / 2);
        context->DrawTextString(m_text, textFormat.get(), textPos, textBrush.get());
    }
}

void RadioButton::OnMouseDown(MouseEventArgs& args) {
    SetIsSelected(true);
    args.Handled = true;
}

void RadioButton::OnKeyDown(KeyEventArgs& args) {
    if (args.KeyCode == VK_SPACE) {
        SetIsSelected(true);
        args.Handled = true;
    }
}

} // namespace controls
} // namespace luaui
