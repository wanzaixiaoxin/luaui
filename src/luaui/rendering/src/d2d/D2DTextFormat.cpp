#include "D2DTextFormat.h"
#include "D2DRenderContext.h"

namespace luaui {
namespace rendering {

// D2DTextFormat
D2DTextFormat::D2DTextFormat() = default;
D2DTextFormat::~D2DTextFormat() = default;

bool D2DTextFormat::Initialize(D2DRenderContext* context, const std::wstring& fontFamily, 
                                float fontSize) {
    m_dwriteFactory = context->GetDWriteFactory();
    if (!m_dwriteFactory) return false;
    
    m_fontFamily = fontFamily;
    m_fontSize = fontSize;
    
    UpdateFormat();
    return m_format != nullptr;
}

void D2DTextFormat::UpdateFormat() {
    if (!m_dwriteFactory) return;
    
    IDWriteTextFormat* format = nullptr;
    HRESULT hr = m_dwriteFactory->CreateTextFormat(
        m_fontFamily.c_str(),
        nullptr,
        static_cast<DWRITE_FONT_WEIGHT>(m_weight),
        m_style == FontStyle::Italic ? DWRITE_FONT_STYLE_ITALIC : 
            (m_style == FontStyle::Oblique ? DWRITE_FONT_STYLE_OBLIQUE : DWRITE_FONT_STYLE_NORMAL),
        DWRITE_FONT_STRETCH_NORMAL,
        m_fontSize,
        L"",
        &format
    );
    
    if (SUCCEEDED(hr) && format) {
        m_format = format;
        format->Release();
    }
}

void D2DTextFormat::SetFontFamily(const std::wstring& family) {
    m_fontFamily = family;
    UpdateFormat();
}

void D2DTextFormat::SetFontSize(float size) {
    m_fontSize = size;
    UpdateFormat();
}

void D2DTextFormat::SetFontWeight(FontWeight weight) {
    m_weight = weight;
    // Note: Font weight cannot be changed after creation in DirectWrite
    // We need to recreate the format
    UpdateFormat();
}

void D2DTextFormat::SetFontStyle(FontStyle style) {
    m_style = style;
    UpdateFormat();
}

void D2DTextFormat::SetTextAlignment(TextAlignment align) {
    if (!m_format) return;
    
    DWRITE_TEXT_ALIGNMENT d2dAlign = DWRITE_TEXT_ALIGNMENT_LEADING;
    switch (align) {
        case TextAlignment::Leading: d2dAlign = DWRITE_TEXT_ALIGNMENT_LEADING; break;
        case TextAlignment::Trailing: d2dAlign = DWRITE_TEXT_ALIGNMENT_TRAILING; break;
        case TextAlignment::Center: d2dAlign = DWRITE_TEXT_ALIGNMENT_CENTER; break;
        case TextAlignment::Justified: d2dAlign = DWRITE_TEXT_ALIGNMENT_JUSTIFIED; break;
    }
    m_format->SetTextAlignment(d2dAlign);
}

void D2DTextFormat::SetParagraphAlignment(ParagraphAlignment align) {
    if (!m_format) return;
    
    DWRITE_PARAGRAPH_ALIGNMENT d2dAlign = DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
    switch (align) {
        case ParagraphAlignment::Near: d2dAlign = DWRITE_PARAGRAPH_ALIGNMENT_NEAR; break;
        case ParagraphAlignment::Far: d2dAlign = DWRITE_PARAGRAPH_ALIGNMENT_FAR; break;
        case ParagraphAlignment::Center: d2dAlign = DWRITE_PARAGRAPH_ALIGNMENT_CENTER; break;
    }
    m_format->SetParagraphAlignment(d2dAlign);
}

void D2DTextFormat::SetWordWrapping(WordWrapping wrapping) {
    if (!m_format) return;
    
    DWRITE_WORD_WRAPPING d2dWrap = DWRITE_WORD_WRAPPING_WRAP;
    switch (wrapping) {
        case WordWrapping::NoWrap: d2dWrap = DWRITE_WORD_WRAPPING_NO_WRAP; break;
        case WordWrapping::Wrap: d2dWrap = DWRITE_WORD_WRAPPING_WRAP; break;
        case WordWrapping::WrapWithOverflow: d2dWrap = DWRITE_WORD_WRAPPING_EMERGENCY_BREAK; break;
        case WordWrapping::EmergencyBreak: d2dWrap = DWRITE_WORD_WRAPPING_EMERGENCY_BREAK; break;
    }
    m_format->SetWordWrapping(d2dWrap);
}

void D2DTextFormat::SetTextTrimming(TextTrimming trimming) {
    if (!m_format) return;
    
    DWRITE_TRIMMING d2dTrim = {};
    switch (trimming) {
        case TextTrimming::None: d2dTrim.granularity = DWRITE_TRIMMING_GRANULARITY_NONE; break;
        case TextTrimming::Character: d2dTrim.granularity = DWRITE_TRIMMING_GRANULARITY_CHARACTER; break;
        case TextTrimming::Word: d2dTrim.granularity = DWRITE_TRIMMING_GRANULARITY_WORD; break;
    }
    m_format->SetTrimming(&d2dTrim, nullptr);
}

TextAlignment D2DTextFormat::GetTextAlignment() const {
    if (!m_format) return TextAlignment::Leading;
    
    switch (m_format->GetTextAlignment()) {
        case DWRITE_TEXT_ALIGNMENT_LEADING: return TextAlignment::Leading;
        case DWRITE_TEXT_ALIGNMENT_TRAILING: return TextAlignment::Trailing;
        case DWRITE_TEXT_ALIGNMENT_CENTER: return TextAlignment::Center;
        case DWRITE_TEXT_ALIGNMENT_JUSTIFIED: return TextAlignment::Justified;
        default: return TextAlignment::Leading;
    }
}

ParagraphAlignment D2DTextFormat::GetParagraphAlignment() const {
    if (!m_format) return ParagraphAlignment::Near;
    
    switch (m_format->GetParagraphAlignment()) {
        case DWRITE_PARAGRAPH_ALIGNMENT_NEAR: return ParagraphAlignment::Near;
        case DWRITE_PARAGRAPH_ALIGNMENT_FAR: return ParagraphAlignment::Far;
        case DWRITE_PARAGRAPH_ALIGNMENT_CENTER: return ParagraphAlignment::Center;
        default: return ParagraphAlignment::Near;
    }
}

WordWrapping D2DTextFormat::GetWordWrapping() const {
    if (!m_format) return WordWrapping::Wrap;
    
    switch (m_format->GetWordWrapping()) {
        case DWRITE_WORD_WRAPPING_NO_WRAP: return WordWrapping::NoWrap;
        case DWRITE_WORD_WRAPPING_WRAP: return WordWrapping::Wrap;
        case DWRITE_WORD_WRAPPING_EMERGENCY_BREAK: return WordWrapping::EmergencyBreak;
        default: return WordWrapping::Wrap;
    }
}

TextTrimming D2DTextFormat::GetTextTrimming() const {
    if (!m_format) return TextTrimming::None;
    
    DWRITE_TRIMMING trimming;
    m_format->GetTrimming(&trimming, nullptr);
    
    switch (trimming.granularity) {
        case DWRITE_TRIMMING_GRANULARITY_NONE: return TextTrimming::None;
        case DWRITE_TRIMMING_GRANULARITY_CHARACTER: return TextTrimming::Character;
        case DWRITE_TRIMMING_GRANULARITY_WORD: return TextTrimming::Word;
        default: return TextTrimming::None;
    }
}

void D2DTextFormat::SetLineSpacing(float lineHeight, float baseline) {
    m_lineHeight = lineHeight;
    m_baseline = baseline;
    if (m_format) {
        m_format->SetLineSpacing(DWRITE_LINE_SPACING_METHOD_UNIFORM, lineHeight, baseline);
    }
}

void* D2DTextFormat::GetNativeFormat(IRenderContext* context) {
    return m_format.Get();
}

Size D2DTextFormat::MeasureText(const std::wstring& text, float maxWidth) {
    if (!m_dwriteFactory || !m_format) return Size();
    
    IDWriteTextLayout* layout = nullptr;
    HRESULT hr = m_dwriteFactory->CreateTextLayout(
        text.c_str(),
        static_cast<UINT32>(text.length()),
        m_format.Get(),
        maxWidth > 0 ? maxWidth : 10000,
        10000,
        &layout
    );
    
    if (SUCCEEDED(hr) && layout) {
        DWRITE_TEXT_METRICS metrics;
        layout->GetMetrics(&metrics);
        layout->Release();
        return Size(metrics.width, metrics.height);
    }
    return Size();
}

int D2DTextFormat::HitTest(const std::wstring& text, const Point& point) {
    // Not implemented
    return -1;
}

// D2DTextLayout
D2DTextLayout::D2DTextLayout() = default;
D2DTextLayout::~D2DTextLayout() = default;

bool D2DTextLayout::Initialize(D2DRenderContext* context, const std::wstring& text,
                                ITextFormat* format, const Size& maxSize) {
    m_dwriteFactory = context->GetDWriteFactory();
    if (!m_dwriteFactory) return false;
    
    m_text = text;
    m_maxSize = maxSize;
    
    if (format) {
        m_baseFormat.Attach(static_cast<IDWriteTextFormat*>(format->GetNativeFormat(context)));
    }
    
    UpdateLayout();
    return m_layout != nullptr;
}

void D2DTextLayout::UpdateLayout() {
    if (!m_dwriteFactory || !m_baseFormat) return;
    
    IDWriteTextLayout* layout = nullptr;
    HRESULT hr = m_dwriteFactory->CreateTextLayout(
        m_text.c_str(),
        static_cast<UINT32>(m_text.length()),
        m_baseFormat.Get(),
        m_maxSize.width,
        m_maxSize.height,
        &layout
    );
    
    if (SUCCEEDED(hr) && layout) {
        m_layout = layout;
        layout->Release();
    }
}

void D2DTextLayout::SetText(const std::wstring& text) {
    m_text = text;
    UpdateLayout();
}

void D2DTextLayout::SetMaxSize(const Size& size) {
    m_maxSize = size;
    UpdateLayout();
}

Size D2DTextLayout::MeasureText(const std::wstring& text, float maxWidth) {
    return GetLayoutSize();
}

Size D2DTextLayout::GetLayoutSize() const {
    if (!m_layout) return Size();
    
    DWRITE_TEXT_METRICS metrics;
    m_layout->GetMetrics(&metrics);
    return Size(metrics.widthIncludingTrailingWhitespace, metrics.height);
}

int D2DTextLayout::GetLineCount() const {
    if (!m_layout) return 0;
    
    DWRITE_TEXT_METRICS metrics;
    m_layout->GetMetrics(&metrics);
    return static_cast<int>(metrics.lineCount);
}

float D2DTextLayout::GetLayoutHeight() const {
    if (!m_layout) return 0;
    
    DWRITE_TEXT_METRICS metrics;
    m_layout->GetMetrics(&metrics);
    return metrics.height;
}

} // namespace rendering
} // namespace luaui
