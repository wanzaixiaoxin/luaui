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
        
        // 确保立即进行首次渲染，避免控件延迟显示
        InvalidateRender();
        Render();
        
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
    if (m_root) {
        // 递归设置 Window 指针
        SetWindowForControlTree(m_root.get(), this);
        
        if (m_dispatcher) {
            m_root->SetDispatcher(m_dispatcher.get());
        }
    }
    InvalidateLayout();
}

void Window::SetWindowForControlTree(Control* control, Window* window) {
    if (!control) return;
    
    control->SetWindow(window);
    
    // 递归设置子控件
    size_t childCount = control->GetChildCount();
    for (size_t i = 0; i < childCount; ++i) {
        auto child = control->GetChild(i);
        if (auto childControl = std::dynamic_pointer_cast<Control>(child)) {
            SetWindowForControlTree(childControl.get(), window);
        }
    }
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
    
    // 使用全局作用符调用 Windows API
    ::InvalidateRect(m_hWnd, nullptr, FALSE);
}

void Window::InvalidateRender() {
    // 全屏变脏
    m_dirtyRegion.InvalidateAll(m_width, m_height);
    ::InvalidateRect(m_hWnd, nullptr, FALSE);
}

void Window::InvalidateRect(const rendering::Rect& rect) {
    // 添加到脏矩形区域
    m_dirtyRegion.AddRect(rect);
    
    // 转换为 Windows RECT 触发系统重绘
    RECT rc;
    rc.left = static_cast<LONG>(rect.x);
    rc.top = static_cast<LONG>(rect.y);
    rc.right = static_cast<LONG>(rect.x + rect.width);
    rc.bottom = static_cast<LONG>(rect.y + rect.height);
    ::InvalidateRect(m_hWnd, &rc, FALSE);
}

bool Window::NeedsRedraw(const rendering::Rect& bounds) const {
    return m_dirtyRegion.Intersects(bounds);
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
    
    // 确保资源缓存已创建
    if (!m_resourceCache) {
        m_resourceCache = std::make_unique<rendering::ResourceCache>(context);
    }
    
    // 更新布局（如果需要）
    if (m_layoutDirty) {
        UpdateLayout();
        // 布局变更使全屏变脏
        m_dirtyRegion.InvalidateAll(m_width, m_height);
    }
    
    // 检查是否有脏矩形需要重绘（首次渲染或布局后确保有脏区域）
    if (m_dirtyRegion.IsEmpty()) {
        // 如果没有指定脏区域，默认全屏渲染（首次渲染场景）
        m_dirtyRegion.InvalidateAll(m_width, m_height);
    }
    
    // 获取脏矩形区域列表
    const auto& dirtyRects = m_dirtyRegion.GetRects();
    
    // 如果脏区域接近全屏，直接全屏渲染
    bool fullScreenRender = false;
    if (dirtyRects.size() == 1) {
        const auto& rect = dirtyRects[0];
        float area = rect.width * rect.height;
        float windowArea = m_width * m_height;
        if (area > windowArea * 0.75f) {
            fullScreenRender = true;
        }
    }
    
    if (fullScreenRender) {
        // 全屏渲染（传统方式）
        context->Clear(rendering::Color::White());
        
        if (m_root) {
            if (auto* renderable = m_root->AsRenderable()) {
                renderable->Render(context);
            }
        }
    } else {
        // 局部渲染：对每个脏矩形区域进行裁剪渲染
        for (const auto& dirtyRect : dirtyRects) {
            // 设置裁剪区域
            context->PushClip(dirtyRect);
            
            // 清空该区域（使用白色画刷）
            if (auto* cache = GetResourceCache()) {
                context->FillRectangle(dirtyRect, cache->GetSolidColorBrush(rendering::Color::White()));
            }
            
            // 渲染与脏矩形相交的控件
            if (m_root) {
                RenderWithClipping(m_root.get(), context, dirtyRect);
            }
            
            // 恢复裁剪
            context->PopClip();
        }
    }
    
    // 清除脏矩形区域
    m_dirtyRegion.Clear();
    
    m_renderer->Present();
}

void Window::RenderWithClipping(Control* control, rendering::IRenderContext* context, 
                                 const rendering::Rect& clipRect) {
    if (!control) return;
    
    // 获取控件渲染矩形
    rendering::Rect bounds;
    if (auto* render = control->GetRender()) {
        bounds = render->GetRenderRect();
    }
    
    // 检查是否与裁剪矩形相交
    if (!bounds.Intersects(clipRect)) {
        return;  // 完全在裁剪区域外，跳过渲染
    }
    
    // 渲染当前控件
    if (auto* renderable = control->AsRenderable()) {
        renderable->Render(context);
    }
    
    // 递归渲染子控件
    size_t childCount = control->GetChildCount();
    for (size_t i = 0; i < childCount; ++i) {
        auto child = control->GetChild(i);
        if (auto childControl = std::dynamic_pointer_cast<Control>(child)) {
            RenderWithClipping(childControl.get(), context, clipRect);
        }
    }
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

Control* Window::HitTest(Control* root, float x, float y, float offsetX, float offsetY) {
    if (!root) return nullptr;
    
    auto* render = root->GetRender();
    if (!render) return nullptr;
    
    const auto& rect = render->GetRenderRect();
    // 计算全局坐标
    float globalX = rect.x + offsetX;
    float globalY = rect.y + offsetY;
    
    // 检查点是否在矩形内（使用全局坐标）
    if (x >= globalX && x < globalX + rect.width &&
        y >= globalY && y < globalY + rect.height) {
        
        // 如果是 Panel，递归测试子控件
        if (auto* panel = dynamic_cast<controls::Panel*>(root)) {
            // 子控件使用父控件的全局坐标作为偏移
            float childOffsetX = globalX;
            float childOffsetY = globalY;
            
            // 从后向前遍历（后添加的在上面）
            for (int i = static_cast<int>(panel->GetChildCount()) - 1; i >= 0; --i) {
                auto* child = static_cast<Control*>(panel->GetChild(i).get());
                if (auto* result = HitTest(child, x, y, childOffsetX, childOffsetY)) {
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
    auto* control = HitTest(m_root.get(), x, y, 0, 0);
    
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
    
    utils::Logger::DebugF("[Window] MouseUp at (%.1f,%.1f), captured=%s", 
        x, y, m_capturedControl ? m_capturedControl->GetTypeName().c_str() : "null");
    
    if (m_capturedControl) {
        if (auto* inputComp = m_capturedControl->GetInput()) {
            controls::MouseEventArgs args{x, y, button, false};
            inputComp->RaiseMouseUp(args);
            
            // 如果鼠标仍在控件上，触发点击
            auto* hitControl = HitTest(m_root.get(), x, y, 0, 0);
            utils::Logger::DebugF("[Window] HitTest result: %s", 
                hitControl ? hitControl->GetTypeName().c_str() : "null");
            if (hitControl == m_capturedControl) {
                utils::Logger::DebugF("[Window] Raising Click for %s", m_capturedControl->GetTypeName().c_str());
                inputComp->RaiseClick();
            }
        }
        m_capturedControl = nullptr;
    } else {
        // 没有捕获的控件，正常处理
        auto* control = HitTest(m_root.get(), x, y, 0, 0);
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
    auto* control = m_capturedControl ? m_capturedControl : HitTest(m_root.get(), x, y, 0, 0);
    
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
