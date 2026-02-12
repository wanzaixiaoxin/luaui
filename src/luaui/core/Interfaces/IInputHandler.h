#pragma once

namespace luaui {
namespace controls {
    struct MouseEventArgs;
    struct KeyEventArgs;
    struct FocusEventArgs;
}

namespace interfaces {

/**
 * @brief 输入处理器接口
 * 
 * 符合 ISP 原则：只有需要处理输入的控件才实现
 * Shape、TextBlock 等静态控件无需实现
 */
class IInputHandler {
public:
    virtual ~IInputHandler() = default;

    // ========== 鼠标事件 ==========
    virtual void OnPreviewMouseDown(controls::MouseEventArgs& args) {}
    virtual void OnMouseDown(controls::MouseEventArgs& args) {}
    virtual void OnPreviewMouseUp(controls::MouseEventArgs& args) {}
    virtual void OnMouseUp(controls::MouseEventArgs& args) {}
    virtual void OnPreviewMouseMove(controls::MouseEventArgs& args) {}
    virtual void OnMouseMove(controls::MouseEventArgs& args) {}
    virtual void OnMouseEnter() {}
    virtual void OnMouseLeave() {}
    virtual void OnMouseWheel(controls::MouseEventArgs& args) {}
    
    // ========== 键盘事件 ==========
    virtual void OnPreviewKeyDown(controls::KeyEventArgs& args) {}
    virtual void OnKeyDown(controls::KeyEventArgs& args) {}
    virtual void OnPreviewKeyUp(controls::KeyEventArgs& args) {}
    virtual void OnKeyUp(controls::KeyEventArgs& args) {}
    
    // 字符输入（文本控件专用）
    virtual void OnChar(wchar_t ch) {}
    
    // ========== 焦点事件 ==========
    virtual void OnPreviewGotFocus(controls::FocusEventArgs& args) {}
    virtual void OnGotFocus() {}
    virtual void OnPreviewLostFocus(controls::FocusEventArgs& args) {}
    virtual void OnLostFocus() {}
    
    // ========== 点击事件 ==========
    virtual void OnClick() {}
};

/**
 * @brief 可聚焦接口
 * 
 * 只有需要接收键盘输入的控件才实现
 */
class IFocusable {
public:
    virtual ~IFocusable() = default;
    
    virtual bool GetIsFocusable() const = 0;
    virtual void SetIsFocusable(bool focusable) = 0;
    
    virtual bool GetIsFocused() const = 0;
    virtual bool Focus() = 0;
    virtual void KillFocus() = 0;
};

} // namespace interfaces
} // namespace luaui
