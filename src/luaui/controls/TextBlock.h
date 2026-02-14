#pragma once

#include "Control.h"
#include "../core/Components/LayoutComponent.h"
#include "../core/Components/RenderComponent.h"
#include "../rendering/Types.h"
#include "../rendering/ITextFormat.h"
#include <string>

namespace luaui {
namespace controls {

/**
 * @brief TextBlock 文本显示控件（新架构）
 * 
 * 只负责显示文本，不处理输入
 */
class TextBlock : public luaui::Control {
public:
    TextBlock();
    
    std::string GetTypeName() const override { return "TextBlock"; }
    
    // 文本
    std::wstring GetText() const { return m_text; }
    void SetText(const std::wstring& text);
    
    // 字体大小
    float GetFontSize() const { return m_fontSize; }
    void SetFontSize(float size);
    
    // 前景色（文本颜色）
    rendering::Color GetForeground() const { return m_foreground; }
    void SetForeground(const rendering::Color& color);
    
    // 字体粗细
    rendering::FontWeight GetFontWeight() const { return m_fontWeight; }
    void SetFontWeight(rendering::FontWeight weight);
    
    // 字体样式
    rendering::FontStyle GetFontStyle() const { return m_fontStyle; }
    void SetFontStyle(rendering::FontStyle style);

protected:
    void InitializeComponents() override;
    
    // 渲染覆盖
    virtual void OnRender(rendering::IRenderContext* context);
    
    // 布局覆盖
    virtual rendering::Size OnMeasure(const rendering::Size& availableSize);

private:
    std::wstring m_text;
    float m_fontSize = 14.0f;
    rendering::Color m_foreground = rendering::Color::Black();
    rendering::FontWeight m_fontWeight = rendering::FontWeight::Regular;
    rendering::FontStyle m_fontStyle = rendering::FontStyle::Normal;
    
    bool m_textDirty = true;
    rendering::Size m_textSize;
    
    void UpdateTextSize();
};

} // namespace controls
} // namespace luaui
