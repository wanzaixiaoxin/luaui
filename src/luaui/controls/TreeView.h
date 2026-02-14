#pragma once

#include "Panel.h"
#include "../rendering/Types.h"
#include <memory>
#include <vector>
#include <string>

namespace luaui {
namespace controls {

// 前向声明
class TreeView;
class TreeViewItem;

/**
 * @brief TreeViewItem 树节点项（新架构）
 * 
 * 支持：
 * - 展开/折叠子节点
 * - 选中状态
 * - 自定义数据
 * - 层级缩进
 */
class TreeViewItem : public luaui::Control,
                     public std::enable_shared_from_this<TreeViewItem> {
public:
    TreeViewItem();
    
    std::string GetTypeName() const override { return "TreeViewItem"; }
    
    // 数据
    std::wstring GetHeader() const { return m_header; }
    void SetHeader(const std::wstring& header);
    
    // 层级
    int GetLevel() const { return m_level; }
    void SetLevel(int level) { m_level = level; }
    
    // 展开/折叠
    bool GetIsExpanded() const { return m_isExpanded; }
    void SetIsExpanded(bool expanded);
    void ToggleExpand();
    
    // 选中状态
    bool GetIsSelected() const { return m_isSelected; }
    void SetIsSelected(bool selected);
    
    // 是否有子节点
    bool GetHasItems() const { return !m_children.empty(); }
    
    // 子节点操作
    void AddItem(const std::shared_ptr<TreeViewItem>& item);
    void RemoveItem(const std::shared_ptr<TreeViewItem>& item);
    void ClearItems();
    size_t GetItemCount() const { return m_children.size(); }
    std::shared_ptr<TreeViewItem> GetItem(size_t index) { 
        return (index < m_children.size()) ? m_children[index] : nullptr; 
    }
    const std::vector<std::shared_ptr<TreeViewItem>>& GetChildren() const { return m_children; }
    
    // 父节点
    std::shared_ptr<TreeViewItem> GetParentItem() const { return m_parentItem.lock(); }
    void SetParentItem(const std::shared_ptr<TreeViewItem>& parent) { m_parentItem = parent; }
    
    // 关联的 TreeView
    TreeView* GetTreeView() const { return m_treeView; }
    void SetTreeView(TreeView* treeView) { m_treeView = treeView; }
    
    // 计算总高度（包含子节点）
    float CalculateTotalHeight() const;
    
    // 查找项（递归）
    std::shared_ptr<TreeViewItem> FindItem(const std::wstring& header);

protected:
    void InitializeComponents() override;
    void OnRender(rendering::IRenderContext* context) override;
    rendering::Size OnMeasure(const rendering::Size& availableSize) override;
    
    // 输入处理
    void OnClick() override;
    void OnMouseEnter() override;
    void OnMouseLeave() override;
    void OnMouseDown(MouseEventArgs& args) override;

private:
    void UpdateVisualState();
    void DrawExpandButton(rendering::IRenderContext* context, const rendering::Rect& rect);
    
    std::wstring m_header;
    int m_level = 0;
    bool m_isExpanded = false;
    bool m_isSelected = false;
    bool m_isHovered = false;
    
    std::vector<std::shared_ptr<TreeViewItem>> m_children;
    std::weak_ptr<TreeViewItem> m_parentItem;
    TreeView* m_treeView = nullptr;
    
    // 外观
    float m_itemHeight = 28.0f;
    float m_indentSize = 20.0f;     // 每级缩进
    float m_expandButtonSize = 16.0f;
    float m_padding = 4.0f;
    float m_fontSize = 14.0f;
    
    // 颜色
    rendering::Color m_bgColor = rendering::Color::Transparent();
    rendering::Color m_hoverColor = rendering::Color::FromHex(0xE5F3FF);
    rendering::Color m_selectedColor = rendering::Color::FromHex(0x0078D4);
    rendering::Color m_textColor = rendering::Color::Black();
    rendering::Color m_selectedTextColor = rendering::Color::White();
    rendering::Color m_expandButtonColor = rendering::Color::FromHex(0x666666);
};

/**
 * @brief TreeView 树形控件（新架构）
 * 
 * 应用场景：
 * - 组织架构树
 * - 文件浏览器
 * - 分类导航
 * - 权限树
 */
class TreeView : public Panel {
public:
    TreeView();
    
