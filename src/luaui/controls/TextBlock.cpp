#include "TextBlock.h"
#include "IRenderContext.h"
#include "ITextFormat.h"
#include "ITextLayout.h"

namespace luaui {
namespace controls {

TextBlock::TextBlock() {}

void TextBlock::InitializeComponents() {
    // 添加布局组件 - TextBlock 根据内容调整大小
    GetComponents().AddComponent<components::LayoutComponent>(this);
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
    // 比较颜色分量
    if (m_foreground.r != color.r || m_foreground.g != color.g || 
        m_foreground.b != color.b || m_foreground.a != color.a) {
        m_foreground = color;
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void TextBlock::OnRender(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    rendering::Rect localRect(0, 0, render->GetRenderRect().width, render->GetRenderRect().height);
    
    if (m_text.empty()) return;
    
    // 创建文本格式和画刷
    auto format = context->CreateTextFormat(L"Microsoft YaHei", m_fontSize);
    auto brush = context->CreateSolidColorBrush(m_foreground);
    
    if (format && brush) {
        // 使用本地坐标绘制文本
        context->DrawTextString(m_text, format.get(), 
                                rendering::Point(0, 0), brush.get());
    }
}

rendering::Size TextBlock::OnMeasure(const rendering::Size& availableSize) {
    if (!m_textDirty) {
        return m_textSize;
    }
    
    // 简化测量：基于字符数估算（中文约1em宽，英文约0.5em宽）
    // 实际项目应使用文本布局引擎，但这需要访问渲染上下文
    float lineHeight = m_fontSize * 1.2f;
    size_t lineCount = 1;
    float maxWidth = 0;
    float currentLineWidth = 0;
    
    for (wchar_t ch : m_text) {
        if (ch == L'\n') {
            lineCount++;
            maxWidth = std::max(maxWidth, currentLineWidth);
            currentLineWidth = 0;
        } else if (ch >= 0x4E00 && ch <= 0x9FFF) {
            // 中文字符
            currentLineWidth += m_fontSize;
        } else {
            // 其他字符（英文字符等）
            currentLineWidth += m_fontSize * 0.6f;
        }
    }
    maxWidth = std::max(maxWidth, currentLineWidth);
    
    // 空文本时给一个默认高度
    if (m_text.empty()) {
        maxWidth = 0;
        lineCount = 1;
    }
    
    m_textSize.width = maxWidth;
    m_textSize.height = static_cast<float>(lineCount) * lineHeight;
    m_textDirty = false;
    
    return m_textSize;
}

void TextBlock::UpdateTextSize() {
    // 在 OnMeasure 中处理
}

} // namespace controls
} // namespace luaui
