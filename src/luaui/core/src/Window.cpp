#include "Window.h"
#include "src/d2d/D2DRenderEngine.h"
#include "layout.h"
#include "RangeControls.h"
#include <windowsx.h>
#include <algorithm>

namespace luaui {

using namespace luaui::controls;

const wchar_t* Window::s_className = L"LuaUI_WindowClass";
bool Window::s_classRegistered = false;

Window::Window() = default;

Window::~Window() {
    StopTimer();
    // Release renderer
    if (m_renderer) {
        m_renderer->Shutdown();
        m_renderer.reset();
    }
    // Note: m_hWnd is destroyed by WM_DESTROY, don't call DestroyWindow here
}

bool Window::Create(HINSTANCE hInstance, const wchar_t* title, int width, int height) {
    m_hInstance = hInstance;
    
    // Register window class
    if (!s_classRegistered) {
        WNDCLASSEXW wcex = {};
        wcex.cbSize = sizeof(WNDCLASSEXW);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = StaticWindowProc;
        wcex.hInstance = hInstance;
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = nullptr; // We handle painting ourselves
        wcex.lpszClassName = s_className;
        
        if (!RegisterClassExW(&wcex)) {
            return false;
        }
        s_classRegistered = true;
    }
    
    // Calculate window size for desired client area
    RECT rc = { 0, 0, width, height };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    
    // Create window
    m_hWnd = CreateWindowExW(
        0,
        s_className,
        title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top,
        nullptr, nullptr, hInstance, this
    );
    
    if (!m_hWnd) {
        return false;
    }
    
    // Initialize renderer
    InitializeRenderer();
    
    return true;
}

void Window::InitializeRenderer() {
    m_renderer = std::make_unique<luaui::rendering::D2DRenderEngine>();
    if (!m_renderer->Initialize(RenderAPI::Direct2D)) {
        m_renderer.reset();
        return;
    }
    
    // Create render target for window
    RenderTargetDesc desc;
    desc.type = RenderTargetType::Window;
    desc.nativeHandle = m_hWnd;
    desc.width = 0;  // Use window size
    desc.height = 0;
    
    if (!m_renderer->CreateRenderTarget(desc)) {
        m_renderer->Shutdown();
        m_renderer.reset();
    }
}

LRESULT CALLBACK Window::StaticWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    Window* window = nullptr;
    
    if (message == WM_NCCREATE) {
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        window = static_cast<Window*>(cs->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
        window->m_hWnd = hWnd;
    } else {
        window = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }
    
    if (window) {
        return window->WindowProc(message, wParam, lParam);
    }
    
    return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT Window::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        StartTimer();
        return 0;
        
    case WM_DESTROY:
        OnClosing();
        StopTimer();
        OnClosed();
        PostQuitMessage(0);
        return 0;
        
    case WM_SIZE:
        if (m_renderer) {
            m_renderer->ResizeRenderTarget(LOWORD(lParam), HIWORD(lParam));
        }
        m_needsLayout = true;
        OnSizeChanged(GetClientSize());
        Invalidate();
        return 0;
        
    case WM_PAINT: {
        PAINTSTRUCT ps;
        BeginPaint(m_hWnd, &ps);
        Render();
        EndPaint(m_hWnd, &ps);
        return 0;
    }
    
    case WM_ERASEBKGND:
        return 1; // Prevent flicker
        
    case WM_LBUTTONDOWN:
        SetCapture(m_hWnd);
        HandleMouseDown(Point((float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam)), MouseButton::Left);
        return 0;
        
    case WM_LBUTTONUP:
        if (GetCapture() == m_hWnd) {
            ReleaseCapture();
        }
        HandleMouseUp(Point((float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam)), MouseButton::Left);
        return 0;
        
    case WM_RBUTTONDOWN:
        SetCapture(m_hWnd);
        HandleMouseDown(Point((float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam)), MouseButton::Right);
        return 0;
        
    case WM_RBUTTONUP:
        if (GetCapture() == m_hWnd) {
            ReleaseCapture();
        }
        HandleMouseUp(Point((float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam)), MouseButton::Right);
        return 0;
        
    case WM_MOUSEMOVE:
        HandleMouseMove(Point((float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam)));
        return 0;
        
    case WM_MOUSEWHEEL:
        HandleMouseWheel((float)(short)HIWORD(wParam) / (float)WHEEL_DELTA);
        return 0;
        
    case WM_KEYDOWN:
        if (OnKeyDown((int)wParam)) {
            return 0;
        }
        HandleKeyDown((int)wParam);
        return 0;
        
    case WM_KEYUP:
        if (OnKeyUp((int)wParam)) {
            return 0;
        }
        HandleKeyUp((int)wParam);
        return 0;
        
    case WM_CHAR:
        if (OnChar((wchar_t)wParam)) {
            return 0;
        }
        HandleChar((wchar_t)wParam);
        return 0;
        
    case WM_TIMER:
        if (wParam == TimerId) {
            OnTimer();
        }
        return 0;
        
    case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT) {
            SetCursor(LoadCursor(nullptr, IDC_ARROW));
            return TRUE;
        }
        break;
    }
    
    return DefWindowProc(m_hWnd, message, wParam, lParam);
}

