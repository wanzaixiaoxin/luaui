#pragma once

#include "Control.h"
#include "../rendering/Types.h"
#include <string>

namespace luaui {
namespace controls {

/**
 * @brief ProgressBar 进度条（新架构）
 * 
 * 支持：
 * - 确定模式（显示具体进度 0-100）
 * - 不确定模式（循环动画）
 * - 水平/垂直方向
 * - 文本显示（百分比或自定义）
 * - 颜色定制
 */
class ProgressBar : public luaui::Control {
public:
    // 进度条方向
    enum class Orientation {
        Horizontal,  // 水平（默认）
        Vertical     // 垂直
    };
    
    ProgressBar();
    
    std::string GetTypeName() const override { return "ProgressBar"; }
    
    // 最小/最大值
    double GetMinimum() const { return m_minimum; }
    void SetMinimum(double value);
    
    double GetMaximum() const { return m_maximum; }
    void SetMaximum(double value);
    
    // 当前值
    double GetValue() const { return m_value; }
    void SetValue(double value);
    
    // 进度百分比 (0.0 - 1.0)
    double GetPercent() const;
    
    // 是否不确定模式
    bool GetIsIndeterminate() const { return m_isIndeterminate; }
    void SetIsIndeterminate(bool indeterminate);
    
    // 方向
    Orientation GetOrientation() const { return m_orientation; }
    void SetOrientation(Orientation orientation);
    
    // 是否显示文本
    bool GetShowText() const { return m_showText; }
    void SetShowText(bool show) { m_showText = show; }
    
    // 文本格式（占位符：{0}=百分比, {1}=当前值, {2}=最大值）
    std::wstring GetTextFormat() const { return m_textFormat; }
    void SetTextFormat(const std::wstring& format) { m_textFormat = format; }
    
    // 动画进度（不确定模式用）
    void SetAnimationOffset(float offset) { m_animationOffset = offset; }
    float GetAnimationOffset() const { return m_animationOffset; }

protected:
    void InitializeComponents() override;
    void OnRender(rendering::IRenderContext* context) override;
    rendering::Size OnMeasure(const rendering::Size& availableSize) override;

private:
    void UpdateVisualState();
    std::wstring GetDisplayText() const;
    
    double m_minimum = 0.0;
    double m_maximum = 100.0;
    double m_value = 0.0;
    bool m_isIndeterminate = false;
    Orientation m_orientation = Orientation::Horizontal;
    bool m_showText = false;
    std::wstring m_textFormat = L"{0}%";
    float m_animationOffset = 0.0f;  // 0.0 - 1.0
    
    // 外观
    float m_cornerRadius = 2.0f;
    float m_fontSize = 12.0f;
    float m_minimumBarSize = 4.0f;   // 不确定模式最小块大小
    
    // 颜色
    rendering::Color m_backgroundColor = rendering::Color::FromHex(0xE0E0E0);
    rendering::Color m_foregroundColor = rendering::Color::FromHex(0x0078D4);
    rendering::Color m_borderColor = rendering::Color::FromHex(0xCCCCCC);
    rendering::Color m_textColor = rendering::Color::Black();
};

/**
 * @brief ProgressRing 环形进度条（加载指示器）
 * 
 * 支持：
 * - 确定模式（环形填充）
 * - 不确定模式（旋转动画）
 */
class ProgressRing : public luaui::Control {
public:
    ProgressRing();
    
    std::string GetTypeName() const override { return "ProgressRing"; }
    
    // 是否不确定模式
    bool GetIsIndeterminate() const { return m_isIndeterminate; }
    void SetIsIndeterminate(bool indeterminate);
    
    // 当前值 (0.0 - 1.0)
    double GetValue() const { return m_value; }
    void SetValue(double value);
    
    // 环粗细
    float GetRingThickness() const { return m_ringThickness; }
    void SetRingThickness(float thickness) { m_ringThickness = thickness; }
    
    // 动画角度（不确定模式用）
    void SetAnimationAngle(float angle) { m_animationAngle = angle; }
    float GetAnimationAngle() const { return m_animationAngle; }

protected:
    void InitializeComponents() override;
    void OnRender(rendering::IRenderContext* context) override;
    rendering::Size OnMeasure(const rendering::Size& availableSize) override;

private:
    bool m_isIndeterminate = true;
    double m_value = 0.0;
    float m_ringThickness = 4.0f;
    float m_animationAngle = 0.0f;
    
    // 颜色
    rendering::Color m_foregroundColor = rendering::Color::FromHex(0x0078D4);
    rendering::Color m_backgroundColor = rendering::Color::FromHex(0xE0E0E0);
};

} // namespace controls
} // namespace luaui
