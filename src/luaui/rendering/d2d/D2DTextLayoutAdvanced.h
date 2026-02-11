#pragma once

#include "ITextLayout.h"
#include <dwrite.h>
#include <wrl/client.h>
#include <map>

namespace luaui {
namespace rendering {

using Microsoft::WRL::ComPtr;

// DirectWrite text layout implementation
class D2DTextLayoutAdvanced : public ITextLayoutAdvanced {
public:
    explicit D2DTextLayoutAdvanced(IDWriteFactory* factory);
    ~D2DTextLayoutAdvanced() override;
    
    // ITextLayoutAdvanced implementation
    void SetText(const std::wstring& text) override;
    std::wstring GetText() const override { return m_text; }
    
    void SetMaxWidth(float width) override;
    void SetMaxHeight(float height) override;
    float GetMaxWidth() const override { return m_maxWidth; }
    float GetMaxHeight() const override { return m_maxHeight; }
    
    Size GetLayoutSize() const override;
    uint32_t GetLineCount() const override;
    float GetLineHeight(uint32_t lineIndex) const override;
    
    void SetDefaultFormat(ITextFormat* format) override;
    void SetFormatting(const TextRange& range, const TextFormatting& formatting) override;
    void ClearFormatting(const TextRange& range) override;
    
    void SetInlineObject(uint32_t position, std::shared_ptr<IInlineObject> object) override;
    void RemoveInlineObject(uint32_t position) override;
    
    HitTestMetrics HitTestPoint(const Point& point) const override;
    Point HitTestTextPosition(uint32_t textPosition) const override;
    std::vector<HitTestMetrics> HitTestTextRange(const TextRange& range) const override;
    
    bool GetCaretPosition(uint32_t textPosition, bool trailing, Point* caretPosition, float* caretHeight) const override;
    uint32_t GetTextPositionAtPoint(const Point& point) const override;
    
    void Draw(IRenderContext* context, const Point& origin) override;
    void DrawSelection(IRenderContext* context, const Point& origin, 
                        const std::vector<TextRange>& selections, 
                        const Color& selectionColor) override;
    
    void* GetNativeLayout() const override { return m_textLayout.Get(); }
    
    // Rebuild layout (call after changing text or format)
    bool Rebuild();
    
private:
    IDWriteFactory* m_factory = nullptr;
    ComPtr<IDWriteTextLayout> m_textLayout;
    ComPtr<IDWriteTextFormat> m_defaultFormat;
    
    std::wstring m_text;
    float m_maxWidth = 0;
    float m_maxHeight = 0;
    
    struct FormatOverride {
        TextFormatting formatting;
        ComPtr<IDWriteTextLayout> layout;
    };
    std::map<uint32_t, std::shared_ptr<IInlineObject>> m_inlineObjects;
    
    bool m_needsRebuild = true;
};

} // namespace rendering
} // namespace luaui
