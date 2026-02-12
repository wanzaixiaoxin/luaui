#include "Window.h"
#include "../rendering/d2d/D2DRenderEngine.h"
#include "../controls/Control.h"
#include "../controls/Panel.h"
#include "Components/InputComponent.h"
#include "../utils/Logger.h"
#include <objbase.h>
#include <windowsx.h>

using namespace luaui::utils;

namespace luaui {

const wchar_t* Window::s_className = L"LuaUI_WindowClass";
bool Window::s_classRegistered = false;

// ============================================================================
// 构造/析构
// ============================================================================

Window::Window() = default;

Window::~Window() {
    if (m_dispatcher) m_dispatcher->Shutdown();
    if (m_renderer) m_renderer->Shutdown();
    if (m_hWnd) DestroyWindow(m_hWnd);
    CoUninitialize();
}

// ============================================================================
// 窗口创建与管理
// ============================================================================

bool Window::Create(HINSTANCE hInstance, const wchar_t* title, int width, int height) {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) return false;
    
    m_hInstance = hInstance;
    
    // 注册窗口类
    if (!s_classRegistered) {
        WNDCLASSEXW wcex = {};
        wcex.cbSize = sizeof(WNDCLASSEXW);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = StaticWndProc;
        wcex.hInstance = hInstance;
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszClassName = s_className;
        
        if (!RegisterClassExW(&wcex)) return false;
        s_classRegistered = true;
    }
    
    // 创建窗口
    m_hWnd = CreateWindowExW(0, s_className, title, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        nullptr, nullptr, hInstance, this);
    
    if (!m_hWnd) return false;
    
    // 初始化渲染引擎
    m_renderer = std::make_unique<rendering::D2DRenderEngine>();
    if (!m_renderer->Initialize()) return false;
    
    RECT rc;
    GetClientRect(m_hWnd, &rc);
    rendering::RenderTargetDesc desc;
    desc.type = rendering::RenderTargetType::Window;
    desc.nativeHandle = m_hWnd;
    desc.width = rc.right - rc.left;
    desc.height = rc.bottom - rc.top;
    m_renderer->CreateRenderTarget(desc);
    
    m_width = static_cast<float>(rc.right - rc.left);
    m_height = static_cast<float>(rc.bottom - rc.top);
    
    // 初始化调度器
    m_dispatcher = std::make_unique<Dispatcher>();
    
    utils::Logger::Info("Window created successfully");
    OnLoaded();
    return true;
}

void Window::Show(int nCmdShow) {
    if (m_hWnd) {
        ShowWindow(m_hWnd, nCmdShow);
        UpdateWindow(m_hWnd);
        SetForegroundWindow(m_hWnd);  // 强制激活窗口
        SetFocus(m_hWnd);             // 确保窗口获得焦点以接收键盘事件
    }
}

