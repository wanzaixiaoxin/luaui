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
}

void InputComponent::RaiseMouseUp(controls::MouseEventArgs& args) {
    OnMouseUp(args);
}

void InputComponent::RaiseMouseMove(controls::MouseEventArgs& args) {
    OnMouseMove(args);
}

void InputComponent::RaiseMouseWheel(controls::MouseEventArgs& args) {
    OnMouseWheel(args);
}

void InputComponent::RaiseMouseEnter() {
    m_mouseOver = true;
    OnMouseEnter();
}

void InputComponent::RaiseMouseLeave() {
    m_mouseOver = false;
    OnMouseLeave();
}

void InputComponent::RaiseKeyDown(controls::KeyEventArgs& args) {
    OnKeyDown(args);
}

void InputComponent::RaiseKeyUp(controls::KeyEventArgs& args) {
    OnKeyUp(args);
}

void InputComponent::RaiseGotFocus() {
    OnGotFocus();
}

void InputComponent::RaiseLostFocus() {
    OnLostFocus();
}

void InputComponent::RaiseClick() {
    OnClick();
}

} // namespace components
} // namespace luaui
