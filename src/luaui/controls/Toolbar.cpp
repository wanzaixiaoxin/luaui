#include "Toolbar.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include "Interfaces/IRenderable.h"
#include "IRenderContext.h"
#include "layouts/DockPanel.h"

namespace luaui {
namespace controls {

// ============================================================================
// ToolbarItem
// ============================================================================
ToolbarItem::ToolbarItem() {}

ToolbarItem::ToolbarItem(const std::wstring& text) : m_text(text) {}

void ToolbarItem::InitializeComponents() {
    GetComponents().AddComponent<components::LayoutComponent>(this);
    GetComponents().AddComponent<components::RenderComponent>(this);
    GetComponents().AddComponent<components::InputComponent>(this);
}

void ToolbarItem::SetText(const std::wstring& text) {
    if (m_text != text) {
        m_text = text;
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void ToolbarItem::SetIcon(const std::wstring& icon) {
    if (m_icon != icon) {
        m_icon = icon;
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void ToolbarItem::SetIsChecked(bool checked) {
    if (m_isChecked != checked) {
        m_isChecked = checked;
        UpdateVisualState();
    }
}

void ToolbarItem::SetIsPressed(bool pressed) {
    if (m_isPressed != pressed) {
        m_isPressed = pressed;
        UpdateVisualState();
    }
}

void ToolbarItem::OnMouseEnter() {
    m_isHovered = true;
    UpdateVisualState();
}

void ToolbarItem::OnMouseLeave() {
    m_isHovered = false;
    m_isPressed = false;
    UpdateVisualState();
}

void ToolbarItem::OnMouseDown(MouseEventArgs& args) {
    (void)args;
    if (m_isCheckable) {
        Toggle();
    } else {
        m_isPressed = true;
    }
    UpdateVisualState();
}

void ToolbarItem::OnMouseUp(MouseEventArgs& args) {
    (void)args;
    if (!m_isCheckable) {
        m_isPressed = false;
        UpdateVisualState();
    }
}

void ToolbarItem::OnClick() {
    InvokeCommand();
}

void ToolbarItem::UpdateVisualState() {
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void ToolbarItem::DrawIcon(rendering::IRenderContext* context, const rendering::Rect& rect) {
    (void)context;
    (void)rect;
    // 简化实现：实际应该绘制图标图片
    // 可以绘制一个简单的占位符
}

void ToolbarItem::DrawDropDownArrow(rendering::IRenderContext* context, 
                                     const rendering::Rect& rect) {
    if (!m_hasDropDown) return;
    
    auto brush = context->CreateSolidColorBrush(m_textColor);
    if (!brush) return;
    
    float centerX = rect.x + rect.width / 2;
    float centerY = rect.y + rect.height / 2;
    
    // 向下箭头
    context->DrawLine(rendering::Point(centerX - 3, centerY - 1),
                      rendering::Point(centerX, centerY + 2),
                      brush.get(), 1.0f);
    context->DrawLine(rendering::Point(centerX, centerY + 2),
                      rendering::Point(centerX + 3, centerY - 1),
                      brush.get(), 1.0f);
}

rendering::Size ToolbarItem::OnMeasure(const rendering::Size& availableSize) {
    (void)availableSize;
    
    float width = m_padding * 2;
    float height = m_padding * 2;
    
    // 图标宽度
    bool hasIcon = m_showIcons && !m_icon.empty();
    if (hasIcon) {
        width += m_iconSize;
        height = std::max(height, m_iconSize + m_padding * 2);
    }
    
    // 文本宽度
    bool hasText = m_showTextLabels && !m_text.empty();
    if (hasText) {
        if (hasIcon) width += m_spacing;
        width += static_cast<float>(m_text.length()) * m_fontSize * 0.6f;
        height = std::max(height, m_fontSize + m_padding * 2);
    }
    
    // 下拉箭头宽度
    if (m_hasDropDown) {
        width += m_spacing + m_dropDownArrowWidth;
    }
    
    // 最小尺寸
    width = std::max(width, 24.0f);
    height = std::max(height, 24.0f);
    
    return rendering::Size(width, height);
}

void ToolbarItem::OnRender(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    auto rect = render->GetRenderRect();
    
    // 选择背景色
    rendering::Color bgColor = m_normalBg;
    if (m_isPressed) {
        bgColor = m_pressedBg;
    } else if (m_isChecked) {
        bgColor = m_checkedBg;
    } else if (m_isHovered) {
        bgColor = m_hoverBg;
    }
    
    // 绘制背景
    if (bgColor.a > 0) {
        auto bgBrush = context->CreateSolidColorBrush(bgColor);
        if (bgBrush) {
            context->FillRectangle(rect, bgBrush.get());
        }
    }
    
    // 绘制选中边框
    if (m_isChecked) {
        auto borderBrush = context->CreateSolidColorBrush(rendering::Color::FromHex(0x0078D4));
        if (borderBrush) {
            context->DrawRectangle(rect, borderBrush.get(), 1.0f);
        }
    }
    
    float currentX = rect.x + m_padding;
    float centerY = rect.y + rect.height / 2;
    
    // 绘制图标
    if (m_showIcons && !m_icon.empty()) {
        rendering::Rect iconRect(currentX, centerY - m_iconSize / 2, 
                                  m_iconSize, m_iconSize);
        DrawIcon(context, iconRect);
        currentX += m_iconSize + m_spacing;
    }
    
    // 绘制文本
    if (m_showTextLabels && !m_text.empty()) {
        auto textBrush = context->CreateSolidColorBrush(m_textColor);
        auto textFormat = context->CreateTextFormat(L"Microsoft YaHei", m_fontSize);
        
        if (textBrush && textFormat) {
            rendering::Point textPos(currentX, centerY - m_fontSize / 2);
            context->DrawTextString(m_text, textFormat.get(), textPos, textBrush.get());
            currentX += static_cast<float>(m_text.length()) * m_fontSize * 0.6f + m_spacing;
        }
    }
    
    // 绘制下拉箭头
    if (m_hasDropDown) {
        rendering::Rect arrowRect(rect.x + rect.width - m_padding - m_dropDownArrowWidth,
                                   centerY - 3, m_dropDownArrowWidth, 6);
        DrawDropDownArrow(context, arrowRect);
    }
}

// ============================================================================
// ToolbarSeparator
// ============================================================================
ToolbarSeparator::ToolbarSeparator() {}

void ToolbarSeparator::InitializeComponents() {
    GetComponents().AddComponent<components::LayoutComponent>(this);
    GetComponents().AddComponent<components::RenderComponent>(this);
}

void ToolbarSeparator::OnRender(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    auto rect = render->GetRenderRect();
    
    auto lineBrush = context->CreateSolidColorBrush(m_lineColor);
    if (lineBrush) {
        // 垂直线
        float lineX = rect.x + rect.width / 2;
        context->DrawLine(rendering::Point(lineX, rect.y + 4),
                          rendering::Point(lineX, rect.y + rect.height - 4),
                          lineBrush.get(), m_lineWidth);
    }
}

rendering::Size ToolbarSeparator::OnMeasure(const rendering::Size& availableSize) {
    (void)availableSize;
    return rendering::Size(m_width, 24);
}

// ============================================================================
// Toolbar
// ============================================================================
Toolbar::Toolbar() {}

void Toolbar::InitializeComponents() {
    Panel::InitializeComponents();
    
    // 设置默认高度
    if (auto* layout = GetLayout()) {
        if (m_orientation == Orientation::Horizontal) {
            layout->SetHeight(m_thickness);
        } else {
            layout->SetWidth(m_thickness);
        }
    }
}

void Toolbar::AddItem(const std::shared_ptr<ToolbarItem>& item) {
    if (!item) return;
    
    m_items.push_back(item);
    AddChild(item);
    
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

void Toolbar::AddSeparator() {
    auto separator = std::make_shared<ToolbarSeparator>();
    m_items.push_back(separator);
    AddChild(separator);
    
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

void Toolbar::AddStretch() {
    // 添加弹性空间（简化实现）
    // 实际应该添加一个特殊的 Spacer 控件
}

void Toolbar::RemoveItem(const std::shared_ptr<ToolbarItem>& item) {
    auto it = std::find(m_items.begin(), m_items.end(), 
                        std::static_pointer_cast<Control>(item));
    if (it != m_items.end()) {
        RemoveChild(*it);
        m_items.erase(it);
        
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
    }
}

void Toolbar::ClearItems() {
    for (auto& item : m_items) {
        RemoveChild(item);
    }
    m_items.clear();
    
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

void Toolbar::SetOrientation(Orientation orientation) {
    if (m_orientation != orientation) {
        m_orientation = orientation;
        
        // 更新约束
        if (auto* layout = GetLayout()) {
            if (orientation == Orientation::Horizontal) {
                layout->ClearWidth();
                layout->SetHeight(m_thickness);
            } else {
                layout->SetWidth(m_thickness);
                layout->ClearHeight();
            }
            layout->InvalidateMeasure();
        }
    }
}

void Toolbar::SetShowTextLabels(bool show) {
    if (m_showTextLabels != show) {
        m_showTextLabels = show;
        UpdateItemDisplay();
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
    }
}

void Toolbar::UpdateItemDisplay() {
    for (auto& item : m_items) {
        if (auto toolbarItem = std::dynamic_pointer_cast<ToolbarItem>(item)) {
            // 更新显示模式
            (void)toolbarItem;
        }
    }
}

rendering::Size Toolbar::OnMeasureChildren(const rendering::Size& availableSize) {
    float totalMain = 0;  // 主轴总长度
    float maxCross = 0;   // 副轴最大长度
    
    for (auto& item : m_items) {
        if (auto* layoutable = item->AsLayoutable()) {
            interfaces::LayoutConstraint constraint;
            constraint.available = availableSize;
            layoutable->Measure(constraint);
            
            auto size = layoutable->GetDesiredSize();
            
            if (m_orientation == Orientation::Horizontal) {
                totalMain += size.width + m_itemSpacing;
                maxCross = std::max(maxCross, size.height);
            } else {
                totalMain += size.height + m_itemSpacing;
                maxCross = std::max(maxCross, size.width);
            }
        }
    }
    
    if (!m_items.empty()) {
        totalMain -= m_itemSpacing; // 减去最后一个间距
    }
    
    if (m_orientation == Orientation::Horizontal) {
        return rendering::Size(totalMain, std::max(maxCross, m_thickness));
    } else {
        return rendering::Size(std::max(maxCross, m_thickness), totalMain);
    }
}

rendering::Size Toolbar::OnArrangeChildren(const rendering::Size& finalSize) {
    rendering::Rect rect;
    if (auto* renderable = AsRenderable()) {
        rect = renderable->GetRenderRect();
    }
    
    float mainPos = (m_orientation == Orientation::Horizontal) ? rect.x : rect.y;
    
    for (auto& item : m_items) {
        if (auto* layoutable = item->AsLayoutable()) {
            auto size = layoutable->GetDesiredSize();
            
            if (m_orientation == Orientation::Horizontal) {
                float y = rect.y + (rect.height - size.height) / 2;
                layoutable->Arrange(rendering::Rect(mainPos, y, size.width, size.height));
                mainPos += size.width + m_itemSpacing;
            } else {
                float x = rect.x + (rect.width - size.width) / 2;
                layoutable->Arrange(rendering::Rect(x, mainPos, size.width, size.height));
                mainPos += size.height + m_itemSpacing;
            }
        }
    }
    
    return finalSize;
}

void Toolbar::OnRenderChildren(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    auto rect = render->GetRenderRect();
    
    // 绘制背景
    if (m_style != ToolbarStyle::Transparent) {
        auto bgBrush = context->CreateSolidColorBrush(m_bgColor);
        if (bgBrush) {
            context->FillRectangle(rect, bgBrush.get());
        }
    }
    
    // 绘制边框
    if (m_style == ToolbarStyle::Default) {
        auto borderBrush = context->CreateSolidColorBrush(m_borderColor);
        if (borderBrush) {
            context->DrawRectangle(rect, borderBrush.get(), 1.0f);
        }
    }
    
    // 绘制子项
    for (auto& item : m_items) {
        if (auto* itemRenderable = item->AsRenderable()) {
            itemRenderable->Render(context);
        }
    }
}

// ============================================================================
// ToolStripContainer
// ============================================================================
ToolStripContainer::ToolStripContainer() {}

void ToolStripContainer::InitializeComponents() {
    Panel::InitializeComponents();
    
    // 创建停靠面板
    auto dockPanel = std::make_shared<DockPanel>();
    AddChild(dockPanel);
    
    // 创建各边缘面板
    m_topPanel = std::make_shared<Panel>();
    m_bottomPanel = std::make_shared<Panel>();
    m_leftPanel = std::make_shared<Panel>();
    m_rightPanel = std::make_shared<Panel>();
    m_contentPanel = std::make_shared<Panel>();
    
    // 添加到停靠面板
    DockPanel::SetDock(m_topPanel, layouts::Dock::Top);
    DockPanel::SetDock(m_bottomPanel, layouts::Dock::Bottom);
    DockPanel::SetDock(m_leftPanel, layouts::Dock::Left);
    DockPanel::SetDock(m_rightPanel, layouts::Dock::Right);
    
    dockPanel->AddChild(m_topPanel);
    dockPanel->AddChild(m_bottomPanel);
    dockPanel->AddChild(m_leftPanel);
    dockPanel->AddChild(m_rightPanel);
    dockPanel->AddChild(m_contentPanel);
}

void ToolStripContainer::AddToolbar(const std::shared_ptr<Toolbar>& toolbar, 
                                     layouts::Dock dock) {
    if (!toolbar) return;
    
    switch (dock) {
        case layouts::Dock::Top:
            toolbar->SetOrientation(Toolbar::Orientation::Horizontal);
            m_topPanel->AddChild(toolbar);
            break;
        case layouts::Dock::Bottom:
            toolbar->SetOrientation(Toolbar::Orientation::Horizontal);
            m_bottomPanel->AddChild(toolbar);
            break;
        case layouts::Dock::Left:
            toolbar->SetOrientation(Toolbar::Orientation::Vertical);
            m_leftPanel->AddChild(toolbar);
            break;
        case layouts::Dock::Right:
            toolbar->SetOrientation(Toolbar::Orientation::Vertical);
            m_rightPanel->AddChild(toolbar);
            break;
        default:
            break;
    }
}

void ToolStripContainer::SetContent(const std::shared_ptr<Control>& content) {
    if (m_contentPanel) {
        m_contentPanel->AddChild(content);
    }
}

} // namespace controls
} // namespace luaui
