#include "TabControl.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include "Interfaces/IRenderable.h"
#include "Interfaces/ILayoutable.h"
#include "IRenderContext.h"

namespace luaui {
namespace controls {

// ============================================================================
// TabItem
// ============================================================================
TabItem::TabItem() {}

void TabItem::InitializeComponents() {
    GetComponents().AddComponent<components::LayoutComponent>(this);
    GetComponents().AddComponent<components::RenderComponent>(this);
    GetComponents().AddComponent<components::InputComponent>(this);
}

void TabItem::SetHeader(const std::wstring& header) {
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

void TabItem::SetContent(const std::shared_ptr<Control>& content) {
    m_content = content;
}

void TabItem::SetIsSelected(bool selected) {
    if (m_isSelected != selected) {
        m_isSelected = selected;
        UpdateVisualState();
    }
}

void TabItem::OnMouseEnter() {
    m_isHovered = true;
    UpdateVisualState();
}

void TabItem::OnMouseLeave() {
    m_isHovered = false;
    m_isCloseHovered = false;
    UpdateVisualState();
}

void TabItem::OnMouseDown(MouseEventArgs& args) {
    // 检查是否点击关闭按钮
    if (m_canClose && HitTestCloseButton(args.x, args.y)) {
        // 通知 TabControl 关闭此标签
        if (auto parent = GetParent()) {
            if (auto* tabControl = dynamic_cast<TabControl*>(parent.get())) {
                tabControl->OnTabClose(this);
            }
        }
    } else {
        // 选中标签
        SetIsSelected(true);
        if (auto parent = GetParent()) {
            if (auto* tabControl = dynamic_cast<TabControl*>(parent.get())) {
                tabControl->OnTabSelected(this);
            }
        }
    }
    args.Handled = true;
}

void TabItem::UpdateVisualState() {
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

bool TabItem::HitTestCloseButton(float x, float y) {
    if (!m_canClose) return false;
    
    rendering::Rect rect;
    if (auto* renderable = AsRenderable()) {
        rect = renderable->GetRenderRect();
    }
    
    float closeX = rect.x + rect.width - m_padding - m_closeButtonSize;
    float closeY = rect.y + (rect.height - m_closeButtonSize) / 2;
    
    return x >= closeX && x <= closeX + m_closeButtonSize &&
           y >= closeY && y <= closeY + m_closeButtonSize;
}

void TabItem::DrawCloseButton(rendering::IRenderContext* context, const rendering::Rect& rect) {
    if (!m_canClose) return;
    
    auto brush = context->CreateSolidColorBrush(
        m_isCloseHovered ? m_closeButtonHoverColor : m_closeButtonColor);
    if (!brush) return;
    
    float centerX = rect.x + rect.width / 2;
    float centerY = rect.y + rect.height / 2;
    float size = 3.0f;
    
    // 绘制 X
    context->DrawLine(rendering::Point(centerX - size, centerY - size),
                      rendering::Point(centerX + size, centerY + size),
                      brush.get(), 1.5f);
    context->DrawLine(rendering::Point(centerX + size, centerY - size),
                      rendering::Point(centerX - size, centerY + size),
                      brush.get(), 1.5f);
}

rendering::Size TabItem::OnMeasure(const rendering::Size& availableSize) {
    (void)availableSize;
    // 估算宽度：文本宽度 + 边距 + 关闭按钮
    float width = m_padding * 2;
    if (!m_header.empty()) {
        width += static_cast<float>(m_header.length()) * m_fontSize * 0.6f;
    }
    if (m_canClose) {
        width += m_closeButtonSize + m_padding;
    }
    return rendering::Size(width, m_tabHeight);
}

void TabItem::OnRender(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    auto rect = render->GetRenderRect();
    
    // 绘制背景
    rendering::Color bgColor = m_normalBg;
    if (m_isSelected) {
        bgColor = m_selectedBg;
    } else if (m_isHovered) {
        bgColor = m_hoverBg;
    }
    
    if (bgColor.a > 0) {
        auto bgBrush = context->CreateSolidColorBrush(bgColor);
        if (bgBrush) {
            context->FillRectangle(rect, bgBrush.get());
        }
    }
    
    // 绘制文本
    if (!m_header.empty()) {
        rendering::Color textColor = m_isSelected ? m_selectedTextColor : m_textColor;
        auto textBrush = context->CreateSolidColorBrush(textColor);
        auto textFormat = context->CreateTextFormat(L"Microsoft YaHei", m_fontSize);
        
        if (textBrush && textFormat) {
            float textWidth = rect.width - m_padding * 2;
            if (m_canClose) {
                textWidth -= m_closeButtonSize + m_padding;
            }
            
            rendering::Point textPos(rect.x + m_padding, 
                                     rect.y + (rect.height - m_fontSize) / 2);
            context->DrawTextString(m_header, textFormat.get(), textPos, textBrush.get());
        }
    }
    
    // 绘制关闭按钮
    if (m_canClose) {
        float closeX = rect.x + rect.width - m_padding - m_closeButtonSize;
        float closeY = rect.y + (rect.height - m_closeButtonSize) / 2;
        rendering::Rect closeRect(closeX, closeY, m_closeButtonSize, m_closeButtonSize);
        DrawCloseButton(context, closeRect);
    }
}

// ============================================================================
// TabControl
// ============================================================================
TabControl::TabControl() {}

void TabControl::InitializeComponents() {
    Panel::InitializeComponents();
}

void TabControl::AddTab(const std::shared_ptr<TabItem>& tab) {
    if (!tab) return;
    
    m_tabs.push_back(tab);
    AddChild(tab);
    
    // 如果是第一个标签，自动选中
    if (m_tabs.size() == 1) {
        SetSelectedIndex(0);
    }
    
    TabAdded.Invoke(this, tab.get());
    
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

void TabControl::RemoveTab(const std::shared_ptr<TabItem>& tab) {
    auto it = std::find(m_tabs.begin(), m_tabs.end(), tab);
    if (it != m_tabs.end()) {
        int index = static_cast<int>(it - m_tabs.begin());
        RemoveChild(*it);
        m_tabs.erase(it);
        
        // 更新选中索引
        if (m_selectedIndex == index) {
            if (m_tabs.empty()) {
                m_selectedIndex = -1;
            } else if (index >= static_cast<int>(m_tabs.size())) {
                m_selectedIndex = static_cast<int>(m_tabs.size()) - 1;
            }
            // 否则保持当前索引（下一个标签会自动前移）
            if (m_selectedIndex >= 0) {
                m_tabs[m_selectedIndex]->SetIsSelected(true);
            }
        } else if (m_selectedIndex > index) {
            m_selectedIndex--;
        }
        
        TabRemoved.Invoke(this, tab.get());
        SelectionChanged.Invoke(this, m_selectedIndex);
        
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
    }
}

void TabControl::RemoveTab(int index) {
    if (index >= 0 && index < static_cast<int>(m_tabs.size())) {
        RemoveTab(m_tabs[index]);
    }
}

void TabControl::ClearTabs() {
    for (auto& tab : m_tabs) {
        RemoveChild(tab);
    }
    m_tabs.clear();
    m_selectedIndex = -1;
    
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

std::shared_ptr<TabItem> TabControl::GetTab(int index) {
    if (index >= 0 && index < static_cast<int>(m_tabs.size())) {
        return m_tabs[index];
    }
    return nullptr;
}

std::shared_ptr<TabItem> TabControl::GetTab(const std::wstring& header) {
    for (auto& tab : m_tabs) {
        if (tab->GetHeader() == header) {
            return tab;
        }
    }
    return nullptr;
}

void TabControl::SetSelectedIndex(int index) {
    if (index >= -1 && index < static_cast<int>(m_tabs.size())) {
        if (m_selectedIndex != index) {
            // 取消之前选中的标签
            if (m_selectedIndex >= 0) {
                m_tabs[m_selectedIndex]->SetIsSelected(false);
            }
            
            m_selectedIndex = index;
            
            // 选中新标签
            if (m_selectedIndex >= 0) {
                m_tabs[m_selectedIndex]->SetIsSelected(true);
            }
            
            UpdateTabStates();
            SelectionChanged.Invoke(this, index);
            
            if (auto* layout = GetLayout()) {
                layout->InvalidateMeasure();
            }
        }
    }
}

std::shared_ptr<TabItem> TabControl::GetSelectedTab() const {
    if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_tabs.size())) {
        return m_tabs[m_selectedIndex];
    }
    return nullptr;
}

void TabControl::SetTabStripPlacement(TabStripPlacement placement) {
    if (m_tabStripPlacement != placement) {
        m_tabStripPlacement = placement;
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
    }
}

void TabControl::OnTabSelected(TabItem* tab) {
    for (size_t i = 0; i < m_tabs.size(); ++i) {
        if (m_tabs[i].get() == tab) {
            SetSelectedIndex(static_cast<int>(i));
            break;
        }
    }
}

void TabControl::OnTabClose(TabItem* tab) {
    TabClosed.Invoke(this, tab);
    // 默认行为：关闭标签
    for (auto& t : m_tabs) {
        if (t.get() == tab) {
            RemoveTab(t);
            break;
        }
    }
}

void TabControl::UpdateTabStates() {
    for (size_t i = 0; i < m_tabs.size(); ++i) {
        bool isSelected = (static_cast<int>(i) == m_selectedIndex);
        m_tabs[i]->SetIsSelected(isSelected);
        
        // 更新内容可见性
        if (auto content = m_tabs[i]->GetContent()) {
            content->SetIsVisible(isSelected);
        }
    }
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

rendering::Rect TabControl::GetTabStripRect() const {
    rendering::Rect contentRect;
    if (auto* renderable = AsRenderable()) {
        contentRect = renderable->GetRenderRect();
    }
    
    switch (m_tabStripPlacement) {
        case TabStripPlacement::Top:
            return rendering::Rect(contentRect.x, contentRect.y, 
                                   contentRect.width, m_tabHeight);
        case TabStripPlacement::Bottom:
            return rendering::Rect(contentRect.x, 
                                   contentRect.y + contentRect.height - m_tabHeight,
                                   contentRect.width, m_tabHeight);
        case TabStripPlacement::Left:
            return rendering::Rect(contentRect.x, contentRect.y,
                                   m_tabHeight, contentRect.height); // 侧边栏使用 tabHeight 作为宽度
        case TabStripPlacement::Right:
            return rendering::Rect(contentRect.x + contentRect.width - m_tabHeight,
                                   contentRect.y, m_tabHeight, contentRect.height);
    }
    return contentRect;
}

rendering::Rect TabControl::GetContentRect() const {
    rendering::Rect contentRect;
    if (auto* renderable = AsRenderable()) {
        contentRect = renderable->GetRenderRect();
    }
    
    switch (m_tabStripPlacement) {
        case TabStripPlacement::Top:
            return rendering::Rect(contentRect.x, contentRect.y + m_tabHeight,
                                   contentRect.width, contentRect.height - m_tabHeight);
        case TabStripPlacement::Bottom:
            return rendering::Rect(contentRect.x, contentRect.y,
                                   contentRect.width, contentRect.height - m_tabHeight);
        case TabStripPlacement::Left:
            return rendering::Rect(contentRect.x + m_tabHeight, contentRect.y,
                                   contentRect.width - m_tabHeight, contentRect.height);
        case TabStripPlacement::Right:
            return rendering::Rect(contentRect.x, contentRect.y,
                                   contentRect.width - m_tabHeight, contentRect.height);
    }
    return contentRect;
}

float TabControl::CalculateTabWidth() const {
    if (m_tabWidth > 0) {
        return m_tabWidth;
    }
    // 自动宽度：均分标签栏宽度
    if (m_tabs.empty()) return 0;
    
    rendering::Rect tabStripRect = GetTabStripRect();
    return tabStripRect.width / m_tabs.size();
}

rendering::Rect TabControl::GetTabRect(int index) const {
    rendering::Rect tabStripRect = GetTabStripRect();
    float tabWidth = CalculateTabWidth();
    
    switch (m_tabStripPlacement) {
        case TabStripPlacement::Top:
        case TabStripPlacement::Bottom:
            return rendering::Rect(tabStripRect.x + index * tabWidth, tabStripRect.y,
                                   tabWidth, m_tabHeight);
        case TabStripPlacement::Left:
        case TabStripPlacement::Right:
            return rendering::Rect(tabStripRect.x, tabStripRect.y + index * m_tabHeight,
                                   m_tabHeight, m_tabHeight);
    }
    return tabStripRect;
}

int TabControl::HitTestTab(float x, float y) {
    rendering::Rect tabStripRect = GetTabStripRect();
    
    // 检查是否在标签栏区域内
    if (x < tabStripRect.x || x > tabStripRect.x + tabStripRect.width ||
        y < tabStripRect.y || y > tabStripRect.y + tabStripRect.height) {
        return -1;
    }
    
    switch (m_tabStripPlacement) {
        case TabStripPlacement::Top:
        case TabStripPlacement::Bottom: {
            float tabWidth = CalculateTabWidth();
            int index = static_cast<int>((x - tabStripRect.x) / tabWidth);
            return (index >= 0 && index < static_cast<int>(m_tabs.size())) ? index : -1;
        }
        case TabStripPlacement::Left:
        case TabStripPlacement::Right: {
            int index = static_cast<int>((y - tabStripRect.y) / m_tabHeight);
            return (index >= 0 && index < static_cast<int>(m_tabs.size())) ? index : -1;
        }
    }
    return -1;
}

rendering::Size TabControl::OnMeasureChildren(const rendering::Size& availableSize) {
    // 测量标签
    for (auto& tab : m_tabs) {
        if (auto* layoutable = tab->AsLayoutable()) {
            interfaces::LayoutConstraint constraint;
            constraint.available = rendering::Size(CalculateTabWidth(), m_tabHeight);
            layoutable->Measure(constraint);
        }
    }
    
    // 测量内容区域
    rendering::Rect contentRect(0, 0, 
        availableSize.width, 
        availableSize.height > m_tabHeight ? availableSize.height - m_tabHeight : 0);
    
    for (auto& tab : m_tabs) {
        if (auto content = tab->GetContent()) {
            if (auto* layoutable = content->AsLayoutable()) {
                interfaces::LayoutConstraint constraint;
                constraint.available = rendering::Size(contentRect.width, contentRect.height);
                layoutable->Measure(constraint);
            }
        }
    }
    
    return availableSize;
}

rendering::Size TabControl::OnArrangeChildren(const rendering::Size& finalSize) {
    // 排列标签
    for (size_t i = 0; i < m_tabs.size(); ++i) {
        if (auto* layoutable = m_tabs[i]->AsLayoutable()) {
            rendering::Rect tabRect = GetTabRect(static_cast<int>(i));
            layoutable->Arrange(tabRect);
        }
    }
    
    // 排列内容
    rendering::Rect contentRect = GetContentRect();
    for (auto& tab : m_tabs) {
        if (auto content = tab->GetContent()) {
            if (auto* layoutable = content->AsLayoutable()) {
                layoutable->Arrange(contentRect);
            }
        }
    }
    
    return finalSize;
}

void TabControl::OnRenderChildren(rendering::IRenderContext* context) {
    if (!context) return;
    
    // 绘制标签栏背景
    rendering::Rect tabStripRect = GetTabStripRect();
    auto tabStripBrush = context->CreateSolidColorBrush(m_tabStripBg);
    if (tabStripBrush) {
        context->FillRectangle(tabStripRect, tabStripBrush.get());
    }
    
    // 绘制内容区域背景
    rendering::Rect contentRect = GetContentRect();
    auto contentBgBrush = context->CreateSolidColorBrush(m_contentBg);
    auto borderBrush = context->CreateSolidColorBrush(m_borderColor);
    
    if (contentBgBrush) {
        context->FillRectangle(contentRect, contentBgBrush.get());
    }
    if (borderBrush) {
        context->DrawRectangle(contentRect, borderBrush.get(), 1.0f);
    }
    
    // 渲染标签
    for (auto& tab : m_tabs) {
        if (auto* itemRenderable = tab->AsRenderable()) {
            itemRenderable->Render(context);
        }
    }
    
    // 渲染当前选中的内容
    if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_tabs.size())) {
        if (auto content = m_tabs[m_selectedIndex]->GetContent()) {
            if (auto* contentRenderable = content->AsRenderable()) {
                contentRenderable->Render(context);
            }
        }
    }
}

void TabControl::OnMouseMove(MouseEventArgs& args) {
    // 更新关闭按钮悬停状态
    int tabIndex = HitTestTab(args.x, args.y);
    if (tabIndex >= 0) {
        rendering::Rect tabRect = GetTabRect(tabIndex);
        float localX = args.x - tabRect.x;
        float localY = args.y - tabRect.y;
        m_tabs[tabIndex]->m_isCloseHovered = m_tabs[tabIndex]->HitTestCloseButton(localX, localY);
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
    
    args.Handled = true;
}

void TabControl::OnMouseDown(MouseEventArgs& args) {
    int tabIndex = HitTestTab(args.x, args.y);
    if (tabIndex >= 0) {
        // 转发给 TabItem 处理
        MouseEventArgs localArgs = args;
        rendering::Rect tabRect = GetTabRect(tabIndex);
        localArgs.x = args.x - tabRect.x;
        localArgs.y = args.y - tabRect.y;
        m_tabs[tabIndex]->OnMouseDown(localArgs);
    }
    args.Handled = true;
}

} // namespace controls
} // namespace luaui