int Window::Run() {
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

void Window::Close() {
    if (m_hWnd) PostMessage(m_hWnd, WM_CLOSE, 0, 0);
}

// ============================================================================
// 内容管理
// ============================================================================

void Window::SetRoot(const std::shared_ptr<Control>& root) {
    m_root = root;
    if (m_root && m_dispatcher) {
        m_root->SetDispatcher(m_dispatcher.get());
    }
    InvalidateLayout();
}

// ============================================================================
// 布局管理
// ============================================================================

void Window::InvalidateLayout() {
    Logger::Debug("[Window] InvalidateLayout called");
    m_layoutDirty = true;
    
    // 使根控件的布局失效，确保 Measure 和 Arrange 会被重新执行
    if (m_root) {
        if (auto* layoutable = m_root->AsLayoutable()) {
            layoutable->InvalidateMeasure();
            layoutable->InvalidateArrange();
        }
    }
    
    InvalidateRect(m_hWnd, nullptr, FALSE);
}

void Window::InvalidateRender() {
    InvalidateRect(m_hWnd, nullptr, FALSE);
}

void Window::UpdateLayout() {
    Logger::DebugF("[Window] UpdateLayout called, m_layoutDirty=%s", m_layoutDirty ? "true" : "false");
    
    if (!m_root || !m_layoutDirty) {
        Logger::Debug("[Window] UpdateLayout: skipped (no root or not dirty)");
        return;
    }
    
    auto* layoutable = m_root->AsLayoutable();
    if (!layoutable) {
        Logger::Debug("[Window] UpdateLayout: skipped (root not layoutable)");
        return;
    }
    
    interfaces::LayoutConstraint constraint;
    constraint.available = rendering::Size(m_width, m_height);
    
    Logger::DebugF("[Window] UpdateLayout: size=%.0fx%.0f, measuring root...", m_width, m_height);
    
    layoutable->Measure(constraint);
    auto desired = layoutable->GetDesiredSize();
    Logger::DebugF("[Window] Root desired size: %.0fx%.0f", desired.width, desired.height);
    
    layoutable->Arrange(rendering::Rect(0, 0, m_width, m_height));
    
    m_layoutDirty = false;
    
    Logger::DebugF("[Window] Layout updated: %.0fx%.0f", m_width, m_height);
}

// ============================================================================
// 渲染
// ============================================================================

void Window::OnRender() {
    if (!m_renderer || !m_renderer->BeginFrame()) {
        return;
    }
    
    auto* context = m_renderer->GetContext();
    if (!context) {
        m_renderer->Present();
        return;
    }
    
    // 清空背景
    context->Clear(rendering::Color::White());
    
    // 更新布局（如果需要）
    if (m_layoutDirty) {
        UpdateLayout();
    }
    
    // 渲染根控件
    if (m_root) {
        if (auto* renderable = m_root->AsRenderable()) {
            renderable->Render(context);
        }
    }
    
    m_renderer->Present();
}

void Window::Render() {
    OnRender();
}

// ============================================================================
// 焦点管理
// ============================================================================

void Window::SetFocusedControl(Control* control) {
    UpdateFocus(control);
}

void Window::UpdateFocus(Control* newFocus) {
    if (m_focusedControl == newFocus) return;
    
    // 清除旧焦点
    if (m_focusedControl) {
        if (auto* inputComp = m_focusedControl->GetInput()) {
            inputComp->KillFocus();
            inputComp->RaiseLostFocus();
        }
    }
    
    // 设置新焦点
    m_focusedControl = newFocus;
    if (m_focusedControl) {
        if (auto* inputComp = m_focusedControl->GetInput()) {
            if (inputComp->GetIsFocusable()) {
                inputComp->Focus();
                inputComp->RaiseGotFocus();
            }
        }
    }
    
    InvalidateRender();
}

void Window::ClearFocus() {
    UpdateFocus(nullptr);
}

// ============================================================================
// 命中测试
// ============================================================================

Control* Window::HitTest(Control* root, float x, float y) {
    if (!root) return nullptr;
    
    auto* render = root->GetRender();
    if (!render) return nullptr;
    
    const auto& rect = render->GetRenderRect();
    
    // 检查点是否在矩形内
    if (x >= rect.x && x < rect.x + rect.width &&
        y >= rect.y && y < rect.y + rect.height) {
        
        // 如果是 Panel，递归测试子控件
        if (auto* panel = dynamic_cast<controls::Panel*>(root)) {
            float localX = x - rect.x;
            float localY = y - rect.y;
            
            // 从后向前遍历（后添加的在上面）
            for (int i = static_cast<int>(panel->GetChildCount()) - 1; i >= 0; --i) {
                auto* child = static_cast<Control*>(panel->GetChild(i).get());
                if (auto* result = HitTest(child, localX, localY)) {
                    return result;
                }
            }
        }
        
        return root;
    }
    
    return nullptr;
}

// ============================================================================
// 输入处理
// ============================================================================

void Window::HandleMouseMove(float x, float y) {
    // 如果有捕获的控件，直接发送给它
    if (m_capturedControl) {
        if (auto* inputComp = m_capturedControl->GetInput()) {
            controls::MouseEventArgs args{x, y, 0, false};
            inputComp->RaiseMouseMove(args);
        }
        InvalidateRender();
        return;
    }
    
    // 命中测试
    auto* control = HitTest(m_root.get(), x, y);
    
    // 处理 MouseLeave
    if (m_lastMouseOver && m_lastMouseOver != control) {
        if (auto* inputComp = m_lastMouseOver->GetInput()) {
            inputComp->RaiseMouseLeave();
        }
    }
    
    // 处理 MouseEnter/MouseMove
    if (control) {
        if (auto* inputComp = control->GetInput()) {
            if (m_lastMouseOver != control) {
                inputComp->RaiseMouseEnter();
            }
            controls::MouseEventArgs args{x, y, 0, false};
            inputComp->RaiseMouseMove(args);
        }
    }
    
    m_lastMouseOver = control;
    InvalidateRender();
}

void Window::HandleMouseDown(float x, float y, int button) {
    auto* control = HitTest(m_root.get(), x, y);
    
    utils::Logger::TraceF("[Window] MouseDown: %s at (%.1f,%.1f)",
        control ? control->GetTypeName().c_str() : "null", x, y);
    
    if (control) {
        // 捕获鼠标
        m_capturedControl = control;
        SetCapture(m_hWnd);
        
        // 设置焦点
        if (auto* inputComp = control->GetInput()) {
            if (inputComp->GetIsFocusable()) {
                UpdateFocus(control);
            }
            
            controls::MouseEventArgs args{x, y, button, false};
            inputComp->RaiseMouseDown(args);
        }
    }
    
    InvalidateRender();
}

void Window::HandleMouseUp(float x, float y, int button) {
    ReleaseCapture();
    
    if (m_capturedControl) {
        if (auto* inputComp = m_capturedControl->GetInput()) {
            controls::MouseEventArgs args{x, y, button, false};
            inputComp->RaiseMouseUp(args);
            
            // 如果鼠标仍在控件上，触发点击
            auto* hitControl = HitTest(m_root.get(), x, y);
            if (hitControl == m_capturedControl) {
                inputComp->RaiseClick();
            }
        }
        m_capturedControl = nullptr;
    } else {
        // 没有捕获的控件，正常处理
        auto* control = HitTest(m_root.get(), x, y);
        if (control) {
            if (auto* inputComp = control->GetInput()) {
                controls::MouseEventArgs args{x, y, button, false};
                inputComp->RaiseMouseUp(args);
                inputComp->RaiseClick();
            }
        }
    }
    
    InvalidateRender();
}

void Window::HandleMouseWheel(float x, float y, int delta) {
    auto* control = m_capturedControl ? m_capturedControl : HitTest(m_root.get(), x, y);
    
    if (control) {
        if (auto* inputComp = control->GetInput()) {
            // TODO: Add wheel event to input component
            (void)inputComp;
        }
    }
}

void Window::HandleKeyDown(int keyCode) {
    // 先调用虚函数，允许派生类拦截
    OnKeyDown(keyCode);
    
    // 再发送给焦点控件
    if (m_focusedControl) {
        if (auto* inputComp = m_focusedControl->GetInput()) {
            controls::KeyEventArgs args{keyCode, false, false, false, false, false};
            inputComp->RaiseKeyDown(args);
        }
    }
    InvalidateRender();
}

void Window::HandleKeyUp(int keyCode) {
    OnKeyUp(keyCode);
    
    if (m_focusedControl) {
        if (auto* inputComp = m_focusedControl->GetInput()) {
            controls::KeyEventArgs args{keyCode, false, false, false, false, false};
            inputComp->RaiseKeyUp(args);
        }
    }
    InvalidateRender();
}

void Window::HandleChar(wchar_t ch) {
    OnChar(ch);
    
    if (m_focusedControl) {
        if (auto* inputComp = m_focusedControl->GetInput()) {
            inputComp->RaiseChar(ch);
        }
    }
    InvalidateRender();
}

// ============================================================================
// 窗口过程
// ============================================================================

LRESULT CALLBACK Window::StaticWndProc(HWND hWnd, UINT msg, WPARAM wP, LPARAM lP) {
    Window* wnd = nullptr;
    if (msg == WM_NCCREATE) {
        wnd = static_cast<Window*>(reinterpret_cast<CREATESTRUCT*>(lP)->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(wnd));
        wnd->m_hWnd = hWnd;
    } else {
        wnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }
    return wnd ? wnd->WndProc(msg, wP, lP) : DefWindowProc(hWnd, msg, wP, lP);
}

LRESULT Window::WndProc(UINT msg, WPARAM wP, LPARAM lP) {
    switch (msg) {
        // ========== 渲染 ==========
        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(m_hWnd, &ps);
            Render();
            EndPaint(m_hWnd, &ps);
            return 0;
        }
        
        // ========== 窗口大小变化 ==========
        case WM_SIZE: {
            m_width = static_cast<float>(LOWORD(lP));
            m_height = static_cast<float>(HIWORD(lP));
            if (m_renderer) {
                m_renderer->ResizeRenderTarget(static_cast<int>(m_width), static_cast<int>(m_height));
            }
            InvalidateLayout();
            return 0;
        }
        
        case WM_SETFOCUS: {
            Logger::Debug("[Window] WM_SETFOCUS");
            return 0;
        }
        
        case WM_KILLFOCUS: {
            Logger::Debug("[Window] WM_KILLFOCUS");
            return 0;
        }
        
        // ========== 鼠标事件 ==========
        case WM_MOUSEMOVE: {
            float x = static_cast<float>(GET_X_LPARAM(lP));
            float y = static_cast<float>(GET_Y_LPARAM(lP));
            HandleMouseMove(x, y);
            return 0;
        }
        
        case WM_LBUTTONDOWN: {
            float x = static_cast<float>(GET_X_LPARAM(lP));
            float y = static_cast<float>(GET_Y_LPARAM(lP));
            HandleMouseDown(x, y, 0);
            return 0;
        }
        
        case WM_LBUTTONUP: {
            float x = static_cast<float>(GET_X_LPARAM(lP));
            float y = static_cast<float>(GET_Y_LPARAM(lP));
            HandleMouseUp(x, y, 0);
            return 0;
        }
        
        case WM_RBUTTONDOWN: {
            float x = static_cast<float>(GET_X_LPARAM(lP));
            float y = static_cast<float>(GET_Y_LPARAM(lP));
            HandleMouseDown(x, y, 1);
            return 0;
        }
        
        case WM_RBUTTONUP: {
            float x = static_cast<float>(GET_X_LPARAM(lP));
            float y = static_cast<float>(GET_Y_LPARAM(lP));
            HandleMouseUp(x, y, 1);
            return 0;
        }
        
        case WM_MOUSEWHEEL: {
            float x = static_cast<float>(GET_X_LPARAM(lP));
            float y = static_cast<float>(GET_Y_LPARAM(lP));
            int delta = GET_WHEEL_DELTA_WPARAM(wP);
            HandleMouseWheel(x, y, delta);
            return 0;
        }
        
        // ========== 键盘事件 ==========
        case WM_KEYDOWN: {
            Logger::DebugF("[Window] WM_KEYDOWN: %d", (int)wP);
            HandleKeyDown(static_cast<int>(wP));
            return 0;
        }
        
        case WM_KEYUP: {
            HandleKeyUp(static_cast<int>(wP));
            return 0;
        }
        
        case WM_CHAR: {
            HandleChar(static_cast<wchar_t>(wP));
            return 0;
        }
        
        // ========== 窗口关闭 ==========
        case WM_CLOSE:
            OnClosing();
            DestroyWindow(m_hWnd);
            return 0;
            
        case WM_DESTROY:
            OnClosed();
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProc(m_hWnd, msg, wP, lP);
}

// ============================================================================
// 可重写的虚拟函数（默认实现）
// ============================================================================

void Window::OnMouseMove(float x, float y) {
    (void)x;
    (void)y;
}

void Window::OnMouseDown(float x, float y, int button) {
    (void)x;
    (void)y;
    (void)button;
}

void Window::OnMouseUp(float x, float y, int button) {
    (void)x;
    (void)y;
    (void)button;
}

void Window::OnMouseWheel(float x, float y, int delta) {
    (void)x;
    (void)y;
    (void)delta;
}

void Window::OnKeyDown(int keyCode) {
    (void)keyCode;
}

void Window::OnKeyUp(int keyCode) {
    (void)keyCode;
}

void Window::OnChar(wchar_t ch) {
    (void)ch;
}

} // namespace luaui
