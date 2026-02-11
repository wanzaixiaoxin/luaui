#pragma once

#include "Panel.h"
#include <memory>
#include <vector>

namespace luaui {
namespace controls {

// Forward declaration
class ListBoxItem;

/**
 * @brief ListBoxItem 列表项（新架构）
 */
class ListBoxItem : public Control {
public:
    ListBoxItem();
    
    std::string GetTypeName() const override { return "ListBoxItem"; }
    
    std::wstring GetContent() const { return m_content; }
    void SetContent(const std::wstring& content);
    
    bool GetIsSelected() const { return m_isSelected; }
    void SetIsSelected(bool selected);
    
    bool GetIsHovered() const { return m_isHovered; }
    void SetIsHovered(bool hovered);

protected:
    void InitializeComponents() override;

private:
    std::wstring m_content;
    bool m_isSelected = false;
    bool m_isHovered = false;
    
    float m_itemHeight = 24.0f;
    rendering::Color m_normalBg = rendering::Color::White();
    rendering::Color m_hoverBg = rendering::Color::FromHex(0xE5F3FF);
    rendering::Color m_selectedBg = rendering::Color::FromHex(0x0078D4);
};

/**
 * @brief ListBox 列表框（新架构）
 */
class ListBox : public Panel {
public:
    ListBox();
    
    std::string GetTypeName() const override { return "ListBox"; }
    
    void AddItem(const std::wstring& item);
    void AddItem(const std::shared_ptr<ListBoxItem>& item);
    void RemoveItem(int index);
    void ClearItems();
    size_t GetItemCount() const { return m_items.size(); }
    std::shared_ptr<ListBoxItem> GetItem(int index);
    
    int GetSelectedIndex() const { return m_selectedIndex; }
    void SetSelectedIndex(int index);
    std::wstring GetSelectedItem() const;
    
    float GetItemHeight() const { return m_itemHeight; }
    void SetItemHeight(float height) { m_itemHeight = height; }
    
    // 事件
    luaui::Delegate<ListBox*, int> SelectionChanged;

protected:
    void InitializeComponents() override;
    void OnRenderChildren(rendering::IRenderContext* context) override;
    rendering::Size OnMeasureChildren(const rendering::Size& availableSize) override;
    rendering::Size OnArrangeChildren(const rendering::Size& finalSize) override;

private:
    void UpdateItemStates();
    
    std::vector<std::shared_ptr<ListBoxItem>> m_items;
    int m_selectedIndex = -1;
    float m_scrollOffset = 0;
    float m_itemHeight = 24.0f;
};

} // namespace controls
} // namespace luaui
