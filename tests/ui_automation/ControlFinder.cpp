// ControlFinder.cpp - Implementation of control discovery API

#include "UIAutomationFramework.h"
#include "TestHooks.h"
#include "Window.h"
#include "Control.h"
#include "Panel.h"
#include <thread>
#include <chrono>

namespace luaui {
namespace test {
namespace automation {

ControlFinder::ControlFinder(Window* window) : m_window(window) {}

Control* ControlFinder::FindRecursive(Control* root, const std::function<bool(Control*)>& predicate) {
    if (!root) return nullptr;
    
    if (predicate(root)) {
        return root;
    }
    
    // Try to find in children
    // This requires access to Panel's children
    if (auto* panel = dynamic_cast<controls::Panel*>(root)) {
        // Access panel children - this would require Panel to expose GetChildren()
        // For now, placeholder implementation
    }
    
    return nullptr;
}

Control* ControlFinder::FindById(const std::string& id) {
    if (!m_window) return nullptr;
    
    auto* root = dynamic_cast<TestWindow*>(m_window)->GetRootControl();
    if (!root) return nullptr;
    
    return FindRecursive(dynamic_cast<Control*>(root), [&id](Control* c) {
        return ControlTestHook::GetAutomationId(c) == id;
    });
}

Control* ControlFinder::FindById(const std::wstring& id) {
    return FindById(std::string(id.begin(), id.end()));
}

Control* ControlFinder::FindByTypeName(const char* typeName) {
    if (!m_window) return nullptr;
    
    auto* root = dynamic_cast<TestWindow*>(m_window)->GetRootControl();
    if (!root) return nullptr;
    
    // This is a simplified implementation
    // Full implementation would traverse the tree and check type
    return nullptr;
}

Control* ControlFinder::FindByName(const std::string& name) {
    return FindById(name);  // Use ID as name
}

Control* ControlFinder::FindByText(const std::wstring& text) {
    // This would require controls to expose their text content
    // Placeholder implementation
    (void)text;
    return nullptr;
}

Control* ControlFinder::FindAtPosition(float x, float y) {
    if (!m_window) return nullptr;
    
    // Use window's HitTest functionality
    auto* root = dynamic_cast<TestWindow*>(m_window)->GetRootControl();
    if (!root) return nullptr;
    
    // This would use Window::HitTest if it were accessible
    // Placeholder implementation
    (void)x;
    (void)y;
    return nullptr;
}

std::vector<Control*> ControlFinder::FindChildren(controls::Panel* panel) {
    std::vector<Control*> result;
    // Implementation would iterate panel's children
    (void)panel;
    return result;
}

Control* ControlFinder::WaitForId(const std::string& id, int timeoutMs) {
    auto start = std::chrono::steady_clock::now();
    
    while (true) {
        auto* control = FindById(id);
        if (control) return control;
        
        auto elapsed = std::chrono::steady_clock::now() - start;
        if (std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() > timeoutMs) {
            return nullptr;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

bool ControlFinder::WaitForIdToDisappear(const std::string& id, int timeoutMs) {
    auto start = std::chrono::steady_clock::now();
    
    while (true) {
        auto* control = FindById(id);
        if (!control) return true;
        
        auto elapsed = std::chrono::steady_clock::now() - start;
        if (std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() > timeoutMs) {
            return false;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

} // namespace automation
} // namespace test
} // namespace luaui
