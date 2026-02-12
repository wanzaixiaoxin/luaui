#include "Components/InputComponent.h"
#include "Control.h"
// #include "Event.h"  // TODO: Migrate to new architecture

namespace luaui {
namespace components {

InputComponent::InputComponent(Control* owner) : Component(owner) {}

bool InputComponent::Focus() {
    if (!m_focusable) return false;
    
    // TODO: 通知窗口管理器设置焦点
    m_focused = true;
    
    return true;
}

void InputComponent::KillFocus() {
    m_focused = false;
}

void InputComponent::RaiseMouseDown(controls::MouseEventArgs& args) {
    OnMouseDown(args);
    if (m_owner) m_owner->OnMouseDown(args);
}

void InputComponent::RaiseMouseUp(controls::MouseEventArgs& args) {
    OnMouseUp(args);
    if (m_owner) m_owner->OnMouseUp(args);
}

void InputComponent::RaiseMouseMove(controls::MouseEventArgs& args) {
    OnMouseMove(args);
    if (m_owner) m_owner->OnMouseMove(args);
}

void InputComponent::RaiseMouseWheel(controls::MouseEventArgs& args) {
    OnMouseWheel(args);
    if (m_owner) m_owner->OnMouseWheel(args);
}

void InputComponent::RaiseMouseEnter() {
    m_mouseOver = true;
    OnMouseEnter();
    if (m_owner) m_owner->OnMouseEnter();
}

void InputComponent::RaiseMouseLeave() {
    m_mouseOver = false;
    OnMouseLeave();
    if (m_owner) m_owner->OnMouseLeave();
}

void InputComponent::RaiseKeyDown(controls::KeyEventArgs& args) {
    OnKeyDown(args);
    if (m_owner) m_owner->OnKeyDown(args);
}

void InputComponent::RaiseKeyUp(controls::KeyEventArgs& args) {
    OnKeyUp(args);
    if (m_owner) m_owner->OnKeyUp(args);
}

void InputComponent::RaiseGotFocus() {
    OnGotFocus();
    if (m_owner) m_owner->OnGotFocus();
}

void InputComponent::RaiseLostFocus() {
    OnLostFocus();
    if (m_owner) m_owner->OnLostFocus();
}

void InputComponent::RaiseClick() {
    OnClick();
    if (m_owner) m_owner->OnClick();
}

void InputComponent::RaiseChar(wchar_t ch) {
    OnChar(ch);
    if (m_owner) m_owner->OnChar(ch);
}

} // namespace components
} // namespace luaui
