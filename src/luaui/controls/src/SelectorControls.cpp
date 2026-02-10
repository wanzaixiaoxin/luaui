// SelectorControls.cpp - Selection controls implementation
#include "SelectorControls.h"
#include "IRenderContext.h"
#include <algorithm>

namespace luaui {
namespace controls {

// ============================================================================
// ComboBox
// ============================================================================

ComboBox::ComboBox() {
    SetWidth(150);
    SetHeight(32);
    SetFocusable(true);
}

ComboBox::~ComboBox() = default;

void ComboBox::AddItem(const std::wstring& text) {
    m_items.push_back(text);
    if (m_selectedIndex < 0 && !m_items.empty()) {
        m_selectedIndex = 0;
    }
    InvalidateMeasure();
}

void ComboBox::RemoveItem(int index) {
    if (index >= 0 && index < static_cast<int>(m_items.size())) {
        m_items.erase(m_items.begin() + index);
        if (m_selectedIndex == index) {
            m_selectedIndex = m_items.empty() ? -1 : 0;
        } else if (m_selectedIndex > index) {
            m_selectedIndex--;
        }
        InvalidateMeasure();
    }
}

void ComboBox::ClearItems() {
    m_items.clear();
    m_selectedIndex = -1;
    CloseDropDown();
    InvalidateMeasure();
}

const std::wstring& ComboBox::GetItem(int index) const {
    static const std::wstring empty;
    if (index >= 0 && index < static_cast<int>(m_items.size())) {
        return m_items[index];
    }
    return empty;
}

void ComboBox::SetSelectedIndex(int index) {
    if (index >= -1 && index < static_cast<int>(m_items.size()) && index != m_selectedIndex) {
        m_selectedIndex = index;
        
        SelectionChangedEventArgs args(nullptr, index);
        for (auto& handler : m_selectionChangedHandlers) {
            handler(this, args);
        }
        
        InvalidateRender();
    }
}

const std::wstring& ComboBox::GetSelectedText() const {
    return GetItem(m_selectedIndex);
}

void ComboBox::SetIsDropDownOpen(bool open) {
    if (m_isDropDownOpen != open) {
        m_isDropDownOpen = open;
        if (open) {
            m_highlightedIndex = m_selectedIndex;
            for (auto& handler : m_dropDownOpenedHandlers) {
                handler(this);
            }
        } else {
            for (auto& handler : m_dropDownClosedHandlers) {
                handler(this);
            }
        }
        InvalidateRender();
    }
}

void ComboBox::ToggleDropDown() {
    SetIsDropDownOpen(!m_isDropDownOpen);
}

void ComboBox::CloseDropDown() {
    SetIsDropDownOpen(false);
}

void ComboBox::SetMaxDropDownHeight(float height) {
    m_maxDropDownHeight = height;
    InvalidateMeasure();
}

void ComboBox::AddSelectionChangedHandler(SelectionChangedHandler handler) {
    m_selectionChangedHandlers.push_back(std::move(handler));
}

void ComboBox::AddDropDownOpenedHandler(DropDownOpenedHandler handler) {
    m_dropDownOpenedHandlers.push_back(std::move(handler));
}

void ComboBox::AddDropDownClosedHandler(DropDownClosedHandler handler) {
    m_dropDownClosedHandlers.push_back(std::move(handler));
}

void ComboBox::MeasureOverride(const Size& availableSize) {
    SetDesiredSize(Size(GetWidth(), GetHeight()));
}

Size ComboBox::ArrangeOverride(const Size& finalSize) {
    Rect renderRect = GetRenderRect();
    SetRenderRect(Rect(renderRect.x, renderRect.y, finalSize.width, GetHeight()));
    return finalSize;
}

void ComboBox::RenderOverride(IRenderContext* context) {
    Rect rect = GetRenderRect();
    
    // Background
    Color bgColor = GetIsFocused() ? Color::FromHex(0xFFFFFF) : Color::FromHex(0xFAFAFA);
    context->FillRectangle(rect, bgColor);
    
    // Border
    Color borderColor = m_isDropDownOpen ? Color::FromHex(0x0078D4) : 
                        (GetIsFocused() ? Color::FromHex(0x666666) : Color::FromHex(0xCCCCCC));
    context->DrawRectangle(rect, borderColor, 1.0f);
    
    // Selected text
    if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_items.size())) {
        context->DrawText(m_items[m_selectedIndex], 
                         Point(rect.x + m_padding, rect.y + (rect.height - m_fontSize) / 2),
                         m_textColor, m_fontSize);
    }
    
