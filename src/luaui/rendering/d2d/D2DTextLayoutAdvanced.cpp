#include "D2DTextLayoutAdvanced.h"
#include "D2DTextFormat.h"
#include "D2DRenderContext.h"
#include "D2DHelpers.h"

namespace luaui {
namespace rendering {

D2DTextLayoutAdvanced::D2DTextLayoutAdvanced(IDWriteFactory* factory)
    : m_factory(factory) {
}

D2DTextLayoutAdvanced::~D2DTextLayoutAdvanced() = default;

void D2DTextLayoutAdvanced::SetText(const std::wstring& text) {
    if (m_text != text) {
        m_text = text;
        m_needsRebuild = true;
    }
}

void D2DTextLayoutAdvanced::SetMaxWidth(float width) {
    if (m_maxWidth != width) {
        m_maxWidth = width;
        m_needsRebuild = true;
    }
}

void D2DTextLayoutAdvanced::SetMaxHeight(float height) {
    if (m_maxHeight != height) {
        m_maxHeight = height;
        m_needsRebuild = true;
    }
}

Size D2DTextLayoutAdvanced::GetLayoutSize() const {
    if (!m_textLayout) return Size(0, 0);
    
    DWRITE_TEXT_METRICS metrics;
    if (SUCCEEDED(m_textLayout->GetMetrics(&metrics))) {
        return Size(metrics.width, metrics.height);
    }
    return Size(0, 0);
}

uint32_t D2DTextLayoutAdvanced::GetLineCount() const {
    if (!m_textLayout) return 0;
    
    DWRITE_TEXT_METRICS metrics;
    if (SUCCEEDED(m_textLayout->GetMetrics(&metrics))) {
        return metrics.lineCount;
    }
    return 0;
}

float D2DTextLayoutAdvanced::GetLineHeight(uint32_t lineIndex) const {
    if (!m_textLayout) return 0;
    
    // Get line metrics
    std::vector<DWRITE_LINE_METRICS> lineMetrics;
    uint32_t lineCount = 0;
    m_textLayout->GetLineMetrics(nullptr, 0, &lineCount);
    
    if (lineIndex >= lineCount) return 0;
    
    lineMetrics.resize(lineCount);
    if (SUCCEEDED(m_textLayout->GetLineMetrics(lineMetrics.data(), lineCount, &lineCount))) {
        return lineMetrics[lineIndex].height;
    }
    return 0;
}

void D2DTextLayoutAdvanced::SetDefaultFormat(ITextFormat* format) {
    if (!format) return;
    
    // Cast to D2D format to get native interface
    auto* d2dFormat = static_cast<D2DTextFormat*>(format);
    if (d2dFormat) {
        m_defaultFormat = d2dFormat->GetNativeFormat();
        m_needsRebuild = true;
    }
}

void D2DTextLayoutAdvanced::SetFormatting(const TextRange& range, const TextFormatting& formatting) {
    if (!m_textLayout || range.startPosition >= m_text.length()) return;
    
    DWRITE_TEXT_RANGE dwriteRange;
    dwriteRange.startPosition = range.startPosition;
    dwriteRange.length = range.length;
    
    // Apply font weight
    m_textLayout->SetFontWeight(static_cast<DWRITE_FONT_WEIGHT>(formatting.fontWeight), dwriteRange);
    
    // Apply font style
    m_textLayout->SetFontStyle(static_cast<DWRITE_FONT_STYLE>(formatting.fontStyle), dwriteRange);
    
    // Apply font size
    if (formatting.fontSize > 0) {
        m_textLayout->SetFontSize(formatting.fontSize, dwriteRange);
    }
    
    // Apply underline
    m_textLayout->SetUnderline(formatting.underline, dwriteRange);
    
    // Apply strikethrough
    m_textLayout->SetStrikethrough(formatting.strikethethrough, dwriteRange);
    
    // Apply font family if specified
    if (!formatting.fontFamily.empty()) {
        m_textLayout->SetFontFamilyName(formatting.fontFamily.c_str(), dwriteRange);
    }
}

void D2DTextLayoutAdvanced::ClearFormatting(const TextRange& range) {
    if (!m_textLayout) return;
    
    DWRITE_TEXT_RANGE dwriteRange;
    dwriteRange.startPosition = range.startPosition;
    dwriteRange.length = range.length;
    
    // Reset to default values
    m_textLayout->SetFontWeight(DWRITE_FONT_WEIGHT_NORMAL, dwriteRange);
    m_textLayout->SetFontStyle(DWRITE_FONT_STYLE_NORMAL, dwriteRange);
    m_textLayout->SetUnderline(FALSE, dwriteRange);
    m_textLayout->SetStrikethrough(FALSE, dwriteRange);
}

void D2DTextLayoutAdvanced::SetInlineObject(uint32_t position, std::shared_ptr<IInlineObject> object) {
    m_inlineObjects[position] = object;
}

void D2DTextLayoutAdvanced::RemoveInlineObject(uint32_t position) {
    m_inlineObjects.erase(position);
}

HitTestMetrics D2DTextLayoutAdvanced::HitTestPoint(const Point& point) const {
    HitTestMetrics result = {};
    if (!m_textLayout) return result;
    
    BOOL isTrailingHit = FALSE;
    BOOL isInside = FALSE;
    DWRITE_HIT_TEST_METRICS metrics;
    
    HRESULT hr = m_textLayout->HitTestPoint(point.x, point.y, &isTrailingHit, &isInside, &metrics);
    
    if (SUCCEEDED(hr)) {
        result.textPosition = metrics.textPosition;
        result.length = metrics.length;
        result.topLeft = Point(metrics.left, metrics.top);
        result.width = metrics.width;
        result.height = metrics.height;
        result.lineIndex = 0; // Would need to calculate from line metrics
    }
    
    return result;
}

Point D2DTextLayoutAdvanced::HitTestTextPosition(uint32_t textPosition) const {
    if (!m_textLayout) return Point(0, 0);
    
    float pointX = 0;
    float pointY = 0;
    DWRITE_HIT_TEST_METRICS metrics;
    
    HRESULT hr = m_textLayout->HitTestTextPosition(
        textPosition, FALSE, &pointX, &pointY, &metrics
    );
    
    if (SUCCEEDED(hr)) {
        return Point(pointX, pointY);
    }
    return Point(0, 0);
}

std::vector<HitTestMetrics> D2DTextLayoutAdvanced::HitTestTextRange(const TextRange& range) const {
    std::vector<HitTestMetrics> results;
    if (!m_textLayout) return results;
    
    uint32_t maxHitTestMetrics = range.length;
    std::vector<DWRITE_HIT_TEST_METRICS> dwriteMetrics(maxHitTestMetrics);
    uint32_t actualCount = 0;
    
    HRESULT hr = m_textLayout->HitTestTextRange(
        range.startPosition,
        range.length,
        0, // originX
        0, // originY
        dwriteMetrics.data(),
        maxHitTestMetrics,
        &actualCount
    );
    
    if (SUCCEEDED(hr)) {
        results.reserve(actualCount);
        for (uint32_t i = 0; i < actualCount; i++) {
            HitTestMetrics m;
            m.textPosition = dwriteMetrics[i].textPosition;
            m.length = dwriteMetrics[i].length;
            m.topLeft = Point(dwriteMetrics[i].left, dwriteMetrics[i].top);
            m.width = dwriteMetrics[i].width;
            m.height = dwriteMetrics[i].height;
            m.lineIndex = 0;
            results.push_back(m);
        }
    }
    
    return results;
}

bool D2DTextLayoutAdvanced::GetCaretPosition(uint32_t textPosition, bool trailing, 
                                               Point* caretPosition, float* caretHeight) const {
    if (!m_textLayout || !caretPosition || !caretHeight) return false;
    
    float x = 0;
    float y = 0;
    DWRITE_HIT_TEST_METRICS metrics;
    
    HRESULT hr = m_textLayout->HitTestTextPosition(
        textPosition, trailing ? TRUE : FALSE, &x, &y, &metrics
    );
    
    if (SUCCEEDED(hr)) {
        *caretPosition = Point(x, y);
        *caretHeight = metrics.height;
        return true;
    }
    return false;
}

uint32_t D2DTextLayoutAdvanced::GetTextPositionAtPoint(const Point& point) const {
    HitTestMetrics metrics = HitTestPoint(point);
    return metrics.textPosition;
}

void D2DTextLayoutAdvanced::Draw(IRenderContext* context, const Point& origin) {
    if (!m_textLayout) {
        Rebuild();
    }
    if (!m_textLayout) return;
    
    // Get D2D render context - use dynamic_cast for proper polymorphism
    D2DRenderContext* d2dContext = dynamic_cast<D2DRenderContext*>(context);
    if (!d2dContext) return;
    
    ID2D1RenderTarget* target = d2dContext->GetRenderTarget();
    if (!target) return;
    
    // Create default brush for text (black)
    ComPtr<ID2D1SolidColorBrush> brush;
    target->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::Black),
        &brush
    );
    
