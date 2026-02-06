#pragma once

#include "luaui/rendering/ITextFormat.h"
#include <dwrite.h>
#include <wrl/client.h>

namespace luaui {
namespace rendering {

class D2DRenderContext;
using Microsoft::WRL::ComPtr;

class D2DTextFormat : public ITextFormat {
public:
    D2DTextFormat();
    ~D2DTextFormat() override;
    
    bool Initialize(D2DRenderContext* context, const std::wstring& fontFamily, float fontSize);
    
    // ITextFormat
    void SetFontFamily(const std::wstring& family) override;
    void SetFontSize(float size) override;
    void SetFontWeight(FontWeight weight) override;
    void SetFontStyle(FontStyle style) override;
    std::wstring GetFontFamily() const override { return m_fontFamily; }
    float GetFontSize() const override { return m_fontSize; }
    FontWeight GetFontWeight() const override { return m_weight; }
    FontStyle GetFontStyle() const override { return m_style; }
    
    void SetTextAlignment(TextAlignment align) override;
    void SetParagraphAlignment(ParagraphAlignment align) override;
    void SetWordWrapping(WordWrapping wrapping) override;
    void SetTextTrimming(TextTrimming trimming) override;
    TextAlignment GetTextAlignment() const override;
    ParagraphAlignment GetParagraphAlignment() const override;
    WordWrapping GetWordWrapping() const override;
    TextTrimming GetTextTrimming() const override;
    
    void SetLineSpacing(float lineHeight, float baseline) override;
    float GetLineHeight() const override { return m_lineHeight; }
    float GetBaseline() const override { return m_baseline; }
    
    void* GetNativeFormat(IRenderContext* context) override;
    IDWriteTextFormat* GetNativeFormat() const { return m_format.Get(); }
    Size MeasureText(const std::wstring& text, float maxWidth) override;
    int HitTest(const std::wstring& text, const Point& point) override;
    
private:
    void UpdateFormat();
    
    ComPtr<IDWriteFactory> m_dwriteFactory;
    ComPtr<IDWriteTextFormat> m_format;
    
    std::wstring m_fontFamily;
    float m_fontSize = 12.0f;
    FontWeight m_weight = FontWeight::Regular;
    FontStyle m_style = FontStyle::Normal;
    float m_lineHeight = 0;
    float m_baseline = 0;
};

class D2DTextLayout : public ITextLayout {
public:
    D2DTextLayout();
    ~D2DTextLayout() override;
    
    bool Initialize(D2DRenderContext* context, const std::wstring& text, 
                    ITextFormat* format, const Size& maxSize);
    
    // ITextFormat (delegated to base format)
    void SetFontFamily(const std::wstring& family) override {}
    void SetFontSize(float size) override {}
    void SetFontWeight(FontWeight weight) override {}
    void SetFontStyle(FontStyle style) override {}
    std::wstring GetFontFamily() const override { return L""; }
    float GetFontSize() const override { return 0; }
    FontWeight GetFontWeight() const override { return FontWeight::Regular; }
    FontStyle GetFontStyle() const override { return FontStyle::Normal; }
    void SetTextAlignment(TextAlignment align) override {}
    void SetParagraphAlignment(ParagraphAlignment align) override {}
    void SetWordWrapping(WordWrapping wrapping) override {}
    void SetTextTrimming(TextTrimming trimming) override {}
    TextAlignment GetTextAlignment() const override { return TextAlignment::Leading; }
    ParagraphAlignment GetParagraphAlignment() const override { return ParagraphAlignment::Near; }
    WordWrapping GetWordWrapping() const override { return WordWrapping::Wrap; }
    TextTrimming GetTextTrimming() const override { return TextTrimming::None; }
    void SetLineSpacing(float lineHeight, float baseline) override {}
    float GetLineHeight() const override { return 0; }
    float GetBaseline() const override { return 0; }
    void* GetNativeFormat(IRenderContext* context) override { return nullptr; }
    int HitTest(const std::wstring& text, const Point& point) override { return 0; }
    
    // ITextLayout
    void SetText(const std::wstring& text) override;
    void SetMaxSize(const Size& size) override;
    std::wstring GetText() const override { return m_text; }
    Size GetMaxSize() const override { return m_maxSize; }
    Size MeasureText(const std::wstring& text, float maxWidth) override;
    
    Size GetLayoutSize() const override;
    int GetLineCount() const override;
    float GetLayoutHeight() const override;
    
    IDWriteTextLayout* GetD2DLayout() const { return m_layout.Get(); }
    
private:
    void UpdateLayout();
    
    ComPtr<IDWriteFactory> m_dwriteFactory;
    ComPtr<IDWriteTextFormat> m_baseFormat;
    ComPtr<IDWriteTextLayout> m_layout;
    
    std::wstring m_text;
    Size m_maxSize;
};

} // namespace rendering
} // namespace luaui
