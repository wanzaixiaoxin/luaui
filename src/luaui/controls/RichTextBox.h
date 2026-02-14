#pragma once

#include "Control.h"
#include "../rendering/Types.h"
#include "../rendering/ITextFormat.h"
#include <memory>
#include <string>
#include <vector>

namespace luaui {
namespace controls {

// 前向声明
class RichTextBox;
class RichTextDocument;
class RichTextParagraph;
class RichTextRun;

/**
 * @brief TextRange 文本范围
 */
struct TextRange {
    int start = 0;
    int length = 0;
    
    TextRange() = default;
    TextRange(int s, int len) : start(s), length(len) {}
    
    int End() const { return start + length; }
    bool Contains(int pos) const { return pos >= start && pos < End(); }
    bool IsEmpty() const { return length <= 0; }
};

/**
 * @brief CharacterFormatting 字符格式
 */
struct CharacterFormatting {
    std::wstring fontFamily = L"Microsoft YaHei";
    float fontSize = 14.0f;
    rendering::Color foreground = rendering::Color::Black();
    rendering::Color background = rendering::Color::Transparent();
    bool bold = false;
    bool italic = false;
    bool underline = false;
    bool strikethrough = false;
};

/**
 * @brief ParagraphFormatting 段落格式
 */
struct ParagraphFormatting {
    float lineHeight = 1.5f;        // 行高倍数
    float spaceBefore = 0.0f;       // 段前间距
    float spaceAfter = 0.0f;        // 段后间距
    float indent = 0.0f;            // 首行缩进
    rendering::TextAlignment alignment = rendering::TextAlignment::Leading;
};

/**
 * @brief RichTextRun 富文本片段
 * 
 * 一段具有相同格式的文本
 */
class RichTextRun {
public:
    RichTextRun();
    RichTextRun(const std::wstring& text, const CharacterFormatting& format);
    
    // 文本
    std::wstring GetText() const { return m_text; }
    void SetText(const std::wstring& text) { m_text = text; }
    
    // 格式
    CharacterFormatting GetFormatting() const { return m_format; }
    void SetFormatting(const CharacterFormatting& format) { m_format = format; }
    
    // 文本长度
    int GetLength() const { return static_cast<int>(m_text.length()); }
    
    // 分割
    std::shared_ptr<RichTextRun> Split(int position);
    
    // 插入文本
    void Insert(int position, const std::wstring& text);
    
    // 删除文本
    void Delete(int position, int length);

private:
    std::wstring m_text;
    CharacterFormatting m_format;
};

/**
 * @brief RichTextParagraph 富文本段落
 */
class RichTextParagraph {
public:
    RichTextParagraph();
    
    // 段落格式
    ParagraphFormatting GetFormatting() const { return m_format; }
    void SetFormatting(const ParagraphFormatting& format) { m_format = format; }
    
    // 文本片段
    void AddRun(const std::shared_ptr<RichTextRun>& run);
    void RemoveRun(const std::shared_ptr<RichTextRun>& run);
    void ClearRuns();
    size_t GetRunCount() const { return m_runs.size(); }
    std::shared_ptr<RichTextRun> GetRun(size_t index);
    
    // 获取文本
    std::wstring GetText() const;
    int GetLength() const;
    
    // 位置转换
    int DocumentToRun(int docPosition, int& outRunIndex) const;
    int RunToDocument(int runIndex, int runPosition) const;

private:
    std::vector<std::shared_ptr<RichTextRun>> m_runs;
    ParagraphFormatting m_format;
};

/**
 * @brief RichTextDocument 富文本文档
 * 
 * 存储富文本的内容和格式
 */
class RichTextDocument {
public:
    RichTextDocument();
    
    // 段落操作
    void AddParagraph(const std::shared_ptr<RichTextParagraph>& para);
    void InsertParagraph(int index, const std::shared_ptr<RichTextParagraph>& para);
    void RemoveParagraph(int index);
    void ClearParagraphs();
    size_t GetParagraphCount() const { return m_paragraphs.size(); }
    std::shared_ptr<RichTextParagraph> GetParagraph(size_t index);
    
    // 文本操作
    std::wstring GetText() const;
    std::wstring GetText(const TextRange& range) const;
    int GetLength() const;
    
    // 插入/删除文本
    void InsertText(int position, const std::wstring& text);
    void DeleteText(const TextRange& range);
    void ReplaceText(const TextRange& range, const std::wstring& text);
    
    // 格式操作
    CharacterFormatting GetCharacterFormatting(int position) const;
    void SetCharacterFormatting(const TextRange& range, const CharacterFormatting& format);
    void ApplyBold(const TextRange& range, bool bold);
    void ApplyItalic(const TextRange& range, bool italic);
    void ApplyUnderline(const TextRange& range, bool underline);
    void ApplyFontSize(const TextRange& range, float size);
    void ApplyForeground(const TextRange& range, const rendering::Color& color);
    
