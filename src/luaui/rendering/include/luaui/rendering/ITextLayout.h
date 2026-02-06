#pragma once

#include "Types.h"
#include "ITextFormat.h"
#include <string>
#include <vector>
#include <memory>

namespace luaui {
namespace rendering {

// Text range for formatting
struct TextRange {
    uint32_t startPosition = 0;
    uint32_t length = 0;
    
    TextRange() = default;
    TextRange(uint32_t start, uint32_t len) : startPosition(start), length(len) {}
    
    bool Contains(uint32_t position) const {
        return position >= startPosition && position < startPosition + length;
    }
};

// Text formatting attributes
struct TextFormatting {
    Color foregroundColor = Color::Black();
    Color backgroundColor = Color::Transparent();
    FontWeight fontWeight = FontWeight::Regular;
    FontStyle fontStyle = FontStyle::Normal;
    float fontSize = 0; // 0 means inherit from base format
    std::wstring fontFamily;
    bool underline = false;
    bool strikethethrough = false;
};

// Inline object (images, etc.)
class IInlineObject {
public:
    virtual ~IInlineObject() = default;
    virtual Size GetSize() const = 0;
    virtual void Draw(IRenderContext* context, const Point& position) = 0;
};

// Hit test metrics
struct HitTestMetrics {
    uint32_t textPosition = 0;
    uint32_t length = 0;
    Point topLeft;
    float width = 0;
    float height = 0;
    uint32_t lineIndex = 0;
};

// Text layout interface (advanced)
class ITextLayoutAdvanced {
public:
    virtual ~ITextLayoutAdvanced() = default;
    
    // Text content
    virtual void SetText(const std::wstring& text) = 0;
    virtual std::wstring GetText() const = 0;
    
    // Layout constraints
    virtual void SetMaxWidth(float width) = 0;
    virtual void SetMaxHeight(float height) = 0;
    virtual float GetMaxWidth() const = 0;
    virtual float GetMaxHeight() const = 0;
    
    // Metrics
    virtual Size GetLayoutSize() const = 0;
    virtual uint32_t GetLineCount() const = 0;
    virtual float GetLineHeight(uint32_t lineIndex) const = 0;
    
    // Rich text formatting
    virtual void SetDefaultFormat(ITextFormat* format) = 0;
    virtual void SetFormatting(const TextRange& range, const TextFormatting& formatting) = 0;
    virtual void ClearFormatting(const TextRange& range) = 0;
    
    // Inline objects
    virtual void SetInlineObject(uint32_t position, std::shared_ptr<IInlineObject> object) = 0;
    virtual void RemoveInlineObject(uint32_t position) = 0;
    
    // Hit testing
    virtual HitTestMetrics HitTestPoint(const Point& point) const = 0;
    virtual Point HitTestTextPosition(uint32_t textPosition) const = 0;
    virtual std::vector<HitTestMetrics> HitTestTextRange(const TextRange& range) const = 0;
    
    // Caret/selection
    virtual bool GetCaretPosition(uint32_t textPosition, bool trailing, Point* caretPosition, float* caretHeight) const = 0;
    virtual uint32_t GetTextPositionAtPoint(const Point& point) const = 0;
    
    // Drawing
    virtual void Draw(IRenderContext* context, const Point& origin) = 0;
    virtual void DrawSelection(IRenderContext* context, const Point& origin, 
                                const std::vector<TextRange>& selections, 
                                const Color& selectionColor) = 0;
    
    // Native access
    virtual void* GetNativeLayout() const = 0;
};

using ITextLayoutAdvancedPtr = std::unique_ptr<ITextLayoutAdvanced>;

// Factory function
ITextLayoutAdvancedPtr CreateTextLayoutAdvanced(IRenderContext* context);

} // namespace rendering
} // namespace luaui
