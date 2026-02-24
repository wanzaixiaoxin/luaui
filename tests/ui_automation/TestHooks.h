// TestHooks.h - Test access hooks for LuaUI internals
// This header provides controlled access to internal APIs for testing purposes

#pragma once

#include "Window.h"
#include "Control.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include <memory>

namespace luaui {
namespace test {
namespace automation {

// ============================================================================
// Control Test Hook - Provides access to Control internals for testing
// ============================================================================
class ControlTestHook {
public:
    static std::string GetAutomationId(Control* control) {
        if (!control) return "";
        return control->GetName();  // Use Name as AutomationId
    }
    
    static void SetAutomationId(Control* control, const std::string& id) {
        if (!control) return;
        control->SetName(id);
    }
    
    static luaui::rendering::Rect GetRenderRect(Control* control) {
        if (!control) return luaui::rendering::Rect();
        if (auto* render = control->GetRender()) {
            return render->GetRenderRect();
        }
        return luaui::rendering::Rect();
    }
    
    static luaui::rendering::Size GetDesiredSize(Control* control) {
        if (!control) return luaui::rendering::Size();
        if (auto* layout = control->GetLayout()) {
            return layout->GetDesiredSize();
        }
        return luaui::rendering::Size();
    }
    
    static std::vector<std::shared_ptr<interfaces::IControl>> GetChildren(Control* control) {
        (void)control;
        return {};
    }
};

// ============================================================================
// Test-friendly Window class that exposes necessary methods
// Uses protected OnXxx methods instead of private HandleXxx methods
// ============================================================================
class TestWindow : public Window {
public:
    // Expose protected methods by creating public wrappers
    void TestHandleMouseMove(float x, float y) {
        OnMouseMove(x, y);
    }
    
    void TestHandleMouseDown(float x, float y, int button) {
        OnMouseDown(x, y, button);
    }
    
    void TestHandleMouseUp(float x, float y, int button) {
        OnMouseUp(x, y, button);
    }
    
    void TestHandleKeyDown(int keyCode) {
        OnKeyDown(keyCode);
    }
    
    void TestHandleKeyUp(int keyCode) {
        OnKeyUp(keyCode);
    }
    
    void TestUpdateLayout() {
        InvalidateLayout();
        // Layout will be updated on next render
    }
    
    void TestRender() {
        // Trigger render through public API
        InvalidateRender();
    }
    
    Control* GetFocusedControl() const { 
        return GetFocusedControl();  // Call public method
    }
    
    Control* GetRootControl() const { 
        return GetRoot().get();  // Use public GetRoot()
    }
    
    void SetSizeForTest(int width, int height) {
        // Use public methods to resize
        HWND hwnd = GetHandle();
        if (hwnd) {
            SetWindowPos(hwnd, nullptr, 0, 0, width, height, 
                         SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
        InvalidateLayout();
    }
    
    luaui::rendering::Size GetWindowSize() const {
        HWND hwnd = GetHandle();
        if (hwnd) {
            RECT rect;
            GetClientRect(hwnd, &rect);
            return luaui::rendering::Size(
                static_cast<float>(rect.right - rect.left),
                static_cast<float>(rect.bottom - rect.top)
            );
        }
        return luaui::rendering::Size();
    }
};

} // namespace automation
} // namespace test
} // namespace luaui
