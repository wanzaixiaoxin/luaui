#include "ListBox.h"
#include "../rendering/IRenderContext.h"
#include "../utils/Logger.h"
#include "../core/Control.h"
#include "../core/Components/LayoutComponent.h"
#include "../core/Components/RenderComponent.h"
#include "../core/Components/InputComponent.h"
#include <algorithm>

namespace luaui {
namespace controls {

// ============================================================================
// ListBoxItem 实现
// ============================================================================
ListBoxItem::ListBoxItem() {
    InitializeComponents();
}

void ListBoxItem::InitializeComponents() {
    // 添加布局组件
    auto* layout = GetComponents().AddComponent<components::LayoutComponent>(this);
    layout->SetWidth(100);
    layout->SetHeight(m_itemHeight);
    
    // 添加渲染组件
    GetComponents().AddComponent<components::RenderComponent>(this);
    
    // 添加输入组件
    GetComponents().AddComponent<components::InputComponent>(this);
}

void ListBoxItem::SetContent(const std::wstring& content) {
    if (m_content != content) {
        m_content = content;
        if (auto* render = GetRender()) render->Invalidate();
    }
}

void ListBoxItem::SetIsSelected(bool selected) {
    if (m_isSelected != selected) {
        m_isSelected = selected;
        if (auto* render = GetRender()) render->Invalidate();
    }
}

void ListBoxItem::SetIsHovered(bool hovered) {
    if (m_isHovered != hovered) {
        m_isHovered = hovered;
        if (auto* render = GetRender()) render->Invalidate();
    }
}

void ListBoxItem::SetItemHeight(float height) {
    m_itemHeight = height;
    if (auto* layout = GetLayout()) {
        layout->SetHeight(height);
    }
}

void ListBoxItem::OnRender(rendering::IRenderContext* context) {
    // 根据状态选择背景色
    rendering::Color bgColor = m_normalBg;
    rendering::Color textColor = m_textColor;
    
    if (m_isSelected) {
        bgColor = m_selectedBg;
        textColor = m_selectedTextColor;
    } else if (m_isHovered) {
        bgColor = m_hoverBg;
    }
    
    auto* render = GetRender();
    if (!render) return;
    
    auto bounds = render->GetRenderRect();
    
    // 绘制背景 - 使用刷子
    auto bgBrush = context->CreateSolidColorBrush(bgColor);
    if (bgBrush) {
        context->FillRectangle(bounds, bgBrush.get());
    }
    
    // 绘制文本
    if (!m_content.empty()) {
        auto textBrush = context->CreateSolidColorBrush(textColor);
        auto textFormat = context->CreateTextFormat(L"Microsoft YaHei", m_fontSize);
        if (textBrush && textFormat) {
            textFormat->SetTextAlignment(rendering::TextAlignment::Leading);
            textFormat->SetParagraphAlignment(rendering::ParagraphAlignment::Center);
            
            // 添加一些左边距
            rendering::Rect textBounds = bounds;
            textBounds.x += 8;
            textBounds.width -= 16;
            context->DrawTextString(m_content, textFormat.get(), textBounds, textBrush.get());
        }
    }
}

rendering::Size ListBoxItem::OnMeasure(const rendering::Size& availableSize) {
    return rendering::Size(availableSize.width, m_itemHeight);
}

void ListBoxItem::OnClick() {
    // 处理点击 - 由 ListBox 处理
}

// ============================================================================
// ListBox 实现
// ============================================================================
ListBox::ListBox() {
    utils::Logger::Debug("[ListBox] Created");
}

ListBox::~ListBox() {
    CleanupVirtualization();
}

void ListBox::SetIsVirtualizing(bool enable) {
    if (m_isVirtualizing != enable) {
        m_isVirtualizing = enable;
        
        if (m_isVirtualizing) {
            InitializeVirtualization();
        } else {
            CleanupVirtualization();
        }
        
        if (auto* layout = GetLayout()) layout->InvalidateMeasure();
    }
}

void ListBox::SetItemHeight(float height) {
    if (m_itemHeight != height) {
        m_itemHeight = height;
        
        if (m_isVirtualizing && m_virtualizingPanel) {
            m_virtualizingPanel->SetItemHeight(height);
        }
        
        if (auto* layout = GetLayout()) layout->InvalidateMeasure();
    }
}

void ListBox::InitializeVirtualization() {
    if (m_isVirtualizationInitialized) return;
    
    utils::Logger::Info("[ListBox] Initializing virtualization");
    
    // 创建虚拟化面板
    m_virtualizingPanel = std::make_shared<VirtualizingPanel>();
    m_virtualizingPanel->SetItemHeight(m_itemHeight);
    m_virtualizingPanel->SetVirtualizationMode(VirtualizationMode::Recycling);
    
    // 设置容器工厂
    auto self = this;
    m_virtualizingPanel->SetContainerFactory([self]() -> std::shared_ptr<Control> {
        return self->CreateItemContainer();
    });
    
    // 设置数据回调
    m_virtualizingPanel->SetDataCallback([self](int index) -> std::any {
        if (self->m_dataSource && index >= 0 && index < self->m_dataSourceCount) {
            return self->m_dataSource(index);
        }
        return std::any();
    });
    
    // 添加虚拟化面板作为子控件
    AddChild(m_virtualizingPanel);
    
    m_isVirtualizationInitialized = true;
}

void ListBox::CleanupVirtualization() {
    if (!m_isVirtualizationInitialized) return;
    
    utils::Logger::Info("[ListBox] Cleaning up virtualization");
    
    if (m_virtualizingPanel) {
        RemoveChild(m_virtualizingPanel);
        m_virtualizingPanel.reset();
    }
    
    m_isVirtualizationInitialized = false;
}

std::shared_ptr<Control> ListBox::CreateItemContainer() {
    auto item = std::make_shared<ListBoxItem>();
    item->SetItemHeight(m_itemHeight);
    return item;
}

void ListBox::BindItemToContainer(std::shared_ptr<Control> container, int index) {
    auto item = std::dynamic_pointer_cast<ListBoxItem>(container);
    if (!item) return;
    
    // 绑定数据
    if (m_dataSource && index >= 0 && index < m_dataSourceCount) {
        std::wstring text = m_dataSource(index);
        item->SetContent(text);
    }
    
    // 设置选中状态
    item->SetIsSelected(index == m_selectedIndex);
    
    // 设置索引
    item->m_index = index;
}

void ListBox::SetDataSource(int count, DataSourceCallback callback) {
    if (!m_isVirtualizing) {
        SetIsVirtualizing(true);
    }
    
    m_dataSourceCount = count;
    m_dataSource = callback;
    
    if (m_virtualizingPanel) {
        m_virtualizingPanel->SetItemCount(count);
    }
    
    // 更新滚动范围
    float totalHeight = count * m_itemHeight;
    float viewportHeight = GetLayout() ? GetLayout()->GetHeight() : 100.0f;
    m_maxScrollOffset = std::max(0.0f, totalHeight - viewportHeight);
    ClampScrollOffset();
    
    if (auto* layout = GetLayout()) layout->InvalidateMeasure();
}

void ListBox::RefreshData() {
    if (m_isVirtualizing && m_virtualizingPanel) {
        m_virtualizingPanel->SetItemCount(m_dataSourceCount);
        if (auto* layout = GetLayout()) layout->InvalidateMeasure();
    }
}

void ListBox::ScrollIntoView(int index) {
    if (index < 0 || index >= static_cast<int>(GetItemCount())) return;
    
    if (m_isVirtualizing && m_virtualizingPanel) {
        m_virtualizingPanel->ScrollIntoView(index);
    } else {
        // 传统模式：计算滚动位置
        float targetOffset = index * m_itemHeight;
        float viewportHeight = GetLayout() ? GetLayout()->GetHeight() : 100.0f;
        
        if (targetOffset < m_scrollOffset) {
            UpdateScrollOffset(targetOffset);
        } else if (targetOffset + m_itemHeight > m_scrollOffset + viewportHeight) {
            UpdateScrollOffset(targetOffset + m_itemHeight - viewportHeight);
        }
    }
}

// ============================================================================
// 传统 API 实现
// ============================================================================
void ListBox::AddItem(const std::wstring& item) {
    auto listItem = std::make_shared<ListBoxItem>();
    listItem->SetContent(item);
    listItem->SetItemHeight(m_itemHeight);
    AddItem(listItem);
}

void ListBox::AddItem(const std::shared_ptr<ListBoxItem>& item) {
    if (m_isVirtualizing) {
        utils::Logger::Warning("[ListBox] AddItem not supported in virtualizing mode. Use SetDataSource instead.");
        return;
    }
    
    item->m_index = static_cast<int>(m_items.size());
    m_items.push_back(item);
    AddChild(item);
    
    if (auto* layout = GetLayout()) layout->InvalidateMeasure();
}

void ListBox::InsertItem(int index, const std::wstring& item) {
    if (m_isVirtualizing) {
        utils::Logger::Warning("[ListBox] InsertItem not supported in virtualizing mode.");
        return;
    }
    
    if (index < 0 || index > static_cast<int>(m_items.size())) return;
    
    auto listItem = std::make_shared<ListBoxItem>();
    listItem->SetContent(item);
    listItem->SetItemHeight(m_itemHeight);
    listItem->m_index = index;
    
    m_items.insert(m_items.begin() + index, listItem);
    AddChild(listItem);
    
    // 更新后续项的索引
    for (size_t i = index + 1; i < m_items.size(); ++i) {
        m_items[i]->m_index = static_cast<int>(i);
    }
    
    if (auto* layout = GetLayout()) layout->InvalidateMeasure();
}

void ListBox::RemoveItem(int index) {
    if (m_isVirtualizing) {
        utils::Logger::Warning("[ListBox] RemoveItem not supported in virtualizing mode.");
        return;
    }
    
    if (index < 0 || index >= static_cast<int>(m_items.size())) return;
    
    auto item = m_items[index];
    RemoveChild(item);
    m_items.erase(m_items.begin() + index);
    
    // 更新选中索引
    if (m_selectedIndex == index) {
        m_selectedIndex = -1;
    } else if (m_selectedIndex > index) {
        --m_selectedIndex;
    }
    
    // 更新后续项的索引
    for (size_t i = index; i < m_items.size(); ++i) {
        m_items[i]->m_index = static_cast<int>(i);
    }
    
    if (auto* layout = GetLayout()) layout->InvalidateMeasure();
}

void ListBox::ClearItems() {
    if (m_isVirtualizing) {
        SetDataSource(0, nullptr);
        return;
    }
    
    for (auto& item : m_items) {
        RemoveChild(item);
    }
    m_items.clear();
    m_selectedIndex = -1;
    
    if (auto* layout = GetLayout()) layout->InvalidateMeasure();
}

size_t ListBox::GetItemCount() const {
    if (m_isVirtualizing) {
        return m_dataSourceCount;
    }
    return m_items.size();
}

std::shared_ptr<ListBoxItem> ListBox::GetItem(int index) {
    if (index < 0 || index >= static_cast<int>(m_items.size())) return nullptr;
    return m_items[index];
}

void ListBox::SetSelectedIndex(int index) {
    if (m_selectedIndex == index) return;
    
    // 清除旧选中状态
    if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_items.size())) {
        if (m_isVirtualizing) {
            // 虚拟化模式下需要刷新
            RefreshData();
        } else if (m_selectedIndex < static_cast<int>(m_items.size())) {
            m_items[m_selectedIndex]->SetIsSelected(false);
        }
    }
    