    // 位置转换
    void DocumentToParagraph(int docPosition, int& outParaIndex, int& outParaPos) const;
    int ParagraphToDocument(int paraIndex, int paraPosition) const;

private:
    std::vector<std::shared_ptr<RichTextParagraph>> m_paragraphs;
    CharacterFormatting m_defaultCharFormat;
    ParagraphFormatting m_defaultParaFormat;
};

/**
 * @brief RichTextBox 富文本编辑框（新架构）
 * 
 * 支持：
 * - 多种字体、大小、颜色
 * - 粗体、斜体、下划线、删除线
 * - 段落对齐
 * - 选区和光标
 * - 撤销/重做（简化版）
 */
class RichTextBox : public luaui::Control {
public:
    RichTextBox();
    
    std::string GetTypeName() const override { return "RichTextBox"; }
    
    // 文档
    std::shared_ptr<RichTextDocument> GetDocument() const { return m_document; }
    void SetDocument(const std::shared_ptr<RichTextDocument>& doc);
    
    // 文本（纯文本）
    std::wstring GetText() const;
    void SetText(const std::wstring& text);
    
    // 富文本（带格式的文本，简化表示）
    std::wstring GetRtfText() const;  // 简化：实际应该返回 RTF 格式
    void SetRtfText(const std::wstring& rtf);  // 简化：实际应该解析 RTF
    
    // HTML 格式
    std::wstring GetHtmlText() const;
    void SetHtmlText(const std::wstring& html);
    
    // 选区
    TextRange GetSelection() const { return m_selection; }
    void SetSelection(const TextRange& range);
    void SelectAll();
    void Select(int start, int length);
    void ClearSelection();
    bool HasSelection() const { return m_selection.length > 0; }
    std::wstring GetSelectedText() const;
    void DeleteSelection();
    
    // 光标位置
    int GetCaretPosition() const { return m_caretPosition; }
    void SetCaretPosition(int position);
    
    // 是否只读
    bool GetIsReadOnly() const { return m_isReadOnly; }
    void SetIsReadOnly(bool readOnly) { m_isReadOnly = readOnly; }
    
    // 是否多行
    bool GetAcceptsReturn() const { return m_acceptsReturn; }
    void SetAcceptsReturn(bool accepts) { m_acceptsReturn = accepts; }
    
    // 自动换行
    bool GetWordWrap() const { return m_wordWrap; }
    void SetWordWrap(bool wrap) { m_wordWrap = wrap; }
    
    // 格式操作
    void ToggleBold();
    void ToggleItalic();
    void ToggleUnderline();
    void ApplyFontSize(float size);
    void ApplyForeground(const rendering::Color& color);
    
    // 段落对齐
    void SetAlignment(rendering::TextAlignment alignment);
    
    // 插入
    void InsertText(const std::wstring& text);
    void InsertParagraphBreak();
    
    // 撤销/重做（简化版）
    bool CanUndo() const { return false; }  // 简化：实际应实现撤销栈
    bool CanRedo() const { return false; }
    void Undo() {}
    void Redo() {}
    
    // 剪贴板操作
    void Cut();
    void Copy();
    void Paste();

protected:
    void InitializeComponents() override;
    void OnRender(rendering::IRenderContext* context) override;
    rendering::Size OnMeasure(const rendering::Size& availableSize) override;
    
    void OnMouseDown(MouseEventArgs& args) override;
    void OnMouseMove(MouseEventArgs& args) override;
    void OnMouseUp(MouseEventArgs& args) override;
    void OnKeyDown(KeyEventArgs& args) override;
    void OnChar(wchar_t ch) override;
    void OnGotFocus() override;
    void OnLostFocus() override;

private:
    void UpdateCaretVisibility();
    void ScrollToCaret();
    void DrawCaret(rendering::IRenderContext* context);
    void DrawSelection(rendering::IRenderContext* context);
    int HitTestPosition(float x, float y);
    
    std::shared_ptr<RichTextDocument> m_document;
    
    int m_caretPosition = 0;      // 光标位置（文档中的字符位置）
    TextRange m_selection;         // 选区
    bool m_caretVisible = false;   // 光标是否可见（闪烁）
    
    bool m_isReadOnly = false;
    bool m_acceptsReturn = true;
    bool m_wordWrap = true;
    bool m_isSelecting = false;    // 是否正在拖拽选择
    int m_selectionStart = 0;      // 选择起始位置
    
    // 滚动
    float m_scrollOffsetX = 0.0f;
    float m_scrollOffsetY = 0.0f;
    
    // 外观
    float m_padding = 4.0f;
    rendering::Color m_bgColor = rendering::Color::White();
    rendering::Color m_borderColor = rendering::Color::FromHex(0x8E8E8E);
    rendering::Color m_selectionColor = rendering::Color::FromHex(0x0078D4);
    rendering::Color m_caretColor = rendering::Color::Black();
};

} // namespace controls
} // namespace luaui
