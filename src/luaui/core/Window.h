#pragma once

#include "Control.h"
#include "IRenderEngine.h"
#include "Types.h"
#include "Dispatcher.h"
#include "Logger.h"
#include <windows.h>
#include <memory>

namespace luaui {

/**
 * @brief 完整的应用程序窗口类
 * 
 * 提供完整的窗口功能：
 * - 渲染管理（自动 Measure/Arrange/Render）
 * - 输入事件路由（鼠标、键盘）
 * - 命中测试（Hit Test）
 * - 焦点管理
 * - 鼠标捕获
 */
class Window {
public:
    Window();
    virtual ~Window();

    // ========== 窗口生命周期 ==========
    bool Create(HINSTANCE hInstance, const wchar_t* title, int width, int height);
    int Run();
    void Close();
    
    HWND GetHandle() const { return m_hWnd; }
    void Show(int nCmdShow = SW_SHOW);
    
    // ========== 内容管理 ==========
    void SetRoot(const std::shared_ptr<Control>& root);
    std::shared_ptr<Control> GetRoot() const { return m_root; }
    
    // ========== 布局管理 ==========
    void InvalidateLayout();
    void InvalidateRender();
    
    // ========== 焦点管理 ==========
    Control* GetFocusedControl() const { return m_focusedControl; }
    void SetFocusedControl(Control* control);
    
    // ========== 调度器 ==========
    Dispatcher* GetDispatcher() const { return m_dispatcher.get(); }

protected:
    // ========== 生命周期回调 ==========
    virtual void OnLoaded() {}
    virtual void OnClosing() {}
    virtual void OnClosed() {}
    
    // ========== 渲染 ==========
    virtual void OnRender();
    
    // ========== 输入事件（可重写）==========
    virtual void OnMouseMove(float x, float y);
    virtual void OnMouseDown(float x, float y, int button);
    virtual void OnMouseUp(float x, float y, int button);
    virtual void OnMouseWheel(float x, float y, int delta);
    virtual void OnKeyDown(int keyCode);
    virtual void OnKeyUp(int keyCode);
    virtual void OnChar(wchar_t ch);

private:
    // ========== 窗口过程 ==========
    static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);

    // ========== 渲染 ==========
    void Render();
    void UpdateLayout();
    
    // ========== 输入处理 ==========
    void HandleMouseMove(float x, float y);
    void HandleMouseDown(float x, float y, int button);
    void HandleMouseUp(float x, float y, int button);
    void HandleMouseWheel(float x, float y, int delta);
    void HandleKeyDown(int keyCode);
    void HandleKeyUp(int keyCode);
    void HandleChar(wchar_t ch);
    
    // ========== 命中测试 ==========
    Control* HitTest(Control* root, float x, float y);
    
    // ========== 焦点管理 ==========
    void UpdateFocus(Control* newFocus);
    void ClearFocus();
    
    // ========== 成员变量 ==========
    HWND m_hWnd = nullptr;
    HINSTANCE m_hInstance = nullptr;
    std::unique_ptr<rendering::IRenderEngine> m_renderer;
    std::unique_ptr<Dispatcher> m_dispatcher;
    std::shared_ptr<Control> m_root;
    
    // 布局状态
    bool m_layoutDirty = true;
    float m_width = 0;
    float m_height = 0;
    
    // 输入状态
    Control* m_capturedControl = nullptr;   // 鼠标捕获的控件
    Control* m_focusedControl = nullptr;    // 焦点控件
    Control* m_lastMouseOver = nullptr;     // 最后鼠标悬停的控件
    
    static const wchar_t* s_className;
    static bool s_classRegistered;
};

} // namespace luaui