    // Draw the text layout
    target->DrawTextLayout(
        D2D1::Point2F(origin.x, origin.y),
        m_textLayout.Get(),
        brush.Get()
    );
}

void D2DTextLayoutAdvanced::DrawSelection(IRenderContext* context, const Point& origin, 
                                           const std::vector<TextRange>& selections, 
                                           const Color& selectionColor) {
    if (!m_textLayout || selections.empty()) return;
    
    // Get D2D render context - use dynamic_cast
    D2DRenderContext* d2dContext = dynamic_cast<D2DRenderContext*>(context);
    if (!d2dContext) return;
    
    ID2D1RenderTarget* target = d2dContext->GetRenderTarget();
    if (!target) return;
    
    // Create selection brush
    ComPtr<ID2D1SolidColorBrush> selectionBrush;
    target->CreateSolidColorBrush(ToD2DColor(selectionColor), &selectionBrush);
    
    // Draw each selection range
    for (const auto& range : selections) {
        auto metrics = HitTestTextRange(range);
        for (const auto& m : metrics) {
            D2D1_RECT_F rect = D2D1::RectF(
                origin.x + m.topLeft.x,
                origin.y + m.topLeft.y,
                origin.x + m.topLeft.x + m.width,
                origin.y + m.topLeft.y + m.height
            );
            target->FillRectangle(rect, selectionBrush.Get());
        }
    }
    
    // Draw text on top
    Draw(context, origin);
}

bool D2DTextLayoutAdvanced::Rebuild() {
    if (!m_factory || !m_defaultFormat) return false;
    
    m_textLayout.Reset();
    m_needsRebuild = false;
    
    if (m_text.empty()) return true;
    
    HRESULT hr = m_factory->CreateTextLayout(
        m_text.c_str(),
        static_cast<uint32_t>(m_text.length()),
        m_defaultFormat.Get(),
        m_maxWidth > 0 ? m_maxWidth : FLT_MAX,
        m_maxHeight > 0 ? m_maxHeight : FLT_MAX,
        &m_textLayout
    );
    
    return SUCCEEDED(hr);
}

} // namespace rendering
} // namespace luaui