    std::string GetTypeName() const override { return "TreeView"; }
    
    // 根节点操作
    void AddRoot(const std::shared_ptr<TreeViewItem>& item);
    void RemoveRoot(const std::shared_ptr<TreeViewItem>& item);
    void ClearRoots();
    size_t GetRootCount() const { return m_roots.size(); }
    std::shared_ptr<TreeViewItem> GetRoot(size_t index) {
        return (index < m_roots.size()) ? m_roots[index] : nullptr;
    }
    
    // 选中项
    std::shared_ptr<TreeViewItem> GetSelectedItem() const { return m_selectedItem.lock(); }
    void SetSelectedItem(const std::shared_ptr<TreeViewItem>& item);
    void ClearSelection();
    
    // 选中项变更事件
    luaui::Delegate<TreeView*, TreeViewItem*> SelectedItemChanged;
    
    // 展开/折叠事件
    luaui::Delegate<TreeViewItem*, bool> ItemExpandedChanged;
    
    // 外观属性
    float GetItemHeight() const { return m_itemHeight; }
    void SetItemHeight(float height) { m_itemHeight = height; }
    
    float GetIndentSize() const { return m_indentSize; }
    void SetIndentSize(float size) { m_indentSize = size; }
    
    // 是否显示连线
    bool GetShowLines() const { return m_showLines; }
    void SetShowLines(bool show) { m_showLines = show; }
    
    // 是否显示展开按钮
    bool GetShowExpandButtons() const { return m_showExpandButtons; }
    void SetShowExpandButtons(bool show) { m_showExpandButtons = show; }
    
    // 滚动
    void ScrollToItem(const std::shared_ptr<TreeViewItem>& item);
    
    // 查找项（递归搜索所有节点）
    std::shared_ptr<TreeViewItem> FindItem(const std::wstring& header);
    
    // 展开/折叠所有
    void ExpandAll();
    void CollapseAll();

protected:
    void InitializeComponents() override;
    rendering::Size OnMeasureChildren(const rendering::Size& availableSize) override;
    rendering::Size OnArrangeChildren(const rendering::Size& finalSize) override;
    void OnRenderChildren(rendering::IRenderContext* context) override;
    
    // 输入处理
    void OnMouseMove(MouseEventArgs& args) override;
    void OnMouseDown(MouseEventArgs& args) override;

private:
    friend class TreeViewItem;
    
    void OnItemSelected(TreeViewItem* item);
    void OnItemExpandedChanged(TreeViewItem* item, bool expanded);
    
    // 递归计算总高度
    float CalculateTotalHeight() const;
    
    // 渲染所有可见项
    void RenderItems(rendering::IRenderContext* context);
    
    // 命中测试
    TreeViewItem* HitTestItem(float x, float y);
    TreeViewItem* HitTestItemRecursive(const std::vector<std::shared_ptr<TreeViewItem>>& items, 
                                        float& currentY, float targetY);
    
    // 展开/折叠所有（递归）
    void ExpandAllRecursive(const std::vector<std::shared_ptr<TreeViewItem>>& items);
    void CollapseAllRecursive(const std::vector<std::shared_ptr<TreeViewItem>>& items);
    
    std::vector<std::shared_ptr<TreeViewItem>> m_roots;
    std::weak_ptr<TreeViewItem> m_selectedItem;
    TreeViewItem* m_hoveredItem = nullptr;
    
    // 滚动偏移
    float m_scrollOffset = 0.0f;
    
    // 外观
    float m_itemHeight = 28.0f;
    float m_indentSize = 20.0f;
    bool m_showLines = false;           // 是否显示层级连线
    bool m_showExpandButtons = true;    // 是否显示展开按钮
};

} // namespace controls
} // namespace luaui
