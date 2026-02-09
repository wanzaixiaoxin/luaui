#pragma once

#include "Control.h"

namespace luaui {
namespace controls {

// ==================== CheckBox ====================
class CheckBox : public Control {
public:
    CheckBox();
    
    std::string GetTypeName() const override { return "CheckBox"; }
    
    // Checked 状态
    bool GetIsChecked() const { return m_isChecked; }
    void SetIsChecked(bool checked);
    
    // 文本
    std::wstring GetText() const { return m_text; }
    void SetText(const std::wstring& text);
    
    // 事件
    using CheckChangedHandler = std::function<void(CheckBox* sender, bool isChecked)>;
    void SetCheckChangedHandler(CheckChangedHandler handler) { m_checkChangedHandler = handler; }
    
    void Render(IRenderContext* context) override;
    
protected:
    Size MeasureOverride(const Size& availableSize) override;
    void OnMouseDown(MouseEventArgs& args) override;
    void OnKeyDown(KeyEventArgs& args) override;

private:
    bool m_isChecked = false;
    std::wstring m_text;
    CheckChangedHandler m_checkChangedHandler;
    
    // 尺寸
    static constexpr float BoxSize = 18.0f;
    static constexpr float TextSpacing = 8.0f;
};

// ==================== RadioButton ====================
class RadioButton : public Control {
public:
    RadioButton();
    ~RadioButton();
    
    std::string GetTypeName() const override { return "RadioButton"; }
    
    // Selected 状态
    bool GetIsSelected() const { return m_isSelected; }
    void SetIsSelected(bool selected);
    
    // 组名
    std::wstring GetGroupName() const { return m_groupName; }
    void SetGroupName(const std::wstring& name) { m_groupName = name; }
    
    // 文本
    std::wstring GetText() const { return m_text; }
    void SetText(const std::wstring& text);
    
    // 事件
    using SelectedHandler = std::function<void(RadioButton* sender)>;
    void SetSelectedHandler(SelectedHandler handler) { m_selectedHandler = handler; }
    
    void Render(IRenderContext* context) override;
    
protected:
    Size MeasureOverride(const Size& availableSize) override;
    void OnMouseDown(MouseEventArgs& args) override;
    void OnKeyDown(KeyEventArgs& args) override;

private:
    bool m_isSelected = false;
    std::wstring m_groupName = L"default";
    std::wstring m_text;
    SelectedHandler m_selectedHandler;
    
    static constexpr float CircleSize = 18.0f;
    static constexpr float TextSpacing = 8.0f;
};

// ==================== RadioButtonGroup ====================
class RadioButtonGroup {
public:
    void AddButton(RadioButton* button);
    void RemoveButton(RadioButton* button);
    void SelectButton(RadioButton* button);
    
private:
    std::vector<RadioButton*> m_buttons;
};

} // namespace controls
} // namespace luaui
