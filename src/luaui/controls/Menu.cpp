#include "Menu.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include "Components/InputComponent.h"
#include "Interfaces/IRenderable.h"
#include "Interfaces/ILayoutable.h"
#include "IRenderContext.h"
#include "Window.h"
#include "Theme.h"

namespace luaui {
namespace controls {

// ============================================================================
// MenuItem
// ============================================================================
MenuItem::MenuItem() {}

MenuItem::MenuItem(const std::wstring& header) : m_header(header) {}

void MenuItem::InitializeComponents() {
    GetComponents().AddComponent<components::LayoutComponent>(this);
    GetComponents().AddComponent<components::RenderComponent>(this);
    GetComponents().AddComponent<components::InputComponent>(this);
}

void MenuItem::SetItemType(ItemType type) {
    if (m_itemType != type) {
        m_itemType = type;
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
    }
}

void MenuItem::SetHeader(const std::wstring& header) {
    if (m_header != header) {
        m_header = header;
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
    }
}

void MenuItem::SetIsChecked(bool checked) {
    if (m_isChecked != checked) {
        m_isChecked = checked;
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void MenuItem::SetSubmenu(const std::shared_ptr<Menu>& menu) {
    m_submenu = menu;
    if (menu) {
        menu->SetParentItem(this);
        m_itemType = ItemType::Submenu;
    } else if (m_itemType == ItemType::Submenu) {
        m_itemType = ItemType::Normal;
    }
}

void MenuItem::OnMouseEnter() {
    m_isHovered = true;
    AnimateBgTo(GetTargetBgColor(), 150.0f);

    // 通知父菜单
    if (m_parentMenu) {
        m_parentMenu->OnItemHovered(this);
    }
}

void MenuItem::OnMouseLeave() {
    m_isHovered = false;
    AnimateBgTo(GetTargetBgColor(), 150.0f);
}

void MenuItem::OnClick() {
    if (!m_itemEnabled) return;
    
    if (m_itemType == ItemType::Normal) {
        if (m_isCheckable) {
            ToggleCheck();
        }
        InvokeCommand();
        Click.Invoke(this);
        
        // 关闭父菜单
        if (m_parentMenu) {
            m_parentMenu->Close();
        }
    } else if (m_itemType == ItemType::Submenu && m_parentMenu) {
        m_parentMenu->ShowSubmenu(this);
    }
}

void MenuItem::UpdateVisualState() {
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void MenuItem::ApplyTheme() {
    auto& t = Theme::GetCurrent();
    using namespace theme;
    m_hoverBg = t.GetColor(kMenuItemHoverBg);
    m_textColor = t.GetColor(kMenuItemText);
    m_hoverTextColor = t.GetColor(kMenuItemText);
    m_disabledTextColor = t.GetColor(kMenuItemDisabledText);
    m_separatorColor = t.GetColor(kMenuItemSeparator);
    m_checkColor = t.GetColor(kMenuItemCheckMark);
    m_arrowColor = t.GetColor(kMenuItemArrow);
    m_animBg = m_normalBg;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

rendering::Color MenuItem::GetTargetBgColor() const {
    if (!m_itemEnabled) return m_normalBg;
    if (m_isHovered) return m_hoverBg;
    return m_normalBg;
}

void MenuItem::AnimateBgTo(const rendering::Color& target, float durationMs) {
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

rendering::Size MenuItem::OnMeasure(const rendering::Size& availableSize) {
    (void)availableSize;
    
    if (m_itemType == ItemType::Separator) {
        return rendering::Size(100, m_separatorHeight);
    }
    
    // 计算宽度
    float width = m_padding; // 左边距
    width += m_iconWidth;    // 图标区域
    width += m_padding;      // 间距
    
    // 文本宽度（估算）
    if (!m_header.empty()) {
        width += static_cast<float>(m_header.length()) * m_fontSize * 0.6f;
    }
    
    width += m_padding * 2;  // 右边距
    
    // 快捷键宽度
    if (!m_inputGestureText.empty()) {
        width += m_shortcutWidth;
    }
    
    // 子菜单箭头
    if (GetHasSubmenu()) {
        width += m_fontSize;
    }
    
    return rendering::Size(width, m_itemHeight);
}

void MenuItem::OnRender(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    auto rect = render->GetRenderRect();
    
    if (m_itemType == ItemType::Separator) {
        // 绘制分隔线
        auto sepBrush = context->CreateSolidColorBrush(m_separatorColor);
        if (sepBrush) {
            float lineY = rect.y + rect.height / 2;
            context->DrawLine(rendering::Point(rect.x + m_padding, lineY),
                              rendering::Point(rect.x + rect.width - m_padding, lineY),
                              sepBrush.get(), 1.0f);
        }
        return;
    }
    
    // 绘制背景（使用动画颜色）
    if (m_animBg.a > 0 && m_itemEnabled) {
        auto bgBrush = context->CreateSolidColorBrush(m_animBg);
        if (bgBrush) {
            context->FillRectangle(rect, bgBrush.get());
        }
    }
    
    rendering::Color textColor = m_itemEnabled ? m_textColor : m_disabledTextColor;
    
    // 绘制勾选标记
    if (m_isCheckable && m_isChecked) {
        auto checkBrush = context->CreateSolidColorBrush(m_checkColor);
        if (checkBrush) {
            float checkX = rect.x + m_padding + 4;
            float checkY = rect.y + (rect.height - 10) / 2;
            // 简化的勾选标记：画一个对勾
            context->DrawLine(rendering::Point(checkX, checkY + 5),
                              rendering::Point(checkX + 4, checkY + 10),
                              checkBrush.get(), 2.0f);
            context->DrawLine(rendering::Point(checkX + 4, checkY + 10),
                              rendering::Point(checkX + 10, checkY + 2),
                              checkBrush.get(), 2.0f);
        }
    }
    
    // 绘制图标（如果有）
    // TODO: 实现图标绘制
    
    // 绘制文本
    if (!m_header.empty()) {
        auto textBrush = context->CreateSolidColorBrush(textColor);
        auto textFormat = context->CreateTextFormat(L"Microsoft YaHei", m_fontSize);
        
        if (textBrush && textFormat) {
            float textX = rect.x + m_padding + m_iconWidth + m_padding;
            float textY = rect.y + (rect.height - m_fontSize) / 2;
            context->DrawTextString(m_header, textFormat.get(), 
                                    rendering::Point(textX, textY), textBrush.get());
        }
    }
    
    // 绘制快捷键
    if (!m_inputGestureText.empty()) {
        auto shortcutBrush = context->CreateSolidColorBrush(
            m_itemEnabled ? rendering::Color::FromHex(0x666666) : m_disabledTextColor);
        auto textFormat = context->CreateTextFormat(L"Microsoft YaHei", m_fontSize);
        
        if (shortcutBrush && textFormat) {
            textFormat->SetTextAlignment(rendering::TextAlignment::Trailing);
            float shortcutX = rect.x + rect.width - m_padding;
            float shortcutY = rect.y + (rect.height - m_fontSize) / 2;
            context->DrawTextString(m_inputGestureText, textFormat.get(),
                                    rendering::Point(shortcutX - m_shortcutWidth + 10, shortcutY),
                                    shortcutBrush.get());
        }
    }
    
    // 绘制子菜单箭头
    if (GetHasSubmenu()) {
        auto arrowBrush = context->CreateSolidColorBrush(m_arrowColor);
        if (arrowBrush) {
            float arrowX = rect.x + rect.width - m_padding - 6;
            float arrowY = rect.y + rect.height / 2;
            context->DrawLine(rendering::Point(arrowX, arrowY - 4),
                              rendering::Point(arrowX + 4, arrowY),
                              arrowBrush.get(), 1.5f);
            context->DrawLine(rendering::Point(arrowX + 4, arrowY),
                              rendering::Point(arrowX, arrowY + 4),
                              arrowBrush.get(), 1.5f);
        }
    }
}

// ============================================================================
// Menu
// ============================================================================
Menu::Menu() {}

void Menu::ApplyTheme() {
    auto& t = Theme::GetCurrent();
    using namespace theme;
    m_bgColor = t.GetColor(kMenuBg);
    m_borderColor = t.GetColor(kMenuBorder);
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void Menu::InitializeComponents() {
    GetComponents().AddComponent<components::LayoutComponent>(this);
    GetComponents().AddComponent<components::RenderComponent>(this);
    GetComponents().AddComponent<components::InputComponent>(this);
    
    SetIsVisible(false);
}

void Menu::AddItem(const std::shared_ptr<MenuItem>& item) {
    if (!item) return;
    
    item->SetParentMenu(this);
    m_items.push_back(item);
    // Note: Menu inherits from Control, not Panel
    // Items are rendered by Menu's custom render logic
    
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

void Menu::RemoveItem(const std::shared_ptr<MenuItem>& item) {
    auto it = std::find(m_items.begin(), m_items.end(), item);
    if (it != m_items.end()) {
        m_items.erase(it);
        
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
    }
}

void Menu::RemoveItem(int index) {
    if (index >= 0 && index < static_cast<int>(m_items.size())) {
        RemoveItem(m_items[index]);
    }
}

void Menu::ClearItems() {
    m_items.clear();
    
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

std::shared_ptr<MenuItem> Menu::GetItem(int index) {
    if (index >= 0 && index < static_cast<int>(m_items.size())) {
        return m_items[index];
    }
    return nullptr;
}

void Menu::Open() {
    m_isOpen = true;
    SetIsVisible(true);
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void Menu::OpenAt(float x, float y) {
    if (auto* layoutable = AsLayoutable()) {
        auto size = GetLayout() ? GetLayout()->GetDesiredSize() : rendering::Size(200, 100);
        layoutable->Arrange(rendering::Rect(x, y, size.width, size.height));
    }
    Open();
}

void Menu::Close() {
    // 关闭子菜单
    HideSubmenu();
    
    m_isOpen = false;
    SetIsVisible(false);
    m_hoveredItem = nullptr;
    
    // 关闭父菜单链
    if (m_parentItem && m_parentItem->GetParentMenu()) {
        m_parentItem->GetParentMenu()->Close();
    }
}

void Menu::OnItemHovered(MenuItem* item) {
    if (m_hoveredItem != item) {
        m_hoveredItem = item;
        
        // 如果悬停在新项上，且有子菜单，显示子菜单
        if (item && item->GetHasSubmenu()) {
            ShowSubmenu(item);
        } else if (m_openSubmenuItem && m_openSubmenuItem != item) {
            // 如果移出子菜单项，关闭子菜单
            // 延迟关闭逻辑可以在这里添加
            HideSubmenu();
        }
    }
}

void Menu::OnItemClicked(MenuItem* item) {
    (void)item;
    // 处理点击逻辑
}

void Menu::ShowSubmenu(MenuItem* item) {
    if (!item || !item->GetHasSubmenu()) return;
    
    // 关闭当前打开的兄弟子菜单
    if (m_openSubmenuItem && m_openSubmenuItem != item) {
        HideSubmenu();
    }
    
    m_openSubmenuItem = item;
    
    // 计算子菜单位置
    rendering::Rect itemRect;
    if (auto* renderable = item->AsRenderable()) {
        itemRect = renderable->GetRenderRect();
    }
    
    auto submenu = item->GetSubmenu();
    if (submenu) {
        submenu->OpenAt(itemRect.x + itemRect.width, itemRect.y);
    }
}

void Menu::HideSubmenu() {
    if (m_openSubmenuItem) {
        if (auto submenu = m_openSubmenuItem->GetSubmenu()) {
            submenu->Close();
        }
        m_openSubmenuItem = nullptr;
    }
}

int Menu::HitTestItem(float y) {
    float currentY = 0;
    for (size_t i = 0; i < m_items.size(); ++i) {
        if (auto* layoutable = m_items[i]->AsLayoutable()) {
            auto size = layoutable->GetDesiredSize();
            if (y >= currentY && y < currentY + size.height) {
                return static_cast<int>(i);
            }
            currentY += size.height;
        }
    }
    return -1;
}

rendering::Size Menu::OnMeasure(const rendering::Size& availableSize) {
    (void)availableSize;
    
    float maxWidth = 0;
    float totalHeight = 0;
    
    for (auto& item : m_items) {
        if (auto* layoutable = item->AsLayoutable()) {
            interfaces::LayoutConstraint constraint;
            constraint.available = rendering::Size(m_itemWidth, 0);
            layoutable->Measure(constraint);
            auto size = layoutable->GetDesiredSize();
            maxWidth = std::max(maxWidth, size.width);
            totalHeight += size.height;
        }
    }
    
    // 限制最大高度
    float height = std::min(totalHeight, m_maxHeight);
    
    return rendering::Size(maxWidth + m_borderWidth * 2, height + m_borderWidth * 2);
}

void Menu::OnRender(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    auto rect = render->GetRenderRect();
    
    // 绘制阴影（简化版）
    auto shadowBrush = context->CreateSolidColorBrush(rendering::Color(0, 0, 0, 0.2f));
    if (shadowBrush) {
        context->FillRectangle(
            rendering::Rect(rect.x + m_shadowOffset, rect.y + m_shadowOffset,
                           rect.width, rect.height),
            shadowBrush.get());
    }
    
    // 绘制背景
    auto bgBrush = context->CreateSolidColorBrush(m_bgColor);
    if (bgBrush) {
        context->FillRectangle(rect, bgBrush.get());
    }
    
    // 绘制边框
    auto borderBrush = context->CreateSolidColorBrush(m_borderColor);
    if (borderBrush) {
        context->DrawRectangle(rect, borderBrush.get(), m_borderWidth);
    }
}

void Menu::OnRenderChildren(rendering::IRenderContext* context) {
    if (!context) return;
    
    rendering::Rect contentRect;
    if (auto* renderable = AsRenderable()) {
        contentRect = renderable->GetRenderRect();
    }
    
    float y = contentRect.y + m_borderWidth - m_scrollOffset;
    
    for (auto& item : m_items) {
        if (auto* itemRenderable = item->AsRenderable()) {
            if (auto* layoutable = item->AsLayoutable()) {
                auto size = layoutable->GetDesiredSize();
                
                // 可见性测试
                if (y + size.height > contentRect.y && y < contentRect.y + contentRect.height) {
                    // 临时调整渲染区域
                    itemRenderable->Render(context);
                }
                
                y += size.height;
            }
        }
    }
}

void Menu::OnMouseMove(MouseEventArgs& args) {
    rendering::Rect rect;
    if (auto* renderable = AsRenderable()) {
        rect = renderable->GetRenderRect();
    }

    float localY = args.y - rect.y - m_borderWidth + m_scrollOffset;
    int index = HitTestItem(localY);

    if (index >= 0) {
        if (m_hoveredItem != m_items[index].get()) {
            if (m_hoveredItem) {
                m_hoveredItem->NotifyMouseLeave();
            }
            m_hoveredItem = m_items[index].get();
            m_hoveredItem->NotifyMouseEnter();
        }
    } else if (m_hoveredItem) {
        m_hoveredItem->NotifyMouseLeave();
        m_hoveredItem = nullptr;
    }

    args.Handled = true;
}

void Menu::OnMouseLeave() {
    if (m_hoveredItem) {
        m_hoveredItem->NotifyMouseLeave();
        m_hoveredItem = nullptr;
    }
}

void Menu::OnKeyDown(KeyEventArgs& args) {
    if (m_items.empty()) {
        Control::OnKeyDown(args);
        return;
    }

    if (args.keyCode == VK_UP) {
        if (m_focusedIndex < 0) m_focusedIndex = static_cast<int>(m_items.size()) - 1;
        else m_focusedIndex = (m_focusedIndex - 1 + static_cast<int>(m_items.size()))
                               % static_cast<int>(m_items.size());
        // 跳过分隔线
        while (m_focusedIndex > 0 && m_items[m_focusedIndex]->GetItemType() == MenuItem::ItemType::Separator) {
            m_focusedIndex = (m_focusedIndex - 1 + static_cast<int>(m_items.size()))
                              % static_cast<int>(m_items.size());
        }
        m_hoveredItem = m_items[m_focusedIndex].get();
        m_hoveredItem->NotifyMouseEnter();
        args.Handled = true;
        return;
    }

    if (args.keyCode == VK_DOWN) {
        if (m_focusedIndex < 0) m_focusedIndex = 0;
        else m_focusedIndex = (m_focusedIndex + 1) % static_cast<int>(m_items.size());
        // 跳过分隔线
        while (m_focusedIndex < static_cast<int>(m_items.size()) - 1 &&
               m_items[m_focusedIndex]->GetItemType() == MenuItem::ItemType::Separator) {
            m_focusedIndex = (m_focusedIndex + 1) % static_cast<int>(m_items.size());
        }
        m_hoveredItem = m_items[m_focusedIndex].get();
        m_hoveredItem->NotifyMouseEnter();
        args.Handled = true;
        return;
    }

    if (args.keyCode == VK_RETURN || args.keyCode == VK_SPACE) {
        if (m_focusedIndex >= 0 && m_focusedIndex < static_cast<int>(m_items.size())) {
            m_items[m_focusedIndex]->InvokeCommand();
            m_items[m_focusedIndex]->Click.Invoke(m_items[m_focusedIndex].get());
            Close();
        }
        args.Handled = true;
        return;
    }

    if (args.keyCode == VK_ESCAPE) {
        Close();
        args.Handled = true;
        return;
    }

    Control::OnKeyDown(args);
}

// ============================================================================
// MenuBar
// ============================================================================
MenuBar::MenuBar() {}

void MenuBar::ApplyTheme() {
    auto& t = Theme::GetCurrent();
    using namespace theme;
    m_bgColor = t.GetColor(kMenuBarBg);
    m_hoverBg = t.GetColor(kMenuBarHoverBg);
    m_openBg = t.GetColor(kMenuBarOpenBg);
    m_textColor = t.GetColor(kMenuBarText);
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void MenuBar::InitializeComponents() {
    Panel::InitializeComponents();

    // 添加InputComponent以接收鼠标事件
    GetComponents().AddComponent<components::InputComponent>(this);
}

void MenuBar::AddMenu(const std::wstring& header, const std::shared_ptr<Menu>& menu) {
    if (!menu) return;

    MenuEntry entry;
    entry.header = header;
    entry.menu = menu;
    m_menus.push_back(entry);

    // Menu不作为Panel的子控件添加，避免被裁剪
    // Menu将在OpenMenu时临时添加到窗口根控件

    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

void MenuBar::RemoveMenu(int index) {
    if (index >= 0 && index < static_cast<int>(m_menus.size())) {
        m_menus.erase(m_menus.begin() + index);
        
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
    }
}

void MenuBar::ClearMenus() {
    CloseAllMenus();
    m_menus.clear();
    
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

void MenuBar::OpenMenu(int index) {
    if (index < 0 || index >= static_cast<int>(m_menus.size())) return;

    // 关闭其他菜单
    if (m_openMenuIndex >= 0 && m_openMenuIndex != index) {
        m_menus[m_openMenuIndex].isOpen = false;
        m_menus[m_openMenuIndex].menu->Close();
    }

    m_openMenuIndex = index;
    m_menus[index].isOpen = true;

    // 计算菜单位置
    auto* barRender = GetRender();
    if (!barRender) return;

    rendering::Rect barRect = barRender->GetRenderRect();

    float x = barRect.x + m_padding;
    for (int i = 0; i < index; ++i) {
        x += static_cast<float>(m_menus[i].header.length()) * m_fontSize * 0.6f + m_padding * 2;
    }

    // 先测量Menu的大小
    auto& menu = m_menus[index].menu;
    if (auto* layout = menu->GetLayout()) {
        interfaces::LayoutConstraint constraint;
        constraint.available = rendering::Size(barRect.width, 400); // 最大高度400
        constraint.maxHeight = 400;
        layout->Measure(constraint);
    }

    menu->OpenAt(x, barRect.y + barRect.height);

    if (auto* r = GetRender()) {
        r->Invalidate();
    }
}

void MenuBar::CloseAllMenus() {
    if (m_openMenuIndex >= 0) {
        m_menus[m_openMenuIndex].isOpen = false;
        m_menus[m_openMenuIndex].menu->Close();
        m_openMenuIndex = -1;
    }
    
    for (auto& entry : m_menus) {
        entry.isHovered = false;
    }
    
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

int MenuBar::HitTestMenu(float x, float y) {
    auto* render = GetRender();
    if (!render) return -1;

    rendering::Rect barRect = render->GetRenderRect();

    // 检查Y坐标是否在菜单栏范围内
    if (y < barRect.y || y >= barRect.y + barRect.height) {
        return -1;
    }

    float currentX = barRect.x + m_padding;
    for (size_t i = 0; i < m_menus.size(); ++i) {
        float menuWidth = static_cast<float>(m_menus[i].header.length()) * m_fontSize * 0.6f + m_padding * 2;
        if (x >= currentX && x <= currentX + menuWidth) {
            return static_cast<int>(i);
        }
        currentX += menuWidth;
    }
    return -1;
}

rendering::Size MenuBar::OnMeasureChildren(const rendering::Size& availableSize) {
    (void)availableSize;
    float totalWidth = m_padding;
    for (const auto& entry : m_menus) {
        totalWidth += static_cast<float>(entry.header.length()) * m_fontSize * 0.6f + m_padding * 2;
    }
    return rendering::Size(totalWidth, m_menuHeight);
}

rendering::Size MenuBar::OnArrangeChildren(const rendering::Size& finalSize) {
    (void)finalSize;
    return finalSize;
}

void MenuBar::OnRenderChildren(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    auto barRect = render->GetRenderRect();
    
    // 绘制背景
    auto bgBrush = context->CreateSolidColorBrush(m_bgColor);
    if (bgBrush) {
        context->FillRectangle(barRect, bgBrush.get());
    }
    
    // 绘制底边框
    auto& t = Theme::GetCurrent();
    auto borderBrush = context->CreateSolidColorBrush(t.GetColor(theme::kMenuBorder));
    if (borderBrush) {
        context->DrawLine(rendering::Point(barRect.x, barRect.y + barRect.height - 1),
                          rendering::Point(barRect.x + barRect.width, barRect.y + barRect.height - 1),
                          borderBrush.get(), 1.0f);
    }
    
    // 计算窗口按钮占用的宽度
    float btnAreaWidth = m_showWindowControls ? (m_btnWidth * 3) : 0;
    float maxMenuWidth = barRect.width - btnAreaWidth - m_padding * 2;
    
    // 绘制菜单项
    float x = barRect.x + m_padding;
    auto textFormat = context->CreateTextFormat(L"Microsoft YaHei", m_fontSize);
    
    for (size_t i = 0; i < m_menus.size(); ++i) {
        const auto& entry = m_menus[i];
        float menuWidth = static_cast<float>(entry.header.length()) * m_fontSize * 0.6f + m_padding * 2;
        
        // 确保不超出可用空间
        if (x + menuWidth > barRect.x + barRect.width - btnAreaWidth - m_padding) {
            break;
        }
        
        rendering::Rect menuRect(x, barRect.y, menuWidth, barRect.height);
        
        // 绘制背景
        if (entry.isOpen) {
            auto openBrush = context->CreateSolidColorBrush(m_openBg);
            if (openBrush) {
                context->FillRectangle(menuRect, openBrush.get());
            }
        } else if (entry.isHovered) {
            auto hoverBrush = context->CreateSolidColorBrush(m_hoverBg);
            if (hoverBrush) {
                context->FillRectangle(menuRect, hoverBrush.get());
            }
        }
        
        // 绘制文本
        auto textBrush = context->CreateSolidColorBrush(m_textColor);
        if (textBrush && textFormat) {
            float textY = barRect.y + (barRect.height - m_fontSize) / 2;
            context->DrawTextString(entry.header, textFormat.get(),
                                    rendering::Point(x + m_padding, textY), textBrush.get());
        }
        
        x += menuWidth;
    }
    
    // 绘制窗口控制按钮
    DrawWindowButtons(context, barRect);

    // 手动渲染打开的Menu（Menu不作为子控件，避免被裁剪）
    if (m_openMenuIndex >= 0 && m_openMenuIndex < static_cast<int>(m_menus.size())) {
        auto& menu = m_menus[m_openMenuIndex].menu;
        if (menu && menu->GetIsVisible()) {
            // 获取Menu的渲染矩形
            auto* menuRender = menu->GetRender();
            if (menuRender) {
                const auto& menuRect = menuRender->GetRenderRect();
                // 渲染Menu
                menu->OnRender(context);
            }
        }
    }
}

MenuBar::WindowButton MenuBar::HitTestWindowButton(float x, float y) {
    if (!m_showWindowControls) return WindowButton::None;

    auto* render = GetRender();
    if (!render) return WindowButton::None;

    rendering::Rect barRect = render->GetRenderRect();
    float btnY = barRect.y;
    float btnH = barRect.height;

    // 按钮从右向左排列：关闭、最大化、最小化
    float closeX = barRect.x + barRect.width - m_btnWidth;
    float maxX = closeX - m_btnWidth;
    float minX = maxX - m_btnWidth;

    // 检查Y坐标是否在菜单栏范围内
    if (y >= btnY && y < btnY + btnH) {
        // 检查X坐标落在哪个按钮区域
        if (x >= closeX && x < closeX + m_btnWidth) return WindowButton::Close;
        if (x >= maxX && x < maxX + m_btnWidth) return WindowButton::Maximize;
        if (x >= minX && x < minX + m_btnWidth) return WindowButton::Minimize;
    }
    return WindowButton::None;
}

void MenuBar::DrawWindowButtons(rendering::IRenderContext* context, const rendering::Rect& rect) {
    if (!m_showWindowControls) return;
    
    float btnY = rect.y;
    float btnH = rect.height;
    
    // 按钮从右向左排列：最小化、最大化、关闭
    float closeX = rect.x + rect.width - m_btnWidth;
    float maxX = closeX - m_btnWidth;
    float minX = maxX - m_btnWidth;
    
    auto textFormat = context->CreateTextFormat(L"Segoe MDL2 Assets", 10.0f);
    if (!textFormat) {
        textFormat = context->CreateTextFormat(L"Microsoft YaHei", 10.0f);
    }
    if (textFormat) {
        textFormat->SetTextAlignment(rendering::TextAlignment::Center);
        textFormat->SetParagraphAlignment(rendering::ParagraphAlignment::Center);
    }
    
    // 绘制最小化按钮
    rendering::Color minBg = (m_pressedBtn == WindowButton::Minimize) ? m_btnPressBg :
                              (m_hoveredBtn == WindowButton::Minimize) ? m_btnHoverBg : rendering::Color::Transparent();
    auto minBrush = context->CreateSolidColorBrush(minBg);
    if (minBrush) {
        context->FillRectangle(rendering::Rect(minX, btnY, m_btnWidth, btnH), minBrush.get());
    }
    auto minTextBrush = context->CreateSolidColorBrush(m_textColor);
    if (minTextBrush && textFormat) {
        context->DrawTextString(L"\uE921", textFormat.get(), 
                                rendering::Rect(minX, btnY, m_btnWidth, btnH), minTextBrush.get());
    }
    
    // 绘制最大化按钮
    rendering::Color maxBg = (m_pressedBtn == WindowButton::Maximize) ? m_btnPressBg :
                              (m_hoveredBtn == WindowButton::Maximize) ? m_btnHoverBg : rendering::Color::Transparent();
    auto maxBrush = context->CreateSolidColorBrush(maxBg);
    if (maxBrush) {
        context->FillRectangle(rendering::Rect(maxX, btnY, m_btnWidth, btnH), maxBrush.get());
    }
    auto maxTextBrush = context->CreateSolidColorBrush(m_textColor);
    if (maxTextBrush && textFormat) {
        context->DrawTextString(L"\uE922", textFormat.get(),
                                rendering::Rect(maxX, btnY, m_btnWidth, btnH), maxTextBrush.get());
    }
    
    // 绘制关闭按钮
    rendering::Color closeBg = (m_pressedBtn == WindowButton::Close) ? m_closePressBg :
                                (m_hoveredBtn == WindowButton::Close) ? m_closeHoverBg : rendering::Color::Transparent();
    auto closeBrush = context->CreateSolidColorBrush(closeBg);
    if (closeBrush) {
        context->FillRectangle(rendering::Rect(closeX, btnY, m_btnWidth, btnH), closeBrush.get());
    }
    rendering::Color closeTextColor = (m_hoveredBtn == WindowButton::Close) ? rendering::Color::White() : m_textColor;
    auto closeTextBrush = context->CreateSolidColorBrush(closeTextColor);
    if (closeTextBrush && textFormat) {
        context->DrawTextString(L"\uE8BB", textFormat.get(),
                                rendering::Rect(closeX, btnY, m_btnWidth, btnH), closeTextBrush.get());
    }
}

void MenuBar::ExecuteWindowButton(WindowButton btn) {
    auto* wnd = GetWindow();
    if (!wnd) return;
    
    HWND hwnd = wnd->GetHandle();
    if (!hwnd) return;
    
    switch (btn) {
        case WindowButton::Minimize:
            ShowWindow(hwnd, SW_MINIMIZE);
            break;
        case WindowButton::Maximize:
            if (IsZoomed(hwnd)) {
                ShowWindow(hwnd, SW_RESTORE);
            } else {
                ShowWindow(hwnd, SW_MAXIMIZE);
            }
            break;
        case WindowButton::Close:
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            break;
        default:
            break;
    }
}

void MenuBar::OnMouseMove(MouseEventArgs& args) {
    int index = HitTestMenu(args.x, args.y);

    for (size_t i = 0; i < m_menus.size(); ++i) {
        bool wasHovered = m_menus[i].isHovered;
        m_menus[i].isHovered = (static_cast<int>(i) == index);

        // 如果有一个菜单已打开，悬停到其他菜单时自动打开
        if (m_openMenuIndex >= 0 && m_menus[i].isHovered && !wasHovered && static_cast<int>(i) != m_openMenuIndex) {
            OpenMenu(static_cast<int>(i));
        }
    }

    // 检测窗口按钮悬停
    WindowButton prevHovered = m_hoveredBtn;
    m_hoveredBtn = HitTestWindowButton(args.x, args.y);
    if (prevHovered != m_hoveredBtn) {
        if (auto* render = GetRender()) render->Invalidate();
    }

    if (auto* render = GetRender()) {
        render->Invalidate();
    }

    args.Handled = true;
}

void MenuBar::OnMouseDown(MouseEventArgs& args) {
    // 先检测窗口按钮
    WindowButton btn = HitTestWindowButton(args.x, args.y);
    if (btn != WindowButton::None) {
        m_pressedBtn = btn;
        if (auto* render = GetRender()) render->Invalidate();
        args.Handled = true;
        return;
    }
    
    int index = HitTestMenu(args.x, args.y);
    if (index >= 0) {
        if (m_openMenuIndex == index) {
            // 点击已打开的菜单，关闭它
            CloseAllMenus();
        } else {
            OpenMenu(index);
        }
    } else {
        CloseAllMenus();
    }
    args.Handled = true;
}

void MenuBar::OnMouseUp(MouseEventArgs& args) {
    if (m_pressedBtn != WindowButton::None) {
        WindowButton btn = HitTestWindowButton(args.x, args.y);
        if (btn == m_pressedBtn) {
            ExecuteWindowButton(btn);
        }
        m_pressedBtn = WindowButton::None;
        if (auto* render = GetRender()) render->Invalidate();
    }
    args.Handled = true;
}

void MenuBar::OnKeyDown(KeyEventArgs& args) {
    if (m_menus.empty()) {
        Panel::OnKeyDown(args);
        return;
    }

    if (args.keyCode == VK_LEFT) {
        if (m_openMenuIndex >= 0) {
            m_menus[m_openMenuIndex].isOpen = false;
            m_menus[m_openMenuIndex].menu->Close();
            m_openMenuIndex = (m_openMenuIndex - 1 + static_cast<int>(m_menus.size()))
                               % static_cast<int>(m_menus.size());
            m_isKeyboardNavigating = true;
            OpenMenu(m_openMenuIndex);
        }
        args.Handled = true;
        return;
    }

    if (args.keyCode == VK_RIGHT) {
        if (m_openMenuIndex >= 0) {
            m_menus[m_openMenuIndex].isOpen = false;
            m_menus[m_openMenuIndex].menu->Close();
            m_openMenuIndex = (m_openMenuIndex + 1) % static_cast<int>(m_menus.size());
            m_isKeyboardNavigating = true;
            OpenMenu(m_openMenuIndex);
        }
        args.Handled = true;
        return;
    }

    if (args.keyCode == VK_ESCAPE) {
        CloseAllMenus();
        args.Handled = true;
        return;
    }

    Panel::OnKeyDown(args);
}

// ============================================================================
// ContextMenu
// ============================================================================
ContextMenu::ContextMenu() {}

void ContextMenu::ShowAtMouse() {
    POINT pt;
    GetCursorPos(&pt);
    OpenAt(static_cast<float>(pt.x), static_cast<float>(pt.y));
}

void ContextMenu::ShowRelativeTo(Control* control, float offsetX, float offsetY) {
    if (!control) return;
    
    rendering::Rect rect;
    if (auto* renderable = control->AsRenderable()) {
        rect = renderable->GetRenderRect();
    }
    
    OpenAt(rect.x + offsetX, rect.y + rect.height + offsetY);
}

void ContextMenu::AttachTo(Control* control, const std::shared_ptr<ContextMenu>& menu) {
    (void)control;
    (void)menu;
    // 实际实现：绑定控件的右键事件
}

} // namespace controls
} // namespace luaui
