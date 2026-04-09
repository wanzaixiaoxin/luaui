#include "TextBox.h"
#include "IRenderContext.h"
#include "ITextFormat.h"
#include "ResourceCache.h"
#include "Window.h"
#include "Theme.h"
#include "ThemeKeys.h"
#include <algorithm>
#include <cstring>
#include <cwctype>
#include <utility>
#include <windows.h>

namespace luaui {
namespace controls {

namespace {

struct TextFormatAccess {
    rendering::ITextFormat* raw = nullptr;
    rendering::ITextFormatPtr owned;
};

TextFormatAccess AcquireTextFormat(const TextBox* textBox, rendering::IRenderContext* context,
                                   float fontSize) {
    TextFormatAccess access;

    if (auto* window = textBox->GetWindow()) {
        if (auto* cache = window->GetResourceCache()) {
            access.raw = cache->GetTextFormat(L"Microsoft YaHei", fontSize);
            return access;
        }
    }

    if (context) {
        access.owned = context->CreateTextFormat(L"Microsoft YaHei", fontSize);
        access.raw = access.owned.get();
    }

    return access;
}

rendering::ISolidColorBrush* AcquireBrush(const TextBox* textBox,
                                          rendering::IRenderContext* context,
                                          const rendering::Color& color,
                                          rendering::ISolidColorBrushPtr& owned) {
    if (auto* window = textBox->GetWindow()) {
        if (auto* cache = window->GetResourceCache()) {
            return cache->GetSolidColorBrush(color);
        }
    }

    owned = context ? context->CreateSolidColorBrush(color) : nullptr;
    return owned.get();
}

} // namespace

TextBox::TextBox() = default;

void TextBox::ApplyTheme() {
    auto& t = Theme::GetCurrent();
    using namespace theme;
    m_background = t.GetColor(kTextBoxBackground);
    m_readOnlyBackground = t.GetColor(kTextBoxReadOnlyBg);
    m_borderColor = t.GetColor(kTextBoxBorder);
    m_focusedBorderColor = t.GetColor(kTextBoxFocusedBorder);
    m_selectionColor = t.GetColor(kTextBoxSelection);
    m_inactiveSelectionColor = t.GetColor(kTextBoxInactiveSel);
    m_foreground = t.GetColor(kTextPrimary);
    m_placeholderColor = t.GetColor(kTextBoxPlaceholder);
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void TextBox::InitializeComponents() {
    auto* layout = GetComponents().AddComponent<components::LayoutComponent>(this);
    layout->SetWidth(160.0f);
    layout->SetHeight(28.0f);

    GetComponents().AddComponent<components::RenderComponent>(this);

    auto* input = GetComponents().AddComponent<components::InputComponent>(this);
    input->SetIsFocusable(true);
}

void TextBox::SetText(const std::wstring& text) {
    const std::wstring clampedText = ClampTextToMaxLength(text);
    if (m_text == clampedText) {
        m_caretPosition = ClampPosition(m_caretPosition);
        m_selectionAnchor = m_caretPosition;
        m_selectionEnd = m_caretPosition;
        EnsureCaretVisible();
        InvalidateTextPresentation();
        return;
    }

    m_text = clampedText;
    m_caretPosition = ClampPosition(m_caretPosition);
    m_selectionAnchor = m_caretPosition;
    m_selectionEnd = m_caretPosition;
    m_horizontalScrollOffset = 0.0f;
    m_undoStack.clear();
    m_redoStack.clear();

    EnsureCaretVisible();
    UpdateCaretVisible();
    TextChanged.Invoke(this, m_text);
    InvalidateTextPresentation();
}

std::wstring TextBox::GetPlaceholder() const {
    return m_placeholder;
}

void TextBox::SetPlaceholder(const std::wstring& placeholder) {
    if (m_placeholder == placeholder) {
        return;
    }

    m_placeholder = placeholder;
    InvalidateTextPresentation();
}

void TextBox::SetIsReadOnly(bool readOnly) {
    if (m_isReadOnly == readOnly) {
        return;
    }

    m_isReadOnly = readOnly;
    InvalidateTextPresentation();
}

void TextBox::SetIsPassword(bool isPassword) {
    if (m_isPassword == isPassword) {
        return;
    }

    m_isPassword = isPassword;
    EnsureCaretVisible();
    InvalidateTextPresentation();
}

int TextBox::GetMaxLength() const {
    return m_maxLength;
}

void TextBox::SetMaxLength(int maxLength) {
    maxLength = std::max(0, maxLength);
    if (m_maxLength == maxLength) {
        return;
    }

    m_maxLength = maxLength;

    const std::wstring clampedText = ClampTextToMaxLength(m_text);
    if (clampedText != m_text) {
        SetText(clampedText);
    } else {
        InvalidateTextPresentation();
    }
}

void TextBox::SetCaretPosition(int pos) {
    MoveCaret(pos, false);
}

int TextBox::GetSelectionStart() const {
    return std::min(m_selectionAnchor, m_selectionEnd);
}

int TextBox::GetSelectionEnd() const {
    return std::max(m_selectionAnchor, m_selectionEnd);
}

int TextBox::GetSelectionLength() const {
    return GetSelectionEnd() - GetSelectionStart();
}

std::wstring TextBox::GetSelectedText() const {
    if (!HasSelection()) {
        return L"";
    }

    return m_text.substr(static_cast<size_t>(GetSelectionStart()),
                         static_cast<size_t>(GetSelectionLength()));
}

void TextBox::Select(int start, int length) {
    const int clampedStart = ClampPosition(start);
    const int clampedEnd = ClampPosition(start + length);

    m_selectionAnchor = clampedStart;
    m_selectionEnd = clampedEnd;
    m_caretPosition = clampedEnd;

    EnsureCaretVisible();
    UpdateCaretVisible();
    InvalidateTextPresentation();
}

void TextBox::SelectAll() {
    m_selectionAnchor = 0;
    m_selectionEnd = static_cast<int>(m_text.length());
    m_caretPosition = m_selectionEnd;

    EnsureCaretVisible();
    UpdateCaretVisible();
    InvalidateTextPresentation();
}

void TextBox::ClearSelection() {
    if (!HasSelection()) {
        return;
    }

    m_selectionAnchor = m_caretPosition;
    m_selectionEnd = m_caretPosition;
    InvalidateTextPresentation();
}

void TextBox::Undo() {
    if (!CanUndo()) {
        return;
    }

    m_redoStack.push_back(CaptureSnapshot());

    const EditSnapshot snapshot = m_undoStack.back();
    m_undoStack.pop_back();
    RestoreSnapshot(snapshot, true);
}

void TextBox::Redo() {
    if (!CanRedo()) {
        return;
    }

    m_undoStack.push_back(CaptureSnapshot());

    const EditSnapshot snapshot = m_redoStack.back();
    m_redoStack.pop_back();
    RestoreSnapshot(snapshot, true);
}

void TextBox::Copy() const {
    if (m_isPassword || !HasSelection()) {
        return;
    }

    WriteClipboardText(GetSelectedText());
}

void TextBox::Cut() {
    if (m_isReadOnly || m_isPassword || !HasSelection()) {
        return;
    }

    Copy();
    DeleteSelection();
}

void TextBox::Paste() {
    if (m_isReadOnly) {
        return;
    }

    std::wstring clipboardText;
    if (!ReadClipboardText(clipboardText)) {
        return;
    }

    InsertText(clipboardText);
}

void TextBox::OnRender(rendering::IRenderContext* context) {
    if (!context) {
        return;
    }

    auto* render = GetRender();
    auto* input = GetInput();
    if (!render || !input) {
        return;
    }

    const rendering::Rect localRect(0.0f, 0.0f,
                                    render->GetRenderRect().width,
                                    render->GetRenderRect().height);
    const rendering::Rect contentRect = GetContentRect(localRect);
    const bool isFocused = input->GetIsFocused();

    rendering::ISolidColorBrushPtr ownedBackgroundBrush;
    rendering::ISolidColorBrushPtr ownedBorderBrush;
    rendering::ISolidColorBrushPtr ownedTextBrush;
    rendering::ISolidColorBrushPtr ownedPlaceholderBrush;
    rendering::ISolidColorBrushPtr ownedSelectionBrush;
    rendering::ISolidColorBrushPtr ownedSelectionTextBrush;
    rendering::ISolidColorBrushPtr ownedCaretBrush;

    const rendering::Color backgroundColor = m_isReadOnly ? m_readOnlyBackground : m_background;
    const rendering::Color borderColor = isFocused ? m_focusedBorderColor : m_borderColor;
    const rendering::Color selectionColor = isFocused ? m_selectionColor : m_inactiveSelectionColor;

    auto* backgroundBrush = AcquireBrush(this, context, backgroundColor, ownedBackgroundBrush);
    auto* borderBrush = AcquireBrush(this, context, borderColor, ownedBorderBrush);
    auto* textBrush = AcquireBrush(this, context, m_foreground, ownedTextBrush);
    auto* placeholderBrush = AcquireBrush(this, context, m_placeholderColor, ownedPlaceholderBrush);
    auto* selectionBrush = AcquireBrush(this, context, selectionColor, ownedSelectionBrush);
    auto* selectionTextBrush = AcquireBrush(this, context, m_selectionForeground,
                                            ownedSelectionTextBrush);
    auto* caretBrush = AcquireBrush(this, context, m_foreground, ownedCaretBrush);

    if (backgroundBrush) {
        context->FillRectangle(localRect, backgroundBrush);
    }

    if (borderBrush) {
        context->DrawRectangle(localRect, borderBrush, 1.0f);
    }

    TextFormatAccess formatAccess = AcquireTextFormat(this, context, m_fontSize);
    rendering::ITextFormat* format = formatAccess.raw;

    const std::wstring displayText = GetDisplayText();
    const float lineHeight = format
        ? std::max(m_fontSize * 1.2f, format->MeasureText(L"Ag", 0.0f).height)
        : m_fontSize * 1.2f;
    const float textY = contentRect.y + std::max(0.0f, (contentRect.height - lineHeight) / 2.0f);
    const float textX = contentRect.x + m_padding - m_horizontalScrollOffset;

    context->PushClip(contentRect);

    if (displayText.empty()) {
        if (!m_placeholder.empty() && !isFocused && placeholderBrush && format) {
            context->DrawTextString(m_placeholder, format,
                                    rendering::Point(textX, textY), placeholderBrush);
        }
    } else if (format && textBrush) {
        if (HasSelection()) {
            const int selectionStart = GetSelectionStart();
            const int selectionEnd = GetSelectionEnd();

            const std::wstring prefix = displayText.substr(0, static_cast<size_t>(selectionStart));
            const std::wstring selected = displayText.substr(
                static_cast<size_t>(selectionStart),
                static_cast<size_t>(selectionEnd - selectionStart));
            const std::wstring suffix = displayText.substr(static_cast<size_t>(selectionEnd));

            const float prefixWidth = MeasureTextWidth(prefix, format);
            const float selectionWidth = MeasureTextWidth(selected, format);
            const float selectionX = textX + prefixWidth;

            if (selectionBrush && selectionWidth > 0.0f) {
                const rendering::Rect selectionRect(
                    selectionX,
                    contentRect.y + 2.0f,
                    selectionWidth,
                    std::max(0.0f, contentRect.height - 4.0f));
                context->FillRectangle(selectionRect, selectionBrush);
            }

            if (!prefix.empty()) {
                context->DrawTextString(prefix, format, rendering::Point(textX, textY), textBrush);
            }

            if (!selected.empty()) {
                context->DrawTextString(
                    selected,
                    format,
                    rendering::Point(selectionX, textY),
                    (isFocused && selectionTextBrush) ? selectionTextBrush : textBrush);
            }

            if (!suffix.empty()) {
                context->DrawTextString(
                    suffix,
                    format,
                    rendering::Point(selectionX + selectionWidth, textY),
                    textBrush);
            }
        } else {
            context->DrawTextString(displayText, format, rendering::Point(textX, textY), textBrush);
        }
    }

    if (isFocused && caretBrush) {
        UpdateCaretBlink();
        if (m_isCaretVisible) {
            const float caretX = textX + MeasureTextWidth(
                displayText.substr(0, static_cast<size_t>(m_caretPosition)), format);
            context->DrawLine(rendering::Point(caretX, contentRect.y + 2.0f),
                              rendering::Point(caretX, contentRect.y + contentRect.height - 2.0f),
                              caretBrush, 1.0f);
        }
    }

    context->PopClip();
}

void TextBox::OnMouseDown(MouseEventArgs& args) {
    if (args.button != 0) {
        return;
    }

    if (auto* input = GetInput()) {
        input->Focus();
    }

    const bool extendSelection = IsModifierPressed(VK_SHIFT);
    const int hitPosition = HitTestTextPosition(args.x);

    if (extendSelection) {
        MoveCaret(hitPosition, true);
    } else {
        m_selectionAnchor = hitPosition;
        m_selectionEnd = hitPosition;
        m_caretPosition = hitPosition;
        EnsureCaretVisible();
        UpdateCaretVisible();
        InvalidateTextPresentation();
    }

    m_isMouseSelecting = true;
    args.Handled = true;
}

void TextBox::OnMouseMove(MouseEventArgs& args) {
    if (!m_isMouseSelecting) {
        return;
    }

    MoveCaret(HitTestTextPosition(args.x), true);
    args.Handled = true;
}

void TextBox::OnMouseUp(MouseEventArgs& args) {
    if (!m_isMouseSelecting) {
        return;
    }

    m_isMouseSelecting = false;
    MoveCaret(HitTestTextPosition(args.x), true);
    args.Handled = true;
}

void TextBox::OnKeyDown(KeyEventArgs& args) {
    const bool control = args.Control || IsModifierPressed(VK_CONTROL);
    const bool shift = args.Shift || IsModifierPressed(VK_SHIFT);

    switch (args.keyCode) {
        case 'A':
            if (control) {
                SelectAll();
                args.Handled = true;
            }
            return;

        case 'C':
            if (control) {
                Copy();
                args.Handled = true;
            }
            return;

        case 'X':
            if (control) {
                Cut();
                args.Handled = true;
            }
            return;

        case 'V':
            if (control) {
                Paste();
                args.Handled = true;
            }
            return;

        case 'Z':
            if (control) {
                if (shift) {
                    Redo();
                } else {
                    Undo();
                }
                args.Handled = true;
            }
            return;

        case 'Y':
            if (control) {
                Redo();
                args.Handled = true;
            }
            return;

        case VK_INSERT:
            if (control) {
                Copy();
                args.Handled = true;
            } else if (shift) {
                Paste();
                args.Handled = true;
            }
            return;

        case VK_LEFT:
            MoveCaretHorizontal(-1, shift, control);
            args.Handled = true;
            return;

        case VK_RIGHT:
            MoveCaretHorizontal(1, shift, control);
            args.Handled = true;
            return;

        case VK_HOME:
            MoveCaretToBoundary(false, shift);
            args.Handled = true;
            return;

        case VK_END:
            MoveCaretToBoundary(true, shift);
            args.Handled = true;
            return;

        case VK_BACK:
            if (!m_isReadOnly) {
                if (control) {
                    DeleteWordBackward();
                } else {
                    DeleteBackward();
                }
            }
            args.Handled = true;
            return;

        case VK_DELETE:
            if (!m_isReadOnly) {
                if (control) {
                    DeleteWordForward();
                } else {
                    DeleteForward();
                }
            }
            args.Handled = true;
            return;

        default:
            break;
    }
}

void TextBox::OnChar(wchar_t ch) {
    if (m_isReadOnly || ch < 32) {
        return;
    }

    InsertText(std::wstring(1, ch));
}

void TextBox::OnGotFocus() {
    UpdateCaretVisible();
}

void TextBox::OnLostFocus() {
    m_isMouseSelecting = false;
    m_isCaretVisible = false;
    InvalidateTextPresentation();
}

rendering::Size TextBox::OnMeasure(const rendering::Size& availableSize) {
    (void)availableSize;

    float width = 160.0f;
    float height = std::max(28.0f, m_fontSize * 1.8f);

    if (auto* layout = GetLayout()) {
        if (layout->GetWidth() > 0.0f) {
            width = layout->GetWidth();
        }
        if (layout->GetHeight() > 0.0f) {
            height = layout->GetHeight();
        }
    }

    return rendering::Size(width, height);
}

void TextBox::InsertText(const std::wstring& text) {
    if (m_isReadOnly) {
        return;
    }

    std::wstring normalized = NormalizeInsertedText(text);
    if (normalized.empty()) {
        return;
    }

    const int selectionStart = GetSelectionStart();
    const int selectionEnd = GetSelectionEnd();
    const int selectedLength = selectionEnd - selectionStart;

    if (m_maxLength > 0) {
        const int remainingCapacity =
            m_maxLength - (static_cast<int>(m_text.length()) - selectedLength);
        if (remainingCapacity <= 0) {
            return;
        }

        if (static_cast<int>(normalized.length()) > remainingCapacity) {
            normalized = normalized.substr(0, static_cast<size_t>(remainingCapacity));
        }
    }

    if (normalized.empty()) {
        return;
    }

    PushUndoSnapshot();

    if (HasSelection()) {
        m_text.erase(static_cast<size_t>(selectionStart),
                     static_cast<size_t>(selectionEnd - selectionStart));
        m_caretPosition = selectionStart;
    }

    m_text.insert(static_cast<size_t>(m_caretPosition), normalized);
    m_caretPosition += static_cast<int>(normalized.length());
    m_selectionAnchor = m_caretPosition;
    m_selectionEnd = m_caretPosition;
    m_redoStack.clear();

    EnsureCaretVisible();
    UpdateCaretVisible();
    TextChanged.Invoke(this, m_text);
    InvalidateTextPresentation();
}

void TextBox::DeleteSelection() {
    if (m_isReadOnly || !HasSelection()) {
        return;
    }

    PushUndoSnapshot();
    DeleteTextRange(GetSelectionStart(), GetSelectionEnd());
    m_redoStack.clear();

    EnsureCaretVisible();
    UpdateCaretVisible();
    TextChanged.Invoke(this, m_text);
    InvalidateTextPresentation();
}

void TextBox::DeleteBackward() {
    if (m_isReadOnly) {
        return;
    }

    if (HasSelection()) {
        DeleteSelection();
        return;
    }

    if (m_caretPosition <= 0) {
        return;
    }

    PushUndoSnapshot();
    DeleteTextRange(m_caretPosition - 1, m_caretPosition);
    m_redoStack.clear();

    EnsureCaretVisible();
    UpdateCaretVisible();
    TextChanged.Invoke(this, m_text);
    InvalidateTextPresentation();
}

void TextBox::DeleteForward() {
    if (m_isReadOnly) {
        return;
    }

    if (HasSelection()) {
        DeleteSelection();
        return;
    }

    if (m_caretPosition >= static_cast<int>(m_text.length())) {
        return;
    }

    PushUndoSnapshot();
    DeleteTextRange(m_caretPosition, m_caretPosition + 1);
    m_redoStack.clear();

    EnsureCaretVisible();
    UpdateCaretVisible();
    TextChanged.Invoke(this, m_text);
    InvalidateTextPresentation();
}

std::wstring TextBox::NormalizeInsertedText(const std::wstring& text) const {
    std::wstring normalized;
    normalized.reserve(text.length());

    for (wchar_t ch : text) {
        if (ch == L'\r') {
            continue;
        }

        if (ch == L'\n' || ch == L'\t') {
            normalized.push_back(L' ');
            continue;
        }

        if (ch >= 32) {
            normalized.push_back(ch);
        }
    }

    return normalized;
}

std::wstring TextBox::ClampTextToMaxLength(const std::wstring& text) const {
    if (m_maxLength <= 0 || static_cast<int>(text.length()) <= m_maxLength) {
        return text;
    }

    return text.substr(0, static_cast<size_t>(m_maxLength));
}

float TextBox::EstimateTextWidth(const std::wstring& text) const {
    float width = 0.0f;

    for (wchar_t ch : text) {
        if (std::iswspace(static_cast<wint_t>(ch)) != 0) {
            width += m_fontSize * 0.35f;
        } else if (ch >= 0x4E00 && ch <= 0x9FFF) {
            width += m_fontSize;
        } else {
            width += m_fontSize * 0.6f;
        }
    }

    return width;
}

float TextBox::MeasureTextWidth(const std::wstring& text, rendering::ITextFormat* format) const {
    if (text.empty()) {
        return 0.0f;
    }

    if (format) {
        return format->MeasureText(text, 0.0f).width;
    }

    TextFormatAccess formatAccess = AcquireTextFormat(this, nullptr, m_fontSize);
    if (formatAccess.raw) {
        return formatAccess.raw->MeasureText(text, 0.0f).width;
    }

    return EstimateTextWidth(text);
}

int TextBox::ClampPosition(int pos) const {
    return std::max(0, std::min(pos, static_cast<int>(m_text.length())));
}

int TextBox::FindPreviousWordBoundary(int position) const {
    int pos = ClampPosition(position);
    while (pos > 0 && std::iswspace(static_cast<wint_t>(m_text[static_cast<size_t>(pos - 1)])) != 0) {
        --pos;
    }

    if (pos <= 0) {
        return 0;
    }

    const bool isWord = IsWordCharacter(m_text[static_cast<size_t>(pos - 1)]);
    while (pos > 0) {
        const wchar_t ch = m_text[static_cast<size_t>(pos - 1)];
        if (std::iswspace(static_cast<wint_t>(ch)) != 0 || IsWordCharacter(ch) != isWord) {
            break;
        }
        --pos;
    }

    return pos;
}

int TextBox::FindNextWordBoundary(int position) const {
    const int length = static_cast<int>(m_text.length());
    int pos = ClampPosition(position);
    if (pos >= length) {
        return length;
    }

    const bool isWord = IsWordCharacter(m_text[static_cast<size_t>(pos)]);
    while (pos < length) {
        const wchar_t ch = m_text[static_cast<size_t>(pos)];
        if (std::iswspace(static_cast<wint_t>(ch)) != 0 || IsWordCharacter(ch) != isWord) {
            break;
        }
        ++pos;
    }

    while (pos < length && std::iswspace(static_cast<wint_t>(m_text[static_cast<size_t>(pos)])) != 0) {
        ++pos;
    }

    return pos;
}

int TextBox::HitTestTextPosition(float globalX) const {
    const rendering::Rect absoluteRect = GetAbsoluteRenderRect();
    const rendering::Rect contentRect = GetContentRect(
        rendering::Rect(0.0f, 0.0f, absoluteRect.width, absoluteRect.height));
    const float localTextX =
        (globalX - absoluteRect.x) + m_horizontalScrollOffset - (contentRect.x + m_padding);
    if (localTextX <= 0.0f) {
        return 0;
    }

    const std::wstring displayText = GetDisplayText();
    TextFormatAccess formatAccess = AcquireTextFormat(this, nullptr, m_fontSize);
    rendering::ITextFormat* format = formatAccess.raw;

    float accumulatedWidth = 0.0f;
    for (size_t i = 0; i < displayText.length(); ++i) {
        const float charWidth = MeasureTextWidth(displayText.substr(i, 1), format);
        if (localTextX < accumulatedWidth + (charWidth / 2.0f)) {
            return static_cast<int>(i);
        }
        accumulatedWidth += charWidth;
    }

    return static_cast<int>(displayText.length());
}

rendering::Rect TextBox::GetAbsoluteRenderRect() const {
    rendering::Rect rect;
    if (auto* render = const_cast<TextBox*>(this)->GetRender()) {
        rect = render->GetRenderRect();
    }

    auto parent = GetParent();
    while (parent) {
        if (auto parentControl = std::dynamic_pointer_cast<Control>(parent)) {
            if (auto* parentRender = parentControl->GetRender()) {
                rect.x += parentRender->GetRenderRect().x;
                rect.y += parentRender->GetRenderRect().y;
            }
        }
        parent = parent->GetParent();
    }

    return rect;
}

rendering::Rect TextBox::GetContentRect(const rendering::Rect& localRect) const {
    return rendering::Rect(
        localRect.x + 1.0f,
        localRect.y + 1.0f,
        std::max(0.0f, localRect.width - 2.0f),
        std::max(0.0f, localRect.height - 2.0f));
}

TextBox::EditSnapshot TextBox::CaptureSnapshot() const {
    EditSnapshot snapshot;
    snapshot.text = m_text;
    snapshot.caretPosition = m_caretPosition;
    snapshot.selectionAnchor = m_selectionAnchor;
    snapshot.selectionEnd = m_selectionEnd;
    snapshot.horizontalScrollOffset = m_horizontalScrollOffset;
    return snapshot;
}

void TextBox::RestoreSnapshot(const EditSnapshot& snapshot, bool notifyTextChanged) {
    m_text = ClampTextToMaxLength(snapshot.text);
    m_caretPosition = ClampPosition(snapshot.caretPosition);
    m_selectionAnchor = ClampPosition(snapshot.selectionAnchor);
    m_selectionEnd = ClampPosition(snapshot.selectionEnd);
    m_horizontalScrollOffset = std::max(0.0f, snapshot.horizontalScrollOffset);

    EnsureCaretVisible();
    UpdateCaretVisible();

    if (notifyTextChanged) {
        TextChanged.Invoke(this, m_text);
    }

    InvalidateTextPresentation();
}

void TextBox::PushUndoSnapshot() {
    if (m_undoStack.size() == MAX_HISTORY_DEPTH) {
        m_undoStack.erase(m_undoStack.begin());
    }

    m_undoStack.push_back(CaptureSnapshot());
}

void TextBox::DeleteTextRange(int start, int end) {
    const int rangeStart = ClampPosition(std::min(start, end));
    const int rangeEnd = ClampPosition(std::max(start, end));
    if (rangeStart >= rangeEnd) {
        return;
    }

    m_text.erase(static_cast<size_t>(rangeStart), static_cast<size_t>(rangeEnd - rangeStart));
    m_caretPosition = rangeStart;
    m_selectionAnchor = rangeStart;
    m_selectionEnd = rangeStart;
}

void TextBox::DeleteWordBackward() {
    if (HasSelection()) {
        DeleteSelection();
        return;
    }

    const int newPosition = FindPreviousWordBoundary(m_caretPosition);
    if (newPosition == m_caretPosition) {
        return;
    }

    PushUndoSnapshot();
    DeleteTextRange(newPosition, m_caretPosition);
    m_redoStack.clear();

    EnsureCaretVisible();
    UpdateCaretVisible();
    TextChanged.Invoke(this, m_text);
    InvalidateTextPresentation();
}

void TextBox::DeleteWordForward() {
    if (HasSelection()) {
        DeleteSelection();
        return;
    }

    const int newPosition = FindNextWordBoundary(m_caretPosition);
    if (newPosition == m_caretPosition) {
        return;
    }

    PushUndoSnapshot();
    DeleteTextRange(m_caretPosition, newPosition);
    m_redoStack.clear();

    EnsureCaretVisible();
    UpdateCaretVisible();
    TextChanged.Invoke(this, m_text);
    InvalidateTextPresentation();
}

void TextBox::MoveCaret(int newPosition, bool extendSelection) {
    const int targetPosition = ClampPosition(newPosition);
    const int oldCaret = m_caretPosition;
    const int oldSelectionAnchor = m_selectionAnchor;
    const int oldSelectionEnd = m_selectionEnd;

    if (extendSelection) {
        if (!HasSelection()) {
            m_selectionAnchor = m_caretPosition;
        }
        m_caretPosition = targetPosition;
        m_selectionEnd = targetPosition;
    } else {
        m_caretPosition = targetPosition;
        m_selectionAnchor = targetPosition;
        m_selectionEnd = targetPosition;
    }

    if (oldCaret == m_caretPosition &&
        oldSelectionAnchor == m_selectionAnchor &&
        oldSelectionEnd == m_selectionEnd) {
        return;
    }

    EnsureCaretVisible();
    UpdateCaretVisible();
    InvalidateTextPresentation();
}

void TextBox::MoveCaretToBoundary(bool toEnd, bool extendSelection) {
    MoveCaret(toEnd ? static_cast<int>(m_text.length()) : 0, extendSelection);
}

void TextBox::MoveCaretHorizontal(int direction, bool extendSelection, bool byWord) {
    if (direction == 0) {
        return;
    }

    if (!extendSelection && HasSelection()) {
        if (direction < 0) {
            MoveCaret(byWord ? FindPreviousWordBoundary(GetSelectionStart()) : GetSelectionStart(),
                      false);
        } else {
            MoveCaret(byWord ? FindNextWordBoundary(GetSelectionEnd()) : GetSelectionEnd(), false);
        }
        return;
    }

    int newPosition = m_caretPosition;
    if (byWord) {
        newPosition = direction < 0 ? FindPreviousWordBoundary(m_caretPosition)
                                    : FindNextWordBoundary(m_caretPosition);
    } else {
        newPosition += direction;
    }

    MoveCaret(newPosition, extendSelection);
}

void TextBox::EnsureCaretVisible() {
    rendering::Rect bounds;
    if (auto* render = GetRender()) {
        bounds = render->GetRenderRect();
    } else if (auto* layout = GetLayout()) {
        bounds.width = layout->GetWidth();
        bounds.height = layout->GetHeight();
    }

    const rendering::Rect contentRect = GetContentRect(
        rendering::Rect(0.0f, 0.0f, bounds.width, bounds.height));
    const float viewportWidth = std::max(0.0f, contentRect.width - (m_padding * 2.0f));
    if (viewportWidth <= 0.0f) {
        m_horizontalScrollOffset = 0.0f;
        return;
    }

    const std::wstring displayText = GetDisplayText();
    const float caretX = MeasureTextWidth(
        displayText.substr(0, static_cast<size_t>(m_caretPosition)));
    const float totalTextWidth = MeasureTextWidth(displayText);

    if (caretX < m_horizontalScrollOffset) {
        m_horizontalScrollOffset = caretX;
    } else if (caretX > m_horizontalScrollOffset + viewportWidth) {
        m_horizontalScrollOffset = caretX - viewportWidth + 1.0f;
    }

    m_horizontalScrollOffset = std::clamp(
        m_horizontalScrollOffset,
        0.0f,
        std::max(0.0f, totalTextWidth - viewportWidth));
}

void TextBox::InvalidateTextPresentation() {
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void TextBox::UpdateCaretVisible() {
    m_isCaretVisible = true;
    m_lastCaretBlinkTime = GetTickCount64();
    InvalidateTextPresentation();
}

void TextBox::UpdateCaretBlink() {
    const uint64_t currentTime = GetTickCount64();
    if (currentTime - m_lastCaretBlinkTime < CARET_BLINK_INTERVAL_MS) {
        return;
    }

    m_isCaretVisible = !m_isCaretVisible;
    m_lastCaretBlinkTime = currentTime;
    InvalidateTextPresentation();
}

std::wstring TextBox::GetDisplayText() const {
    if (m_isPassword) {
        return std::wstring(m_text.length(), L'*');
    }

    return m_text;
}

bool TextBox::IsModifierPressed(int virtualKey) {
    return (GetKeyState(virtualKey) & 0x8000) != 0;
}

bool TextBox::IsWordCharacter(wchar_t ch) {
    return std::iswalnum(static_cast<wint_t>(ch)) != 0 || ch == L'_' || ch >= 0x80;
}

bool TextBox::ReadClipboardText(std::wstring& text) {
    text.clear();

    if (!IsClipboardFormatAvailable(CF_UNICODETEXT) || !OpenClipboard(nullptr)) {
        return false;
    }

    const HANDLE handle = GetClipboardData(CF_UNICODETEXT);
    if (!handle) {
        CloseClipboard();
        return false;
    }

    const wchar_t* clipboardData = static_cast<const wchar_t*>(GlobalLock(handle));
    if (!clipboardData) {
        CloseClipboard();
        return false;
    }

    text = clipboardData;
    GlobalUnlock(handle);
    CloseClipboard();
    return true;
}

bool TextBox::WriteClipboardText(const std::wstring& text) {
    if (!OpenClipboard(nullptr)) {
        return false;
    }

    if (!EmptyClipboard()) {
        CloseClipboard();
        return false;
    }

    const SIZE_T bytes = (text.length() + 1) * sizeof(wchar_t);
    HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (!memory) {
        CloseClipboard();
        return false;
    }

    wchar_t* buffer = static_cast<wchar_t*>(GlobalLock(memory));
    if (!buffer) {
        GlobalFree(memory);
        CloseClipboard();
        return false;
    }

    memcpy(buffer, text.c_str(), bytes);
    GlobalUnlock(memory);

    if (!SetClipboardData(CF_UNICODETEXT, memory)) {
        GlobalFree(memory);
        CloseClipboard();
        return false;
    }

    CloseClipboard();
    return true;
}

} // namespace controls
} // namespace luaui
