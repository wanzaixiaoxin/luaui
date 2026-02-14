#pragma once

#include "Control.h"
#include "Panel.h"
#include "../rendering/Types.h"
#include <functional>
#include <memory>
#include <vector>
#include <string>

namespace luaui {
namespace controls {

// 前向声明
class Menu;
class MenuItem;
class ContextMenu;

/**
 * @brief MenuItem 菜单项（新架构）
 * 
 * 支持：
 * - 普通菜单项（带点击命令）
 * - 分隔线
 * - 子菜单
 * - 图标
 * - 快捷键显示
 * - 复选标记
 * - 禁用状态
 */
class MenuItem : public luaui::Control,
                 public std::enable_shared_from_this<MenuItem> {
public:
    // 菜单项类型
    enum class ItemType {
        Normal,     // 普通项
        Separator,  // 分隔线
        Submenu     // 子菜单
    };
    
    MenuItem();
    explicit MenuItem(const std::wstring& header);
    
    std::string GetTypeName() const override { return "MenuItem"; }
    
    // 类型
    ItemType GetItemType() const { return m_itemType; }
    void SetItemType(ItemType type);
    
    // 标题
    std::wstring GetHeader() const { return m_header; }
    void SetHeader(const std::wstring& header);
    
    // 图标
    std::wstring GetIcon() const { return m_icon; }
    void SetIcon(const std::wstring& icon) { m_icon = icon; }
    
    // 快捷键文本（如 "Ctrl+C"）
    std::wstring GetInputGestureText() const { return m_inputGestureText; }
    void SetInputGestureText(const std::wstring& text) { m_inputGestureText = text; }
    
    // 命令回调
    using CommandHandler = std::function<void(MenuItem*)>;
    void SetCommand(CommandHandler handler) { m_command = handler; }
    void InvokeCommand() { if (m_command) m_command(this); }
    
    // 复选状态
    bool GetIsChecked() const { return m_isChecked; }
    void SetIsChecked(bool checked);
    void ToggleCheck() { SetIsChecked(!m_isChecked); }
    
    // 是否可复选
    bool GetIsCheckable() const { return m_isCheckable; }
    void SetIsCheckable(bool checkable) { m_isCheckable = checkable; }
    
    // 禁用状态
    bool GetIsEnabled() const override { return m_itemEnabled; }
    void SetIsEnabled(bool enabled) override { m_itemEnabled = enabled; }
    
    // 子菜单
    std::shared_ptr<Menu> GetSubmenu() const { return m_submenu; }
    void SetSubmenu(const std::shared_ptr<Menu>& menu);
    bool GetHasSubmenu() const { return m_submenu != nullptr; }
    
    // 父菜单
    Menu* GetParentMenu() const { return m_parentMenu; }
    void SetParentMenu(Menu* menu) { m_parentMenu = menu; }
    
    // 点击事件
    luaui::Delegate<MenuItem*> Click;

protected:
    void InitializeComponents() override;
    void OnRender(rendering::IRenderContext* context) override;
    rendering::Size OnMeasure(const rendering::Size& availableSize) override;
    
    void OnMouseEnter() override;
    void OnMouseLeave() override;
    void OnClick() override;

private:
    void UpdateVisualState();
    
    ItemType m_itemType = ItemType::Normal;
    std::wstring m_header;
    std::wstring m_icon;
    std::wstring m_inputGestureText;
    CommandHandler m_command;
    
    bool m_isChecked = false;
    bool m_isCheckable = false;
    bool m_itemEnabled = true;
    bool m_isHovered = false;
    bool m_isPressed = false;
    
    std::shared_ptr<Menu> m_submenu;
    Menu* m_parentMenu = nullptr;
    
    // 外观
    float m_itemHeight = 28.0f;
    float m_iconWidth = 24.0f;
    float m_shortcutWidth = 100.0f;
    float m_padding = 8.0f;
    float m_fontSize = 14.0f;
    float m_separatorHeight = 9.0f;
    
    // 颜色
    rendering::Color m_normalBg = rendering::Color::Transparent();
    rendering::Color m_hoverBg = rendering::Color::FromHex(0xE5F3FF);
    rendering::Color m_disabledTextColor = rendering::Color::FromHex(0x999999);
    rendering::Color m_textColor = rendering::Color::Black();
    rendering::Color m_hoverTextColor = rendering::Color::Black();
    rendering::Color m_separatorColor = rendering::Color::FromHex(0xDDDDDD);
    rendering::Color m_checkColor = rendering::Color::FromHex(0x0078D4);
    rendering::Color m_arrowColor = rendering::Color::FromHex(0x666666);
};

/**
 * @brief Menu 菜单（新架构）
 * 
 * 支持：
 * - 垂直菜单列表
 * - 子菜单展开
 * - 滚动（内容过多时）
 */
class Menu : public luaui::Control {
public:
    Menu();
    
