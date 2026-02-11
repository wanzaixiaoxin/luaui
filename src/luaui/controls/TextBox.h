#pragma once

#include "Control.h"
#include <string>
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include "Components/InputComponent.h"

namespace luaui {
namespace controls {

/**
 * @brief TextBox 文本输入控件（新架构）
 * 
 * 可编辑的文本控件，处理键盘输入
 */
class TextBox : public Control {
public:
    TextBox();
    
    std::string GetTypeName() const override { return "TextBox"; }
    
    // 文本
    std::wstring GetText() const { return m_text; }
    void SetText(const std::wstring& text);
    
    // 占位符
    std::wstring GetPlaceholder() const { return m_placeholder; }
    void SetPlaceholder(const std::wstring& placeholder);
    
    // 只读
    bool GetIsReadOnly() const { return m_isReadOnly; }
    void SetIsReadOnly(bool readOnly) { m_isReadOnly = readOnly; }
    
    // 密码模式
    bool GetIsPassword() const { return m_isPassword; }
    void SetIsPassword(bool isPassword);
    
    // 最大长度
    int GetMaxLength() const { return m_maxLength; }
    void SetMaxLength(int maxLength) { m_maxLength = maxLength; }
    
    // 光标位置
    int GetCaretPosition() const { return m_caretPosition; }
    void SetCaretPosition(int pos);
    
    // 选择
    bool HasSelection() const { return m_selectionStart != m_selectionEnd; }
    void SelectAll();
    void ClearSelection();
    
    // 事件
    luaui::Delegate<TextBox*, const std::wstring&> TextChanged;

protected:
    void InitializeComponents() override;
    
    // 渲染覆盖
    virtual void OnRender(rendering::IRenderContext* context);
    
    // 输入处理
    virtual void OnMouseDown(MouseEventArgs& args);
    virtual void OnKeyDown(KeyEventArgs& args);
    virtual void OnChar(wchar_t ch);
    virtual void OnGotFocus();
    virtual void OnLostFocus();
    
    // 布局覆盖
    virtual rendering::Size OnMeasure(const rendering::Size& availableSize);
    
    // 编辑操作
    virtual void InsertText(const std::wstring& text);
    virtual void DeleteSelection();
    virtual void DeleteBackward();
    virtual void DeleteForward();
    
private:
    std::wstring m_text;
    std::wstring m_placeholder;
    bool m_isReadOnly = false;
    bool m_isPassword = false;
    int m_maxLength = 0;
    
    int m_caretPosition = 0;
    int m_selectionStart = 0;
    int m_selectionEnd = 0;
    
    float m_fontSize = 14.0f;
    rendering::Color m_foreground = rendering::Color::Black();
    rendering::Color m_borderColor = rendering::Color::FromHex(0x808080);
    rendering::Color m_focusedBorderColor = rendering::Color::FromHex(0x0078D4);
    
    void UpdateCaretVisible();
    std::wstring GetDisplayText() const;
};

} // namespace controls
} // namespace luaui
