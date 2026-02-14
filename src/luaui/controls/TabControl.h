#pragma once

#include "Panel.h"
#include "../rendering/Types.h"
#include <memory>
#include <vector>
#include <string>

namespace luaui {
namespace controls {

// 前向声明
class TabItem;

/**
 * @brief TabItem 标签页项（新架构）
 * 
 * 包含：
 * - 标签头（Header）
 * - 内容区域（Content）
 * - 关闭按钮（可选）
 */
class TabItem : public luaui::Control {
public:
    TabItem();
    
    std::string GetTypeName() const override { return "TabItem"; }
    
    // 标签头
    std::wstring GetHeader() const { return m_header; }
    void SetHeader(const std::wstring& header);
    
    // 内容控件
    std::shared_ptr<Control> GetContent() const { return m_content; }
    void SetContent(const std::shared_ptr<Control>& content);
    
    // 是否选中
    bool GetIsSelected() const { return m_isSelected; }
    void SetIsSelected(bool selected);
    
    // 是否可以关闭
    bool GetCanClose() const { return m_canClose; }
    void SetCanClose(bool canClose) { m_canClose = canClose; }
    
    // 图标（可选）
    std::wstring GetIcon() const { return m_icon; }
    void SetIcon(const std::wstring& icon) { m_icon = icon; }
    
    // 用户数据
    void* GetTag() const { return m_tag; }
    void SetTag(void* tag) { m_tag = tag; }

protected:
    void InitializeComponents() override;
    void OnRender(rendering::IRenderContext* context) override;
    rendering::Size OnMeasure(const rendering::Size& availableSize) override;
    
    void OnMouseEnter() override;
    void OnMouseLeave() override;
    void OnMouseDown(MouseEventArgs& args) override;

private:
    void UpdateVisualState();
    void DrawCloseButton(rendering::IRenderContext* context, const rendering::Rect& rect);
    bool HitTestCloseButton(float x, float y);
    
    std::wstring m_header;
    std::wstring m_icon;
    std::shared_ptr<Control> m_content;
    void* m_tag = nullptr;
    
    bool m_isSelected = false;
    bool m_isHovered = false;
    bool m_canClose = false;
    bool m_isCloseHovered = false;
    
    // 外观
    float m_tabHeight = 32.0f;
    float m_padding = 12.0f;
    float m_closeButtonSize = 14.0f;
    float m_iconSize = 16.0f;
    float m_fontSize = 14.0f;
    
    // 颜色
    rendering::Color m_normalBg = rendering::Color::Transparent();
    rendering::Color m_hoverBg = rendering::Color::FromHex(0xE5F3FF);
    rendering::Color m_selectedBg = rendering::Color::White();
    rendering::Color m_textColor = rendering::Color::Black();
    rendering::Color m_selectedTextColor = rendering::Color::Black();
    rendering::Color m_closeButtonColor = rendering::Color::FromHex(0x999999);
    rendering::Color m_closeButtonHoverColor = rendering::Color::FromHex(0xE81123);
    
    friend class TabControl;
};

/**
 * @brief TabControl 多标签页控件（新架构）
 * 
 * 特点：
 * - 顶部/底部/左侧/右侧标签栏位置
 * - 支持滚动标签栏
 * - 支持关闭标签
 * - 支持拖拽排序（简化版暂不支持）
 */
class TabControl : public Panel {
public:
    // 标签栏位置
    enum class TabStripPlacement {
        Top,      // 顶部（默认）
        Bottom,   // 底部
        Left,     // 左侧
        Right     // 右侧
    };
    
    TabControl();
    
    std::string GetTypeName() const override { return "TabControl"; }
    
    // 标签页操作
    void AddTab(const std::shared_ptr<TabItem>& tab);
    void RemoveTab(const std::shared_ptr<TabItem>& tab);
    void RemoveTab(int index);
    void ClearTabs();
    size_t GetTabCount() const { return m_tabs.size(); }
    std::shared_ptr<TabItem> GetTab(int index);
    std::shared_ptr<TabItem> GetTab(const std::wstring& header);
    
    // 选中项
    int GetSelectedIndex() const { return m_selectedIndex; }
    void SetSelectedIndex(int index);
    std::shared_ptr<TabItem> GetSelectedTab() const;
    
    // 标签栏位置
    TabStripPlacement GetTabStripPlacement() const { return m_tabStripPlacement; }
    void SetTabStripPlacement(TabStripPlacement placement);
    
    // 外观属性
    float GetTabHeight() const { return m_tabHeight; }
    void SetTabHeight(float height) { m_tabHeight = height; }
    
    float GetTabWidth() const { return m_tabWidth; }
    void SetTabWidth(float width) { m_tabWidth = width; } // 0 表示自动宽度
    
    // 事件
    luaui::Delegate<TabControl*, int> SelectionChanged;
    luaui::Delegate<TabControl*, TabItem*> TabClosed;
    luaui::Delegate<TabControl*, TabItem*> TabAdded;
    luaui::Delegate<TabControl*, TabItem*> TabRemoved;

protected:
    void InitializeComponents() override;
    rendering::Size OnMeasureChildren(const rendering::Size& availableSize) override;
    rendering::Size OnArrangeChildren(const rendering::Size& finalSize) override;
    void OnRenderChildren(rendering::IRenderContext* context) override;
    
    void OnMouseMove(MouseEventArgs& args) override;
    void OnMouseDown(MouseEventArgs& args) override;

private:
    friend class TabItem;
    
    void OnTabSelected(TabItem* tab);
    void OnTabClose(TabItem* tab);
    void UpdateTabStates();
    
    // 计算标签栏和内容区域
    rendering::Rect GetTabStripRect() const;
    rendering::Rect GetContentRect() const;
    
    // 计算标签位置
    float CalculateTabWidth() const;
    rendering::Rect GetTabRect(int index) const;
    int HitTestTab(float x, float y);
    
    std::vector<std::shared_ptr<TabItem>> m_tabs;
    int m_selectedIndex = -1;
    TabItem* m_hoveredTab = nullptr;
    
    // 配置
    TabStripPlacement m_tabStripPlacement = TabStripPlacement::Top;
    float m_tabHeight = 32.0f;
    float m_tabWidth = 0;        // 0 = 自动宽度
    float m_tabSpacing = 0;      // 标签间距
    
    // 颜色
    rendering::Color m_tabStripBg = rendering::Color::FromHex(0xF0F0F0);
    rendering::Color m_contentBg = rendering::Color::White();
    rendering::Color m_borderColor = rendering::Color::FromHex(0xCCCCCC);
};

} // namespace controls
} // namespace luaui
