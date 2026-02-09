#include "FocusManager.h"
#include "Control.h"
#include <algorithm>

namespace luaui {
namespace controls {

// ==================== FocusManager ====================
FocusManager& FocusManager::GetInstance() {
    static FocusManager instance;
    return instance;
}

void FocusManager::SetFocusedControl(Control* control) {
    if (m_focusedControl == control) return;
    
    // 检查新控件是否可获得焦�?
    if (control && !control->GetIsFocusable()) return;
    
    Control* oldFocus = m_focusedControl;
    Control* newFocus = control;
    
    // 更新内部状态（不调用控件的 Focus/KillFocus 避免递归�?
    m_focusedControl = newFocus;
    
    // 更新旧控件的焦点状�?
    if (oldFocus && oldFocus != newFocus) {
        oldFocus->m_isFocused = false;
        oldFocus->Invalidate();
    }
    
    // 更新新控件的焦点状�?
    if (newFocus) {
        newFocus->m_isFocused = true;
        newFocus->Invalidate();
    }
    
    // 通知回调
    if (m_focusChangedCallback) {
        m_focusChangedCallback(oldFocus, newFocus);
    }
}

void FocusManager::ClearFocus() {
    SetFocusedControl(nullptr);
}

void FocusManager::RegisterFocusable(Control* control) {
    if (!control) return;
    
    auto it = std::find(m_focusableControls.begin(), m_focusableControls.end(), control);
    if (it == m_focusableControls.end()) {
        m_focusableControls.push_back(control);
    }
}

void FocusManager::UnregisterFocusable(Control* control) {
    if (!control) return;
    
    auto it = std::find(m_focusableControls.begin(), m_focusableControls.end(), control);
    if (it != m_focusableControls.end()) {
        m_focusableControls.erase(it);
    }
    
    // 如果注销的是当前焦点控件，清除焦�?
    if (m_focusedControl == control) {
        ClearFocus();
    }
}

void FocusManager::MoveFocusNext() {
    if (m_focusableControls.empty()) return;
    
    Control* next = GetNextTabControl(m_focusedControl, true);
    if (next) {
        SetFocusedControl(next);
    }
}

void FocusManager::MoveFocusPrevious() {
    if (m_focusableControls.empty()) return;
    
    Control* prev = GetNextTabControl(m_focusedControl, false);
    if (prev) {
        SetFocusedControl(prev);
    }
}

Control* FocusManager::GetNextTabControl(Control* current, bool forward) {
    if (m_focusableControls.empty()) return nullptr;
    
    // 简单的 Tab 顺序：按注册顺序
    if (!current) {
        // 如果没有当前焦点，返回第一�?最后一�?
        return forward ? m_focusableControls.front() : m_focusableControls.back();
    }
    
    auto it = std::find(m_focusableControls.begin(), m_focusableControls.end(), current);
    if (it == m_focusableControls.end()) {
        return forward ? m_focusableControls.front() : m_focusableControls.back();
    }
    
    if (forward) {
        ++it;
        if (it == m_focusableControls.end()) {
            it = m_focusableControls.begin();  // 循环
        }
    } else {
        if (it == m_focusableControls.begin()) {
            it = m_focusableControls.end();
        }
        --it;
    }
    
    return *it;
}

void FocusManager::SetFocusScope(Control* scopeRoot) {
    m_focusScopeRoot = scopeRoot;
}

void FocusManager::ClearFocusScope() {
    m_focusScopeRoot = nullptr;
}

} // namespace controls
} // namespace luaui