    std::string GetTypeName() const override { return "Menu"; }
    
    // 菜单项操作
    void AddItem(const std::shared_ptr<MenuItem>& item);
    void RemoveItem(const std::shared_ptr<MenuItem>& item);
    void RemoveItem(int index);
    void ClearItems();
    size_t GetItemCount() const { return m_items.size(); }
    std::shared_ptr<MenuItem> GetItem(int index);
    
    // 打开/关闭
    void Open();
    void OpenAt(float x, float y);
    void Close();
    bool GetIsOpen() const { return m_isOpen; }
    
    // 父菜单项（子菜单用）
    MenuItem* GetParentItem() const { return m_parentItem; }
    void SetParentItem(MenuItem* item) { m_parentItem = item; }
    
    // 最大高度（超出时显示滚动条）
    float GetMaxHeight() const { return m_maxHeight; }
    void SetMaxHeight(float height) { m_maxHeight = height; }

protected:
    void InitializeComponents() override;
    rendering::Size OnMeasure(const rendering::Size& availableSize) override;
    void OnRender(rendering::IRenderContext* context) override;
    void OnRenderChildren(rendering::IRenderContext* context) override;
    
    void OnMouseMove(MouseEventArgs& args) override;
    void OnMouseLeave() override;

private:
    friend class MenuItem;
    
    void OnItemHovered(MenuItem* item);
    void OnItemClicked(MenuItem* item);
    void ShowSubmenu(MenuItem* item);
    void HideSubmenu();
    int HitTestItem(float y);
    
    std::vector<std::shared_ptr<MenuItem>> m_items;
    MenuItem* m_hoveredItem = nullptr;
    MenuItem* m_openSubmenuItem = nullptr;
    MenuItem* m_parentItem = nullptr;
    
    bool m_isOpen = false;
    float m_maxHeight = 400.0f;
    float m_scrollOffset = 0.0f;
    float m_itemWidth = 200.0f;
    
    // 外观
    rendering::Color m_bgColor = rendering::Color::White();
    rendering::Color m_borderColor = rendering::Color::FromHex(0xCCCCCC);
    float m_borderWidth = 1.0f;
    float m_shadowOffset = 4.0f;
};

/**
 * @brief MenuBar 顶部菜单栏（新架构）
 * 
 * 应用场景：
 * - 文件、编辑、视图等主菜单
 */
class MenuBar : public Panel {
public:
    MenuBar();
    
    std::string GetTypeName() const override { return "MenuBar"; }
    
    // 添加顶级菜单
    void AddMenu(const std::wstring& header, const std::shared_ptr<Menu>& menu);
    void RemoveMenu(int index);
    void ClearMenus();
    
    // 高度
    float GetMenuHeight() const { return m_menuHeight; }
    void SetMenuHeight(float height) { m_menuHeight = height; }

protected:
    void InitializeComponents() override;
    rendering::Size OnMeasureChildren(const rendering::Size& availableSize) override;
    rendering::Size OnArrangeChildren(const rendering::Size& finalSize) override;
    void OnRenderChildren(rendering::IRenderContext* context) override;
    
    void OnMouseMove(MouseEventArgs& args) override;
    void OnMouseDown(MouseEventArgs& args) override;

private:
    struct MenuEntry {
        std::wstring header;
        std::shared_ptr<Menu> menu;
        bool isHovered = false;
        bool isOpen = false;
    };
    
    void OpenMenu(int index);
    void CloseAllMenus();
    int HitTestMenu(float x);
    
    std::vector<MenuEntry> m_menus;
    int m_openMenuIndex = -1;
    float m_menuHeight = 28.0f;
    float m_padding = 12.0f;
    
    rendering::Color m_bgColor = rendering::Color::FromHex(0xF5F5F5);
    rendering::Color m_hoverBg = rendering::Color::FromHex(0xE5F3FF);
    rendering::Color m_openBg = rendering::Color::FromHex(0xCCE4F7);
    rendering::Color m_textColor = rendering::Color::Black();
    float m_fontSize = 14.0f;
};

/**
 * @brief ContextMenu 上下文菜单（右键菜单）
 * 
 * 便捷类，封装 Menu 的上下文显示功能
 */
class ContextMenu : public Menu {
public:
    ContextMenu();
    
    std::string GetTypeName() const override { return "ContextMenu"; }
    
    // 在鼠标位置显示
    void ShowAtMouse();
    
    // 在控件位置显示
    void ShowRelativeTo(Control* control, float offsetX = 0, float offsetY = 0);
    
    // 便捷方法：关联控件右键事件
    static void AttachTo(Control* control, const std::shared_ptr<ContextMenu>& menu);
};

} // namespace controls
} // namespace luaui
