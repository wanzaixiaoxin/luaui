#pragma once

#include "Components/Component.h"
#include "Interfaces/IInputHandler.h"

namespace luaui {

class Control;

namespace controls {
    struct MouseEventArgs;
    struct KeyEventArgs;
    struct FocusEventArgs;
}

namespace components {

using namespace luaui::interfaces;

/**
 * @brief 输入处理组件
 * 
 * 将输入相关行为从 Control 中分离
 * 符合 SRP：只负责事件处理
 */
class InputComponent : public Component, public IInputHandler, public IFocusable {
public:
    InputComponent(Control* owner);
    
    // ========== IFocusable 实现 ==========
    bool GetIsFocusable() const override { return m_focusable; }
    void SetIsFocusable(bool focusable) override { m_focusable = focusable; }
    
    bool GetIsFocused() const override { return m_focused; }
    bool Focus() override;
    void KillFocus() override;
    
    // ========== 事件触发（供外部调用） ==========
    void RaiseMouseDown(controls::MouseEventArgs& args);
    void RaiseMouseUp(controls::MouseEventArgs& args);
    void RaiseMouseMove(controls::MouseEventArgs& args);
    void RaiseMouseWheel(controls::MouseEventArgs& args);
    void RaiseMouseEnter();
    void RaiseMouseLeave();
    void RaiseKeyDown(controls::KeyEventArgs& args);
    void RaiseKeyUp(controls::KeyEventArgs& args);
    void RaiseGotFocus();
    void RaiseLostFocus();
    void RaiseClick();
    void RaiseChar(wchar_t ch);
    
    // ========== 状态查询 ==========
    bool IsMouseOver() const { return m_mouseOver; }
    bool IsMouseCaptured() const { return m_mouseCaptured; }
    void SetMouseOver(bool over) { m_mouseOver = over; }
    void SetMouseCaptured(bool captured) { m_mouseCaptured = captured; }

private:
    bool m_focusable = false;
    bool m_focused = false;
    bool m_mouseOver = false;
    bool m_mouseCaptured = false;
};

} // namespace components
} // namespace luaui
