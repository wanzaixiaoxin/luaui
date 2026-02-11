#include "TextBox.h"
#include "IRenderContext.h"
#include "Control.h"  // For KeyEventArgs

namespace luaui {
namespace controls {

TextBox::TextBox() {
    InitializeComponents();
}

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
    
    const auto& rect = render->GetRenderRect();
    
    // 绘制边框
    auto borderColor = input->GetIsFocused() ? m_focusedBorderColor : m_borderColor;
    // context->DrawRectangle(rect, brush.get(), 1.0f);
    
    // 绘制背景
    // context->FillRectangle(rect, bgBrush.get());
    
    // 绘制文本
    std::wstring displayText = GetDisplayText();
    if (!displayText.empty()) {
        // context->DrawTextString(displayText, format.get(), point, fgBrush.get());
    } else if (!m_placeholder.empty() && !input->GetIsFocused()) {
        // 绘制占位符
        // context->DrawTextString(m_placeholder, format.get(), point, placeholderBrush.get());
    }
    
    // 绘制光标
    if (input->GetIsFocused()) {
        // 计算光标位置并绘制
        // context->DrawLine(caretTop, caretBottom, caretBrush.get(), 1.0f);
    }
}

void TextBox::OnMouseDown(MouseEventArgs& args) {
    // 设置焦点
    if (auto* input = GetInput()) {
        input->Focus();
    }
    
    // TODO: 根据点击位置计算光标位置
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
    // TODO: 实现光标闪烁
}

std::wstring TextBox::GetDisplayText() const {
    if (m_isPassword) {
        return std::wstring(m_text.length(), L'*');
    }
    return m_text;
}

} // namespace controls
} // namespace luaui
