#include "StatusBar.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include "Interfaces/IRenderable.h"
#include "IRenderContext.h"

namespace luaui {
namespace controls {

// ============================================================================
// StatusBarItem
// ============================================================================
StatusBarItem::StatusBarItem() {}

StatusBarItem::StatusBarItem(const std::wstring& text) : m_text(text), m_itemType(ItemType::Text) {}

StatusBarItem::StatusBarItem(ItemType type) : m_itemType(type) {}

void StatusBarItem::InitializeComponents() {
    GetComponents().AddComponent<components::LayoutComponent>(this);
    GetComponents().AddComponent<components::RenderComponent>(this);
    
    // 根据类型创建内容
    switch (m_itemType) {
        case ItemType::Text:
            m_textBlock = std::make_shared<TextBlock>();
            m_textBlock->SetText(m_text);
            AddChild(m_textBlock);
            break;
            
        case ItemType::Progress:
            m_progressBar = std::make_shared<ProgressBar>();
            m_progressBar->SetIsIndeterminate(true);
            AddChild(m_progressBar);
            break;
            
        case ItemType::Panel:
            // 面板类型由用户设置内容
            break;
            
        case ItemType::Spring:
            // 弹簧不需要子控件
            break;
    }
}

std::wstring StatusBarItem::GetText() const {
    if (m_textBlock) {
        return m_textBlock->GetText();
    }
    return m_text;
}

void StatusBarItem::SetText(const std::wstring& text) {
    m_text = text;
    if (m_textBlock) {
        m_textBlock->SetText(text);
    }
    UpdateVisualState();
}

void StatusBarItem::SetContent(const std::shared_ptr<Control>& content) {
    if (m_itemType == ItemType::Panel) {
        // 移除旧内容
        if (m_content) {
            RemoveChild(m_content);
        }
        
        m_content = content;
        if (content) {
            AddChild(content);
        }
        
        UpdateVisualState();
    }
}

void StatusBarItem::UpdateVisualState() {
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

rendering::Size StatusBarItem::OnMeasure(const rendering::Size& availableSize) {
    switch (m_itemType) {
        case ItemType::Spring:
            // 弹簧在测量时返回0，由父容器分配空间
            return rendering::Size(0, availableSize.height);
            
        case ItemType::Text:
            if (m_autoSize) {
                // 根据文本内容计算宽度
                float textWidth = static_cast<float>(m_text.length()) * m_fontSize * 0.6f;
                return rendering::Size(textWidth + m_padding * 2, availableSize.height);
            }
            return rendering::Size(m_width, availableSize.height);
            
        case ItemType::Progress:
            return rendering::Size(m_width, availableSize.height);
            
        case ItemType::Panel:
            if (m_content) {
                if (auto* layoutable = m_content->AsLayoutable()) {
                    interfaces::LayoutConstraint constraint;
                    constraint.available = availableSize;
                    layoutable->Measure(constraint);
                    return layoutable->GetDesiredSize();
                }
            }
            return rendering::Size(m_width, availableSize.height);
    }
    
    return rendering::Size(m_width, availableSize.height);
}

void StatusBarItem::OnRender(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    auto rect = render->GetRenderRect();
    
    // 绘制边框
    if (m_showBorder) {
        auto borderBrush = context->CreateSolidColorBrush(m_borderColor);
        if (borderBrush) {
            context->DrawRectangle(rect, borderBrush.get(), 1.0f);
        }
    }
    
    // 文本类型已在子控件中绘制
    // 这里可以绘制图标等其他元素
}

// ============================================================================
// StatusBar
// ============================================================================
StatusBar::StatusBar() {}

void StatusBar::InitializeComponents() {
    Panel::InitializeComponents();
    
    // 设置固定高度
    if (auto* layout = GetLayout()) {
        layout->SetHeight(m_height);
        layout->SetMinHeight(m_height);
    }
}

std::shared_ptr<StatusBarItem> StatusBar::AddItem(const std::wstring& text, 
                                                   float width,
                                                   bool spring) {
    auto item = std::make_shared<StatusBarItem>(spring ? StatusBarItem::ItemType::Spring 
                                                        : StatusBarItem::ItemType::Text);
    if (!spring) {
        item->SetText(text);
        if (width > 0) {
            item->SetWidth(width);
            item->SetAutoSize(false);
        }
    }
    
    AddItem(item);
    return item;
}

std::shared_ptr<StatusBarItem> StatusBar::AddProgressItem(float width) {
    auto item = std::make_shared<StatusBarItem>(StatusBarItem::ItemType::Progress);
    item->SetWidth(width);
    item->SetAutoSize(false);
    
    AddItem(item);
    return item;
}

std::shared_ptr<StatusBarItem> StatusBar::AddSpring() {
    auto item = std::make_shared<StatusBarItem>(StatusBarItem::ItemType::Spring);
    
    AddItem(item);
    return item;
}

std::shared_ptr<StatusBarItem> StatusBar::AddSeparator() {
    auto item = std::make_shared<StatusBarItem>(StatusBarItem::ItemType::Text);
    item->SetText(L"|");
    item->SetWidth(10);
    item->SetAutoSize(false);
    item->SetShowBorder(false);
    
    AddItem(item);
    return item;
}

void StatusBar::AddItem(const std::shared_ptr<StatusBarItem>& item) {
    if (!item) return;
    
    m_items.push_back(item);
    AddChild(item);
    
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

void StatusBar::RemoveItem(const std::shared_ptr<StatusBarItem>& item) {
    auto it = std::find(m_items.begin(), m_items.end(), item);
    if (it != m_items.end()) {
        RemoveChild(*it);
        m_items.erase(it);
        
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
    }
}

void StatusBar::ClearItems() {
    for (auto& item : m_items) {
        RemoveChild(item);
    }
    m_items.clear();
    
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

std::shared_ptr<StatusBarItem> StatusBar::GetItem(size_t index) {
    if (index < m_items.size()) {
        return m_items[index];
    }
    return nullptr;
}

std::wstring StatusBar::GetMainText() const {
    if (!m_items.empty() && m_items[0]->GetItemType() == StatusBarItem::ItemType::Text) {
        return m_items[0]->GetText();
    }
    return L"";
}

void StatusBar::SetMainText(const std::wstring& text) {
    if (!m_items.empty() && m_items[0]->GetItemType() == StatusBarItem::ItemType::Text) {
        m_items[0]->SetText(text);
    }
}

void StatusBar::SetStatusText(int panelIndex, const std::wstring& text) {
    if (panelIndex >= 0 && panelIndex < static_cast<int>(m_items.size())) {
        m_items[panelIndex]->SetText(text);
    }
}

void StatusBar::DrawSizingGrip(rendering::IRenderContext* context, 
                                const rendering::Rect& rect) {
    if (!context) return;
    
    auto gripBrush = context->CreateSolidColorBrush(m_gripColor);
    if (!gripBrush) return;
    
    // 绘制斜线网格
    float startX = rect.x + rect.width - m_gripSize;
    float startY = rect.y + rect.height - m_gripSize;
    
    for (int i = 1; i <= 3; ++i) {
        for (int j = 1; j <= 3; ++j) {
            if (i + j <= 4) {
                float x = startX + j * 4;
                float y = startY + i * 4;
                context->DrawLine(rendering::Point(x, y),
                                  rendering::Point(x + 2, y - 2),
                                  gripBrush.get(), 1.0f);
            }
        }
    }
}

bool StatusBar::HitTestSizingGrip(float x, float y) {
    if (!m_showSizingGrip) return false;
    
    rendering::Rect rect;
    if (auto* renderable = AsRenderable()) {
        rect = renderable->GetRenderRect();
    }
    
    float gripX = rect.x + rect.width - m_gripSize;
    float gripY = rect.y + rect.height - m_gripSize;
    
    return x >= gripX && x <= rect.x + rect.width &&
           y >= gripY && y <= rect.y + rect.height;
}

void StatusBar::OnMouseDown(MouseEventArgs& args) {
    if (HitTestSizingGrip(args.x, args.y)) {
        m_isResizing = true;
        m_resizeStartX = args.x;
        m_resizeStartY = args.y;
        args.Handled = true;
    }
}

void StatusBar::OnMouseMove(MouseEventArgs& args) {
    if (m_isResizing) {
        // 触发窗口调整大小
        // 简化实现：实际应该向父窗口发送调整大小消息
        (void)args;
    } else {
        // 更新鼠标光标
        if (HitTestSizingGrip(args.x, args.y)) {
            // SetCursor(SizeNWSE);
        }
    }
}

rendering::Size StatusBar::OnMeasureChildren(const rendering::Size& availableSize) {
    // 首先测量非弹簧项
    float fixedWidth = 0;
    int springCount = 0;
    
    for (auto& item : m_items) {
        if (item->GetItemType() == StatusBarItem::ItemType::Spring) {
            springCount++;
        } else {
            if (auto* layoutable = item->AsLayoutable()) {
                interfaces::LayoutConstraint constraint;
                constraint.available = availableSize;
                layoutable->Measure(constraint);
                fixedWidth += layoutable->GetDesiredSize().width + m_itemSpacing;
            }
        }
    }
    
    if (!m_items.empty()) {
        fixedWidth -= m_itemSpacing; // 减去最后一个间距
    }
    
    // 计算弹簧宽度
    float gripWidth = m_showSizingGrip ? m_gripSize : 0;
    float remainingWidth = availableSize.width - fixedWidth - gripWidth;
    if (remainingWidth < 0) remainingWidth = 0;
    float springWidth = springCount > 0 ? remainingWidth / springCount : 0;
    
    // 重新测量弹簧项
    for (auto& item : m_items) {
        if (item->GetItemType() == StatusBarItem::ItemType::Spring) {
            item->SetWidth(springWidth);
            if (auto* layoutable = item->AsLayoutable()) {
                interfaces::LayoutConstraint constraint;
                constraint.available = rendering::Size(springWidth, availableSize.height);
                layoutable->Measure(constraint);
            }
        }
    }
    
    return availableSize;
}

rendering::Size StatusBar::OnArrangeChildren(const rendering::Size& finalSize) {
    rendering::Rect rect;
    if (auto* renderable = AsRenderable()) {
        rect = renderable->GetRenderRect();
    }
    
    float x = rect.x + m_itemSpacing;
    float y = rect.y;
    float height = rect.height;
    
    for (auto& item : m_items) {
        if (auto* layoutable = item->AsLayoutable()) {
            auto size = layoutable->GetDesiredSize();
            
            // 垂直居中
            float itemY = y + (height - size.height) / 2;
            layoutable->Arrange(rendering::Rect(x, itemY, size.width, size.height));
            
            x += size.width + m_itemSpacing;
        }
    }
    
    return finalSize;
}

void StatusBar::OnRenderChildren(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    auto rect = render->GetRenderRect();
    
    // 绘制背景
    auto bgBrush = context->CreateSolidColorBrush(m_bgColor);
    if (bgBrush) {
        context->FillRectangle(rect, bgBrush.get());
    }
    
    // 绘制上边框
    auto borderBrush = context->CreateSolidColorBrush(m_borderColor);
    if (borderBrush) {
        context->DrawLine(rendering::Point(rect.x, rect.y),
                          rendering::Point(rect.x + rect.width, rect.y),
                          borderBrush.get(), 1.0f);
    }
    
    // 绘制子项
    for (auto& item : m_items) {
        if (auto* itemRenderable = item->AsRenderable()) {
            itemRenderable->Render(context);
        }
    }
    
    // 绘制尺寸调整手柄
    if (m_showSizingGrip) {
        rendering::Rect gripRect(rect.x + rect.width - m_gripSize,
                                  rect.y,
                                  m_gripSize, rect.height);
        DrawSizingGrip(context, gripRect);
    }
}

} // namespace controls
} // namespace luaui
