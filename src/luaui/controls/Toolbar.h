#pragma once

#include "Panel.h"
#include "Button.h"
#include "../rendering/Types.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace luaui {
namespace controls {

// 前向声明
class Toolbar;
class ToolbarItem;
class ToolbarSeparator;

/**
 * @brief ToolbarItem 工具栏项
 * 
 * 支持：
 * - 图标 + 文本
 * - 仅图标
 * - 仅文本
 * - 下拉菜单
 * - 复选状态
 * - 禁用状态
 */
class ToolbarItem : public luaui::Control {
public:
    ToolbarItem();
    explicit ToolbarItem(const std::wstring& text);
    
    std::string GetTypeName() const override { return "ToolbarItem"; }
    
    // 文本
    std::wstring GetText() const { return m_text; }
    void SetText(const std::wstring& text);
    
    // 图标
    std::wstring GetIcon() const { return m_icon; }
    void SetIcon(const std::wstring& icon);
    
    // 工具提示
    std::wstring GetToolTip() const { return m_toolTip; }
    void SetToolTip(const std::wstring& tip) { m_toolTip = tip; }
    
    // 命令
    using CommandHandler = std::function<void(ToolbarItem*)>;
    void SetCommand(CommandHandler handler) { m_command = handler; }
    void InvokeCommand() { if (m_command) m_command(this); }
    
    // 复选状态
    bool GetIsChecked() const { return m_isChecked; }
    void SetIsChecked(bool checked);
    void Toggle() { SetIsChecked(!m_isChecked); }
    
    // 是否可复选
    bool GetIsCheckable() const { return m_isCheckable; }
    void SetIsCheckable(bool checkable) { m_isCheckable = checkable; }
    
    // 是否按下（按下式按钮）
    bool GetIsPressed() const { return m_isPressed; }
    void SetIsPressed(bool pressed);
    
    // 是否有下拉菜单
    bool GetHasDropDown() const { return m_hasDropDown; }
    void SetHasDropDown(bool has) { m_hasDropDown = has; }
    
    // 快捷键
    std::wstring GetShortcutKey() const { return m_shortcutKey; }
    void SetShortcutKey(const std::wstring& key) { m_shortcutKey = key; }

protected:
    void InitializeComponents() override;
    void OnRender(rendering::IRenderContext* context) override;
    rendering::Size OnMeasure(const rendering::Size& availableSize) override;
    
    void OnMouseEnter() override;
    void OnMouseLeave() override;
    void OnMouseDown(MouseEventArgs& args) override;
    void OnMouseUp(MouseEventArgs& args) override;
    void OnClick() override;

private:
    void UpdateVisualState();
    void DrawIcon(rendering::IRenderContext* context, const rendering::Rect& rect);
    void DrawDropDownArrow(rendering::IRenderContext* context, const rendering::Rect& rect);
    
    std::wstring m_text;
    std::wstring m_icon;
    std::wstring m_toolTip;
    std::wstring m_shortcutKey;
    CommandHandler m_command;
    
    bool m_isChecked = false;
    bool m_isCheckable = false;
    bool m_isPressed = false;
    bool m_isHovered = false;
    bool m_hasDropDown = false;
    
    // 显示模式
    enum class DisplayMode {
        Default,      // 图标+文本（默认）
        IconOnly,     // 仅图标
        TextOnly      // 仅文本
    };
    DisplayMode m_displayMode = DisplayMode::Default;
    
    // 外观
    float m_padding = 6.0f;
    float m_iconSize = 16.0f;
    float m_spacing = 4.0f;
    float m_fontSize = 12.0f;
    float m_dropDownArrowWidth = 10.0f;
    
    // 颜色
    rendering::Color m_normalBg = rendering::Color::Transparent();
    rendering::Color m_hoverBg = rendering::Color::FromHex(0xE5F3FF);
    rendering::Color m_pressedBg = rendering::Color::FromHex(0xCCE4F7);
    rendering::Color m_checkedBg = rendering::Color::FromHex(0xCCE4F7);
    rendering::Color m_textColor = rendering::Color::Black();
    rendering::Color m_disabledColor = rendering::Color::FromHex(0x999999);
};

/**
 * @brief ToolbarSeparator 工具栏分隔线
 */
class ToolbarSeparator : public luaui::Control {
public:
    ToolbarSeparator();
    
