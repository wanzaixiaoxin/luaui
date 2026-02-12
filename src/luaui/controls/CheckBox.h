#pragma once

#include "Control.h"
#include "../core/Components/LayoutComponent.h"
#include "../core/Components/RenderComponent.h"
#include "../core/Components/InputComponent.h"
#include "../rendering/Types.h"
#include <string>
#include <memory>

namespace luaui {
namespace controls {

/**
 * @brief CheckBox 复选框（新架构）
 */
class CheckBox : public luaui::Control {
public:
    CheckBox();
    
    std::string GetTypeName() const override { return "CheckBox"; }
    
    // 属性
    std::wstring GetText() const { return m_text; }
    void SetText(const std::wstring& text);
    
    bool GetIsChecked() const { return m_isChecked; }
    void SetIsChecked(bool checked);
    
    bool GetIsThreeState() const { return m_isThreeState; }
    void SetIsThreeState(bool threeState);
    
    // 事件
    luaui::Delegate<CheckBox*, bool> CheckedChanged;

protected:
    void InitializeComponents() override;
    rendering::Size OnMeasure(const rendering::Size& availableSize) override;
    void OnRender(rendering::IRenderContext* context) override;
    
    void OnClick() override;
    void OnMouseEnter() override;
    void OnMouseLeave() override;
    void OnMouseDown(MouseEventArgs& args) override;
    void OnMouseUp(MouseEventArgs& args) override;

private:
    void Toggle();
    void UpdateVisualState();
    
    std::wstring m_text;
    bool m_isChecked = false;
    bool m_isThreeState = false;
    bool m_isIndeterminate = false;
    bool m_isHovered = false;
    bool m_isPressed = false;
    
    float m_boxSize = 16.0f;
    float m_spacing = 6.0f;
    float m_fontSize = 14.0f;
    
    rendering::Color m_normalBorder = rendering::Color::FromHex(0x8E8E8E);
    rendering::Color m_hoverBorder = rendering::Color::FromHex(0x0078D4);
    rendering::Color m_pressedBorder = rendering::Color::FromHex(0x005A9E);
    rendering::Color m_checkColor = rendering::Color::FromHex(0x0078D4);
    rendering::Color m_boxBackground = rendering::Color::White();
};

/**
 * @brief RadioButton 单选按钮（新架构）
 */
class RadioButton : public luaui::Control {
public:
    RadioButton();
    
    std::string GetTypeName() const override { return "RadioButton"; }
    
    // 属性
    std::wstring GetText() const { return m_text; }
    void SetText(const std::wstring& text);
    
    bool GetIsChecked() const { return m_isChecked; }
    void SetIsChecked(bool checked);
    
    std::string GetGroupName() const { return m_groupName; }
    void SetGroupName(const std::string& name);
    
    // 事件
    luaui::Delegate<RadioButton*, bool> CheckedChanged;

protected:
    void InitializeComponents() override;
    void OnRender(rendering::IRenderContext* context) override;

private:
    void OnChecked();
    void UpdateVisualState();
    
    std::wstring m_text;
    bool m_isChecked = false;
    bool m_isHovered = false;
    bool m_isPressed = false;
    std::string m_groupName;
    
    float m_circleSize = 16.0f;
    float m_spacing = 6.0f;
    float m_fontSize = 14.0f;
    float m_dotSize = 8.0f;
    
    rendering::Color m_normalBorder = rendering::Color::FromHex(0x8E8E8E);
    rendering::Color m_hoverBorder = rendering::Color::FromHex(0x0078D4);
    rendering::Color m_pressedBorder = rendering::Color::FromHex(0x005A9E);
    rendering::Color m_checkColor = rendering::Color::FromHex(0x0078D4);
    rendering::Color m_circleBackground = rendering::Color::White();
};

} // namespace controls
} // namespace luaui
