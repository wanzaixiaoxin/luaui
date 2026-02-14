#pragma once

#include "Panel.h"
#include "TextBlock.h"
#include "ProgressBar.h"
#include "../rendering/Types.h"
#include <memory>
#include <string>
#include <vector>

namespace luaui {
namespace controls {

// 前向声明
class StatusBar;
class StatusBarItem;

/**
 * @brief StatusBarItem 状态栏项
 * 
 * 支持：
 * - 文本显示
 * - 进度条
 * - 图标
 * - 弹簧（自动填充）
 * - 边框
 */
class StatusBarItem : public Panel {
public:
    // 项类型
    enum class ItemType {
        Text,       // 文本
        Progress,   // 进度条
        Panel,      // 面板容器
        Spring      // 弹簧（自动填充剩余空间）
    };
    
    StatusBarItem();
    explicit StatusBarItem(const std::wstring& text);
    explicit StatusBarItem(ItemType type);
    
    std::string GetTypeName() const override { return "StatusBarItem"; }
    
    // 类型
    ItemType GetItemType() const { return m_itemType; }
    void SetItemType(ItemType type) { m_itemType = type; }
    
    // 内容（文本类型）
    std::wstring GetText() const;
    void SetText(const std::wstring& text);
    
    // 图标
    std::wstring GetIcon() const { return m_icon; }
    void SetIcon(const std::wstring& icon) { m_icon = icon; }
    
    // 宽度（Spring 类型无效）
    float GetWidth() const { return m_width; }
    void SetWidth(float width) { m_width = width; }
    
    // 自动大小
    bool GetAutoSize() const { return m_autoSize; }
    void SetAutoSize(bool autoSize) { m_autoSize = autoSize; }
    
    // 边框
    bool GetShowBorder() const { return m_showBorder; }
    void SetShowBorder(bool show) { m_showBorder = show; }
    
    // 内容控件（Panel 类型用）
    std::shared_ptr<Control> GetContent() const { return m_content; }
    void SetContent(const std::shared_ptr<Control>& content);
    
    // 进度条访问（Progress 类型用）
    std::shared_ptr<ProgressBar> GetProgressBar() const { return m_progressBar; }

protected:
    void InitializeComponents() override;
    void OnRender(rendering::IRenderContext* context) override;
    rendering::Size OnMeasure(const rendering::Size& availableSize) override;

private:
    void UpdateVisualState();
    
    ItemType m_itemType = ItemType::Text;
    std::wstring m_text;
    std::wstring m_icon;
    float m_width = 100.0f;
    bool m_autoSize = true;
    bool m_showBorder = false;
    
    std::shared_ptr<TextBlock> m_textBlock;
    std::shared_ptr<ProgressBar> m_progressBar;
    std::shared_ptr<Control> m_content;
    
    // 外观
    float m_padding = 4.0f;
    float m_fontSize = 12.0f;
    rendering::Color m_textColor = rendering::Color::Black();
    rendering::Color m_borderColor = rendering::Color::FromHex(0xCCCCCC);
};

/**
 * @brief StatusBar 状态栏（新架构）
 * 
 * 特点：
 * - 多段显示
 * - 支持弹簧自动填充
 * - 支持进度条嵌入
 * - 尺寸调整手柄（可调整窗口大小）
 * - 上下文菜单支持
 */
class StatusBar : public Panel {
public:
    StatusBar();
    
    std::string GetTypeName() const override { return "StatusBar"; }
    
    // 添加项
    std::shared_ptr<StatusBarItem> AddItem(const std::wstring& text, 
                                            float width = -1,  // -1 = auto
                                            bool spring = false);
    std::shared_ptr<StatusBarItem> AddProgressItem(float width = 100.0f);
    std::shared_ptr<StatusBarItem> AddSpring();
    std::shared_ptr<StatusBarItem> AddSeparator();
    
    void AddItem(const std::shared_ptr<StatusBarItem>& item);
    void RemoveItem(const std::shared_ptr<StatusBarItem>& item);
    void ClearItems();
    
    // 通过索引访问项
    std::shared_ptr<StatusBarItem> GetItem(size_t index);
    size_t GetItemCount() const { return m_items.size(); }
    
    // 尺寸调整手柄
    bool GetShowSizingGrip() const { return m_showSizingGrip; }
    void SetShowSizingGrip(bool show) { m_showSizingGrip = show; }
    
    // 状态栏高度
    float GetStatusBarHeight() const { return m_height; }
    void SetStatusBarHeight(float height) { m_height = height; }
    
    // 主文本（第一项快捷访问）
    std::wstring GetMainText() const;
    void SetMainText(const std::wstring& text);
    
    // 设置状态文本（自动添加或更新项）
    void SetStatusText(int panelIndex, const std::wstring& text);

protected:
    void InitializeComponents() override;
    rendering::Size OnMeasureChildren(const rendering::Size& availableSize) override;
    rendering::Size OnArrangeChildren(const rendering::Size& finalSize) override;
    void OnRenderChildren(rendering::IRenderContext* context) override;
    
    void OnMouseDown(MouseEventArgs& args) override;
    void OnMouseMove(MouseEventArgs& args) override;

private:
    void DrawSizingGrip(rendering::IRenderContext* context, const rendering::Rect& rect);
    bool HitTestSizingGrip(float x, float y);
    
    std::vector<std::shared_ptr<StatusBarItem>> m_items;
    
    bool m_showSizingGrip = true;
    float m_height = 24.0f;
    float m_gripSize = 16.0f;
    float m_itemSpacing = 2.0f;
    
    // 颜色
    rendering::Color m_bgColor = rendering::Color::FromHex(0xF0F0F0);
    rendering::Color m_borderColor = rendering::Color::FromHex(0xCCCCCC);
    rendering::Color m_gripColor = rendering::Color::FromHex(0x999999);
    
    // 尺寸调整状态
    bool m_isResizing = false;
    float m_resizeStartX = 0;
    float m_resizeStartY = 0;
};

} // namespace controls
} // namespace luaui