void Window::HandleMouseDown(const Point& pt, MouseButton button) {
    m_lastMousePos = pt;
    
    // Track double-click
    DWORD now = GetTickCount();
    float dist = std::abs(pt.x - m_lastClickPos.x) + std::abs(pt.y - m_lastClickPos.y);
    if ((now - m_lastClickTime < DoubleClickTime) && (dist < DoubleClickDistance)) {
        m_clickCount++;
    } else {
        m_clickCount = 1;
    }
    m_lastClickTime = now;
    m_lastClickPos = pt;
    
    // Hit test
    auto hit = HitTest(pt);
    if (!hit) return;
    
    m_lastMouseDownControl = hit;
    
    // Focus
    if (hit->GetIsFocusable()) {
        SetFocus(hit);
    }
    
    // Dispatch event
    MouseEventArgs args(pt.x, pt.y, m_clickCount);
    hit->OnMouseDown(args);
    
    // Check for specific control types and handle their specific behavior
    if (auto* btn = dynamic_cast<Button*>(hit.get())) {
        btn->RaiseClick();
    }
    
    // Slider: start drag if within bounds
    if (auto* slider = dynamic_cast<luaui::controls::Slider*>(hit.get())) {
        if (hit->GetRenderRect().Contains(pt)) {
            m_isDragging = true;
            m_capturedControl = hit;
            slider->HandleMouseDown(pt);
        }
    }
    
    Invalidate();
}

void Window::HandleMouseUp(const Point& pt, MouseButton button) {
    m_lastMousePos = pt;
    m_isDragging = false;
    m_capturedControl.reset();
    
    auto hit = HitTest(pt);
    if (hit) {
        MouseEventArgs args(pt.x, pt.y, 0);
        hit->OnMouseUp(args);
    }
    
    Invalidate();
}

void Window::HandleMouseMove(const Point& pt) {
    Point delta(pt.x - m_lastMousePos.x, pt.y - m_lastMousePos.y);
    m_lastMousePos = pt;
    
    // Update hover state
    UpdateHoveredControl(pt);
    
    // Handle drag operations
    if (m_isDragging) {
        if (auto captured = m_capturedControl.lock()) {
            if (auto* slider = dynamic_cast<luaui::controls::Slider*>(captured.get())) {
                slider->HandleMouseMove(pt);
            }
        }
    }
    
    // Dispatch event to hit control
    auto hit = HitTest(pt);
    if (hit) {
        MouseEventArgs args(pt.x, pt.y, 0);
        hit->OnMouseMove(args);
    }
    
    Invalidate();
}

void Window::HandleMouseWheel(float delta) {
    auto hit = HitTest(m_lastMousePos);
    if (hit) {
        // MouseWheelEventArgs args(m_lastMousePos.x, m_lastMousePos.y, delta);
        hit->OnMouseWheel(MouseEventArgs(m_lastMousePos.x, m_lastMousePos.y, 0));
    }
}

void Window::HandleKeyDown(int keyCode) {
    if (auto focused = m_focusedControl.lock()) {
        KeyEventArgs args(keyCode, false);
        focused->OnKeyDown(args);
        
        // Special handling for TextBox
        if (auto* tb = dynamic_cast<TextBox*>(focused.get())) {
            // Let TextBox handle its own key processing via OnKeyDown
        }
    }
}

void Window::HandleKeyUp(int keyCode) {
    if (auto focused = m_focusedControl.lock()) {
        KeyEventArgs args(keyCode, false);
        focused->OnKeyUp(args);
    }
}

void Window::HandleChar(wchar_t ch) {
    if (auto focused = m_focusedControl.lock()) {
        if (auto* tb = dynamic_cast<TextBox*>(focused.get())) {
            tb->OnChar(ch);
        }
    }
}

void Window::UpdateHoveredControl(const Point& pt) {
    auto hit = HitTest(pt);
    auto current = m_hoveredControl.lock();
    
    if (hit != current) {
        if (current) {
            current->OnMouseLeave();
            current->RaiseMouseLeave();
        }
        if (hit) {
            hit->OnMouseEnter();
            hit->RaiseMouseEnter();
        }
        m_hoveredControl = hit;
    }
}