    m_selectedIndex = index;
    
    // 设置新选中状态
    if (m_selectedIndex >= 0) {
        if (m_isVirtualizing) {
            RefreshData();
        } else if (m_selectedIndex < static_cast<int>(m_items.size())) {
            m_items[m_selectedIndex]->SetIsSelected(true);
        }
    }
    
    SelectionChanged.Invoke(this, m_selectedIndex);
    if (auto* layout = GetLayout()) layout->InvalidateMeasure();
}

std::wstring ListBox::GetSelectedItem() const {
    if (m_selectedIndex < 0) return L"";
    
    if (m_isVirtualizing && m_dataSource) {
        return m_dataSource(m_selectedIndex);
    } else if (m_selectedIndex < static_cast<int>(m_items.size())) {
        return m_items[m_selectedIndex]->GetContent();
    }
    
    return L"";
}

// ============================================================================
// 布局实现
// ============================================================================
void ListBox::InitializeComponents() {
    Panel::InitializeComponents();
}

rendering::Size ListBox::OnMeasure(const rendering::Size& availableSize) {
    if (m_isVirtualizing && m_virtualizingPanel) {
        auto* layout = m_virtualizingPanel->GetLayout();
        if (layout) {
            interfaces::LayoutConstraint constraint;
            constraint.available = availableSize;
            return layout->Measure(constraint);
        }
    }
    
    // 传统模式
    float totalHeight = m_items.size() * m_itemHeight;
    return rendering::Size(availableSize.width, std::min(totalHeight, availableSize.height));
}

