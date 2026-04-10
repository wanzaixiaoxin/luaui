#include "TreeView.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include "Components/InputComponent.h"
#include "Interfaces/IRenderable.h"
#include "Interfaces/ILayoutable.h"
#include "IRenderContext.h"
#include "Theme.h"
#include "Window.h"
#include <windows.h> // For VK_ constants

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
    AnimateBgTo(GetTargetBgColor(), 150.0f);
}

void TreeViewItem::OnMouseLeave() {
    m_isHovered = false;
    AnimateBgTo(GetTargetBgColor(), 150.0f);
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

void TreeViewItem::ApplyTheme() {
    auto& t = Theme::GetCurrent();
    using namespace theme;
    m_hoverColor = t.GetColor(kTreeViewItemHoverBg);
    m_selectedColor = t.GetColor(kTreeViewItemSelectedBg);
    m_textColor = t.GetColor(kTreeViewItemText);
    m_selectedTextColor = t.GetColor(kTreeViewItemSelectedText);
    m_expandButtonColor = t.GetColor(kTreeViewExpandBtn);
    m_animBg = m_bgColor;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

rendering::Color TreeViewItem::GetTargetBgColor() const {
    if (m_isSelected) return m_selectedColor;
    if (m_isHovered) return m_hoverColor;
    return m_bgColor;
}

void TreeViewItem::AnimateBgTo(const rendering::Color& target, float durationMs) {
    auto* wnd = GetWindow();
    if (!wnd || !wnd->GetTimeline()) {
        m_animBg = target;
        if (auto* render = GetRender()) render->Invalidate();
        return;
    }

    auto anim = wnd->GetTimeline()->CreateAnimation();
    anim->SetDuration(durationMs);
    anim->SetEasing(rendering::Easing::CubicOut);
    anim->SetFillMode(rendering::FillMode::Forwards);

    rendering::Color start = m_animBg;
    anim->SetStartValue(rendering::AnimationValue(start));
    anim->SetEndValue(rendering::AnimationValue(target));

    anim->SetUpdateCallback([this](const rendering::AnimationValue& val) {
        m_animBg = val.AsColor();
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    });

    anim->Play();
    wnd->GetTimeline()->Add(std::move(anim));
    wnd->StartAnimTimer();
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
    
    // 绘制背景（使用动画颜色）
    rendering::Color bgColor = m_animBg;
    
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

// ============================================================================
// 键盘导航
// ============================================================================
void TreeView::OnKeyDown(KeyEventArgs& args) {
    switch (args.keyCode) {
        case VK_UP:
            NavigateUp();
            args.Handled = true;
            break;
        case VK_DOWN:
            NavigateDown();
            args.Handled = true;
            break;
        case VK_LEFT:
            NavigateLeft();
            args.Handled = true;
            break;
        case VK_RIGHT:
            NavigateRight();
            args.Handled = true;
            break;
        case VK_RETURN:
            SelectCurrent();
            args.Handled = true;
            break;
        case VK_SPACE:
            ToggleCurrent();
            args.Handled = true;
            break;
        case VK_HOME:
            // 选中第一项
            if (!m_roots.empty()) {
                SetSelectedItem(m_roots[0]);
            }
            args.Handled = true;
            break;
        case VK_END:
            // 选中最后一项（递归找到最后一个可见项）
            if (!m_roots.empty()) {
                SetSelectedItem(GetLastVisibleItem());
            }
            args.Handled = true;
            break;
    }
}

void TreeView::NavigateUp() {
    auto current = m_selectedItem.lock();
    if (!current) {
        // 没有选中项，选中最后一项
        if (!m_roots.empty()) {
            SetSelectedItem(GetLastVisibleItem());
        }
        return;
    }
    
    auto prev = GetPreviousVisibleItem(current);
    if (prev) {
        SetSelectedItem(prev);
    }
}

void TreeView::NavigateDown() {
    auto current = m_selectedItem.lock();
    if (!current) {
        // 没有选中项，选中第一项
        if (!m_roots.empty()) {
            SetSelectedItem(m_roots[0]);
        }
        return;
    }
    
    auto next = GetNextVisibleItem(current);
    if (next) {
        SetSelectedItem(next);
    }
}

void TreeView::NavigateLeft() {
    auto current = m_selectedItem.lock();
    if (!current) return;
    
    if (current->GetIsExpanded() && current->GetHasItems()) {
        // 已展开，折叠它
        current->SetIsExpanded(false);
    } else {
        // 已折叠或没有子项，移动到父节点
        auto parent = current->GetParentItem();
        if (parent) {
            SetSelectedItem(parent);
        }
    }
}

void TreeView::NavigateRight() {
    auto current = m_selectedItem.lock();
    if (!current) return;
    
    if (!current->GetIsExpanded() && current->GetHasItems()) {
        // 未展开，展开它
        current->SetIsExpanded(true);
    } else if (current->GetIsExpanded() && current->GetHasItems()) {
        // 已展开，移动到第一个子节点
        SetSelectedItem(current->GetChildren()[0]);
    }
}

void TreeView::SelectCurrent() {
    auto current = m_selectedItem.lock();
    if (current) {
        SelectedItemChanged.Invoke(this, current.get());
    }
}

void TreeView::ToggleCurrent() {
    auto current = m_selectedItem.lock();
    if (current && current->GetHasItems()) {
        current->ToggleExpand();
    }
}

// 辅助函数：获取下一个可见项
std::shared_ptr<TreeViewItem> TreeView::GetNextVisibleItem(const std::shared_ptr<TreeViewItem>& item) {
    if (!item) return nullptr;
    
    // 如果有子项且已展开，返回第一个子项
    if (item->GetIsExpanded() && item->GetHasItems()) {
        return item->GetChildren()[0];
    }
    
    // 向上查找兄弟或叔伯
    auto current = item;
    while (current) {
        auto parent = current->GetParentItem();
        if (parent) {
            // 找下一个兄弟
            const auto& siblings = parent->GetChildren();
            for (size_t i = 0; i < siblings.size(); ++i) {
                if (siblings[i] == current && i + 1 < siblings.size()) {
                    return siblings[i + 1];
                }
            }
            // 没有下一个兄弟，继续向上
            current = parent;
        } else {
            // 根节点，找下一个根
            for (size_t i = 0; i < m_roots.size(); ++i) {
                if (m_roots[i] == current && i + 1 < m_roots.size()) {
                    return m_roots[i + 1];
                }
            }
            break;
        }
    }
    return nullptr;
}

// 辅助函数：获取上一个可见项
std::shared_ptr<TreeViewItem> TreeView::GetPreviousVisibleItem(const std::shared_ptr<TreeViewItem>& item) {
    if (!item) return nullptr;
    
    auto parent = item->GetParentItem();
    if (parent) {
        // 找前一个兄弟
        const auto& siblings = parent->GetChildren();
        for (size_t i = 0; i < siblings.size(); ++i) {
            if (siblings[i] == item && i > 0) {
                // 返回前一个兄弟的最后一个可见后代
                return GetLastVisibleDescendant(siblings[i - 1]);
            }
        }
        // 没有前一个兄弟，返回父节点
        return parent;
    } else {
        // 根节点，找前一个根
        for (size_t i = 0; i < m_roots.size(); ++i) {
            if (m_roots[i] == item && i > 0) {
                return GetLastVisibleDescendant(m_roots[i - 1]);
            }
        }
    }
    return nullptr;
}

// 辅助函数：获取最后一个可见项
std::shared_ptr<TreeViewItem> TreeView::GetLastVisibleItem() {
    if (m_roots.empty()) return nullptr;
    return GetLastVisibleDescendant(m_roots.back());
}

// 辅助函数：获取最后一个可见后代
std::shared_ptr<TreeViewItem> TreeView::GetLastVisibleDescendant(const std::shared_ptr<TreeViewItem>& item) {
    if (!item) return nullptr;
    
    if (item->GetIsExpanded() && item->GetHasItems()) {
        return GetLastVisibleDescendant(item->GetChildren().back());
    }
    return item;
}

} // namespace controls
} // namespace luaui
