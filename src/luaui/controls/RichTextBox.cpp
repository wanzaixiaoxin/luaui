#include "RichTextBox.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include "Components/InputComponent.h"
#include "Interfaces/IRenderable.h"
#include "IRenderContext.h"

namespace luaui {
namespace controls {

// ============================================================================
// RichTextRun
// ============================================================================
RichTextRun::RichTextRun() {}

RichTextRun::RichTextRun(const std::wstring& text, const CharacterFormatting& format)
    : m_text(text), m_format(format) {}

std::shared_ptr<RichTextRun> RichTextRun::Split(int position) {
    if (position <= 0 || position >= GetLength()) {
        return nullptr;
    }
    
    auto newRun = std::make_shared<RichTextRun>();
    newRun->m_text = m_text.substr(position);
    newRun->m_format = m_format;
    
    m_text = m_text.substr(0, position);
    
    return newRun;
}

void RichTextRun::Insert(int position, const std::wstring& text) {
    if (position < 0 || position > GetLength()) return;
    m_text.insert(position, text);
}

void RichTextRun::Delete(int position, int length) {
    if (position < 0 || position >= GetLength()) return;
    m_text.erase(position, length);
}

// ============================================================================
// RichTextParagraph
// ============================================================================
RichTextParagraph::RichTextParagraph() {}

void RichTextParagraph::AddRun(const std::shared_ptr<RichTextRun>& run) {
    if (run) {
        m_runs.push_back(run);
    }
}

void RichTextParagraph::RemoveRun(const std::shared_ptr<RichTextRun>& run) {
    auto it = std::find(m_runs.begin(), m_runs.end(), run);
    if (it != m_runs.end()) {
        m_runs.erase(it);
    }
}

void RichTextParagraph::ClearRuns() {
    m_runs.clear();
}

std::shared_ptr<RichTextRun> RichTextParagraph::GetRun(size_t index) {
    if (index < m_runs.size()) {
        return m_runs[index];
    }
    return nullptr;
}

std::wstring RichTextParagraph::GetText() const {
    std::wstring text;
    for (const auto& run : m_runs) {
        text += run->GetText();
    }
    return text;
}

int RichTextParagraph::GetLength() const {
    int length = 0;
    for (const auto& run : m_runs) {
        length += run->GetLength();
    }
    return length;
}

int RichTextParagraph::DocumentToRun(int docPosition, int& outRunIndex) const {
    int pos = 0;
    for (size_t i = 0; i < m_runs.size(); ++i) {
        int runLen = m_runs[i]->GetLength();
        if (docPosition < pos + runLen) {
            outRunIndex = static_cast<int>(i);
            return docPosition - pos;
        }
        pos += runLen;
    }
    outRunIndex = static_cast<int>(m_runs.size()) - 1;
    return m_runs.empty() ? 0 : m_runs.back()->GetLength();
}

int RichTextParagraph::RunToDocument(int runIndex, int runPosition) const {
    int pos = 0;
    for (int i = 0; i < runIndex && i < static_cast<int>(m_runs.size()); ++i) {
        pos += m_runs[i]->GetLength();
    }
    return pos + runPosition;
}

// ============================================================================
// RichTextDocument
// ============================================================================
RichTextDocument::RichTextDocument() {
    // 创建默认段落
    auto para = std::make_shared<RichTextParagraph>();
    auto run = std::make_shared<RichTextRun>();
    para->AddRun(run);
    m_paragraphs.push_back(para);
}

void RichTextDocument::AddParagraph(const std::shared_ptr<RichTextParagraph>& para) {
    if (para) {
        m_paragraphs.push_back(para);
    }
}

void RichTextDocument::InsertParagraph(int index, 
                                        const std::shared_ptr<RichTextParagraph>& para) {
    if (para && index >= 0 && index <= static_cast<int>(m_paragraphs.size())) {
        m_paragraphs.insert(m_paragraphs.begin() + index, para);
    }
}

void RichTextDocument::RemoveParagraph(int index) {
    if (index >= 0 && index < static_cast<int>(m_paragraphs.size())) {
        m_paragraphs.erase(m_paragraphs.begin() + index);
    }
}

void RichTextDocument::ClearParagraphs() {
    m_paragraphs.clear();
}

std::shared_ptr<RichTextParagraph> RichTextDocument::GetParagraph(size_t index) {
    if (index < m_paragraphs.size()) {
        return m_paragraphs[index];
    }
    return nullptr;
}

std::wstring RichTextDocument::GetText() const {
    std::wstring text;
    for (size_t i = 0; i < m_paragraphs.size(); ++i) {
        text += m_paragraphs[i]->GetText();
        if (i < m_paragraphs.size() - 1) {
            text += L'\n';
        }
    }
    return text;
}

std::wstring RichTextDocument::GetText(const TextRange& range) const {
    std::wstring fullText = GetText();
    if (range.start < 0 || range.start >= static_cast<int>(fullText.length())) {
        return L"";
    }
    return fullText.substr(range.start, range.length);
}

int RichTextDocument::GetLength() const {
    int length = 0;
    for (const auto& para : m_paragraphs) {
        length += para->GetLength() + 1; // +1 for newline
    }
    return length > 0 ? length - 1 : 0; // -1 for last newline
}

void RichTextDocument::DocumentToParagraph(int docPosition, 
                                            int& outParaIndex, 
                                            int& outParaPos) const {
    int pos = 0;
    for (size_t i = 0; i < m_paragraphs.size(); ++i) {
        int paraLen = m_paragraphs[i]->GetLength() + 1; // +1 for newline
        if (docPosition < pos + paraLen) {
            outParaIndex = static_cast<int>(i);
            outParaPos = docPosition - pos;
            return;
        }
        pos += paraLen;
    }
    outParaIndex = static_cast<int>(m_paragraphs.size()) - 1;
    outParaPos = m_paragraphs.empty() ? 0 : m_paragraphs.back()->GetLength();
}

int RichTextDocument::ParagraphToDocument(int paraIndex, int paraPosition) const {
    int pos = 0;
    for (int i = 0; i < paraIndex && i < static_cast<int>(m_paragraphs.size()); ++i) {
        pos += m_paragraphs[i]->GetLength() + 1;
    }
    return pos + paraPosition;
}

void RichTextDocument::SetCharacterFormatting(const TextRange& range, 
                                               const CharacterFormatting& format) {
    // 简化实现：遍历所有段落和应用格式
    (void)range;
    (void)format;
}

void RichTextDocument::ApplyBold(const TextRange& range, bool bold) {
    (void)range;
    (void)bold;
}

void RichTextDocument::ApplyItalic(const TextRange& range, bool italic) {
    (void)range;
    (void)italic;
}

void RichTextDocument::ApplyUnderline(const TextRange& range, bool underline) {
    (void)range;
    (void)underline;
}

void RichTextDocument::ApplyFontSize(const TextRange& range, float size) {
    (void)range;
    (void)size;
}

void RichTextDocument::ApplyForeground(const TextRange& range, 
                                        const rendering::Color& color) {
    (void)range;
    (void)color;
}

// ============================================================================
// RichTextBox
// ============================================================================
RichTextBox::RichTextBox() {
    m_document = std::make_shared<RichTextDocument>();
}

void RichTextBox::InitializeComponents() {
    GetComponents().AddComponent<components::LayoutComponent>(this);
    GetComponents().AddComponent<components::RenderComponent>(this);
    GetComponents().AddComponent<components::InputComponent>(this);
}

void RichTextBox::SetDocument(const std::shared_ptr<RichTextDocument>& doc) {
    if (doc) {
        m_document = doc;
        m_caretPosition = 0;
        m_selection = TextRange();
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

std::wstring RichTextBox::GetText() const {
    return m_document ? m_document->GetText() : L"";
}

void RichTextBox::SetText(const std::wstring& text) {
    if (!m_document) return;
    
    // 创建新文档
    auto doc = std::make_shared<RichTextDocument>();
    auto para = std::make_shared<RichTextParagraph>();
    auto run = std::make_shared<RichTextRun>(text, CharacterFormatting());
    para->AddRun(run);
    doc->AddParagraph(para);
    
    SetDocument(doc);
}

std::wstring RichTextBox::GetRtfText() const {
    // 简化实现：返回纯文本
    return GetText();
}

void RichTextBox::SetRtfText(const std::wstring& rtf) {
    // 简化实现：作为纯文本设置
    SetText(rtf);
}

std::wstring RichTextBox::GetHtmlText() const {
    // 简化实现：返回纯文本
    return GetText();
}

void RichTextBox::SetHtmlText(const std::wstring& html) {
    // 简化实现：作为纯文本设置
    SetText(html);
}

void RichTextBox::SetSelection(const TextRange& range) {
    m_selection = range;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void RichTextBox::SelectAll() {
    if (m_document) {
        m_selection = TextRange(0, m_document->GetLength());
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void RichTextBox::Select(int start, int length) {
    SetSelection(TextRange(start, length));
}

void RichTextBox::ClearSelection() {
    m_selection = TextRange();
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

std::wstring RichTextBox::GetSelectedText() const {
    if (m_document && m_selection.length > 0) {
        return m_document->GetText(m_selection);
    }
    return L"";
}

void RichTextBox::DeleteSelection() {
    if (!m_document || m_selection.IsEmpty()) return;
    
    // 简化实现
    m_selection = TextRange();
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void RichTextBox::SetCaretPosition(int position) {
    m_caretPosition = position;
    UpdateCaretVisibility();
}

void RichTextBox::ToggleBold() {
    if (m_document) {
        m_document->ApplyBold(m_selection, true);
    }
}

void RichTextBox::ToggleItalic() {
    if (m_document) {
        m_document->ApplyItalic(m_selection, true);
    }
}

void RichTextBox::ToggleUnderline() {
    if (m_document) {
        m_document->ApplyUnderline(m_selection, true);
    }
}

void RichTextBox::ApplyFontSize(float size) {
    if (m_document) {
        m_document->ApplyFontSize(m_selection, size);
    }
}

void RichTextBox::ApplyForeground(const rendering::Color& color) {
    if (m_document) {
        m_document->ApplyForeground(m_selection, color);
    }
}

void RichTextBox::SetAlignment(rendering::TextAlignment alignment) {
    (void)alignment;
    // 简化实现
}

void RichTextBox::InsertText(const std::wstring& text) {
    if (m_isReadOnly || !m_document) return;
    
    // 如果有选区，先删除
    if (!m_selection.IsEmpty()) {
        DeleteSelection();
    }
    
    // 插入文本（简化）
    (void)text;
    
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void RichTextBox::InsertParagraphBreak() {
    if (m_isReadOnly || !m_acceptsReturn) return;
    
    InsertText(L"\n");
}

void RichTextBox::Cut() {
    if (m_isReadOnly) return;
    Copy();
    DeleteSelection();
}

void RichTextBox::Copy() {
    std::wstring text = GetSelectedText();
    if (!text.empty()) {
        // 复制到剪贴板（简化）
        (void)text;
    }
}

void RichTextBox::Paste() {
    if (m_isReadOnly) return;
    // 从剪贴板粘贴（简化）
}

void RichTextBox::UpdateCaretVisibility() {
    m_caretVisible = true;
    // 简化：实际应该启动闪烁定时器
}

void RichTextBox::ScrollToCaret() {
    // 简化实现
}

void RichTextBox::DrawCaret(rendering::IRenderContext* context) {
    if (!context || !m_caretVisible) return;
    
    // 简化：在固定位置绘制光标
    auto caretBrush = context->CreateSolidColorBrush(m_caretColor);
    if (!caretBrush) return;
    
    rendering::Rect rect;
    if (auto* renderable = AsRenderable()) {
        rect = renderable->GetRenderRect();
    }
    
    // 简化：在第一行显示光标
    float caretX = rect.x + m_padding;
    float caretY = rect.y + m_padding;
    
    context->DrawLine(rendering::Point(caretX, caretY),
                      rendering::Point(caretX, caretY + 16),
                      caretBrush.get(), 1.0f);
}

void RichTextBox::DrawSelection(rendering::IRenderContext* context) {
    if (!context || m_selection.IsEmpty()) return;
    
    auto selBrush = context->CreateSolidColorBrush(m_selectionColor);
    if (!selBrush) return;
    
    // 简化：假设选区在可视区域
    rendering::Rect rect;
    if (auto* renderable = AsRenderable()) {
        rect = renderable->GetRenderRect();
    }
    
    // 简化绘制：在实际实现中需要计算精确的选区位置
    (void)rect;
}

int RichTextBox::HitTestPosition(float x, float y) {
    (void)x;
    (void)y;
    // 简化：返回0
    return 0;
}

void RichTextBox::OnMouseDown(MouseEventArgs& args) {
    if (!m_document) return;
    
    int pos = HitTestPosition(args.x, args.y);
    SetCaretPosition(pos);
    
    m_isSelecting = true;
    m_selectionStart = pos;
    ClearSelection();
    
    args.Handled = true;
}

void RichTextBox::OnMouseMove(MouseEventArgs& args) {
    if (!m_isSelecting) return;
    
    int pos = HitTestPosition(args.x, args.y);
    int start = std::min(m_selectionStart, pos);
    int end = std::max(m_selectionStart, pos);
    
    SetSelection(TextRange(start, end - start));
    SetCaretPosition(pos);
    
    args.Handled = true;
}

void RichTextBox::OnMouseUp(MouseEventArgs& args) {
    (void)args;
    m_isSelecting = false;
}

void RichTextBox::OnKeyDown(KeyEventArgs& args) {
    if (m_isReadOnly) return;
    
    switch (args.KeyCode) {
        case Key::Left:
            if (m_caretPosition > 0) {
                SetCaretPosition(m_caretPosition - 1);
            }
            break;
            
        case Key::Right:
            if (m_document && m_caretPosition < m_document->GetLength()) {
                SetCaretPosition(m_caretPosition + 1);
            }
            break;
            
        case Key::Home:
            SetCaretPosition(0);
            break;
            
        case Key::End:
            if (m_document) {
                SetCaretPosition(m_document->GetLength());
            }
            break;
            
        case Key::Return:
            if (m_acceptsReturn) {
                InsertParagraphBreak();
            }
            break;
            
        case Key::Back:
            // 删除前一个字符
            break;
            
        case Key::Delete:
            // 删除后一个字符
            break;
            
        case Key::A:
            if (args.Control) {
                SelectAll();
            }
            break;
            
        case Key::C:
            if (args.Control) {
                Copy();
            }
            break;
            
        case Key::V:
            if (args.Control) {
                Paste();
            }
            break;
            
        case Key::X:
            if (args.Control) {
                Cut();
            }
            break;
    }
    
    args.Handled = true;
}

void RichTextBox::OnTextInput(TextCompositionEventArgs& args) {
    if (m_isReadOnly) return;
    
    InsertText(args.Text);
    args.Handled = true;
}

void RichTextBox::OnGotFocus() {
    UpdateCaretVisibility();
}

void RichTextBox::OnLostFocus() {
    m_caretVisible = false;
    ClearSelection();
}

rendering::Size RichTextBox::OnMeasure(const rendering::Size& availableSize) {
    // 最小尺寸
    float minWidth = 100.0f;
    float minHeight = 50.0f;
    
    if (availableSize.width > 0) {
        minWidth = std::max(minWidth, availableSize.width);
    }
    if (availableSize.height > 0) {
        minHeight = std::max(minHeight, availableSize.height);
    }
    
    return rendering::Size(minWidth, minHeight);
}

void RichTextBox::OnRender(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    auto rect = render->GetRenderRect();
    
    // 绘制背景
    auto bgBrush = context->CreateSolidColorBrush(m_bgColor);
    if (bgBrush) {
        context->FillRectangle(rect, bgBrush.get());
    }
    
    // 绘制边框
    auto borderBrush = context->CreateSolidColorBrush(m_borderColor);
    if (borderBrush) {
        context->DrawRectangle(rect, borderBrush.get(), 1.0f);
    }
    
    // 绘制文档内容（简化）
    if (m_document) {
        float y = rect.y + m_padding - m_scrollOffsetY;
        
        for (size_t p = 0; p < m_document->GetParagraphCount(); ++p) {
            auto para = m_document->GetParagraph(p);
            if (!para) continue;
            
            float x = rect.x + m_padding - m_scrollOffsetX;
            
            for (size_t r = 0; r < para->GetRunCount(); ++r) {
                auto run = para->GetRun(r);
                if (!run) continue;
                
                const auto& format = run->GetFormatting();
                const std::wstring& text = run->GetText();
                
                if (!text.empty()) {
                    auto textBrush = context->CreateSolidColorBrush(format.foreground);
                    float fontSize = format.fontSize;
                    auto textFormat = context->CreateTextFormat(format.fontFamily, fontSize);
                    
                    if (textBrush && textFormat) {
                        rendering::Point textPos(x, y);
                        context->DrawTextString(text, textFormat.get(), 
                                                textPos, textBrush.get());
                    }
                    
                    // 估算宽度前进
                    x += static_cast<float>(text.length()) * fontSize * 0.6f;
                }
            }
            
            y += 20; // 行高（简化）
        }
    }
    
    // 绘制选区
    DrawSelection(context);
    
    // 绘制光标
    DrawCaret(context);
}

} // namespace controls
} // namespace luaui