rendering::Size ListBox::OnArrangeChildren(const rendering::Size& finalSize) {
    if (m_isVirtualizing && m_virtualizingPanel) {
        auto* layout = m_virtualizingPanel->GetLayout();
        if (layout) {
            layout->SetWidth(finalSize.width);
            layout->SetHeight(finalSize.height);
            layout->Arrange(rendering::Rect(0, 0, finalSize.width, finalSize.height));
        }
        return finalSize;
    }
    
    // 传统模式
    float y = -m_scrollOffset;
    for (auto& item : m_items) {
        auto* layout = item->GetLayout();
        if (layout) {
            layout->SetWidth(finalSize.width);
            layout->SetHeight(m_itemHeight);
            layout->Arrange(rendering::Rect(0, y, finalSize.width, m_itemHeight));
        }
        y += m_itemHeight;
    }
    
    // 计算最大滚动偏移
    float totalHeight = m_items.size() * m_itemHeight;
    m_maxScrollOffset = std::max(0.0f, totalHeight - finalSize.height);
    ClampScrollOffset();
    
    return finalSize;
}

void ListBox::OnRenderChildren(rendering::IRenderContext* context) {
    if (m_isVirtualizing && m_virtualizingPanel) {
        auto* render = m_virtualizingPanel->GetRender();
        if (render) {
            render->Render(context);
        }
        return;
    }
    
    // 传统模式 - 只渲染可见项（简单裁剪）
    auto* layout = GetLayout();
    float viewportHeight = layout ? layout->GetHeight() : 0;
    
    for (auto& item : m_items) {
        auto* render = item->GetRender();
        if (!render) continue;
        
        auto bounds = render->GetRenderRect();
        // 简单可见性检查
        if (bounds.y + bounds.height > 0 && bounds.y < viewportHeight) {
            render->Render(context);
        }
    }
}