    // Dropdown arrow
    float arrowSize = 8.0f;
    float arrowX = rect.x + rect.width - m_padding - arrowSize;
    float arrowY = rect.y + (rect.height - arrowSize) / 2;
    
    std::vector<Point> arrow;
    if (m_isDropDownOpen) {
        arrow.push_back(Point(arrowX, arrowY + arrowSize));
        arrow.push_back(Point(arrowX + arrowSize / 2, arrowY));
        arrow.push_back(Point(arrowX + arrowSize, arrowY + arrowSize));
    } else {
        arrow.push_back(Point(arrowX, arrowY));
        arrow.push_back(Point(arrowX + arrowSize / 2, arrowY + arrowSize));
        arrow.push_back(Point(arrowX + arrowSize, arrowY));
    }
    context->FillPolygon(arrow, Color::FromHex(0x666666));
    
    // Render dropdown if open
    if (m_isDropDownOpen) {
        RenderDropDown(context);
    }
}

void ComboBox::RenderDropDown(IRenderContext* context) {
    Rect rect = GetRenderRect();
    
    float dropDownHeight = std::min(m_maxDropDownHeight, 
                                   static_cast<float>(m_items.size() * m_itemHeight));
    
    Rect dropDownRect(rect.x, rect.y + rect.height, rect.width, dropDownHeight);
    
    // Shadow
    context->FillRectangle(Rect(dropDownRect.x + 2, dropDownRect.y + 2, 
                                dropDownRect.width, dropDownRect.height),
                           Color(0, 0, 0, 30));
    
    // Background
    context->FillRectangle(dropDownRect, Color::White());
    
    // Border
    context->DrawRectangle(dropDownRect, Color::FromHex(0xCCCCCC), 1.0f);
    
    // Items
    for (size_t i = 0; i < m_items.size(); i++) {
        float itemY = dropDownRect.y + i * m_itemHeight;
        if (itemY + m_itemHeight < dropDownRect.y || itemY > dropDownRect.y + dropDownHeight) {
            continue; // Clip items outside visible area
        }
        
        Rect itemRect(dropDownRect.x, itemY, dropDownRect.width, m_itemHeight);
        
        // Background
        if (static_cast<int>(i) == m_selectedIndex) {
            context->FillRectangle(itemRect, m_highlightColor);
        } else if (static_cast<int>(i) == m_highlightedIndex) {
            context->FillRectangle(itemRect, m_hoverColor);
        }
        
        // Text
        Color textColor = (static_cast<int>(i) == m_selectedIndex) ? Color::White() : m_textColor;
        context->DrawText(m_items[i], 
                         Point(itemRect.x + m_padding, itemRect.y + (m_itemHeight - m_fontSize) / 2),
                         textColor, m_fontSize);
    }
}

void ComboBox::OnMouseDown(MouseEventArgs& args) {
    Rect rect = GetRenderRect();
    
    // Check if clicking dropdown arrow
    float arrowX = rect.x + rect.width - m_padding - 16;
    if (args.Position.X >= arrowX) {
        ToggleDropDown();
        return;
    }
    
    // Check if clicking in dropdown
    if (m_isDropDownOpen) {
        float dropDownHeight = std::min(m_maxDropDownHeight, 
                                       static_cast<float>(m_items.size() * m_itemHeight));
        Rect dropDownRect(rect.x, rect.y + rect.height, rect.width, dropDownHeight);
        
        if (args.Position.X >= dropDownRect.x && args.Position.X < dropDownRect.x + dropDownRect.width &&
            args.Position.Y >= dropDownRect.y && args.Position.Y < dropDownRect.y + dropDownRect.height) {
            int index = HitTestItem(args.Position);
            if (index >= 0) {
                SelectItem(index);
            }
            return;
        }
    }
    
    // Click on the combobox itself
    ToggleDropDown();
}

void ComboBox::OnMouseMove(MouseEventArgs& args) {
    if (m_isDropDownOpen) {
        int index = HitTestItem(args.Position);
        if (index != m_highlightedIndex) {
            m_highlightedIndex = index;
            InvalidateRender();
        }
    }
}

