#pragma once

#include "Panel.h"
#include "ListBox.h"
#include "../rendering/Types.h"
#include <memory>
#include <vector>
#include <string>

namespace luaui {
namespace controls {

/**
 * @brief ComboBox 下拉选择框（新架构）
 * 
 * 特点：
 * - 显示当前选中项
 * - 点击展开/收起下拉列表
 * - 支持下拉动画
 * - 支持最大下拉高度限制
 */
class ComboBox : public Panel {
public:
    ComboBox();
    
    std::string GetTypeName() const override { return "ComboBox"; }
    
    // 数据操作
    void AddItem(const std::wstring& item);
    void RemoveItem(int index);
    void ClearItems();
    size_t GetItemCount() const { return m_items.size(); }
    std::wstring GetItem(int index) const;
    
    // 选中操作
    int GetSelectedIndex() const { return m_selectedIndex; }
    void SetSelectedIndex(int index);
    std::wstring GetSelectedItem() const;
    std::wstring GetText() const { return GetSelectedItem(); }
    void SetText(const std::wstring& text); // 通过文本设置选中
    
    // 外观属性
    float GetDropDownHeight() const { return m_dropDownHeight; }
    void SetDropDownHeight(float height) { m_dropDownHeight = height; }
    
    float GetItemHeight() const { return m_itemHeight; }
    void SetItemHeight(float height) { m_itemHeight = height; }
    
    // 状态
    bool GetIsDropDownOpen() const { return m_isDropDownOpen; }
    void SetIsDropDownOpen(bool open);
    
    // 占位符文本（未选中时显示）
    std::wstring GetPlaceholder() const { return m_placeholder; }
    void SetPlaceholder(const std::wstring& text) { m_placeholder = text; }
    
    // 事件
    luaui::Delegate<ComboBox*, int> SelectionChanged;
    luaui::Delegate<ComboBox*, bool> DropDownOpenedChanged;

protected:
    void InitializeComponents() override;
    void OnRender(rendering::IRenderContext* context) override;
    rendering::Size OnMeasure(const rendering::Size& availableSize) override;
    rendering::Size OnArrangeChildren(const rendering::Size& finalSize) override;
    void OnRenderChildren(rendering::IRenderContext* context) override;
    
    // 输入处理
    void OnClick() override;
    void OnMouseDown(MouseEventArgs& args) override;
    void OnMouseEnter() override;
    void OnMouseLeave() override;
    
    // 焦点处理
    void OnGotFocus() override;
    void OnLostFocus() override;

private:
    void ToggleDropDown();
    void OpenDropDown();
    void CloseDropDown();
    void UpdateVisualState();
    void OnItemSelected(int index);
    int HitTestDropDownItem(float x, float y);
    
    // 绘制下拉按钮箭头
    void DrawDropDownArrow(rendering::IRenderContext* context, const rendering::Rect& rect);
    
    // 数据
    std::vector<std::wstring> m_items;
    int m_selectedIndex = -1;
    
    // 状态
    bool m_isDropDownOpen = false;
    bool m_isHovered = false;
    bool m_isPressed = false;
    float m_dropDownAnimation = 0.0f; // 0.0 ~ 1.0 动画进度
    
    // 外观
    float m_dropDownHeight = 200.0f;  // 最大下拉高度
    float m_itemHeight = 28.0f;
    float m_padding = 8.0f;
    float m_arrowWidth = 20.0f;
    std::wstring m_placeholder = L"请选择...";
    
    // 颜色
    rendering::Color m_borderColor = rendering::Color::FromHex(0x8E8E8E);
    rendering::Color m_borderHoverColor = rendering::Color::FromHex(0x0078D4);
    rendering::Color m_bgColor = rendering::Color::White();
    rendering::Color m_dropDownBgColor = rendering::Color::White();
    rendering::Color m_itemHoverColor = rendering::Color::FromHex(0xE5F3FF);
    rendering::Color m_itemSelectedColor = rendering::Color::FromHex(0x0078D4);
    rendering::Color m_textColor = rendering::Color::Black();
    rendering::Color m_placeholderColor = rendering::Color::FromHex(0x999999);
    rendering::Color m_arrowColor = rendering::Color::FromHex(0x666666);
};

} // namespace controls
} // namespace luaui
