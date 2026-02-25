#pragma once

#include "Panel.h"
#include "VirtualizingPanel.h"
#include "../core/Interfaces/IControl.h"
#include "../rendering/Types.h"
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <any>

namespace luaui {
namespace controls {

// Forward declaration
class ListBoxItem;

/**
 * @brief ListBoxItem 列表项（新架构）
 */
class ListBoxItem : public luaui::Control {
public:
    ListBoxItem();
    
    std::string GetTypeName() const override { return "ListBoxItem"; }
    
    std::wstring GetContent() const { return m_content; }
    void SetContent(const std::wstring& content);
    
    bool GetIsSelected() const { return m_isSelected; }
    void SetIsSelected(bool selected);
    
    bool GetIsHovered() const { return m_isHovered; }
    void SetIsHovered(bool hovered);
    
    // 设置项高度（通过布局组件）
    void SetItemHeight(float height);

protected:
    void InitializeComponents() override;
    void OnRender(rendering::IRenderContext* context) override;
    rendering::Size OnMeasure(const rendering::Size& availableSize) override;
    void OnClick() override;

private:
    int m_index = -1;  // 在ListBox中的索引
    friend class ListBox;
    std::wstring m_content;
    bool m_isSelected = false;
    bool m_isHovered = false;
    
    float m_itemHeight = 24.0f;  // 缓存的高度值
    float m_fontSize = 14.0f;
    rendering::Color m_normalBg = rendering::Color::White();
    rendering::Color m_hoverBg = rendering::Color::FromHex(0xE5F3FF);
    rendering::Color m_selectedBg = rendering::Color::FromHex(0x0078D4);
    rendering::Color m_textColor = rendering::Color::Black();
    rendering::Color m_selectedTextColor = rendering::Color::White();
};

/**
 * @brief ListBox 列表框（支持虚拟化）
 * 
 * 性能特性：
 * - 虚拟化模式（默认）：只创建可见项，支持 10,000+ 项
 * - 标准模式：创建所有项，适合少量数据
 */
class ListBox : public Panel {
public:
    ListBox();
    virtual ~ListBox();
    
    std::string GetTypeName() const override { return "ListBox"; }
    
    // 虚拟化设置
    bool GetIsVirtualizing() const { return m_isVirtualizing; }
    void SetIsVirtualizing(bool enable);
    
    // 项高度（虚拟化模式下需要固定高度）
    float GetItemHeight() const { return m_itemHeight; }
    void SetItemHeight(float height);
    
    // 传统 API（兼容旧代码）
    void AddItem(const std::wstring& item);
    void AddItem(const std::shared_ptr<ListBoxItem>& item);
    void InsertItem(int index, const std::wstring& item);
    void RemoveItem(int index);
    void ClearItems();
    size_t GetItemCount() const;
    std::shared_ptr<ListBoxItem> GetItem(int index);
    
    // 虚拟化数据源（新 API）
    using DataSourceCallback = std::function<std::wstring(int index)>;
    void SetDataSource(int count, DataSourceCallback callback);
    void RefreshData();
    
    // 选中操作
    int GetSelectedIndex() const { return m_selectedIndex; }
    void SetSelectedIndex(int index);
    std::wstring GetSelectedItem() const;
    
    // 滚动到指定项
    void ScrollIntoView(int index);
    
    // 事件
    luaui::Delegate<ListBox*, int> SelectionChanged;

protected:
    void InitializeComponents() override;
    void OnRenderChildren(rendering::IRenderContext* context) override;
    rendering::Size OnMeasure(const rendering::Size& availableSize) override;
    rendering::Size OnArrangeChildren(const rendering::Size& finalSize) override;
    
    // 输入处理
    void OnClick() override;
    void OnMouseMove(MouseEventArgs& args) override;
    void OnMouseWheel(MouseEventArgs& args) override;
    void OnKeyDown(KeyEventArgs& args) override;

private:
    // 初始化虚拟化
    void InitializeVirtualization();
    void CleanupVirtualization();
    
    // 创建列表项容器（用于虚拟化）
    std::shared_ptr<Control> CreateItemContainer();
    void BindItemToContainer(std::shared_ptr<Control> container, int index);
    
    // 传统模式下的更新
    void UpdateItemStates();
    int HitTestItem(float x, float y);
    
    // 虚拟化模式下的命中测试
    int VirtualizedHitTest(float y);
    
    // 滚动处理
    void UpdateScrollOffset(float newOffset);
    void ClampScrollOffset();
    
    // 模式标志
    bool m_isVirtualizing = false;
    bool m_isVirtualizationInitialized = false;
    
    // 传统模式数据
    std::vector<std::shared_ptr<ListBoxItem>> m_items;
    
    // 虚拟化模式数据
    std::shared_ptr<VirtualizingPanel> m_virtualizingPanel;
    DataSourceCallback m_dataSource;
    int m_dataSourceCount = 0;
    
    // 通用状态
    int m_selectedIndex = -1;
    float m_scrollOffset = 0;
    float m_itemHeight = 24.0f;
    float m_maxScrollOffset = 0;
    
    // 滚动条
    bool m_showScrollbar = true;
    float m_scrollbarWidth = 16.0f;
};

} // namespace controls
} // namespace luaui
