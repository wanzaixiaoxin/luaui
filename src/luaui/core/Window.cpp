#include "Window.h"
#include "../rendering/d2d/D2DRenderEngine.h"
#include "../rendering/d2d/D2DAnimation.h"
#include "../controls/Control.h"
#include "../controls/Panel.h"
#include "../controls/Menu.h"
#include "Components/InputComponent.h"
#include "../utils/Logger.h"
#include "../style/Theme.h"
#include "../style/ThemeKeys.h"
#include <objbase.h>
#include <windowsx.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

using namespace luaui::utils;

namespace luaui {
namespace {

BYTE ToColorByte(float channel) {
    if (channel <= 0.0f) return 0;
    if (channel >= 1.0f) return 255;
    return static_cast<BYTE>(channel * 255.0f + 0.5f);
}

COLORREF ToColorRef(const rendering::Color& color) {
    return RGB(ToColorByte(color.r), ToColorByte(color.g), ToColorByte(color.b));
}

} // namespace

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
    
    // 初始化动画 Timeline
    m_timeline = rendering::CreateAnimationTimeline();
    QueryPerformanceCounter(&m_lastAnimTick);
    
    utils::Logger::Info("Window created successfully");

    // 注册主题回调：主题切换时同步更新标题栏 + 全屏重绘
    m_themeCallbackId = controls::Theme::GetCurrent().AddCallback([this]() {
        UpdateTitleBarTheme();
        InvalidateRender();
    });

    // 初始设置标题栏主题
    UpdateTitleBarTheme();

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
    
    // 获取主题背景色用于清屏
    auto windowBgColor = controls::Theme::GetCurrent().GetColor(theme::kBackgroundPrimary);