void ComboBox::OnMouseUp(MouseEventArgs& args) {
    // Handled in OnMouseDown
}

void ComboBox::OnGotFocus() {
    Control::OnGotFocus();
    InvalidateRender();
}

void ComboBox::OnLostFocus() {
    Control::OnLostFocus();
    CloseDropDown();
    InvalidateRender();
}

void ComboBox::OnKeyDown(KeyEventArgs& args) {
    int keyCode = args.KeyCode;
    if (!m_isDropDownOpen) {
        switch (keyCode) {
            case VK_DOWN:
            case VK_F4:
                SetIsDropDownOpen(true);
                return;
            case VK_UP:
                if (m_selectedIndex > 0) {
                    SetSelectedIndex(m_selectedIndex - 1);
                }
                return;
        }
    } else {
        switch (keyCode) {
            case VK_ESCAPE:
            case VK_F4:
                CloseDropDown();
                return;
            case VK_DOWN:
                if (m_highlightedIndex < static_cast<int>(m_items.size()) - 1) {
                    m_highlightedIndex++;
                    InvalidateRender();
                }
                return;
            case VK_UP:
                if (m_highlightedIndex > 0) {
                    m_highlightedIndex--;
                    InvalidateRender();
                }
                return;
            case VK_RETURN:
                if (m_highlightedIndex >= 0) {
                    SelectItem(m_highlightedIndex);
                }
                return;
        }
    }
}

int ComboBox::HitTestItem(const MouseEventArgs::Point& point) const {
    Rect rect = GetRenderRect();
    float dropDownTop = rect.y + rect.height;
    
    int index = static_cast<int>((point.Y - dropDownTop) / m_itemHeight);
    if (index >= 0 && index < static_cast<int>(m_items.size())) {
        return index;
    }
    return -1;
}

void ComboBox::SelectItem(int index) {
    SetSelectedIndex(index);
    CloseDropDown();
}

// ============================================================================
// ListBoxItem
// ============================================================================

ListBoxItem::ListBoxItem() {
    SetHeight(28);
}

ListBoxItem::~ListBoxItem() = default;

void ListBoxItem::SetText(const std::wstring& text) {
    if (m_text != text) {
        m_text = text;
        InvalidateRender();
    }
}

void ListBoxItem::SetIsSelected(bool selected) {
    if (m_isSelected != selected) {
        m_isSelected = selected;
        InvalidateRender();
    }
}

void ListBoxItem::SetIsHighlighted(bool highlighted) {
    if (m_isHighlighted != highlighted) {
        m_isHighlighted = highlighted;
        InvalidateRender();
    }
}

void ListBoxItem::AddClickHandler(ClickHandler handler) {
    m_clickHandlers.push_back(std::move(handler));
}

void ListBoxItem::MeasureOverride(const Size& availableSize) {
    SetDesiredSize(Size(availableSize.width, m_itemHeight));
}

Size ListBoxItem::ArrangeOverride(const Size& finalSize) {
    SetRenderRect(Rect(GetRenderRect().x, GetRenderRect().y, finalSize.width, m_itemHeight));
    return finalSize;
}

void ListBoxItem::RenderOverride(IRenderContext* context) {
    Rect rect = GetRenderRect();
    
    // Background
    if (m_isSelected) {
        context->FillRectangle(rect, m_selectedBg);
    } else if (m_isHighlighted) {
        context->FillRectangle(rect, m_hoverBg);
    }
    
    // Text
    Color textColor = m_isSelected ? Color::White() : m_textColor;
    context->DrawText(m_text, Point(rect.x + m_padding, rect.y + (rect.height - m_fontSize) / 2),
                     textColor, m_fontSize);
}

void ListBoxItem::OnMouseDown(MouseEventArgs& args) {
    for (auto& handler : m_clickHandlers) {
        handler(this);
    }
}

void ListBoxItem::OnMouseEnter() {
    SetIsHighlighted(true);
}

void ListBoxItem::OnMouseLeave() {
    SetIsHighlighted(false);
}

// ============================================================================
// ListBox
// ============================================================================

ListBox::ListBox() {
    SetWidth(150);
    SetHeight(150);
}

ListBox::~ListBox() = default;

