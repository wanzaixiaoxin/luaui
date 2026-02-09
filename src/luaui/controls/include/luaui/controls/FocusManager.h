#pragma once

#include <memory>
#include <vector>
#include <functional>

namespace luaui {
namespace controls {

class Control;
using ControlPtr = std::shared_ptr<Control>;

// ==================== Focus 管理器 ====================
class FocusManager {
public:
    static FocusManager& GetInstance();

    // 获取/设置当前焦点控件
    Control* GetFocusedControl() const { return m_focusedControl; }
    void SetFocusedControl(Control* control);

    // 清除焦点
    void ClearFocus();

    // 焦点移动到下一个/上一个控件
    void MoveFocusNext();
    void MoveFocusPrevious();

    // 注册可焦点控件
    void RegisterFocusable(Control* control);
    void UnregisterFocusable(Control* control);

    // 获取 Tab 顺序中的下一个控件
    Control* GetNextTabControl(Control* current, bool forward = true);

    // 设置焦点作用域（如对话框）
    void SetFocusScope(Control* scopeRoot);
    void ClearFocusScope();

    // 事件回调
    using FocusChangedCallback = std::function<void(Control* oldFocus, Control* newFocus)>;
    void SetFocusChangedCallback(FocusChangedCallback callback) { m_focusChangedCallback = callback; }

private:
    FocusManager() = default;
    ~FocusManager() = default;
    FocusManager(const FocusManager&) = delete;
    FocusManager& operator=(const FocusManager&) = delete;

    Control* m_focusedControl = nullptr;
    Control* m_focusScopeRoot = nullptr;
    std::vector<Control*> m_focusableControls;
    FocusChangedCallback m_focusChangedCallback;

    void BuildTabOrder(std::vector<Control*>& order, Control* root);
};

} // namespace controls
} // namespace luaui
