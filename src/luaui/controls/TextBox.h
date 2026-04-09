#pragma once

#include "Control.h"
#include "../core/Components/InputComponent.h"
#include "../core/Components/LayoutComponent.h"
#include "../core/Components/RenderComponent.h"
#include "../rendering/Types.h"
#include <cstdint>
#include <string>
#include <vector>

namespace luaui {
namespace rendering {
class ITextFormat;
}

namespace controls {

/**
 * @brief Single-line text editing control.
 *
 * Provides cursor movement, selection, clipboard, undo/redo and
 * Ctrl/Shift navigation for a production-quality single-line editor.
 */
class TextBox : public luaui::Control {
public:
    TextBox();

    std::string GetTypeName() const override { return "TextBox"; }

    // Text
    std::wstring GetText() const { return m_text; }
    void SetText(const std::wstring& text);

    // Placeholder
    std::wstring GetPlaceholder() const;
    void SetPlaceholder(const std::wstring& placeholder);

    // Read-only
    bool GetIsReadOnly() const { return m_isReadOnly; }
    void SetIsReadOnly(bool readOnly);

    // Password mode
    bool GetIsPassword() const { return m_isPassword; }
    void SetIsPassword(bool isPassword);

    // Maximum length
    int GetMaxLength() const;
    void SetMaxLength(int maxLength);

    // Caret position
    int GetCaretPosition() const { return m_caretPosition; }
    void SetCaretPosition(int pos);

    // Selection
    bool HasSelection() const { return m_selectionAnchor != m_selectionEnd; }
    int GetSelectionStart() const;
    int GetSelectionEnd() const;
    int GetSelectionLength() const;
    std::wstring GetSelectedText() const;
    void Select(int start, int length);
    void SelectAll();
    void ClearSelection();

    // Editing commands
    bool CanUndo() const { return !m_undoStack.empty(); }
    bool CanRedo() const { return !m_redoStack.empty(); }
    void Undo();
    void Redo();
    void Copy() const;
    void Cut();
    void Paste();

    // Events
    luaui::Delegate<TextBox*, const std::wstring&> TextChanged;

protected:
    void InitializeComponents() override;
    void ApplyTheme() override;

    void OnRender(rendering::IRenderContext* context) override;
    void OnMouseDown(MouseEventArgs& args) override;
    void OnMouseMove(MouseEventArgs& args) override;
    void OnMouseUp(MouseEventArgs& args) override;
    void OnKeyDown(KeyEventArgs& args) override;
    void OnChar(wchar_t ch) override;
    void OnGotFocus() override;
    void OnLostFocus() override;
    rendering::Size OnMeasure(const rendering::Size& availableSize) override;

    void InsertText(const std::wstring& text);
    void DeleteSelection();
    void DeleteBackward();
    void DeleteForward();

private:
    struct EditSnapshot {
        std::wstring text;
        int caretPosition = 0;
        int selectionAnchor = 0;
        int selectionEnd = 0;
        float horizontalScrollOffset = 0.0f;
    };

    std::wstring NormalizeInsertedText(const std::wstring& text) const;
    std::wstring ClampTextToMaxLength(const std::wstring& text) const;
    float EstimateTextWidth(const std::wstring& text) const;
    float MeasureTextWidth(const std::wstring& text,
                           rendering::ITextFormat* format = nullptr) const;

    int ClampPosition(int pos) const;
    int FindPreviousWordBoundary(int position) const;
    int FindNextWordBoundary(int position) const;
    int HitTestTextPosition(float globalX) const;

    rendering::Rect GetAbsoluteRenderRect() const;
    rendering::Rect GetContentRect(const rendering::Rect& localRect) const;

    EditSnapshot CaptureSnapshot() const;
    void RestoreSnapshot(const EditSnapshot& snapshot, bool notifyTextChanged);
    void PushUndoSnapshot();
    void DeleteTextRange(int start, int end);
    void DeleteWordBackward();
    void DeleteWordForward();
    void MoveCaret(int newPosition, bool extendSelection);
    void MoveCaretToBoundary(bool toEnd, bool extendSelection);
    void MoveCaretHorizontal(int direction, bool extendSelection, bool byWord);
    void EnsureCaretVisible();
    void InvalidateTextPresentation();
    void UpdateCaretVisible();
    void UpdateCaretBlink();
    std::wstring GetDisplayText() const;

    static bool IsModifierPressed(int virtualKey);
    static bool IsWordCharacter(wchar_t ch);
    static bool ReadClipboardText(std::wstring& text);
    static bool WriteClipboardText(const std::wstring& text);

    std::wstring m_text;
    std::wstring m_placeholder;
    bool m_isReadOnly = false;
    bool m_isPassword = false;
    int m_maxLength = 0;

    int m_caretPosition = 0;
    int m_selectionAnchor = 0;
    int m_selectionEnd = 0;
    bool m_isMouseSelecting = false;

    float m_fontSize = 14.0f;
    float m_padding = 6.0f;
    float m_horizontalScrollOffset = 0.0f;

    rendering::Color m_foreground = rendering::Color::Black();
    rendering::Color m_background = rendering::Color::White();
    rendering::Color m_readOnlyBackground = rendering::Color::FromHex(0xF5F5F5);
    rendering::Color m_placeholderColor = rendering::Color::FromHex(0x8A8A8A);
    rendering::Color m_borderColor = rendering::Color::FromHex(0x808080);
    rendering::Color m_focusedBorderColor = rendering::Color::FromHex(0x0078D4);
    rendering::Color m_selectionColor = rendering::Color::FromHex(0x0078D4);
    rendering::Color m_inactiveSelectionColor = rendering::Color::FromHex(0xD8D8D8);
    rendering::Color m_selectionForeground = rendering::Color::White();

    bool m_isCaretVisible = true;
    uint64_t m_lastCaretBlinkTime = 0;

    std::vector<EditSnapshot> m_undoStack;
    std::vector<EditSnapshot> m_redoStack;

    static constexpr uint64_t CARET_BLINK_INTERVAL_MS = 530;
    static constexpr size_t MAX_HISTORY_DEPTH = 64;
};

} // namespace controls
} // namespace luaui