void ListBox::AddItem(const std::wstring& text) {
    auto item = std::make_shared<ListBoxItem>();
    item->SetText(text);
    item->AddClickHandler([this](ListBoxItem* item) {
        OnItemClicked(item);
    });
    m_items.push_back(item);
    InvalidateMeasure();
}

void ListBox::InsertItem(int index, const std::wstring& text) {
    if (index < 0) index = 0;
    if (index > static_cast<int>(m_items.size())) index = static_cast<int>(m_items.size());
    
    auto item = std::make_shared<ListBoxItem>();
    item->SetText(text);
    item->AddClickHandler([this](ListBoxItem* item) {
        OnItemClicked(item);
    });
    m_items.insert(m_items.begin() + index, item);
    InvalidateMeasure();
}

void ListBox::RemoveItem(int index) {
    if (index >= 0 && index < static_cast<int>(m_items.size())) {
        m_items.erase(m_items.begin() + index);
        if (m_selectedIndex == index) {
            m_selectedIndex = -1;
        } else if (m_selectedIndex > index) {
            m_selectedIndex--;
        }
        InvalidateMeasure();
    }
}

void ListBox::ClearItems() {
    m_items.clear();
    m_selectedIndex = -1;
    m_selectedIndices.clear();
    InvalidateMeasure();
}

ListBoxItem* ListBox::GetItem(int index) const {
    if (index >= 0 && index < static_cast<int>(m_items.size())) {
        return m_items[index].get();
    }
    return nullptr;
}

void ListBox::SetSelectedIndex(int index) {
    if (m_isMultiSelect) {
        // Multi-select logic
        auto it = std::find(m_selectedIndices.begin(), m_selectedIndices.end(), index);
        if (it != m_selectedIndices.end()) {
            m_selectedIndices.erase(it);
        } else {
            m_selectedIndices.push_back(index);
        }
        UpdateItemStates();
    } else {
        // Single-select logic
        if (index != m_selectedIndex) {
            m_selectedIndex = index;
            UpdateItemStates();
            
            SelectionChangedEventArgs args(
                index >= 0 ? m_items[index].get() : nullptr, index);
            for (auto& handler : m_selectionChangedHandlers) {
                handler(this, args);
            }
        }
    }
}

ListBoxItem* ListBox::GetSelectedItem() const {
    return GetItem(m_selectedIndex);
}

void ListBox::SetIsMultiSelect(bool multi) {
    m_isMultiSelect = multi;
}

std::vector<int> ListBox::GetSelectedIndices() const {
    if (m_isMultiSelect) {
        return m_selectedIndices;
    }
    if (m_selectedIndex >= 0) {
        return { m_selectedIndex };
    }
    return {};
}

void ListBox::AddSelectionChangedHandler(SelectionChangedHandler handler) {
    m_selectionChangedHandlers.push_back(std::move(handler));
}

void ListBox::OnItemClicked(ListBoxItem* item) {
    for (size_t i = 0; i < m_items.size(); i++) {
        if (m_items[i].get() == item) {
            SetSelectedIndex(static_cast<int>(i));
            break;
        }
    }
}

void ListBox::UpdateItemStates() {
    for (size_t i = 0; i < m_items.size(); i++) {
        if (m_isMultiSelect) {
            bool selected = std::find(m_selectedIndices.begin(), m_selectedIndices.end(), i) 
                          != m_selectedIndices.end();
            m_items[i]->SetIsSelected(selected);
        } else {
            m_items[i]->SetIsSelected(static_cast<int>(i) == m_selectedIndex);
        }
    }
}

void ListBox::MeasureOverride(const Size& availableSize) {
    SetDesiredSize(Size(GetWidth(), GetHeight()));
}

Size ListBox::ArrangeOverride(const Size& finalSize) {
    Rect renderRect = GetRenderRect();
    
    for (size_t i = 0; i < m_items.size(); i++) {
        float y = renderRect.y + i * m_itemHeight - m_scrollOffset;
        Rect itemRect(renderRect.x, y, finalSize.width, m_itemHeight);
        m_items[i]->Arrange(itemRect);
    }
    
    return finalSize;
}