ControlPtr Window::HitTest(const Point& pt) {
    if (!m_root) return nullptr;
    return m_root->HitTestPoint(pt);
}

void Window::UpdateLayout() {
    if (!m_root || !m_renderer) return;
    
    auto size = GetClientSize();
    if (size.width <= 0 || size.height <= 0) return;
    
    // Measure
    m_root->Measure(Size(size.width, size.height));
    
    // Arrange
    m_root->Arrange(Rect(0, 0, size.width, size.height));
    
    m_needsLayout = false;
}

void Window::Render() {
    if (!m_renderer) return;
    
    if (m_needsLayout) {
        UpdateLayout();
    }
    
    if (!m_renderer->BeginFrame()) return;
    
    // Get render context and clear background
    auto* context = m_renderer->GetContext();
    if (context) {
        context->Clear(Color::White());
        
        // Render root control
        if (m_root) {
            m_root->Render(context);
        }
    }
    
    m_renderer->Present();
}

void Window::OnTimer() {
    // Update TextBox caret blink
    if (auto focused = m_focusedControl.lock()) {
        if (auto* tb = dynamic_cast<TextBox*>(focused.get())) {
            tb->UpdateCaret();
            // Only invalidate if caret visibility changed
            // TextBox::UpdateCaret returns void, so we invalidate conservatively
            Invalidate();
        }
    }
}

void Window::StartTimer() {
    if (!m_timerRunning && m_hWnd) {
        SetTimer(m_hWnd, TimerId, TimerInterval, TimerProc);
        m_timerRunning = true;
    }
}

void Window::StopTimer() {
    if (m_timerRunning && m_hWnd) {
        KillTimer(m_hWnd, TimerId);
        m_timerRunning = false;
    }
}

void CALLBACK Window::TimerProc(HWND hWnd, UINT message, UINT_PTR idTimer, DWORD dwTime) {
    Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (window && idTimer == TimerId) {
        window->OnTimer();
    }
}

void Window::SetRoot(const ControlPtr& root) {
    m_root = root;
    m_needsLayout = true;
    
    if (m_hWnd && m_root) {
        OnLoaded();
        Invalidate();
    }
}

ControlPtr Window::FindControl(const std::string& name) const {
    if (!m_root || name.empty()) return nullptr;
    
    // BFS search
    std::vector<ControlPtr> queue = { m_root };
    size_t index = 0;
    
    while (index < queue.size()) {
        auto current = queue[index++];
        
        if (current->GetName() == name) {
            return current;
        }
        
        // Add children
        for (size_t i = 0; i < current->GetChildCount(); ++i) {
            if (auto child = current->GetChild(i)) {
                queue.push_back(child);
            }
        }
        
        // Check ContentControl content
        if (auto contentControl = std::dynamic_pointer_cast<ContentControl>(current)) {
            if (auto content = contentControl->GetContent()) {
                queue.push_back(content);
            }
        }
    }
    
    return nullptr;
}

void Window::SetFocus(const ControlPtr& control) {
    if (auto current = m_focusedControl.lock()) {
        if (current == control) return;
        current->KillFocus();
        current->OnLostFocus();
    }
    
    if (control && control->GetIsFocusable()) {
        m_focusedControl = control;
        control->Focus();
        control->OnGotFocus();
    } else {
        m_focusedControl.reset();
    }
}

void Window::ClearFocus() {
    SetFocus(nullptr);
}

Size Window::GetClientSize() const {
    RECT rc;
    GetClientRect(m_hWnd, &rc);
    return Size((float)(rc.right - rc.left), (float)(rc.bottom - rc.top));
}

bool Window::IsVisible() const {
    return m_hWnd && IsWindowVisible(m_hWnd);
}

void Window::Show(int nCmdShow) {
    if (m_hWnd) {
        ShowWindow(m_hWnd, nCmdShow);
        UpdateWindow(m_hWnd);
    }
}

void Window::Close() {
    if (m_hWnd) {
        DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
    }
}

int Window::Run() {
    Show();
    
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}

void Window::Invalidate() {
    if (m_hWnd) {
        ::InvalidateRect(m_hWnd, nullptr, FALSE);
    }
}

void Window::InvalidateRect(const luaui::Rect& rect) {
    if (m_hWnd) {
        RECT rc = {
            (LONG)rect.x,
            (LONG)rect.y,
            (LONG)(rect.x + rect.width),
            (LONG)(rect.y + rect.height)
        };
        ::InvalidateRect(m_hWnd, &rc, FALSE);
    }
}

} // namespace luaui
