#include "ComboBox.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include "Components/InputComponent.h"
#include "Interfaces/IRenderable.h"
#include "Interfaces/ILayoutable.h"
#include "IRenderContext.h"

namespace luaui {
namespace controls {

ComboBox::ComboBox() {
    m_itemHeight = 28.0f;
}

void ComboBox::InitializeComponents() {
    Panel::InitializeComponents();
    
    // 设置最小高度
    if (auto* layout = GetLayout()) {
        layout->SetMinHeight(m_itemHeight + m_padding * 2);
    }
}

void ComboBox::AddItem(const std::wstring& item) {
    m_items.push_back(item);
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

void ComboBox::RemoveItem(int index) {
    if (index >= 0 && index < static_cast<int>(m_items.size())) {
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

void ComboBox::ClearItems() {
    m_items.clear();
    m_selectedIndex = -1;
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

std::wstring ComboBox::GetItem(int index) const {
    if (index >= 0 && index < static_cast<int>(m_items.size())) {
        return m_items[index];
    }
    return L"";
}

void ComboBox::SetSelectedIndex(int index) {
    if (index >= -1 && index < static_cast<int>(m_items.size())) {
        if (m_selectedIndex != index) {
            m_selectedIndex = index;
            SelectionChanged.Invoke(this, index);
            if (auto* render = GetRender()) {
                render->Invalidate();
            }
        }
    }
}

std::wstring ComboBox::GetSelectedItem() const {
    if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_items.size())) {
        return m_items[m_selectedIndex];
    }
    return L"";
}

void ComboBox::SetText(const std::wstring& text) {
    for (size_t i = 0; i < m_items.size(); ++i) {
        if (m_items[i] == text) {
            SetSelectedIndex(static_cast<int>(i));
            return;
        }
    }
    // 未找到匹配项，取消选中
    SetSelectedIndex(-1);
}

void ComboBox::SetIsDropDownOpen(bool open) {
    if (m_isDropDownOpen != open) {
        m_isDropDownOpen = open;
        if (open) {
            OpenDropDown();
        } else {
            CloseDropDown();
        }
        DropDownOpenedChanged.Invoke(this, open);
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void ComboBox::ToggleDropDown() {
    SetIsDropDownOpen(!m_isDropDownOpen);
}

void ComboBox::OpenDropDown() {
    m_dropDownAnimation = 1.0f; // 简化：直接打开，实际项目可添加动画
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

void ComboBox::CloseDropDown() {
    m_dropDownAnimation = 0.0f;
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

void ComboBox::UpdateVisualState() {
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void ComboBox::OnItemSelected(int index) {
    SetSelectedIndex(index);
    SetIsDropDownOpen(false);
}

int ComboBox::HitTestDropDownItem(float x, float y) {
    rendering::Rect rect;
    if (auto* renderable = AsRenderable()) {
        rect = renderable->GetRenderRect();
    }
    
    // 计算下拉列表区域
    float headerHeight = m_itemHeight + m_padding * 2;
    float dropDownY = rect.y + headerHeight;
    
    // 检查是否在下拉区域内
    if (y < dropDownY || x < rect.x || x > rect.x + rect.width) {
        return -1;
    }
    
    float itemY = dropDownY;
    for (size_t i = 0; i < m_items.size(); ++i) {
        if (y >= itemY && y < itemY + m_itemHeight) {
            return static_cast<int>(i);
        }
        itemY += m_itemHeight;
    }
    return -1;
}

void ComboBox::OnClick() {
    // 点击头部区域切换下拉
    ToggleDropDown();
    Click.Invoke(this);
}

void ComboBox::OnMouseDown(MouseEventArgs& args) {
    if (m_isDropDownOpen) {
        int itemIndex = HitTestDropDownItem(args.x, args.y);
        if (itemIndex >= 0) {
            OnItemSelected(itemIndex);
        } else {
            // 点击下拉区域外关闭
            rendering::Rect rect;
            if (auto* renderable = AsRenderable()) {
                rect = renderable->GetRenderRect();
            }
            float headerHeight = m_itemHeight + m_padding * 2;
            if (args.y > rect.y + headerHeight) {
                SetIsDropDownOpen(false);
            }
        }
    }
    args.Handled = true;
}

void ComboBox::OnMouseEnter() {
    m_isHovered = true;
    UpdateVisualState();
}

void ComboBox::OnMouseLeave() {
    m_isHovered = false;
    UpdateVisualState();
}

void ComboBox::OnGotFocus() {
    GotFocus.Invoke(this);
}

void ComboBox::OnLostFocus() {
    // 失去焦点时关闭下拉
    if (m_isDropDownOpen) {
        SetIsDropDownOpen(false);
    }
    LostFocus.Invoke(this);
}

rendering::Size ComboBox::OnMeasure(const rendering::Size& availableSize) {
    // 计算头部高度
    float headerHeight = m_itemHeight + m_padding * 2;
    
    // 计算需要的宽度
    float maxTextWidth = 100.0f; // 最小宽度
    auto textFormat = std::shared_ptr<rendering::ITextFormat>(); // 仅用于测量
    
    for (const auto& item : m_items) {
        // 简化：估算文本宽度（实际应使用字体测量）
        maxTextWidth = std::max(maxTextWidth, static_cast<float>(item.length() * 14));
    }
    
    float width = maxTextWidth + m_padding * 2 + m_arrowWidth;
    float height = headerHeight;
    
    // 如果下拉打开，加上下拉高度
    if (m_isDropDownOpen) {
        float actualDropHeight = std::min(m_dropDownHeight, 
                                          static_cast<float>(m_items.size()) * m_itemHeight);
        height += actualDropHeight;
    }
    
    // 限制在可用大小内
    if (availableSize.width > 0) {
        width = std::min(width, availableSize.width);
    }
    if (availableSize.height > 0 && !m_isDropDownOpen) {
        height = std::min(height, availableSize.height);
    }
    
    return rendering::Size(width, height);
}

rendering::Size ComboBox::OnArrangeChildren(const rendering::Size& finalSize) {
    // ComboBox 本身没有子控件，所有内容都是自绘
    return finalSize;
}

void ComboBox::OnRender(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    auto rect = render->GetRenderRect();
    
    // 绘制头部背景
    rendering::Color borderColor = m_isHovered ? m_borderHoverColor : m_borderColor;
    auto bgBrush = context->CreateSolidColorBrush(m_bgColor);
    auto borderBrush = context->CreateSolidColorBrush(borderColor);
    
    if (bgBrush) {
        context->FillRectangle(rect, bgBrush.get());
    }
    
    // 绘制边框
    if (borderBrush) {
        context->DrawRectangle(rect, borderBrush.get(), 1.0f);
    }
    
    // 计算头部区域
    float headerHeight = m_itemHeight + m_padding * 2;
    
    // 绘制选中文本或占位符
    std::wstring displayText = (m_selectedIndex >= 0) ? GetSelectedItem() : m_placeholder;
    rendering::Color textColor = (m_selectedIndex >= 0) ? m_textColor : m_placeholderColor;
    
    if (!displayText.empty()) {
        auto textBrush = context->CreateSolidColorBrush(textColor);
        auto textFormat = context->CreateTextFormat(L"Microsoft YaHei", 14.0f);
        
        if (textBrush && textFormat) {
            textFormat->SetTextAlignment(rendering::TextAlignment::Leading);
            textFormat->SetParagraphAlignment(rendering::ParagraphAlignment::Center);
            
            rendering::Rect textRect(rect.x + m_padding, rect.y, 
                                      rect.width - m_padding * 2 - m_arrowWidth, 
                                      headerHeight);
            context->DrawTextString(displayText, textFormat.get(), 
                                    rendering::Point(textRect.x, textRect.y + (headerHeight - 14) / 2), 
                                    textBrush.get());
        }
    }
    
    // 绘制下拉箭头
    rendering::Rect arrowRect(rect.x + rect.width - m_arrowWidth, rect.y, 
                               m_arrowWidth, headerHeight);
    DrawDropDownArrow(context, arrowRect);
}

void ComboBox::DrawDropDownArrow(rendering::IRenderContext* context, const rendering::Rect& rect) {
    auto arrowBrush = context->CreateSolidColorBrush(m_arrowColor);
    if (!arrowBrush) return;
    
    // 绘制简单的三角形箭头
    float centerX = rect.x + rect.width / 2;
    float centerY = rect.y + rect.height / 2;
    float arrowSize = 4.0f;
    
    if (m_isDropDownOpen) {
        // 向上箭头
        context->DrawLine(rendering::Point(centerX - arrowSize, centerY + arrowSize/2),
                          rendering::Point(centerX, centerY - arrowSize/2),
                          arrowBrush.get(), 1.0f);
        context->DrawLine(rendering::Point(centerX, centerY - arrowSize/2),
                          rendering::Point(centerX + arrowSize, centerY + arrowSize/2),
                          arrowBrush.get(), 1.0f);
    } else {
        // 向下箭头
        context->DrawLine(rendering::Point(centerX - arrowSize, centerY - arrowSize/2),
                          rendering::Point(centerX, centerY + arrowSize/2),
                          arrowBrush.get(), 1.0f);
        context->DrawLine(rendering::Point(centerX, centerY + arrowSize/2),
                          rendering::Point(centerX + arrowSize, centerY - arrowSize/2),
                          arrowBrush.get(), 1.0f);
    }
}

void ComboBox::OnRenderChildren(rendering::IRenderContext* context) {
    if (!context || !m_isDropDownOpen || m_items.empty()) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    auto rect = render->GetRenderRect();
    float headerHeight = m_itemHeight + m_padding * 2;
    
    // 计算下拉区域
    float dropDownHeight = std::min(m_dropDownHeight, 
                                    static_cast<float>(m_items.size()) * m_itemHeight);
    rendering::Rect dropDownRect(rect.x, rect.y + headerHeight, rect.width, dropDownHeight);
    
    // 绘制下拉背景
    auto dropDownBgBrush = context->CreateSolidColorBrush(m_dropDownBgColor);
    auto borderBrush = context->CreateSolidColorBrush(m_borderColor);
    
    if (dropDownBgBrush) {
        context->FillRectangle(dropDownRect, dropDownBgBrush.get());
    }
    if (borderBrush) {
        context->DrawRectangle(dropDownRect, borderBrush.get(), 1.0f);
    }
    
    // 绘制下拉项
    float itemY = dropDownRect.y;
    for (size_t i = 0; i < m_items.size(); ++i) {
        if (itemY >= dropDownRect.y + dropDownRect.height) break;
        
        rendering::Rect itemRect(dropDownRect.x, itemY, dropDownRect.width, m_itemHeight);
        
        // 绘制项背景
        if (static_cast<int>(i) == m_selectedIndex) {
            auto selectedBrush = context->CreateSolidColorBrush(m_itemSelectedColor);
            if (selectedBrush) {
                context->FillRectangle(itemRect, selectedBrush.get());
            }
        }
        
        // 绘制项文本
        auto textBrush = context->CreateSolidColorBrush(
            static_cast<int>(i) == m_selectedIndex ? rendering::Color::White() : m_textColor);
        auto textFormat = context->CreateTextFormat(L"Microsoft YaHei", 14.0f);
        
        if (textBrush && textFormat) {
            context->DrawTextString(m_items[i], textFormat.get(), 
                                    rendering::Point(itemRect.x + m_padding, 
                                                     itemY + (m_itemHeight - 14) / 2),
                                    textBrush.get());
        }
        
        itemY += m_itemHeight;
    }
}

} // namespace controls
} // namespace luaui
