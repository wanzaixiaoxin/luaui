#pragma once

#include "Control.h"
#include <string>
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include "Components/InputComponent.h"

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
class Button : public Control {
public:
    Button();
    
    std::string GetTypeName() const override { return "Button"; }
    
    // 状态查询
    bool GetIsPressed() const { return m_isPressed; }
    bool GetIsHovered() const { return m_isHovered; }
    
    // 外观设置
    void SetStateColors(const rendering::Color& normal, 
                        const rendering::Color& hover, 
                        const rendering::Color& pressed);

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
    
    rendering::Color m_normalBackground = rendering::Color::FromHex(0xE0E0E0);
    rendering::Color m_hoverBackground = rendering::Color::FromHex(0xD0D0D0);
    rendering::Color m_pressedBackground = rendering::Color::FromHex(0xC0C0C0);
};

} // namespace controls
} // namespace luaui