    std::string GetTypeName() const override { return "ToolbarSeparator"; }

protected:
    void InitializeComponents() override;
    void OnRender(rendering::IRenderContext* context) override;
    rendering::Size OnMeasure(const rendering::Size& availableSize) override;

private:
    float m_width = 8.0f;
    float m_lineWidth = 1.0f;
    rendering::Color m_lineColor = rendering::Color::FromHex(0xCCCCCC);
};

/**
 * @brief Toolbar 工具栏（新架构）
 * 
 * 特点：
 * - 水平/垂直布局
 * - 支持溢出（按钮过多时显示更多按钮）
 * - 可拖拽浮动
 * - 可停靠
 */
class Toolbar : public Panel {
public:
    // 工具栏方向
    enum class Orientation {
        Horizontal,  // 水平（默认）
        Vertical     // 垂直
    };
    
    // 工具栏样式
    enum class ToolbarStyle {
        Default,     // 默认（有边框背景）
        Flat,        // 扁平（无边框）
        Transparent  // 透明
    };
    
    Toolbar();
    
    std::string GetTypeName() const override { return "Toolbar"; }
    
    // 添加项
    void AddItem(const std::shared_ptr<ToolbarItem>& item);
    void AddSeparator();
    void AddStretch();  // 弹性空间
    
    // 移除项
    void RemoveItem(const std::shared_ptr<ToolbarItem>& item);
    void ClearItems();
    
    // 方向
    Orientation GetOrientation() const { return m_orientation; }
    void SetOrientation(Orientation orientation);
    
    // 样式
    ToolbarStyle GetToolbarStyle() const { return m_style; }
    void SetToolbarStyle(ToolbarStyle style) { m_style = style; }
    
    // 高度/宽度
    float GetToolbarThickness() const { return m_thickness; }
    void SetToolbarThickness(float thickness) { m_thickness = thickness; }
    
    // 溢出处理
    bool GetAllowOverflow() const { return m_allowOverflow; }
    void SetAllowOverflow(bool allow) { m_allowOverflow = allow; }
    
    // 是否显示文本标签
    bool GetShowTextLabels() const { return m_showTextLabels; }
    void SetShowTextLabels(bool show);
    
    // 是否显示图标
    bool GetShowIcons() const { return m_showIcons; }
    void SetShowIcons(bool show) { m_showIcons = show; }

protected:
    void InitializeComponents() override;
    rendering::Size OnMeasureChildren(const rendering::Size& availableSize) override;
    rendering::Size OnArrangeChildren(const rendering::Size& finalSize) override;
    void OnRenderChildren(rendering::IRenderContext* context) override;

private:
    void UpdateItemDisplay();
    
    std::vector<std::shared_ptr<Control>> m_items;
    Orientation m_orientation = Orientation::Horizontal;
    ToolbarStyle m_style = ToolbarStyle::Default;
    
    float m_thickness = 32.0f;      // 水平=高度，垂直=宽度
    float m_itemSpacing = 2.0f;
    bool m_allowOverflow = true;
    bool m_showTextLabels = true;
    bool m_showIcons = true;
    
    // 颜色
    rendering::Color m_bgColor = rendering::Color::FromHex(0xF5F5F5);
    rendering::Color m_borderColor = rendering::Color::FromHex(0xCCCCCC);
    rendering::Color m_gripperColor = rendering::Color::FromHex(0x999999);
};

/**
 * @brief ToolStripContainer 工具栏容器
 * 
 * 支持多个工具栏停靠（顶部、底部、左侧、右侧）
 */
class ToolStripContainer : public Panel {
public:
    ToolStripContainer();
    
    std::string GetTypeName() const override { return "ToolStripContainer"; }
    
    // 添加工具栏到指定边缘
    void AddToolbar(const std::shared_ptr<Toolbar>& toolbar, 
                    layouts::Dock dock);
    
    // 内容区域
    std::shared_ptr<Panel> GetContentPanel() const { return m_contentPanel; }
    void SetContent(const std::shared_ptr<Control>& content);

protected:
    void InitializeComponents() override;

private:
    std::shared_ptr<Panel> m_topPanel;
    std::shared_ptr<Panel> m_bottomPanel;
    std::shared_ptr<Panel> m_leftPanel;
    std::shared_ptr<Panel> m_rightPanel;
    std::shared_ptr<Panel> m_contentPanel;
};

} // namespace controls
} // namespace luaui