void ListBox::RenderOverride(IRenderContext* context) {
    Rect rect = GetRenderRect();
    
    // Background
    context->FillRectangle(rect, Color::White());
    context->DrawRectangle(rect, Color::FromHex(0xCCCCCC), 1.0f);
    
    // Clip and render items
    context->PushClipRect(rect);
    
    for (auto& item : m_items) {
        Rect itemRect = item->GetRenderRect();
        // Simple culling
        if (itemRect.y + itemRect.height > rect.y && itemRect.y < rect.y + rect.height) {
            item->Render(context);
        }
    }
    
    context->PopClipRect();
}

void ListBox::OnMouseWheel(MouseEventArgs& args) {
    int delta = args.Clicks > 0 ? 120 : -120;  // Approximate delta from clicks
    if (!horizontal) {
        float maxScroll = std::max(0.0f, static_cast<float>(m_items.size() * m_itemHeight - GetHeight()));
        m_scrollOffset = std::max(0.0f, std::min(m_scrollOffset - delta / 3, maxScroll));
        InvalidateArrange();
    }
}

// ============================================================================
// TabItem
// ============================================================================

TabItem::TabItem() {
    SetFocusable(true);
}

TabItem::~TabItem() = default;

void TabItem::SetHeader(const std::wstring& header) {
    if (m_header != header) {
        m_header = header;
        InvalidateRender();
    }
}

void TabItem::SetContent(ControlPtr content) {
    m_content = content;
}

void TabItem::SetIsSelected(bool selected) {
    if (m_isSelected != selected) {
        m_isSelected = selected;
        if (m_content) {
            m_content->SetIsVisible(selected);
        }
        InvalidateRender();
    }
}

void TabItem::MeasureOverride(const Size& availableSize) {
    // Measure tab header size based on text
    m_tabWidth = m_header.length() * m_fontSize * 0.6f + m_padding * 2;
    SetDesiredSize(Size(m_tabWidth, m_tabHeight));
}

Size TabItem::ArrangeOverride(const Size& finalSize) {
    SetRenderRect(Rect(GetRenderRect().x, GetRenderRect().y, finalSize.width, m_tabHeight));
    return finalSize;
}

void TabItem::RenderOverride(IRenderContext* context) {
    Rect rect = GetRenderRect();
    
    // Background
    if (m_isSelected) {
        context->FillRectangle(rect, Color::White());
        // Selected tab indicator
        context->FillRectangle(Rect(rect.x, rect.y, rect.width, 3), m_selectedColor);
    } else {
        Color bg = GetIsMouseOver() ? m_hoverColor : m_backgroundColor;
        context->FillRectangle(rect, bg);
    }
    
    // Border
    context->DrawLine(Point(rect.x, rect.y + rect.height - 1),
                     Point(rect.x + rect.width, rect.y + rect.height - 1),
                     Color::FromHex(0xCCCCCC), 1.0f);
    
    // Text
    Color textColor = m_isSelected ? m_selectedColor : m_textColor;
    float textX = rect.x + (rect.width - m_header.length() * m_fontSize * 0.6f) / 2;
    float textY = rect.y + (rect.height - m_fontSize) / 2 + 2;
    context->DrawText(m_header, Point(textX, textY), textColor, m_fontSize);
}

void TabItem::OnMouseDown(MouseEventArgs& args) {
    if (m_tabControl) {
        m_tabControl->OnTabClicked(this);
    }
}

// ============================================================================
// TabControl
// ============================================================================

TabControl::TabControl() {
    SetWidth(400);
    SetHeight(300);
}

TabControl::~TabControl() = default;

void TabControl::SetTabStripPlacement(TabStripPlacement placement) {
    if (m_tabStripPlacement != placement) {
        m_tabStripPlacement = placement;
        InvalidateMeasure();
    }
}

void TabControl::AddTab(std::shared_ptr<TabItem> tab) {
    tab->SetTabControl(this);
    m_tabs.push_back(tab);
    if (m_selectedIndex < 0) {
        SetSelectedIndex(0);
    }
    InvalidateMeasure();
}

void TabControl::RemoveTab(int index) {
    if (index >= 0 && index < static_cast<int>(m_tabs.size())) {
        m_tabs.erase(m_tabs.begin() + index);
        if (m_selectedIndex == index) {
            m_selectedIndex = m_tabs.empty() ? -1 : 0;
        } else if (m_selectedIndex > index) {
            m_selectedIndex--;
        }
        UpdateTabStates();
        InvalidateMeasure();
    }
}

