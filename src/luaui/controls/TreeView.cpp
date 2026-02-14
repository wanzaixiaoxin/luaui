#include "TreeView.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include "Components/InputComponent.h"
#include "Interfaces/IRenderable.h"
#include "Interfaces/ILayoutable.h"
#include "IRenderContext.h"

namespace luaui {
namespace controls {

// ============================================================================
// TreeViewItem
// ============================================================================
TreeViewItem::TreeViewItem() {}

void TreeViewItem::InitializeComponents() {
    GetComponents().AddComponent<components::LayoutComponent>(this);
    GetComponents().AddComponent<components::RenderComponent>(this);
    GetComponents().AddComponent<components::InputComponent>(this);
}

void TreeViewItem::SetHeader(const std::wstring& header) {
    if (m_header != header) {
        m_header = header;
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void TreeViewItem::SetIsExpanded(bool expanded) {
    if (m_isExpanded != expanded && !m_children.empty()) {
        m_isExpanded = expanded;
        if (m_treeView) {
            m_treeView->OnItemExpandedChanged(this, expanded);
        }
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void TreeViewItem::ToggleExpand() {
    SetIsExpanded(!m_isExpanded);
}

void TreeViewItem::SetIsSelected(bool selected) {
    if (m_isSelected != selected) {
        m_isSelected = selected;
        UpdateVisualState();
        if (m_treeView && selected) {
            m_treeView->OnItemSelected(this);
        }
    }
}

void TreeViewItem::AddItem(const std::shared_ptr<TreeViewItem>& item) {
    if (!item) return;
    
    item->m_parentItem = std::dynamic_pointer_cast<TreeViewItem>(shared_from_this());
    item->m_treeView = m_treeView;
    item->m_level = m_level + 1;
    
    // 递归设置子节点的 TreeView 引用
    std::function<void(const std::shared_ptr<TreeViewItem>&, TreeView*)> setTreeView;
    setTreeView = [&setTreeView](const std::shared_ptr<TreeViewItem>& node, TreeView* tv) {
        node->m_treeView = tv;
        for (auto& child : node->m_children) {
            setTreeView(child, tv);
        }
    };
    setTreeView(item, m_treeView);
    
    m_children.push_back(item);
    
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

void TreeViewItem::RemoveItem(const std::shared_ptr<TreeViewItem>& item) {
    auto it = std::find(m_children.begin(), m_children.end(), item);
    if (it != m_children.end()) {
        (*it)->m_parentItem.reset();
        m_children.erase(it);
        
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
    }
}

void TreeViewItem::ClearItems() {
    for (auto& child : m_children) {
        child->m_parentItem.reset();
    }
    m_children.clear();
    
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

float TreeViewItem::CalculateTotalHeight() const {
    float height = m_itemHeight;
    if (m_isExpanded) {
        for (const auto& child : m_children) {
            height += child->CalculateTotalHeight();
        }
    }
    return height;
}

std::shared_ptr<TreeViewItem> TreeViewItem::FindItem(const std::wstring& header) {
    if (m_header == header) {
        return std::dynamic_pointer_cast<TreeViewItem>(shared_from_this());
    }
    for (const auto& child : m_children) {
        if (auto found = child->FindItem(header)) {
            return found;
        }
    }
    return nullptr;
}

void TreeViewItem::OnClick() {
    // 点击展开按钮区域时切换展开状态
    // 否则选中该项
    // 这里简化处理，直接选中
    SetIsSelected(true);
}

void TreeViewItem::OnMouseEnter() {
    m_isHovered = true;
    UpdateVisualState();
}

void TreeViewItem::OnMouseLeave() {
    m_isHovered = false;
    UpdateVisualState();
}

void TreeViewItem::OnMouseDown(MouseEventArgs& args) {
    // 检查是否点击了展开按钮
    rendering::Rect rect;
    if (auto* renderable = AsRenderable()) {
        rect = renderable->GetRenderRect();
    }
    
    float expandButtonX = rect.x + m_level * m_indentSize + m_padding;
    if (GetHasItems() && 
        args.x >= expandButtonX && args.x <= expandButtonX + m_expandButtonSize &&
        args.y >= rect.y + (m_itemHeight - m_expandButtonSize) / 2 &&
        args.y <= rect.y + (m_itemHeight + m_expandButtonSize) / 2) {
        ToggleExpand();
    } else {
        SetIsSelected(true);
    }
    
    args.Handled = true;
}

void TreeViewItem::UpdateVisualState() {
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void TreeViewItem::DrawExpandButton(rendering::IRenderContext* context, const rendering::Rect& rect) {
    if (!GetHasItems()) return;
    
    auto brush = context->CreateSolidColorBrush(m_expandButtonColor);
    if (!brush) return;
    
    float centerX = rect.x + rect.width / 2;
    float centerY = rect.y + rect.height / 2;
    float size = 3.0f;
    
    if (m_isExpanded) {
        // 向下箭头（已展开）
        context->DrawLine(rendering::Point(centerX - size, centerY - size/2),
                          rendering::Point(centerX, centerY + size/2),
                          brush.get(), 1.0f);
        context->DrawLine(rendering::Point(centerX, centerY + size/2),
                          rendering::Point(centerX + size, centerY - size/2),
                          brush.get(), 1.0f);
    } else {
        // 向右箭头（已折叠）
        context->DrawLine(rendering::Point(centerX - size/2, centerY - size),
                          rendering::Point(centerX + size/2, centerY),
                          brush.get(), 1.0f);
        context->DrawLine(rendering::Point(centerX + size/2, centerY),
                          rendering::Point(centerX - size/2, centerY + size),
                          brush.get(), 1.0f);
    }
}

rendering::Size TreeViewItem::OnMeasure(const rendering::Size& availableSize) {
    (void)availableSize;
    // 固定高度，宽度根据父容器
    return rendering::Size(availableSize.width > 0 ? availableSize.width : 200, m_itemHeight);
}

void TreeViewItem::OnRender(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    auto rect = render->GetRenderRect();
    
    // 绘制背景
    rendering::Color bgColor = m_bgColor;
    if (m_isSelected) {
        bgColor = m_selectedColor;
    } else if (m_isHovered) {
        bgColor = m_hoverColor;
    }
    
    if (bgColor.a > 0) {
        auto bgBrush = context->CreateSolidColorBrush(bgColor);
        if (bgBrush) {
            context->FillRectangle(rect, bgBrush.get());
        }
    }
    
    // 绘制展开按钮
    if (GetHasItems()) {
        float buttonX = rect.x + m_level * m_indentSize + m_padding;
        float buttonY = rect.y + (rect.height - m_expandButtonSize) / 2;
        rendering::Rect buttonRect(buttonX, buttonY, m_expandButtonSize, m_expandButtonSize);
        DrawExpandButton(context, buttonRect);
    }
    
    // 绘制文本
    if (!m_header.empty()) {
        rendering::Color textColor = m_isSelected ? m_selectedTextColor : m_textColor;
        auto textBrush = context->CreateSolidColorBrush(textColor);
        auto textFormat = context->CreateTextFormat(L"Microsoft YaHei", m_fontSize);
        
        if (textBrush && textFormat) {
            float textX = rect.x + m_level * m_indentSize + m_expandButtonSize + m_padding * 2;
            float textY = rect.y + (rect.height - m_fontSize) / 2;
            context->DrawTextString(m_header, textFormat.get(), 
                                    rendering::Point(textX, textY), textBrush.get());
        }
    }
}

// ============================================================================
// TreeView
// ============================================================================
TreeView::TreeView() {}

void TreeView::InitializeComponents() {
    Panel::InitializeComponents();
}

void TreeView::AddRoot(const std::shared_ptr<TreeViewItem>& item) {
    if (!item) return;
    
    item->SetTreeView(this);
    item->SetLevel(0);
    
    // 递归设置子节点的 TreeView 引用
    std::function<void(const std::shared_ptr<TreeViewItem>&)> setTreeView;
    setTreeView = [this, &setTreeView](const std::shared_ptr<TreeViewItem>& node) {
        node->m_treeView = this;
        for (auto& child : node->m_children) {
            setTreeView(child);
        }
    };
    setTreeView(item);
    
    m_roots.push_back(item);
    AddChild(item);
    
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

void TreeView::RemoveRoot(const std::shared_ptr<TreeViewItem>& item) {
    auto it = std::find(m_roots.begin(), m_roots.end(), item);
    if (it != m_roots.end()) {
        RemoveChild(*it);
        m_roots.erase(it);
        
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
    }
}

void TreeView::ClearRoots() {
    for (auto& root : m_roots) {
        RemoveChild(root);
    }
    m_roots.clear();
    m_selectedItem.reset();
    
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

void TreeView::SetSelectedItem(const std::shared_ptr<TreeViewItem>& item) {
    // 清除之前的选中
    if (auto prev = m_selectedItem.lock()) {
        prev->SetIsSelected(false);
    }
    
    m_selectedItem = item;
    if (item) {
        item->SetIsSelected(true);
    }
}

void TreeView::ClearSelection() {
    if (auto prev = m_selectedItem.lock()) {
        prev->SetIsSelected(false);
    }
    m_selectedItem.reset();
}

void TreeView::OnItemSelected(TreeViewItem* item) {
    // 更新选中项
    if (auto prev = m_selectedItem.lock()) {
        if (prev.get() != item) {
            prev->SetIsSelected(false);
        }
    }
    m_selectedItem = std::dynamic_pointer_cast<TreeViewItem>(item->shared_from_this());
    
    SelectedItemChanged.Invoke(this, item);
}

void TreeView::OnItemExpandedChanged(TreeViewItem* item, bool expanded) {
    ItemExpandedChanged.Invoke(item, expanded);
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

float TreeView::CalculateTotalHeight() const {
    float height = 0;
    for (const auto& root : m_roots) {
        height += root->CalculateTotalHeight();
    }
    return height;
}

rendering::Size TreeView::OnMeasureChildren(const rendering::Size& availableSize) {
    // TreeView 使用自绘方式，不依赖子控件的测量
    float totalHeight = CalculateTotalHeight();
    return rendering::Size(availableSize.width > 0 ? availableSize.width : 200, totalHeight);
}

rendering::Size TreeView::OnArrangeChildren(const rendering::Size& finalSize) {
    // 递归排列所有可见项
    rendering::Rect contentRect;
    if (auto* renderable = AsRenderable()) {
        contentRect = renderable->GetRenderRect();
    }
    
    float y = contentRect.y - m_scrollOffset;
    
    std::function<void(const std::shared_ptr<TreeViewItem>&)> arrangeItem = [&](const std::shared_ptr<TreeViewItem>& item) {
        if (auto* layoutable = item->AsLayoutable()) {
            layoutable->Arrange(rendering::Rect(contentRect.x, y, contentRect.width, m_itemHeight));
            y += m_itemHeight;
            
            if (item->GetIsExpanded()) {
                for (const auto& child : item->GetChildren()) {
                    arrangeItem(child);
                }
            }
        }
    };
    
    for (const auto& root : m_roots) {
        arrangeItem(root);
    }
    
    return finalSize;
}

void TreeView::OnRenderChildren(rendering::IRenderContext* context) {
    if (!context) return;
    
    // 渲染所有可见项
    rendering::Rect contentRect;
    if (auto* renderable = AsRenderable()) {
        contentRect = renderable->GetRenderRect();
    }
    
    std::function<void(const std::shared_ptr<TreeViewItem>&)> renderItem;
    renderItem = [&contentRect, &renderItem, context](const std::shared_ptr<TreeViewItem>& item) {
        if (auto* itemRenderable = item->AsRenderable()) {
            auto itemRect = itemRenderable->GetRenderRect();
            
            // 可见性测试
            if (itemRect.y + itemRect.height > contentRect.y && 
                itemRect.y < contentRect.y + contentRect.height) {
                itemRenderable->Render(context);
            }
            
            // 递归渲染子项
            if (item->GetIsExpanded()) {
                for (const auto& child : item->GetChildren()) {
                    renderItem(child);
                }
            }
        }
    };
    
    for (const auto& root : m_roots) {
        renderItem(root);
    }
}

TreeViewItem* TreeView::HitTestItem(float x, float y) {
    (void)x;
    float currentY = 0;
    return HitTestItemRecursive(m_roots, currentY, y);
}

TreeViewItem* TreeView::HitTestItemRecursive(const std::vector<std::shared_ptr<TreeViewItem>>& items, 
                                              float& currentY, float targetY) {
    for (const auto& item : items) {
        if (targetY >= currentY && targetY < currentY + m_itemHeight) {
            return item.get();
        }
        currentY += m_itemHeight;
        
        if (item->GetIsExpanded()) {
            if (auto* found = HitTestItemRecursive(item->GetChildren(), currentY, targetY)) {
                return found;
            }
        }
    }
    return nullptr;
}



void TreeView::OnMouseMove(MouseEventArgs& args) {
    rendering::Rect contentRect;
    if (auto* renderable = AsRenderable()) {
        contentRect = renderable->GetRenderRect();
    }
    
    // 转换到内容坐标
    float localY = args.y - contentRect.y + m_scrollOffset;
    
    TreeViewItem* hitItem = HitTestItem(args.x, localY);
    
    // 更新悬停状态
    if (m_hoveredItem != hitItem) {
        if (m_hoveredItem) {
            m_hoveredItem->OnMouseLeave();
        }
        m_hoveredItem = hitItem;
        if (m_hoveredItem) {
            m_hoveredItem->OnMouseEnter();
        }
    }
    
    args.Handled = true;
}

void TreeView::OnMouseDown(MouseEventArgs& args) {
    rendering::Rect contentRect;
    if (auto* renderable = AsRenderable()) {
        contentRect = renderable->GetRenderRect();
    }
    
    float localY = args.y - contentRect.y + m_scrollOffset;
    
    if (TreeViewItem* item = HitTestItem(args.x, localY)) {
        MouseEventArgs localArgs = args;
        localArgs.y = localY;
        item->OnMouseDown(localArgs);
    }
    
    args.Handled = true;
}

std::shared_ptr<TreeViewItem> TreeView::FindItem(const std::wstring& header) {
    for (const auto& root : m_roots) {
        if (auto found = root->FindItem(header)) {
            return found;
        }
    }
    return nullptr;
}

void TreeView::ExpandAll() {
    ExpandAllRecursive(m_roots);
}

void TreeView::ExpandAllRecursive(const std::vector<std::shared_ptr<TreeViewItem>>& items) {
    for (const auto& item : items) {
        if (item->GetHasItems()) {
            item->SetIsExpanded(true);
            ExpandAllRecursive(item->GetChildren());
        }
    }
}

void TreeView::CollapseAll() {
    CollapseAllRecursive(m_roots);
}

void TreeView::CollapseAllRecursive(const std::vector<std::shared_ptr<TreeViewItem>>& items) {
    for (const auto& item : items) {
        if (item->GetHasItems()) {
            item->SetIsExpanded(false);
            CollapseAllRecursive(item->GetChildren());
        }
    }
}

void TreeView::ScrollToItem(const std::shared_ptr<TreeViewItem>& item) {
    (void)item;
    // 计算目标项的 Y 位置并调整滚动偏移
    // 简化实现
}

} // namespace controls
} // namespace luaui
