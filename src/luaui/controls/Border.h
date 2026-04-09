#pragma once

#include "Panel.h"
#include "../core/Interfaces/IControl.h"
#include "../rendering/Types.h"
#include <string>
#include <memory>

namespace luaui {
namespace controls {

/**
 * @brief Border 带边框容器（新架构）
 * 
 * 可以包含一个子控件，并显示边框和背景
 */
class Border : public Panel {
public:
    Border();
    
    std::string GetTypeName() const override { return "Border"; }
    
    // 子内容
    std::shared_ptr<interfaces::IControl> GetChild() const;
    void SetChild(const std::shared_ptr<interfaces::IControl>& child);
    
    // 边框厚度
    float GetBorderThickness() const { return m_borderThickness; }
    void SetBorderThickness(float thickness);
    
    // 边框颜色
    rendering::Color GetBorderColor() const { return m_borderColor; }
    void SetBorderColor(const rendering::Color& color);
    
    // 背景色（使用 RenderComponent 的 Background）
    rendering::Color GetBackground() const;
    void SetBackground(const rendering::Color& color);

protected:
    void InitializeComponents() override;
    void ApplyTheme() override;
    
    // 渲染覆盖
    virtual void OnRender(rendering::IRenderContext* context);
    
    // 布局覆盖
    rendering::Size OnMeasureChildren(const rendering::Size& availableSize) override;
    rendering::Size OnArrangeChildren(const rendering::Size& finalSize) override;

private:
    float m_borderThickness = 1.0f;
    rendering::Color m_borderColor = rendering::Color::FromHex(0x808080);
    rendering::Color m_background = rendering::Color::Transparent();
    
    // 标记背景是否由主题管理（非用户自定义）
    bool m_isThemeManagedBackground = false;
    
    std::shared_ptr<interfaces::IControl> m_content;
};

} // namespace controls
} // namespace luaui
