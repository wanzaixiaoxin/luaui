#include "Window.h"
#include "../rendering/d2d/D2DRenderEngine.h"
#include <objbase.h>

namespace luaui {

const wchar_t* Window::s_className = L"LuaUI_WindowClass";
bool Window::s_classRegistered = false;

Window::Window() = default;

Window::~Window() {
    if (m_dispatcher) m_dispatcher->Shutdown();
    if (m_renderer) m_renderer->Shutdown();
    if (m_hWnd) DestroyWindow(m_hWnd);
    CoUninitialize();
}

bool Window::Create(HINSTANCE hInstance, const wchar_t* title, int width, int height) {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) return false;
    
    m_hInstance = hInstance;
    
    if (!s_classRegistered) {
        WNDCLASSEXW wcex = {};
        wcex.cbSize = sizeof(WNDCLASSEXW);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = StaticWndProc;
        wcex.hInstance = hInstance;
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.lpszClassName = s_className;
        
        if (!RegisterClassExW(&wcex)) return false;
        s_classRegistered = true;
    }
    
    m_hWnd = CreateWindowExW(0, s_className, title, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        nullptr, nullptr, hInstance, this);
    
    if (!m_hWnd) return false;
    
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
    
    m_dispatcher = std::make_unique<Dispatcher>();
    
    OnLoaded();
    return true;
}

void Window::Show(int nCmdShow) {
    if (m_hWnd) {
        ShowWindow(m_hWnd, nCmdShow);
        UpdateWindow(m_hWnd);
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

void Window::SetRoot(const std::shared_ptr<Control>& root) {
    m_root = root;
    if (m_root && m_dispatcher) {
        m_root->SetDispatcher(m_dispatcher.get());
    }
}

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
        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(m_hWnd, &ps);
            if (m_renderer && m_renderer->BeginFrame()) {
                if (auto* ctx = m_renderer->GetContext()) {
                    RECT rc;
                    GetClientRect(m_hWnd, &rc);
                    ctx->Clear(rendering::Color::White());
                    // TODO: Render m_root
                }
                m_renderer->Present();
            }
            EndPaint(m_hWnd, &ps);
            return 0;
        }
        case WM_SIZE:
            if (m_renderer) m_renderer->ResizeRenderTarget(LOWORD(lP), HIWORD(lP));
            return 0;
        case WM_DESTROY:
            OnClosing();
            OnClosed();
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(m_hWnd, msg, wP, lP);
}

} // namespace luaui
