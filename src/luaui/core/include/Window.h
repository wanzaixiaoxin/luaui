#pragma once

#include "Control.h"
#include "IRenderEngine.h"
#include <windows.h>
#include <memory>
#include <functional>

namespace luaui {

using namespace luaui::controls;
using namespace luaui::rendering;

// Mouse button enum for event handling
enum class MouseButton { Left, Right, Middle };

/**
 * @brief Window base class - encapsulates Win32 window and message handling
 * 
 * Usage:
 *   class MyWindow : public Window {
 *   protected:
 *     void OnLoaded() override {
 *       // Setup UI and bind events
 *       auto btn = FindControl<Button>("submitBtn");
 *       btn->AddClickHandler([](Control*) { });
 *     }
 *   };
 */
class Window {
public:
    Window();
    virtual ~Window();

    // Window creation and lifecycle
    bool Create(HINSTANCE hInstance, const wchar_t* title, int width, int height);
    int Run();
    void Close();

    // Properties
    HWND GetHandle() const { return m_hWnd; }
    bool IsVisible() const;
    void Show(int nCmdShow = SW_SHOW);
    
    // Layout root
    void SetRoot(const ControlPtr& root);
    ControlPtr GetRoot() const { return m_root; }
    
    // Control lookup by name (for x:Name binding)
    ControlPtr FindControl(const std::string& name) const;
    template<typename T>
    T* FindControl(const std::string& name) const {
        auto ctrl = FindControl(name);
        return ctrl ? dynamic_cast<T*>(ctrl.get()) : nullptr;
    }
    
    // Focus management
    ControlPtr GetFocusedControl() const { return m_focusedControl.lock(); }
    void SetFocus(const ControlPtr& control);
    void ClearFocus();
    
    // Invalidation
    void Invalidate();
    void InvalidateRect(const luaui::Rect& rect);
    
    // Size
    Size GetClientSize() const;
    
protected:
    // Override these in derived classes
    virtual void OnLoaded() {}           // Called after window created and root set
    virtual void OnClosing() {}          // Called when window is about to close
    virtual void OnClosed() {}           // Called after window closed
    virtual void OnSizeChanged(const Size& newSize) {}
    
    // Message handlers (optional override)
    virtual bool OnKeyDown(int keyCode) { return false; }
    virtual bool OnKeyUp(int keyCode) { return false; }
    virtual bool OnChar(wchar_t ch) { return false; }

protected:
    // Internal message handling
    static LRESULT CALLBACK StaticWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    
    void ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam);
    void HandleMouseDown(const Point& pt, MouseButton button);
    void HandleMouseUp(const Point& pt, MouseButton button);
    void HandleMouseMove(const Point& pt);
    void HandleMouseWheel(float delta);
    void HandleKeyDown(int keyCode);
    void HandleKeyUp(int keyCode);
    void HandleChar(wchar_t ch);
    
    // Hit testing
    ControlPtr HitTest(const Point& pt);
    void UpdateHoveredControl(const Point& pt);
    
    // Rendering
    void InitializeRenderer();
    void Render();
    void UpdateLayout();
    
    // Timer for caret animation and other updates
    void StartTimer();
    void StopTimer();
    static void CALLBACK TimerProc(HWND hWnd, UINT message, UINT_PTR idTimer, DWORD dwTime);
    void OnTimer();

protected:
    // Win32
    HWND m_hWnd = nullptr;
    HINSTANCE m_hInstance = nullptr;
    static const wchar_t* s_className;
    static bool s_classRegistered;
    
    // UI
    ControlPtr m_root;
    std::unique_ptr<IRenderEngine> m_renderer;
    bool m_needsLayout = true;
    
    // Input state
    std::weak_ptr<Control> m_focusedControl;
    std::weak_ptr<Control> m_hoveredControl;
    std::weak_ptr<Control> m_capturedControl;
    std::weak_ptr<Control> m_lastMouseDownControl;
    Point m_lastMousePos;
    
    // Drag state for Slider and similar controls
    bool m_isDragging = false;
    
    // Timer
    static constexpr UINT_PTR TimerId = 1;
    static constexpr UINT TimerInterval = 16; // ~60fps
    bool m_timerRunning = false;
    
    // Double-click tracking
    DWORD m_lastClickTime = 0;
    Point m_lastClickPos;
    int m_clickCount = 0;
    static constexpr DWORD DoubleClickTime = 500; // ms
    static constexpr float DoubleClickDistance = 4.0f; // pixels
};

} // namespace luaui
