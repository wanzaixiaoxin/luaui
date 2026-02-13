#include "ListBox.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include "Components/InputComponent.h"
#include "Interfaces/IRenderable.h"
#include "Interfaces/ILayoutable.h"
#include "IRenderContext.h"

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

rendering::Size ListBoxItem::OnMeasure(const rendering::Size& availableSize) {
    (void)availableSize;
    // 简单测量：固定高度，宽度根据内容或可用空间
    return rendering::Size(availableSize.width > 0 ? availableSize.width : 100, m_itemHeight);
}

void ListBoxItem::OnClick() {
    // 通知父 ListBox 选中此项
    if (auto parent = GetParent()) {
        if (auto* listBox = dynamic_cast<ListBox*>(parent.get())) {
            listBox->SetSelectedIndex(m_index);
        }
    }
}

void ListBoxItem::OnRender(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    auto rect = render->GetRenderRect();
    
    // 选择背景色
    rendering::Color bgColor = m_normalBg;
    if (m_isSelected) {
        bgColor = m_selectedBg;
    } else if (m_isHovered) {
        bgColor = m_hoverBg;
    }
    
    // 绘制背景
    auto bgBrush = context->CreateSolidColorBrush(bgColor);
    if (bgBrush) {
        context->FillRectangle(rect, bgBrush.get());
    }
    
    // 绘制文本
    if (!m_content.empty()) {
        rendering::Color textColor = m_isSelected ? m_selectedTextColor : m_textColor;
        auto textBrush = context->CreateSolidColorBrush(textColor);
        auto textFormat = context->CreateTextFormat(L"Microsoft YaHei", m_fontSize);
        
        if (textBrush && textFormat) {
            rendering::Point textPos(8, (rect.height - m_fontSize) / 2);
            context->DrawTextString(m_content, textFormat.get(), textPos, textBrush.get());
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
    item->m_index = static_cast<int>(m_items.size());
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

int ListBox::HitTestItem(float x, float y) {
    (void)x;
    rendering::Rect contentRect;
    if (auto* renderable = AsRenderable()) {
        contentRect = renderable->GetRenderRect();
    }
    
    float itemY = contentRect.y - m_scrollOffset;
    for (size_t i = 0; i < m_items.size(); ++i) {
        if (auto* layoutable = m_items[i]->AsLayoutable()) {
            auto desired = layoutable->GetDesiredSize();
            if (y >= itemY && y < itemY + desired.height) {
                return static_cast<int>(i);
            }
            itemY += desired.height;
        }
    }
    return -1;
}

void ListBox::OnClick() {
    // 点击处理通过子项的点击事件处理
    // 这里可以添加额外的点击逻辑
}

void ListBox::OnMouseMove(MouseEventArgs& args) {
    // 更新悬停状态
    int index = HitTestItem(args.x, args.y);
    for (size_t i = 0; i < m_items.size(); ++i) {
        m_items[i]->SetIsHovered(static_cast<int>(i) == index);
    }
    
    args.Handled = true;
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
