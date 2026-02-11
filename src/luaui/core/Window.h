#pragma once

#include "Control.h"
#include "IRenderEngine.h"
#include "Types.h"
#include "Dispatcher.h"
#include <windows.h>
#include <memory>

namespace luaui {

// 窗口基类 - 简化版本
class Window {
public:
    Window();
    virtual ~Window();

    bool Create(HINSTANCE hInstance, const wchar_t* title, int width, int height);
    int Run();
    void Close();
    
    HWND GetHandle() const { return m_hWnd; }
    void Show(int nCmdShow = SW_SHOW);
    
    void SetRoot(const std::shared_ptr<Control>& root);
    
    Dispatcher* GetDispatcher() const { return m_dispatcher.get(); }

protected:
    virtual void OnLoaded() {}
    virtual void OnClosing() {}
    virtual void OnClosed() {}

private:
    static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);

    HWND m_hWnd = nullptr;
    HINSTANCE m_hInstance = nullptr;
    std::unique_ptr<rendering::IRenderEngine> m_renderer;
    std::unique_ptr<Dispatcher> m_dispatcher;
    std::shared_ptr<Control> m_root;
    
    static const wchar_t* s_className;
    static bool s_classRegistered;
};

} // namespace luaui
