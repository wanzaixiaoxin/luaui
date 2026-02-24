// InputSimulator.cpp - UI Input Simulation for LuaUI Automation

#include "UIAutomationFramework.h"
#include "Window.h"
#include "Control.h"
#include "Components/InputComponent.h"
#include "Components/LayoutComponent.h"
#include <windows.h>
#include <thread>

namespace luaui {
namespace test {
namespace automation {

InputSimulator::InputSimulator(Window* window) : m_window(window) {}

void InputSimulator::SendMouseMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    if (!m_window) return;
    
    HWND hwnd = nullptr;
    // Use window handle from the Window class
    // This requires adding a getter in Window.h or making test class a friend
    // For now, we'll use SendMessage to the window's message handler directly
    
    // Alternative: Post the message to the window
    // We need access to the HWND from Window class
}

void InputSimulator::UpdateWindow() {
    // Force window to process pending messages
    if (auto* tw = dynamic_cast<TestWindow*>(m_window)) {
        tw->TestRender();
    }
}

void InputSimulator::MouseMove(float x, float y) {
    m_lastMouseX = x;
    m_lastMouseY = y;
    
    // Send WM_MOUSEMOVE message
    LPARAM lParam = MAKELPARAM(static_cast<short>(x), static_cast<short>(y));
    
    // Access window's WndProc directly through a test hook
    // This would require adding a test hook in Window class
    
    // Alternative: Use Window's public methods
    // Window::HandleMouseMove is protected, we need to make InputSimulator a friend
    // or add a test hook interface
    
    // For now, we'll simulate by calling protected methods through test hooks
}

void InputSimulator::MouseMoveToControl(Control* control) {
    if (!control) return;
    
    // Get control's position from layout component
    if (auto* layout = control->GetLayout()) {
        // We need to calculate center position
        // This requires access to render rect or layout info
    }
}

void InputSimulator::MouseDown(int button) {
    UINT msg;
    switch (button) {
        case 0: msg = WM_LBUTTONDOWN; break;
        case 1: msg = WM_RBUTTONDOWN; break;
        case 2: msg = WM_MBUTTONDOWN; break;
        default: return;
    }
    
    LPARAM lParam = MAKELPARAM(static_cast<short>(m_lastMouseX), static_cast<short>(m_lastMouseY));
    WPARAM wParam = MK_LBUTTON; // Simplified
    
    // Send message
}

void InputSimulator::MouseUp(int button) {
    UINT msg;
    switch (button) {
        case 0: msg = WM_LBUTTONUP; break;
        case 1: msg = WM_RBUTTONUP; break;
        case 2: msg = WM_MBUTTONUP; break;
        default: return;
    }
    
    LPARAM lParam = MAKELPARAM(static_cast<short>(m_lastMouseX), static_cast<short>(m_lastMouseY));
    // Send message
}

void InputSimulator::MouseClick(int button) {
    MouseDown(button);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    MouseUp(button);
    UpdateWindow();
}

void InputSimulator::MouseDoubleClick(int button) {
    MouseClick(button);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    MouseClick(button);
}

void InputSimulator::MouseWheel(int delta) {
    LPARAM lParam = MAKELPARAM(static_cast<short>(m_lastMouseX), static_cast<short>(m_lastMouseY));
    WPARAM wParam = MAKEWPARAM(0, delta);
    // Send WM_MOUSEWHEEL
}

void InputSimulator::KeyDown(int keyCode) {
    // Send WM_KEYDOWN
}

void InputSimulator::KeyUp(int keyCode) {
    // Send WM_KEYUP
}

void InputSimulator::KeyPress(int keyCode) {
    KeyDown(keyCode);
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    KeyUp(keyCode);
    UpdateWindow();
}

void InputSimulator::SendText(const std::wstring& text) {
    for (wchar_t ch : text) {
        // Send WM_CHAR
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    UpdateWindow();
}

void InputSimulator::SendText(const std::string& text) {
    // Convert to wide string and send
    std::wstring wide(text.begin(), text.end());
    SendText(wide);
}

void InputSimulator::ClickAt(float x, float y) {
    MouseMove(x, y);
    MouseClick(0);
}

void InputSimulator::ClickOn(Control* control) {
    if (!control) return;
    MouseMoveToControl(control);
    MouseClick(0);
}

void InputSimulator::Drag(float fromX, float fromY, float toX, float toY) {
    MouseMove(fromX, fromY);
    MouseDown(0);
    
    // Simulate intermediate points for smooth drag
    int steps = 10;
    for (int i = 1; i <= steps; i++) {
        float t = static_cast<float>(i) / steps;
        float x = fromX + (toX - fromX) * t;
        float y = fromY + (toY - fromY) * t;
        MouseMove(x, y);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    MouseUp(0);
    UpdateWindow();
}

} // namespace automation
} // namespace test
} // namespace luaui