// ============================================================================
// 输入处理
// ============================================================================
void ListBox::OnClick() {
    // 命中测试 - 简化实现
    // 实际应该从 MouseEventArgs 获取鼠标位置
    utils::Logger::Debug("[ListBox] OnClick");
}

void ListBox::OnMouseMove(MouseEventArgs& args) {
    // 更新悬停状态
    if (!m_isVirtualizing) {
        int hoverIndex = HitTestItem(args.x, args.y);
        for (size_t i = 0; i < m_items.size(); ++i) {
            m_items[i]->SetIsHovered(static_cast<int>(i) == hoverIndex);
        }
    }
}

void ListBox::OnMouseWheel(MouseEventArgs& args) {
    // 简化实现：使用固定滚动量
    float delta = (args.y > 0 ? 1 : -1) * m_itemHeight * 3;  // 一次滚动 3 行
    UpdateScrollOffset(m_scrollOffset - delta);
}

void ListBox::OnKeyDown(KeyEventArgs& args) {
    // 键盘导航简化实现
    switch (args.keyCode) {
        case 38: // Up arrow (VK_UP)
            if (m_selectedIndex > 0) {
                SetSelectedIndex(m_selectedIndex - 1);
                ScrollIntoView(m_selectedIndex);
            }
            break;
        case 40: // Down arrow (VK_DOWN)
            if (m_selectedIndex < static_cast<int>(GetItemCount()) - 1) {
                SetSelectedIndex(m_selectedIndex + 1);
                ScrollIntoView(m_selectedIndex);
            }
            break;
        case 33: // Page Up (VK_PRIOR)
            {
                float viewportHeight = GetLayout() ? GetLayout()->GetHeight() : 100.0f;
                int pageSize = std::max(1, static_cast<int>(viewportHeight / m_itemHeight));
                int newIndex = std::max(0, m_selectedIndex - pageSize);
                SetSelectedIndex(newIndex);
                ScrollIntoView(m_selectedIndex);
            }
            break;
        case 34: // Page Down (VK_NEXT)
            {
                float viewportHeight = GetLayout() ? GetLayout()->GetHeight() : 100.0f;
                int pageSize = std::max(1, static_cast<int>(viewportHeight / m_itemHeight));
                int newIndex = std::min(static_cast<int>(GetItemCount()) - 1, m_selectedIndex + pageSize);
                SetSelectedIndex(newIndex);
                ScrollIntoView(m_selectedIndex);
            }
            break;
        case 36: // Home (VK_HOME)
            SetSelectedIndex(0);
            ScrollIntoView(0);
            break;
        case 35: // End (VK_END)
            SetSelectedIndex(static_cast<int>(GetItemCount()) - 1);
            ScrollIntoView(m_selectedIndex);
            break;
    }
    args.Handled = true;
}

int ListBox::HitTestItem(float x, float y) {
    (void)x;
    float adjustedY = y + m_scrollOffset;
    int index = static_cast<int>(adjustedY / m_itemHeight);
    
    if (index >= 0 && index < static_cast<int>(m_items.size())) {
        return index;
    }
    return -1;
}

int ListBox::VirtualizedHitTest(float y) {
    // 考虑滚动偏移
    float adjustedY = y + m_scrollOffset;
    int index = static_cast<int>(adjustedY / m_itemHeight);
    
    if (index >= 0 && index < m_dataSourceCount) {
        return index;
    }
    return -1;
}

void ListBox::UpdateItemStates() {
    // 空实现 - 状态更新已在 OnMouseMove 中处理
}

void ListBox::UpdateScrollOffset(float newOffset) {
    m_scrollOffset = newOffset;
    ClampScrollOffset();
    if (auto* layout = GetLayout()) layout->InvalidateMeasure();
}

void ListBox::ClampScrollOffset() {
    m_scrollOffset = std::clamp(m_scrollOffset, 0.0f, m_maxScrollOffset);
}

} // namespace controls
} // namespace luaui
