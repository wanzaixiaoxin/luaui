#pragma once

#include "Types.h"
#include <string>
#include <memory>

namespace luaui {
namespace rendering {

class IRenderContext;

// Font weight
enum class FontWeight {
    Thin = 100,
    ExtraLight = 200,
    Light = 300,
    Regular = 400,
    Medium = 500,
    SemiBold = 600,
    Bold = 700,
    ExtraBold = 800,
    Black = 900
};

// Font style
enum class FontStyle {
    Normal,
    Italic,
    Oblique
};

// Text alignment
enum class TextAlignment {
    Leading,    // Left for LTR, Right for RTL
    Trailing,   // Right for LTR, Left for RTL
    Center,
    Justified
};

// Paragraph alignment
enum class ParagraphAlignment {
    Near,       // Top
    Far,        // Bottom
    Center
};

// Word wrapping
enum class WordWrapping {
    NoWrap,             // Single line
    Wrap,               // Wrap at word boundaries
    WrapWithOverflow,   // Wrap at character boundaries if needed
    EmergencyBreak      // Wrap at character boundaries
};

// Text trimming
enum class TextTrimming {
    None,
    Character,
    Word,
    Clip
};

// Text format interface
class ITextFormat {
public:
    virtual ~ITextFormat() = default;
    
    // Font properties
    virtual void SetFontFamily(const std::wstring& family) = 0;
    virtual void SetFontSize(float size) = 0;
    virtual void SetFontWeight(FontWeight weight) = 0;
    virtual void SetFontStyle(FontStyle style) = 0;
    
    virtual std::wstring GetFontFamily() const = 0;
    virtual float GetFontSize() const = 0;
    virtual FontWeight GetFontWeight() const = 0;
    virtual FontStyle GetFontStyle() const = 0;
    
    // Layout properties
    virtual void SetTextAlignment(TextAlignment align) = 0;
    virtual void SetParagraphAlignment(ParagraphAlignment align) = 0;
    virtual void SetWordWrapping(WordWrapping wrapping) = 0;
    virtual void SetTextTrimming(TextTrimming trimming) = 0;
    
    virtual TextAlignment GetTextAlignment() const = 0;
    virtual ParagraphAlignment GetParagraphAlignment() const = 0;
    virtual WordWrapping GetWordWrapping() const = 0;
    virtual TextTrimming GetTextTrimming() const = 0;
    
    // Line spacing
    virtual void SetLineSpacing(float lineHeight, float baseline) = 0;
    virtual float GetLineHeight() const = 0;
    virtual float GetBaseline() const = 0;
    
    // Native access
    virtual void* GetNativeFormat(IRenderContext* context) = 0;
    
    // Measurement
    virtual Size MeasureText(const std::wstring& text, float maxWidth = 0) = 0;
    virtual int HitTest(const std::wstring& text, const Point& point) = 0;
};

using ITextFormatPtr = std::shared_ptr<ITextFormat>;

// Text layout for complex scenarios
class ITextLayout : public ITextFormat {
public:
    virtual void SetText(const std::wstring& text) = 0;
    virtual void SetMaxSize(const Size& size) = 0;
    virtual std::wstring GetText() const = 0;
    virtual Size GetMaxSize() const = 0;
    
    // Layout metrics
    virtual Size GetLayoutSize() const = 0;
    virtual int GetLineCount() const = 0;
    virtual float GetLayoutHeight() const = 0;
};

using ITextLayoutPtr = std::shared_ptr<ITextLayout>;

} // namespace rendering
} // namespace luaui
