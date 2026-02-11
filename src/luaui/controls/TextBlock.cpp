#include "TextBlock.h"
#include "IRenderContext.h"

namespace luaui {
namespace controls {

TextBlock::TextBlock() {
    InitializeComponents();
}

void TextBlock::InitializeComponents() {
    // 添加布局组件
    auto* layout = GetComponents().AddComponent<components::LayoutComponent>(this);
    // TextBlock 默认根据内容调整大小
    
    // 添加渲染组件
    GetComponents().AddComponent<components::RenderComponent>(this);
}

void TextBlock::SetText(const std::wstring& text) {
    if (m_text != text) {
        m_text = text;
        m_textDirty = true;
        
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void TextBlock::SetFontSize(float size) {
    if (m_fontSize != size) {
        m_fontSize = size;
        m_textDirty = true;
        
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void TextBlock::SetForeground(const rendering::Color& color) {
    m_foreground = color;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void TextBlock::OnRender(rendering::IRenderContext* context) {
    if (!context || m_text.empty()) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    const auto& rect = render->GetRenderRect();
    
    // TODO: 实际文本渲染需要文本布局和画刷
    // auto format = context->CreateTextFormat(L"Segoe UI", m_fontSize);
    // auto brush = context->CreateSolidColorBrush(m_foreground);
    // context->DrawTextString(m_text, format.get(), 
    //                        rendering::Point(rect.x, rect.y), brush.get());
    
    // 简化：渲染背景表示文本区域
    // context->FillRectangle(rect, brush.get());
}

rendering::Size TextBlock::OnMeasure(const rendering::Size& availableSize) {
    if (m_textDirty) {
        UpdateTextSize();
    }
    return m_textSize;
}

void TextBlock::UpdateTextSize() {
    // 简化实现：基于字体大小估算文本大小
    // 实际应该使用文本布局引擎
    float avgCharWidth = m_fontSize * 0.6f;
    float lineHeight = m_fontSize * 1.2f;
    
    // 计算行数和每行宽度
    size_t lineCount = 1;
    size_t maxLineLength = 0;
    size_t currentLineLength = 0;
    
    for (wchar_t ch : m_text) {
        if (ch == L'\n') {
            lineCount++;
            maxLineLength = std::max(maxLineLength, currentLineLength);
            currentLineLength = 0;
        } else {
            currentLineLength++;
        }
    }
    maxLineLength = std::max(maxLineLength, currentLineLength);
    
    m_textSize.width = static_cast<float>(maxLineLength) * avgCharWidth;
    m_textSize.height = static_cast<float>(lineCount) * lineHeight;
    m_textDirty = false;
}

} // namespace controls
} // namespace luaui
