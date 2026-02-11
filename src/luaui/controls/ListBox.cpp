#include "ListBox.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include "Components/InputComponent.h"
#include "Interfaces/IRenderable.h"
#include "Interfaces/ILayoutable.h"

namespace luaui {
namespace controls {

// ============================================================================
// ListBoxItem
// ============================================================================
ListBoxItem::ListBoxItem() {}

void ListBoxItem::InitializeComponents() {
    GetComponents().AddComponent<components::LayoutComponent>(this);
    GetComponents().AddComponent<components::RenderComponent>(this);
    GetComponents().AddComponent<components::InputComponent>(this);
    
    if (auto* layout = GetLayout()) {
        layout->SetHeight(m_itemHeight);
    }
}

void ListBoxItem::SetContent(const std::wstring& content) {
    m_content = content;
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

void ListBoxItem::SetIsSelected(bool selected) {
    if (m_isSelected != selected) {
        m_isSelected = selected;
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void ListBoxItem::SetIsHovered(bool hovered) {
    if (m_isHovered != hovered) {
        m_isHovered = hovered;
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

// ============================================================================
// ListBox
// ============================================================================
ListBox::ListBox() {
    m_itemHeight = 24.0f;
}

void ListBox::InitializeComponents() {
    Panel::InitializeComponents();
    
    // 设置裁剪子控件
    // SetClipChildren(true);
}

void ListBox::AddItem(const std::wstring& item) {
    auto listItem = std::make_shared<ListBoxItem>();
    listItem->SetContent(item);
    AddItem(listItem);
}

void ListBox::AddItem(const std::shared_ptr<ListBoxItem>& item) {
    m_items.push_back(item);
    AddChild(item);
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

void ListBox::RemoveItem(int index) {
    if (index >= 0 && index < static_cast<int>(m_items.size())) {
        RemoveChild(m_items[index]);
        m_items.erase(m_items.begin() + index);
        if (m_selectedIndex == index) {
            m_selectedIndex = -1;
        } else if (m_selectedIndex > index) {
            m_selectedIndex--;
        }
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
    }
}

void ListBox::ClearItems() {
    for (auto& item : m_items) {
        RemoveChild(item);
    }
    m_items.clear();
    m_selectedIndex = -1;
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

std::shared_ptr<ListBoxItem> ListBox::GetItem(int index) {
    if (index >= 0 && index < static_cast<int>(m_items.size())) {
        return m_items[index];
    }
    return nullptr;
}

void ListBox::SetSelectedIndex(int index) {
    if (m_selectedIndex != index) {
        m_selectedIndex = index;
        UpdateItemStates();
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
        SelectionChanged.Invoke(this, index);
    }
}

std::wstring ListBox::GetSelectedItem() const {
    if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_items.size())) {
        return m_items[m_selectedIndex]->GetContent();
    }
    return L"";
}

void ListBox::UpdateItemStates() {
    for (size_t i = 0; i < m_items.size(); ++i) {
        m_items[i]->SetIsSelected(static_cast<int>(i) == m_selectedIndex);
    }
}

rendering::Size ListBox::OnMeasureChildren(const rendering::Size& availableSize) {
    float totalHeight = 0;
    float maxWidth = 0;
    
    for (auto& item : m_items) {
        if (auto* layoutable = item->AsLayoutable()) {
            interfaces::LayoutConstraint constraint;
            constraint.available = availableSize;
            layoutable->Measure(constraint);
            auto desired = layoutable->GetDesiredSize();
            totalHeight += desired.height;
            maxWidth = std::max(maxWidth, desired.width);
        }
    }
    
    return rendering::Size(maxWidth, totalHeight);
}

rendering::Size ListBox::OnArrangeChildren(const rendering::Size& finalSize) {
    rendering::Rect contentRect;
    if (auto* renderable = AsRenderable()) {
        contentRect = renderable->GetRenderRect();
    }
    
    float y = contentRect.y - m_scrollOffset;
    for (auto& item : m_items) {
        if (auto* layoutable = item->AsLayoutable()) {
            auto desired = layoutable->GetDesiredSize();
            layoutable->Arrange(rendering::Rect(contentRect.x, y, 
                                                 desired.width, 
                                                 desired.height));
            y += desired.height;
        }
    }
    
    return finalSize;
}

void ListBox::OnRenderChildren(rendering::IRenderContext* context) {
    if (!context) return;
    
    rendering::Rect contentRect;
    if (auto* renderable = AsRenderable()) {
        contentRect = renderable->GetRenderRect();
    }
    
    for (auto& item : m_items) {
        if (auto* layoutable = item->AsLayoutable()) {
            auto* itemRenderable = item->AsRenderable();
            if (!itemRenderable) continue;
            
            auto itemRect = itemRenderable->GetRenderRect();
            
            // 简单的可见性测试
            if (itemRect.y + itemRect.height > contentRect.y && 
                itemRect.y < contentRect.y + contentRect.height) {
                itemRenderable->Render(context);
            }
        }
    }
}

} // namespace controls
} // namespace luaui
