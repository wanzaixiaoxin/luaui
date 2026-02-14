#pragma once

#include "Control.h"
#include "../rendering/Types.h"
#include <string>

namespace luaui {
namespace controls {

// 前向声明
class Control;

/**
 * @brief Tooltip 鼠标提示控件（新架构）
 * 
 * 特点：
 * - 显示提示文本
 * - 支持自动定位（跟随鼠标或相对于目标控件）
 * - 延迟显示/自动隐藏
 * - 支持多行文本
 * - 支持最大宽度限制（自动换行）
 */
class Tooltip : public luaui::Control {
public:
    Tooltip();
    
    std::string GetTypeName() const override { return "Tooltip"; }
    
    // 文本内容
    std::wstring GetText() const { return m_text; }
    void SetText(const std::wstring& text);
    
    // 最大宽度（用于自动换行，0表示不限制）
    float GetMaxWidth() const { return m_maxWidth; }
    void SetMaxWidth(float width) { m_maxWidth = width; }
    
    // 显示/隐藏
    void Show();
    void ShowAt(float x, float y);
    void ShowRelativeTo(Control* target, float offsetX = 0, float offsetY = -5);
    void Hide();
    bool GetIsVisible() const { return m_isVisible; }
    
    // 延迟显示时间（毫秒）
    int GetShowDelay() const { return m_showDelayMs; }
    void SetShowDelay(int ms) { m_showDelayMs = ms; }
    
    // 自动隐藏时间（毫秒，0表示不自动隐藏）
    int GetAutoHideDelay() const { return m_autoHideDelayMs; }
    void SetAutoHideDelay(int ms) { m_autoHideDelayMs = ms; }
    
    // 全局单例 - 获取默认 Tooltip 实例（用于简单的全局提示）
    static Tooltip* GetDefault();
    
    // 便捷方法：为目标控件设置提示文本（使用默认 Tooltip）
    static void SetToolTip(Control* target, const std::wstring& text);
    static std::wstring GetToolTip(Control* target);

protected:
    void InitializeComponents() override;
    void OnRender(rendering::IRenderContext* context) override;
    rendering::Size OnMeasure(const rendering::Size& availableSize) override;
    
    // 尺寸计算
    rendering::Size MeasureText(const std::wstring& text, float maxWidth);

private:
    std::wstring m_text;
    float m_maxWidth = 300.0f;      // 默认最大宽度
    int m_showDelayMs = 500;        // 默认延迟 500ms 显示
    int m_autoHideDelayMs = 0;      // 默认不自动隐藏
    bool m_isVisible = false;
    
    // 外观
    float m_padding = 8.0f;
    float m_cornerRadius = 4.0f;
    float m_fontSize = 12.0f;
    
    // 颜色
    rendering::Color m_bgColor = rendering::Color::FromHex(0x333333);
    rendering::Color m_textColor = rendering::Color::White();
    rendering::Color m_borderColor = rendering::Color::FromHex(0x555555);
    
    // 全局默认实例
    static Tooltip* s_defaultTooltip;
};

/**
 * @brief ToolTipService 提示服务
 * 
 * 为任意控件附加 Tooltip 功能
 */
class ToolTipService {
public:
    // 为控件设置提示文本
    static void SetToolTip(Control* control, const std::wstring& text);
    static std::wstring GetToolTip(Control* control);
    
    // 启用/禁用控件的 Tooltip
    static void SetIsEnabled(Control* control, bool enabled);
    static bool GetIsEnabled(Control* control);
    
    // 设置显示延迟（全局）
    static void SetShowDelay(int ms);
    static int GetShowDelay();
    
private:
    static int s_showDelayMs;
    static bool s_initialized;
    static void Initialize();
};

} // namespace controls
} // namespace luaui