    if (fullScreenRender) {
        // 全屏渲染（传统方式）
        context->Clear(windowBgColor);

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

            // 清空该区域（使用主题背景色）
            if (auto* cache = GetResourceCache()) {
                context->FillRectangle(dirtyRect, cache->GetSolidColorBrush(windowBgColor));
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
    
    // 渲染弹出层控件（如 Menu）——在所有其他控件之后渲染，确保在最上层
    for (auto& weak : m_popups) {
        if (auto popup = weak.lock()) {
            if (popup->GetIsVisible()) {
                if (auto* renderable = popup->AsRenderable()) {
                    renderable->Render(context);
                }
            }
        }
    }
    
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
    
    // 渲染当前控件（Panel 的 RenderOverride 会处理子控件渲染）
    if (auto* renderable = control->AsRenderable()) {
        renderable->Render(context);
    }
    
    // 只有非 Panel 类型才递归渲染子控件
    // Panel 的子控件由 PanelRenderComponent::RenderOverride → OnRenderChildren 处理
    // 避免子控件被渲染两次
    if (!dynamic_cast<controls::Panel*>(control)) {
        size_t childCount = control->GetChildCount();
        for (size_t i = 0; i < childCount; ++i) {
            auto child = control->GetChild(i);
            if (auto childControl = std::dynamic_pointer_cast<Control>(child)) {
                RenderWithClipping(childControl.get(), context, clipRect);
            }
        }
    }
}

void Window::Render() {
    OnRender();
}

void Window::RegisterPopup(const std::shared_ptr<Control>& popup) {
    if (!popup) return;
    // 避免重复注册
    for (auto& weak : m_popups) {
        if (auto existing = weak.lock()) {
            if (existing.get() == popup.get()) return;
        }
    }
    m_popups.push_back(popup);
}

void Window::UnregisterPopup(const std::shared_ptr<Control>& popup) {
    if (!popup) return;
    m_popups.erase(
        std::remove_if(m_popups.begin(), m_popups.end(),
            [&popup](const std::weak_ptr<Control>& weak) {
                auto existing = weak.lock();
                return !existing || existing.get() == popup.get();
            }),
        m_popups.end());
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
    // 先检查弹出层控件（它们在最上层）
    for (auto it = m_popups.rbegin(); it != m_popups.rend(); ++it) {
        if (auto popup = it->lock()) {
            if (popup->GetIsVisible()) {
                if (auto* result = HitTestControl(popup.get(), x, y, 0, 0)) {
                    return result;
                }
            }
        }
    }
    
    // 再检查常规控件
    if (root) {
        if (auto* result = HitTestControl(root, x, y, offsetX, offsetY)) {
            return result;
        }
    }
    
    return nullptr;
}

Control* Window::HitTestControl(Control* root, float x, float y, float offsetX, float offsetY) {
    if (!root) return nullptr;
    
    // 检查控件是否可见
    if (!root->GetIsVisible()) return nullptr;
    
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
                if (auto* result = HitTestControl(child, x, y, childOffsetX, childOffsetY)) {
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
    if (m_capturedControl) {
        controls::MouseEventArgs args{x, y, 0, false};
        if (auto* inputComp = m_capturedControl->GetInput()) {
            inputComp->RaiseMouseMove(args);
        } else {
            m_capturedControl->OnMouseMove(args);
        }
        InvalidateRender();
        return;
    }

    auto* control = HitTest(m_root.get(), x, y);
    
    Logger::TraceF("[Window] HandleMouseMove: (%.1f,%.1f) hit=%s captured=%s", 
        x, y, 
        control ? control->GetTypeName().c_str() : "null",
        m_capturedControl ? m_capturedControl->GetTypeName().c_str() : "null");

    if (m_lastMouseOver && m_lastMouseOver != control) {
        if (auto* inputComp = m_lastMouseOver->GetInput()) {
            inputComp->RaiseMouseLeave();
        }
    }

    if (control) {
        // 首先处理命中的控件
        if (auto* inputComp = control->GetInput()) {
            if (m_lastMouseOver != control) {
                inputComp->RaiseMouseEnter();
            }
            controls::MouseEventArgs args{x, y, 0, false};
            inputComp->RaiseMouseMove(args);
        }

        // 向父级链传递 MouseMove，让容器控件（如 ScrollViewer、MenuBar）能检测 hover
        controls::MouseEventArgs parentArgs{x, y, 0, false};
        auto parent = control->GetParent();
        Control* cur = parent ? static_cast<Control*>(parent.get()) : nullptr;
        while (cur) {
            // 始终调用OnMouseMove，让控件自己决定是否处理
            // ScrollViewer等容器需要接收事件来检测滚动条hover
            // MenuBar需要接收事件来检测窗口按钮hover
            cur->OnMouseMove(parentArgs);
            auto p = cur->GetParent();
            cur = p ? static_cast<Control*>(p.get()) : nullptr;
        }
    }

    m_lastMouseOver = control;
    InvalidateRender();
}

void Window::HandleMouseDown(float x, float y, int button) {
    auto* control = HitTest(m_root.get(), x, y, 0, 0);
    
    utils::Logger::InfoF("[Window] MouseDown: %s at (%.1f,%.1f) captured=%s",
        control ? control->GetTypeName().c_str() : "null", x, y,
        m_capturedControl ? m_capturedControl->GetTypeName().c_str() : "null");
    
    if (control) {
        m_capturedControl = control;
        SetCapture(m_hWnd);

        controls::MouseEventArgs args{x, y, button, false};

        Control* current = control;
        while (current && !args.Handled) {
            if (auto* inputComp = current->GetInput()) {
                if (inputComp->GetIsFocusable()) {
                    UpdateFocus(current);
                }
                inputComp->RaiseMouseDown(args);
            } else {
                current->OnMouseDown(args);
            }
            if (!args.Handled) {
                auto parent = current->GetParent();
                current = parent ? static_cast<Control*>(parent.get()) : nullptr;
            }
        }

        if (args.Handled && current && current != m_capturedControl) {
            m_capturedControl = current;
        }
    }
    
    InvalidateRender();
}

void Window::HandleMouseUp(float x, float y, int button) {
    ReleaseCapture();
    
    utils::Logger::DebugF("[Window] MouseUp at (%.1f,%.1f), captured=%s", 
        x, y, m_capturedControl ? m_capturedControl->GetTypeName().c_str() : "null");
    
    if (m_capturedControl) {
        controls::MouseEventArgs args{x, y, button, false};

        if (auto* inputComp = m_capturedControl->GetInput()) {
            inputComp->RaiseMouseUp(args);
        } else {
            m_capturedControl->OnMouseUp(args);
        }

        if (!args.Handled) {
            auto* hitControl = HitTest(m_root.get(), x, y, 0, 0);
            if (hitControl == m_capturedControl) {
                if (auto* inputComp = m_capturedControl->GetInput()) {
                    inputComp->RaiseClick();
                }
            }
        }
        m_capturedControl = nullptr;
    } else {
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

void Window::HandleMouseDoubleClick(float x, float y, int button) {
    auto* control = HitTest(m_root.get(), x, y, 0, 0);
    
    utils::Logger::InfoF("[Window] MouseDoubleClick: %s at (%.1f,%.1f)",
        control ? control->GetTypeName().c_str() : "null", x, y);
    
    if (control) {
        controls::MouseEventArgs args{x, y, button, false};

        // 从命中的控件开始，向上冒泡直到有人处理
        Control* current = control;
        while (current && !args.Handled) {
            if (auto* inputComp = current->GetInput()) {
                inputComp->RaiseMouseDoubleClick(args);
            } else {
                current->OnMouseDoubleClick(args);
            }
            // 向父控件冒泡
            auto parent = current->GetParent();
            current = parent ? static_cast<Control*>(parent.get()) : nullptr;
        }
    }
    
    InvalidateRender();
}

void Window::HandleMouseWheel(float x, float y, int delta) {
    auto* control = m_capturedControl ? m_capturedControl : HitTest(m_root.get(), x, y, 0, 0);

    if (control) {
        controls::MouseEventArgs args{x, y, delta, false};

        // 从命中的控件开始，向上冒泡直到有人处理
        Control* current = control;
        while (current && !args.Handled) {
            // 先尝试通过 InputComponent（Button, TextBox 等控件）
            if (auto* inputComp = current->GetInput()) {
                inputComp->RaiseMouseWheel(args);
            } else {
                // 直接调用虚函数（Panel/ScrollViewer 等没有 InputComponent 的控件）
                current->OnMouseWheel(args);
            }
            // 向父控件冒泡
            auto parent = current->GetParent();
            current = parent ? static_cast<Control*>(parent.get()) : nullptr;
        }
    }

    InvalidateRender();
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
// 动画帧驱动
// ============================================================================

void Window::StartAnimTimer() {
    if (m_animTimerRunning || !m_hWnd) return;
    ::SetTimer(m_hWnd, ANIM_TIMER_ID, ANIM_INTERVAL_MS, nullptr);
    m_animTimerRunning = true;
    QueryPerformanceCounter(&m_lastAnimTick);
}

void Window::StopAnimTimer() {
    if (!m_animTimerRunning || !m_hWnd) return;
    ::KillTimer(m_hWnd, ANIM_TIMER_ID);
    m_animTimerRunning = false;
}

void Window::OnAnimTimerTick() {
    if (!m_timeline) return;

    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);

    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);

    float deltaMs = 0.0f;
    if (freq.QuadPart > 0 && m_lastAnimTick.QuadPart > 0) {
        deltaMs = static_cast<float>(
            (now.QuadPart - m_lastAnimTick.QuadPart) * 1000.0 / freq.QuadPart);
    }
    m_lastAnimTick = now;

    m_timeline->Update(deltaMs);

    // 没有活跃动画时自动停止 Timer，节省 CPU
    if (!m_timeline->HasActiveAnimations()) {
        StopAnimTimer();
    }

    InvalidateRender();
}

// ============================================================================
// 标题栏主题
// ============================================================================

void Window::UpdateTitleBarTheme() {
    if (!m_hWnd) return;

    const auto& currentTheme = controls::Theme::GetCurrent();
    const auto background = currentTheme.GetColor(theme::kBackgroundPrimary);
    const auto titleBarColor = currentTheme.GetColor(theme::kMenuBarBg);

    const BOOL useDarkMode =
        ((background.r + background.g + background.b) / 3.0f < 0.5f) ? TRUE : FALSE;
    DwmSetWindowAttribute(m_hWnd, 20, &useDarkMode, sizeof(useDarkMode));

    const COLORREF borderColor = m_extendFrame
        ? ToColorRef(titleBarColor)
        : static_cast<COLORREF>(0xFFFFFFFFu);
    DwmSetWindowAttribute(m_hWnd, 34, &borderColor, sizeof(borderColor));

    HWND active = ::GetActiveWindow();
    bool isActive = (active == m_hWnd);
    if (isActive) {
        ::SendMessage(m_hWnd, WM_NCACTIVATE, FALSE, 0);
        ::SendMessage(m_hWnd, WM_NCACTIVATE, TRUE, 0);
    } else {
        ::SendMessage(m_hWnd, WM_NCACTIVATE, TRUE, -1);
    }

    SetWindowPos(m_hWnd, nullptr, 0, 0, 0, 0,
                 SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

void Window::SetExtendFrameIntoClientArea(bool enable) {
    m_extendFrame = enable;
    if (!m_hWnd) return;

    if (enable) {
        // Remove caption to hide title bar, keep frame for resize/shadow/buttons
        LONG style = GetWindowLong(m_hWnd, GWL_STYLE);
        style &= ~WS_CAPTION;
        style |= WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
        SetWindowLong(m_hWnd, GWL_STYLE, style);

        // Keep the minimal DWM frame so the window still has shadow/rounded corners
        // without turning the entire client area into a glass frame surface.
        MARGINS margins = { 1, 1, 1, 1 };
        DwmExtendFrameIntoClientArea(m_hWnd, &margins);

        // Force window to recalculate frame
        SetWindowPos(m_hWnd, nullptr, 0, 0, 0, 0,
                     SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    } else {
        MARGINS margins = { 0, 0, 0, 0 };
        DwmExtendFrameIntoClientArea(m_hWnd, &margins);
    }

    UpdateTitleBarTheme();
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
        case WM_NCCALCSIZE: {
            if (!m_extendFrame) {
                break;
            }

            if (wP) {
                auto* calcSize = reinterpret_cast<NCCALCSIZE_PARAMS*>(lP);
                if (calcSize && ::IsZoomed(m_hWnd)) {
                    const int frameX =
                        ::GetSystemMetrics(SM_CXSIZEFRAME) + ::GetSystemMetrics(SM_CXPADDEDBORDER);
                    const int frameY =
                        ::GetSystemMetrics(SM_CYSIZEFRAME) + ::GetSystemMetrics(SM_CXPADDEDBORDER);

                    calcSize->rgrc[0].left += frameX;
                    calcSize->rgrc[0].right -= frameX;
                    calcSize->rgrc[0].top += frameY;
                    calcSize->rgrc[0].bottom -= frameY;
                }
            }

            return 0;
        }

        // ========== 渲染 ==========
        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(m_hWnd, &ps);
            Render();
            EndPaint(m_hWnd, &ps);
            return 0;
        }
        
        // ========== 动画帧驱动 ==========
        case WM_TIMER: {
            if (wP == ANIM_TIMER_ID) {
                OnAnimTimerTick();
                return 0;
            }
            break;
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

        case WM_NCHITTEST: {
            if (!m_extendFrame) break;
            LRESULT hit = DefWindowProc(m_hWnd, WM_NCHITTEST, wP, lP);
            if (hit == HTCLIENT) {
                POINT pt = { GET_X_LPARAM(lP), GET_Y_LPARAM(lP) };
                ScreenToClient(m_hWnd, &pt);

                // 边框缩放区域检测
                const int borderWidth = 5; // 边框宽度为5像素
                bool onLeft   = pt.x < borderWidth;
                bool onRight  = pt.x >= static_cast<LONG>(m_width) - borderWidth;
                bool onTop    = pt.y < borderWidth;
                bool onBottom = pt.y >= static_cast<LONG>(m_height) - borderWidth;

                // 检测角落（优先级最高）
                if (onTop && onLeft)     return HTTOPLEFT;
                if (onTop && onRight)    return HTTOPRIGHT;
                if (onBottom && onLeft)  return HTBOTTOMLEFT;
                if (onBottom && onRight) return HTBOTTOMRIGHT;

                // 检测边框
                if (onLeft)   return HTLEFT;
                if (onRight)  return HTRIGHT;
                if (onTop)    return HTTOP;
                if (onBottom) return HTBOTTOM;

                // 顶部标题栏区域（y < 32）需要区分拖拽和交互
                if (pt.y >= 0 && pt.y < 32) {
                    // 先用框架 HitTest 检查是否点到了可交互控件
                    if (m_root) {
                        auto* hitControl = HitTest(m_root.get(), 
                            static_cast<float>(pt.x), static_cast<float>(pt.y), 0, 0);
                        if (hitControl) {
                            utils::Logger::DebugF("[WM_NCHITTEST] HitTest found: %s at (%d,%d)",
                                hitControl->GetTypeName().c_str(), pt.x, pt.y);
                            
                            // 检查父链上是否有 MenuBar
                            Control* cur = hitControl;
                            while (cur) {
                                if (cur->GetTypeName() == "MenuBar") {
                                    // 找到 MenuBar，检查是否是空白区域
                                    auto* menuBarRender = cur->GetRender();
                                    if (menuBarRender) {
                                        auto barRect = menuBarRender->GetRenderRect();
                                        // 使用全局坐标检查
                                        if (static_cast<controls::MenuBar*>(cur)->IsBlankArea(
                                            static_cast<float>(pt.x), static_cast<float>(pt.y), barRect)) {
                                            // 是空白区域，允许拖拽
                                            return HTCAPTION;
                                        }
                                    }
                                    // 不是空白区域，返回 HTCLIENT 让事件路由到控件
                                    return HTCLIENT;
                                }
                                auto parent = cur->GetParent();
                                cur = parent ? static_cast<Control*>(parent.get()) : nullptr;
                            }
                            
                            // 如果命中的控件有 InputComponent，说明是可交互控件
                            if (hitControl->GetInput()) {
                                return HTCLIENT;
                            }
                        }
                    }
                    // 没有命中可交互控件，允许拖拽窗体
                    if (pt.x < static_cast<LONG>(m_width) - 135) {
                        return HTCAPTION;
                    }
                }
            }
            return hit;
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
        
        case WM_LBUTTONDBLCLK: {
            float x = static_cast<float>(GET_X_LPARAM(lP));
            float y = static_cast<float>(GET_Y_LPARAM(lP));
            HandleMouseDoubleClick(x, y, 0);
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
            POINT pt = { GET_X_LPARAM(lP), GET_Y_LPARAM(lP) };
            ScreenToClient(m_hWnd, &pt);
            float x = static_cast<float>(pt.x);
            float y = static_cast<float>(pt.y);
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
