#include "TextBox.h"
#include "IRenderContext.h"
#include "Control.h"  // For KeyEventArgs
#include <windows.h>

namespace luaui {
namespace controls {

TextBox::TextBox() {}

void TextBox::InitializeComponents() {
    // 添加布局组件
    auto* layout = GetComponents().AddComponent<components::LayoutComponent>(this);
    layout->SetWidth(150);
    layout->SetHeight(24);
    
    // 添加渲染组件
    GetComponents().AddComponent<components::RenderComponent>(this);
    
    // 添加输入组件并配置
    auto* input = GetComponents().AddComponent<components::InputComponent>(this);
    input->SetIsFocusable(true);
}

void TextBox::SetText(const std::wstring& text) {
    if (m_text != text) {
        m_text = text;
        
        // 限制长度
        if (m_maxLength > 0 && m_text.length() > static_cast<size_t>(m_maxLength)) {
            m_text = m_text.substr(0, m_maxLength);
        }
        
        // 调整光标位置
        if (m_caretPosition > static_cast<int>(m_text.length())) {
            m_caretPosition = static_cast<int>(m_text.length());
        }
        
        ClearSelection();
        TextChanged.Invoke(this, m_text);
        
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void TextBox::SetPlaceholder(const std::wstring& placeholder) {
    m_placeholder = placeholder;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void TextBox::SetIsPassword(bool isPassword) {
    if (m_isPassword != isPassword) {
        m_isPassword = isPassword;
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void TextBox::SetCaretPosition(int pos) {
    pos = std::max(0, std::min(pos, static_cast<int>(m_text.length())));
    if (m_caretPosition != pos) {
        m_caretPosition = pos;
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void TextBox::SelectAll() {
    m_selectionStart = 0;
    m_selectionEnd = static_cast<int>(m_text.length());
    m_caretPosition = m_selectionEnd;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void TextBox::ClearSelection() {
    m_selectionStart = 0;
    m_selectionEnd = 0;
}

void TextBox::OnRender(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    auto* input = GetInput();
    if (!render || !input) return;
    
    rendering::Rect localRect(0, 0, render->GetRenderRect().width, render->GetRenderRect().height);
    
    // 绘制背景
    auto bgBrush = context->CreateSolidColorBrush(rendering::Color::White());
    if (bgBrush) {
        context->FillRectangle(localRect, bgBrush.get());
    }
    
    // 绘制边框
    auto borderColor = input->GetIsFocused() ? m_focusedBorderColor : m_borderColor;
    auto borderBrush = context->CreateSolidColorBrush(borderColor);
    if (borderBrush) {
        context->DrawRectangle(localRect, borderBrush.get(), 1.0f);
    }
    
    // 内边距
    float padding = 4.0f;
    rendering::Point textPos(padding, (localRect.height - m_fontSize) / 2);
    
    // 绘制文本或占位符
    std::wstring displayText = GetDisplayText();
    if (!displayText.empty()) {
        auto fgBrush = context->CreateSolidColorBrush(m_foreground);
        auto format = context->CreateTextFormat(L"Microsoft YaHei", m_fontSize);
        if (fgBrush && format) {
            context->DrawTextString(displayText, format.get(), textPos, fgBrush.get());
        }
    } else if (!m_placeholder.empty() && !input->GetIsFocused()) {
        // 绘制占位符（灰色）
        auto placeholderBrush = context->CreateSolidColorBrush(rendering::Color::FromHex(0x999999));
        auto format = context->CreateTextFormat(L"Microsoft YaHei", m_fontSize);
        if (placeholderBrush && format) {
            context->DrawTextString(m_placeholder, format.get(), textPos, placeholderBrush.get());
        }
    }
    
    // 绘制光标（带闪烁）
    if (input->GetIsFocused()) {
        // 更新光标闪烁状态
        UpdateCaretBlink();
        
        // 只在光标可见时绘制
        if (m_isCaretVisible) {
            auto caretBrush = context->CreateSolidColorBrush(m_foreground);
            if (caretBrush) {
                // 计算光标位置（考虑中英文字符宽度差异）
                float caretX = textPos.x;
                for (size_t i = 0; i < displayText.length() && i < static_cast<size_t>(m_caretPosition); ++i) {
                    wchar_t ch = displayText[i];
                    if (ch >= 0x4E00 && ch <= 0x9FFF) {
                        // 中文字符
                        caretX += m_fontSize;
                    } else {
                        // 其他字符（英文字符等）
                        caretX += m_fontSize * 0.6f;
                    }
                }
                
                float caretTopY = 4.0f;
                float caretBottomY = localRect.height - 4.0f;
                
                context->DrawLine(
                    rendering::Point(caretX, caretTopY),
                    rendering::Point(caretX, caretBottomY),
                    caretBrush.get(), 1.0f
                );
            }
        }
    }
}

void TextBox::OnMouseDown(MouseEventArgs& args) {
    // 设置焦点
    if (auto* input = GetInput()) {
        input->Focus();
    }
    
    // 根据点击位置计算光标位置
    float padding = 4.0f;
    float clickX = args.x - padding;
    
    std::wstring displayText = GetDisplayText();
    float currentX = 0;
    int newPosition = 0;
    
    for (size_t i = 0; i < displayText.length(); ++i) {
        wchar_t ch = displayText[i];
        float charWidth = (ch >= 0x4E00 && ch <= 0x9FFF) ? m_fontSize : m_fontSize * 0.6f;
        
        // 如果点击位置在当前字符中间，选择更近的一边
        if (currentX + charWidth / 2 >= clickX) {
            break;
        }
        
        currentX += charWidth;
        newPosition = static_cast<int>(i) + 1;
    }
    
    SetCaretPosition(newPosition);
}

void TextBox::OnKeyDown(KeyEventArgs& args) {
    switch (args.keyCode) {
        case 37: // Left
            if (m_caretPosition > 0) {
                m_caretPosition--;
                if (auto* render = GetRender()) {
                    render->Invalidate();
                }
            }
            break;
            
        case 39: // Right
            if (m_caretPosition < static_cast<int>(m_text.length())) {
                m_caretPosition++;
                if (auto* render = GetRender()) {
                    render->Invalidate();
                }
            }
            break;
            
        case 36: // Home
            m_caretPosition = 0;
            if (auto* render = GetRender()) {
                render->Invalidate();
            }
            break;
            
        case 35: // End
            m_caretPosition = static_cast<int>(m_text.length());
            if (auto* render = GetRender()) {
                render->Invalidate();
            }
            break;
            
        case 8: // Backspace
            DeleteBackward();
            break;
            
        case 46: // Delete
            DeleteForward();
            break;
    }
}

void TextBox::OnChar(wchar_t ch) {
    if (m_isReadOnly) return;
    
    // 忽略控制字符
    if (ch < 32 && ch != '\t' && ch != '\r' && ch != '\n') return;
    
    // 处理回车
    if (ch == '\r' || ch == '\n') return; // 单行文本框忽略回车
    
    InsertText(std::wstring(1, ch));
}

void TextBox::OnGotFocus() {
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void TextBox::OnLostFocus() {
    ClearSelection();
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

rendering::Size TextBox::OnMeasure(const rendering::Size& availableSize) {
    if (auto* layout = GetLayout()) {
        float w = layout->GetWidth();
        float h = layout->GetHeight();
        if (w > 0 && h > 0) {
            return rendering::Size(w, h);
        }
    }
    return rendering::Size(150, 24);
}

void TextBox::InsertText(const std::wstring& text) {
    // 删除选中的文本
    if (HasSelection()) {
        DeleteSelection();
    }
    
    // 检查最大长度
    if (m_maxLength > 0) {
        int remaining = m_maxLength - static_cast<int>(m_text.length());
        if (remaining <= 0) return;
        std::wstring textToInsert = text.substr(0, remaining);
        m_text.insert(m_caretPosition, textToInsert);
        m_caretPosition += static_cast<int>(textToInsert.length());
    } else {
        m_text.insert(m_caretPosition, text);
        m_caretPosition += static_cast<int>(text.length());
    }
    
    TextChanged.Invoke(this, m_text);
    
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void TextBox::DeleteSelection() {
    if (!HasSelection()) return;
    
    int start = std::min(m_selectionStart, m_selectionEnd);
    int end = std::max(m_selectionStart, m_selectionEnd);
    
    m_text.erase(start, end - start);
    m_caretPosition = start;
    ClearSelection();
    
    TextChanged.Invoke(this, m_text);
    
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void TextBox::DeleteBackward() {
    if (HasSelection()) {
        DeleteSelection();
    } else if (m_caretPosition > 0) {
        m_text.erase(m_caretPosition - 1, 1);
        m_caretPosition--;
        TextChanged.Invoke(this, m_text);
        
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void TextBox::DeleteForward() {
    if (HasSelection()) {
        DeleteSelection();
    } else if (m_caretPosition < static_cast<int>(m_text.length())) {
        m_text.erase(m_caretPosition, 1);
        TextChanged.Invoke(this, m_text);
        
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void TextBox::UpdateCaretVisible() {
    // 重置光标可见状态
    m_isCaretVisible = true;
    m_lastCaretBlinkTime = GetTickCount64();
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void TextBox::UpdateCaretBlink() {
    uint64_t currentTime = GetTickCount64();
    if (currentTime - m_lastCaretBlinkTime >= CARET_BLINK_INTERVAL_MS) {
        m_isCaretVisible = !m_isCaretVisible;
        m_lastCaretBlinkTime = currentTime;
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

std::wstring TextBox::GetDisplayText() const {
    if (m_isPassword) {
        return std::wstring(m_text.length(), L'*');
    }
    return m_text;
}

} // namespace controls
} // namespace luaui