void TabControl::RemoveTab(TabItem* tab) {
    for (size_t i = 0; i < m_tabs.size(); i++) {
        if (m_tabs[i].get() == tab) {
            RemoveTab(static_cast<int>(i));
            break;
        }
    }
}

TabItem* TabControl::GetTab(int index) const {
    if (index >= 0 && index < static_cast<int>(m_tabs.size())) {
        return m_tabs[index].get();
    }
    return nullptr;
}

void TabControl::SetSelectedIndex(int index) {
    if (index >= -1 && index < static_cast<int>(m_tabs.size()) && index != m_selectedIndex) {
        m_selectedIndex = index;
        UpdateTabStates();
        UpdateContentVisibility();
        
        SelectionChangedEventArgs args(
            index >= 0 ? m_tabs[index].get() : nullptr, index);
        for (auto& handler : m_selectionChangedHandlers) {
            handler(this, args);
        }
        
        InvalidateRender();
    }
}

TabItem* TabControl::GetSelectedTab() const {
    return GetTab(m_selectedIndex);
}

void TabControl::AddSelectionChangedHandler(SelectionChangedHandler handler) {
    m_selectionChangedHandlers.push_back(std::move(handler));
}

void TabControl::OnTabClicked(TabItem* tab) {
    for (size_t i = 0; i < m_tabs.size(); i++) {
        if (m_tabs[i].get() == tab) {
            SetSelectedIndex(static_cast<int>(i));
            break;
        }
    }
}

void TabControl::UpdateTabStates() {
    for (size_t i = 0; i < m_tabs.size(); i++) {
        m_tabs[i]->SetIsSelected(static_cast<int>(i) == m_selectedIndex);
    }
}

void TabControl::UpdateContentVisibility() {
    for (size_t i = 0; i < m_tabs.size(); i++) {
        auto content = m_tabs[i]->GetContent();
        if (content) {
            content->SetIsVisible(static_cast<int>(i) == m_selectedIndex);
        }
    }
}

void TabControl::MeasureOverride(const Size& availableSize) {
    // Measure all tabs
    float totalTabWidth = 0;
    for (auto& tab : m_tabs) {
        tab->Measure(availableSize);
        totalTabWidth += tab->GetDesiredSize().width;
    }
    
    // Measure content of selected tab
    if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_tabs.size())) {
        auto content = m_tabs[m_selectedIndex]->GetContent();
        if (content) {
            Size contentAvail(availableSize.width, availableSize.height - m_tabHeight);
            content->Measure(contentAvail);
        }
    }
    
    SetDesiredSize(Size(GetWidth(), GetHeight()));
}

Size TabControl::ArrangeOverride(const Size& finalSize) {
    Rect renderRect = GetRenderRect();
    
    // Arrange tabs at top
    float tabX = renderRect.x;
    float tabY = renderRect.y;
    
    for (auto& tab : m_tabs) {
        float tabWidth = tab->GetDesiredSize().width;
        Rect tabRect(tabX, tabY, tabWidth, m_tabHeight);
        tab->Arrange(tabRect);
        tabX += tabWidth;
    }
    
    // Arrange content area
    Rect contentRect(renderRect.x, renderRect.y + m_tabHeight, 
                     finalSize.width, finalSize.height - m_tabHeight);
    
    if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_tabs.size())) {
        auto content = m_tabs[m_selectedIndex]->GetContent();
        if (content) {
            content->Arrange(contentRect);
        }
    }
    
    return finalSize;
}

void TabControl::RenderOverride(IRenderContext* context) {
    Rect rect = GetRenderRect();
    
    // Background
    context->FillRectangle(rect, Color::White());
    context->DrawRectangle(rect, Color::FromHex(0xCCCCCC), 1.0f);
    
    // Tab strip background
    Rect tabStripRect(rect.x, rect.y, rect.width, m_tabHeight);
    context->FillRectangle(tabStripRect, Color::FromHex(0xF5F5F5));
    
    // Render tabs
    for (auto& tab : m_tabs) {
        tab->Render(context);
    }
    
    // Render content of selected tab
    if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_tabs.size())) {
        auto content = m_tabs[m_selectedIndex]->GetContent();
        if (content && content->GetIsVisible()) {
            content->Render(context);
        }
    }
}

} // namespace controls
} // namespace luaui
