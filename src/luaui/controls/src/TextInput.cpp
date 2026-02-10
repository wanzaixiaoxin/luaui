// TextInput.cpp - Text input controls implementation
#include "TextInput.h"
#include "IRenderContext.h"
#include "Event.h"
#include <windows.h>
#include <algorithm>
#include <cctype>

namespace luaui {
namespace controls {

// ============================================================================
// TextInputBase
// ============================================================================

TextInputBase::TextInputBase() {
    SetFocusable(true);
    SetWidth(150);
    SetHeight(32);
}

TextInputBase::~TextInputBase() = default;

void TextInputBase::SetText(const std::wstring& text) {
    if (m_text == text) return;
    
    std::wstring oldText = m_text;
    m_text = text;
    
    // Enforce max length
    if (m_maxLength > 0 && m_text.length() > static_cast<size_t>(m_maxLength)) {
        m_text = m_text.substr(0, m_maxLength);
    }
    
    // Clamp caret position
    if (m_caretPosition > static_cast<int>(m_text.length())) {
        m_caretPosition = static_cast<int>(m_text.length());
    }
    
    // Fire event
    TextChangedEventArgs args(oldText, m_text);
    for (auto& handler : m_textChangedHandlers) {
        handler(this, args);
    }
    
    InvalidateRender();
}

void TextInputBase::SetPlaceholder(const std::wstring& placeholder) {
    if (m_placeholder != placeholder) {
        m_placeholder = placeholder;
        if (m_text.empty()) {
            InvalidateRender();
        }
    }
}

void TextInputBase::SetMaxLength(int maxLength) {
    m_maxLength = maxLength;
    if (maxLength > 0 && m_text.length() > static_cast<size_t>(maxLength)) {
        SetText(m_text.substr(0, maxLength));
    }
}

void TextInputBase::SetIsReadOnly(bool readOnly) {
    m_isReadOnly = readOnly;
}

std::wstring TextInputBase::GetSelectedText() const {
    if (m_selectionLength <= 0) return L"";
    return m_text.substr(m_selectionStart, m_selectionLength);
}

void TextInputBase::Select(int start, int length) {
    int textLen = static_cast<int>(m_text.length());
    start = std::max(0, std::min(start, textLen));
    length = std::max(0, std::min(length, textLen - start));
    
    if (m_selectionStart != start || m_selectionLength != length) {
        int oldStart = m_selectionStart;
        int oldLength = m_selectionLength;
        
        m_selectionStart = start;
        m_selectionLength = length;
        m_caretPosition = start + length;
        
        SelectionChangedEventArgs args(oldStart, oldLength);
        for (auto& handler : m_selectionChangedHandlers) {
            handler(this, args);
        }
        
        InvalidateRender();
    }
}

void TextInputBase::SelectAll() {
    Select(0, static_cast<int>(m_text.length()));
}

void TextInputBase::ClearSelection() {
    if (m_selectionLength > 0) {
        m_selectionLength = 0;
        m_caretPosition = m_selectionStart;
        InvalidateRender();
    }
}

void TextInputBase::SetCaretPosition(int position) {
    position = std::max(0, std::min(position, static_cast<int>(m_text.length())));
    if (m_caretPosition != position) {
        m_caretPosition = position;
        m_caretVisible = true;
        m_lastBlinkTime = GetTickCount();
        InvalidateRender();
    }
}

void TextInputBase::AddTextChangedHandler(TextChangedHandler handler) {
    m_textChangedHandlers.push_back(std::move(handler));
}

void TextInputBase::AddSelectionChangedHandler(SelectionChangedHandler handler) {
    m_selectionChangedHandlers.push_back(std::move(handler));
}

void TextInputBase::HandleCharInput(wchar_t ch) {
    if (m_isReadOnly) return;
    
    // Handle special characters
    if (ch == L'\b') { // Backspace
        if (m_selectionLength > 0) {
            DeleteSelection();
        } else if (m_caretPosition > 0) {
            std::wstring newText = m_text;
            newText.erase(m_caretPosition - 1, 1);
            SetCaretPosition(m_caretPosition - 1);
            SetText(newText);
        }
        return;
    }
    
    if (ch < 32 && ch != L'\t' && ch != L'\r' && ch != L'\n') {
        return; // Ignore control characters except tab/enter
    }
    
    InsertText(std::wstring(1, ch));
}

void TextInputBase::HandleKeyDownInternal(int keyCode, bool ctrl, bool shift) {
    bool handled = false;
    
    switch (keyCode) {
        case VK_LEFT:
            if (ctrl) {
                // Move to start of word
                int pos = m_caretPosition;
                while (pos > 0 && m_text[pos - 1] == L' ') pos--;
                while (pos > 0 && m_text[pos - 1] != L' ') pos--;
                SetCaretPosition(pos);
            } else if (shift) {
                // Extend selection left
                if (m_caretPosition > 0) {
                    if (m_selectionLength == 0) {
                        m_selectionStart = m_caretPosition;
                    }
                    m_caretPosition--;
                    m_selectionLength = m_selectionStart - m_caretPosition;
                    InvalidateRender();
                }
            } else {
                // Move caret left
                if (m_selectionLength > 0) {
                    SetCaretPosition(m_selectionStart);
                    ClearSelection();
                } else if (m_caretPosition > 0) {
                    SetCaretPosition(m_caretPosition - 1);
                }
            }
            handled = true;
            break;
            
        case VK_RIGHT:
            if (ctrl) {
                // Move to end of word
                int pos = m_caretPosition;
                int len = static_cast<int>(m_text.length());
                while (pos < len && m_text[pos] != L' ') pos++;
                while (pos < len && m_text[pos] == L' ') pos++;
                SetCaretPosition(pos);
            } else if (shift) {
                // Extend selection right
                int len = static_cast<int>(m_text.length());
                if (m_caretPosition < len) {
                    if (m_selectionLength == 0) {
                        m_selectionStart = m_caretPosition;
                    }
                    m_caretPosition++;
                    m_selectionLength = m_caretPosition - m_selectionStart;
                    InvalidateRender();
                }
            } else {
                // Move caret right
                if (m_selectionLength > 0) {
                    SetCaretPosition(m_selectionStart + m_selectionLength);
                    ClearSelection();
                } else if (m_caretPosition < static_cast<int>(m_text.length())) {
                    SetCaretPosition(m_caretPosition + 1);
                }
            }
            handled = true;
            break;
            
        case VK_HOME:
            SetCaretPosition(0);
            if (shift) {
                Select(0, m_caretPosition);
            } else {
                ClearSelection();
            }
            handled = true;
            break;
            
        case VK_END:
            SetCaretPosition(static_cast<int>(m_text.length()));
            if (shift) {
                Select(m_selectionStart, m_caretPosition - m_selectionStart);
            } else {
                ClearSelection();
            }
            handled = true;
            break;
            
        case VK_DELETE:
            if (m_isReadOnly) break;
            if (m_selectionLength > 0) {
                DeleteSelection();
            } else if (m_caretPosition < static_cast<int>(m_text.length())) {
                std::wstring newText = m_text;
                newText.erase(m_caretPosition, 1);
                SetText(newText);
            }
            handled = true;
            break;
            
        case 'A':
            if (ctrl) {
                SelectAll();
                handled = true;
            }
            break;
            
        case 'C':
            if (ctrl) {
                Copy();
                handled = true;
            }
            break;
            
        case 'X':
            if (ctrl) {
                Cut();
                handled = true;
            }
            break;
            
        case 'V':
            if (ctrl) {
                Paste();
                handled = true;
            }
            break;
    }
    
    if (handled) {
        EventArgs args;
        // Note: We'd need to set Handled on args if we had access to the event system here
    }
}

void TextInputBase::Cut() {
    if (m_isReadOnly || m_selectionLength == 0) return;
    
    Copy();
    DeleteSelection();
}

void TextInputBase::Copy() {
    if (m_selectionLength == 0) return;
    
    std::wstring selected = GetSelectedText();
    if (OpenClipboard(nullptr)) {
        EmptyClipboard();
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (selected.length() + 1) * sizeof(wchar_t));
        if (hMem) {
            memcpy(GlobalLock(hMem), selected.c_str(), (selected.length() + 1) * sizeof(wchar_t));
            GlobalUnlock(hMem);
            SetClipboardData(CF_UNICODETEXT, hMem);
        }
        CloseClipboard();
    }
}

void TextInputBase::Paste() {
    if (m_isReadOnly) return;
    
    if (OpenClipboard(nullptr)) {
        HANDLE hData = GetClipboardData(CF_UNICODETEXT);
        if (hData) {
            wchar_t* pszText = static_cast<wchar_t*>(GlobalLock(hData));
            if (pszText) {
                std::wstring text(pszText);
                // Remove newlines for single-line input
                size_t pos;
                while ((pos = text.find_first_of(L"\r\n")) != std::wstring::npos) {
                    text.erase(pos, 1);
                }
                InsertText(text);
                GlobalUnlock(hData);
            }
        }
        CloseClipboard();
    }
}

void TextInputBase::DeleteSelection() {
    if (m_selectionLength == 0 || m_isReadOnly) return;
    
    std::wstring newText = m_text;
    newText.erase(m_selectionStart, m_selectionLength);
    SetCaretPosition(m_selectionStart);
    ClearSelection();
    SetText(newText);
}

void TextInputBase::OnGotFocus() {
    Control::OnGotFocus();
    m_caretVisible = true;
    m_lastBlinkTime = GetTickCount();
    InvalidateRender();
}

void TextInputBase::OnLostFocus() {
    Control::OnLostFocus();
    m_caretVisible = false;
    ClearSelection();
    InvalidateRender();
}

void TextInputBase::InsertText(const std::wstring& text) {
    if (m_isReadOnly) return;
    
    std::wstring newText = m_text;
    
    // Delete selection if any
    if (m_selectionLength > 0) {
        newText.erase(m_selectionStart, m_selectionLength);
        m_caretPosition = m_selectionStart;
        m_selectionLength = 0;
    }
    
    // Insert new text
    newText.insert(m_caretPosition, text);
    
    // Check max length
    if (m_maxLength > 0 && newText.length() > static_cast<size_t>(m_maxLength)) {
        newText = newText.substr(0, m_maxLength);
    }
    
    SetCaretPosition(m_caretPosition + static_cast<int>(text.length()));
    SetText(newText);
}

void TextInputBase::RenderOverride(IRenderContext* context) {
    Rect rect = GetRenderRect();
    
    // Background
    Color bgColor = GetIsFocused() ? Color::FromHex(0xFFFFFF) : Color::FromHex(0xFAFAFA);
    context->FillRectangle(rect, bgColor);
    
    // Border
    Color borderColor = GetIsFocused() ? Color::FromHex(0x0078D4) : Color::FromHex(0xCCCCCC);
    context->DrawRectangle(rect, borderColor, 1.0f);
    
    // Content area with padding
    float x = rect.x + m_padding + m_scrollOffset;
    float y = rect.y + (rect.height - m_fontSize) / 2;
    
    // Draw selection background
    if (m_selectionLength > 0 && GetIsFocused()) {
        float selStartX = GetCaretX(m_selectionStart);
        float selEndX = GetCaretX(m_selectionStart + m_selectionLength);
        Rect selRect(rect.x + m_padding + selStartX + m_scrollOffset, 
                     rect.y + 2, 
                     selEndX - selStartX, 
                     rect.height - 4);
        context->FillRectangle(selRect, m_selectionColor);
    }
    
    // Draw text or placeholder
    std::wstring displayText = GetDisplayText();
    if (displayText.empty() && !m_placeholder.empty() && !GetIsFocused()) {
        context->DrawText(m_placeholder, Point(x, y), m_placeholderColor, m_fontSize);
    } else if (!displayText.empty()) {
        context->DrawText(displayText, Point(x, y), m_textColor, m_fontSize);
    }
    
    // Draw caret
    if (GetIsFocused() && m_caretVisible) {
        float caretX = rect.x + m_padding + GetCaretX(m_caretPosition) + m_scrollOffset;
        float caretY1 = rect.y + 4;
        float caretY2 = rect.y + rect.height - 4;
        context->DrawLine(Point(caretX, caretY1), Point(caretX, caretY2), m_caretColor, 1.0f);
    }
    
    // Update caret blink
    DWORD now = GetTickCount();
    if (now - m_lastBlinkTime > CARET_BLINK_INTERVAL) {
        m_caretVisible = !m_caretVisible;
        m_lastBlinkTime = now;
        InvalidateRender();
    }
}

float TextInputBase::GetCaretX(int charIndex) const {
    // Simplified: estimate based on average character width
    // In a real implementation, we'd use proper text measurement
    return charIndex * m_fontSize * 0.6f;
}

int TextInputBase::HitTest(float x) const {
    // Convert X position to character index
    float charWidth = m_fontSize * 0.6f;
    int index = static_cast<int>((x - m_padding - m_scrollOffset) / charWidth + 0.5f);
    return std::max(0, std::min(index, static_cast<int>(m_text.length())));
}

// ============================================================================
// TextBox
// ============================================================================

TextBox::TextBox() = default;
TextBox::~TextBox() = default;

void TextBox::SetAcceptsReturn(bool acceptsReturn) {
    m_acceptsReturn = acceptsReturn;
}

void TextBox::SetIsMultiline(bool multiline) {
    m_isMultiline = multiline;
    if (multiline) {
        SetHeight(100);
    } else {
        SetHeight(32);
    }
    InvalidateMeasure();
}

void TextBox::SetWrapText(bool wrap) {
    m_wrapText = wrap;
    InvalidateMeasure();
}

void TextBox::SetVerticalScrollOffset(float offset) {
    m_verticalScrollOffset = std::max(0.0f, offset);
    InvalidateRender();
}

void TextBox::MeasureOverride(const Size& availableSize) {
    if (m_isMultiline) {
        SetDesiredSize(Size(GetWidth(), GetHeight()));
    } else {
        SetDesiredSize(Size(GetWidth(), 32));
    }
}

void TextBox::ArrangeOverride(const Size& finalSize) {
    // Ensure minimum height
    float height = m_isMultiline ? std::max(finalSize.height, 60.0f) : 32.0f;
    SetRenderRect(Rect(GetRenderRect().x, GetRenderRect().y, finalSize.width, height));
    return finalSize;
}

void TextBox::RenderOverride(IRenderContext* context) {
    TextInputBase::RenderOverride(context);
}

void TextBox::OnMouseDown(MouseEventArgs& args) {
    SetCaretPosition(HitTest(args.Position.X));
    m_isDragging = true;
    m_dragStartPos = m_caretPosition;
    ClearSelection();
}

void TextBox::OnMouseMove(MouseEventArgs& args) {
    if (m_isDragging) {
        int pos = HitTest(args.Position.X);
        if (pos != m_dragStartPos) {
            Select(std::min(m_dragStartPos, pos), std::abs(pos - m_dragStartPos));
        }
    }
}

void TextBox::OnMouseUp(MouseEventArgs& args) {
    m_isDragging = false;
}

void TextBox::OnMouseDoubleClick(MouseEventArgs& args) {
    // Select word at position
    int pos = HitTest(point.x);
    int start = pos;
    int end = pos;
    
    while (start > 0 && m_text[start - 1] != L' ') start--;
    while (end < static_cast<int>(m_text.length()) && m_text[end] != L' ') end++;
    
    Select(start, end - start);
}

// ============================================================================
// PasswordBox
// ============================================================================

PasswordBox::PasswordBox() = default;
PasswordBox::~PasswordBox() = default;

void PasswordBox::SetPasswordChar(wchar_t ch) {
    if (m_passwordChar != ch) {
        m_passwordChar = ch;
        InvalidateRender();
    }
}

void PasswordBox::SetIsPasswordRevealed(bool revealed) {
    if (m_isRevealed != revealed) {
        m_isRevealed = revealed;
        InvalidateRender();
    }
}

std::wstring PasswordBox::GetDisplayText() const {
    if (m_isRevealed) {
        return m_text;
    }
    return std::wstring(m_text.length(), m_passwordChar);
}

void PasswordBox::MeasureOverride(const Size& availableSize) {
    SetDesiredSize(Size(GetWidth(), 32));
}

void PasswordBox::ArrangeOverride(const Size& finalSize) {
    SetRenderRect(Rect(GetRenderRect().x, GetRenderRect().y, finalSize.width, 32));
    return finalSize;
}

void PasswordBox::RenderOverride(IRenderContext* context) {
    TextInputBase::RenderOverride(context);
    
    // Draw reveal button if password is not empty
    if (!m_text.empty()) {
        Rect rect = GetRenderRect();
        float buttonSize = 20;
        float buttonX = rect.x + rect.width - buttonSize - 5;
        float buttonY = rect.y + (rect.height - buttonSize) / 2;
        
        Rect buttonRect(buttonX, buttonY, buttonSize, buttonSize);
        Color buttonColor = m_isRevealed ? Color::FromHex(0x0078D4) : Color::FromHex(0xCCCCCC);
        context->FillRectangle(buttonRect, buttonColor);
    }
}

void PasswordBox::OnMouseDown(MouseEventArgs& args) {
    Rect rect = GetRenderRect();
    float buttonSize = 20;
    float buttonX = rect.x + rect.width - buttonSize - 5;
    float buttonY = rect.y + (rect.height - buttonSize) / 2;
    
    if (args.Position.X >= buttonX && args.Position.X <= buttonX + buttonSize &&
        args.Position.Y >= buttonY && args.Position.Y <= buttonY + buttonSize) {
        SetIsPasswordRevealed(true);
    } else {
        TextInputBase::OnMouseDown(args);
    }
}

void PasswordBox::OnMouseMove(MouseEventArgs& args) {
    // Just handle selection drag
    if (m_isDragging) {
        int pos = HitTest(args.Position.X);
        if (pos != m_dragStartPos) {
            Select(std::min(m_dragStartPos, pos), std::abs(pos - m_dragStartPos));
        }
    }
}

void PasswordBox::OnMouseUp(MouseEventArgs& args) {
    SetIsPasswordRevealed(false);
    m_isDragging = false;
}

} // namespace controls
} // namespace luaui
