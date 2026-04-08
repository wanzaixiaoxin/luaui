#pragma once

#include "Control.h"
#include "../core/Components/LayoutComponent.h"
#include "../core/Components/RenderComponent.h"
#include "../core/Components/InputComponent.h"
#include "../rendering/Types.h"
#include <string>

namespace luaui {
namespace controls {

/**
 * @brief Button 控件（新架构）
 * 
 * 使用组件模式实现：
 * - LayoutComponent: 测量和排列
 * - RenderComponent: 渲染外观
 * - InputComponent: 处理点击和悬停
 */
class Button : public luaui::Control {
public:
    Button();
    
    std::string GetTypeName() const override { return "Button"; }
    
    // 文本
    std::wstring GetText() const { return m_text; }
    void SetText(const std::wstring& text);
    
    // 状态查询
    bool GetIsPressed() const { return m_isPressed; }
    bool GetIsHovered() const { return m_isHovered; }

    // 前景色（文字颜色）
    rendering::Color GetForeground() const { return m_foreground; }
    void SetForeground(const rendering::Color& color);

    // 边框
    rendering::Color GetBorderBrush() const { return m_borderBrush; }
    void SetBorderBrush(const rendering::Color& color);
    float GetBorderThickness() const { return m_borderThickness; }
    void SetBorderThickness(float thickness);

    // 圆角
    rendering::CornerRadius GetCornerRadius() const { return m_cornerRadius; }
    void SetCornerRadius(const rendering::CornerRadius& radius);

    // 内边距
    rendering::Thickness GetPadding() const { return m_padding; }
    void SetPadding(const rendering::Thickness& padding);

    // 字体
    float GetFontSize() const { return m_fontSize; }
    void SetFontSize(float size);
    std::wstring GetFontFamily() const { return m_fontFamily; }
    void SetFontFamily(const std::wstring& family);

    // 外观设置
    void SetStateColors(const rendering::Color& normal,
                        const rendering::Color& hover,
                        const rendering::Color& pressed);

    // 禁用态颜色
    void SetDisabledColors(const rendering::Color& bg,
                           const rendering::Color& fg,
                           const rendering::Color& border);

protected:
    void InitializeComponents() override;
    
    // 渲染覆盖
    virtual void OnRender(rendering::IRenderContext* context);
    
    // 输入处理
    virtual void OnMouseDown(MouseEventArgs& args);
    virtual void OnMouseUp(MouseEventArgs& args);
    virtual void OnMouseEnter();
    virtual void OnMouseLeave();
    virtual void OnClick();
    
    // 布局覆盖
    virtual rendering::Size OnMeasure(const rendering::Size& availableSize);

private:
    bool m_isHovered = false;
    bool m_isPressed = false;
    std::wstring m_text;

    // 外观属性
    rendering::Color m_foreground = rendering::Color::Black();
    rendering::Color m_borderBrush = rendering::Color::Transparent();
    float m_borderThickness = 1.0f;
    rendering::CornerRadius m_cornerRadius = rendering::CornerRadius(4.0f);
    rendering::Thickness m_padding = rendering::Thickness(12, 6);

    // 字体属性
    float m_fontSize = 14.0f;
    std::wstring m_fontFamily = L"Microsoft YaHei";

    // 状态颜色
    rendering::Color m_normalBackground = rendering::Color::FromHex(0xE0E0E0);
    rendering::Color m_hoverBackground = rendering::Color::FromHex(0xD0D0D0);
    rendering::Color m_pressedBackground = rendering::Color::FromHex(0xC0C0C0);

    // 禁用态颜色
    rendering::Color m_disabledBackground = rendering::Color::FromHex(0xF5F5F5);
    rendering::Color m_disabledForeground = rendering::Color::FromHex(0xBDBDBD);
    rendering::Color m_disabledBorderBrush = rendering::Color::FromHex(0xE0E0E0);
};

} // namespace controls
} // namespace luaui
