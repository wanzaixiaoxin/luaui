// LuaUI Controls Demo - 使用新SOLID架构API
// 06_controls_demo的简化版本，展示新控件API

#include "Controls.h"
#include "IRenderContext.h"
#include "IRenderEngine.h"
#include <windows.h>
#include <windowsx.h>
#include <objbase.h>
#include <string>
#include <sstream>
#include <iostream>

using namespace luaui;
using namespace luaui::controls;
using namespace luaui::rendering;

// 简化的演示窗口 - 使用新架构
class ControlsDemoWindow {
public:
    ControlsDemoWindow() = default;
    ~ControlsDemoWindow() { Cleanup(); }

    bool Initialize(HINSTANCE hInstance, int nCmdShow) {
        WNDCLASSEXW wcex = {};
        wcex.cbSize = sizeof(WNDCLASSEXW);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WindowProc;
        wcex.hInstance = hInstance;
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszClassName = L"LuaUIControlsDemoNew";

        if (!RegisterClassExW(&wcex)) return false;

        m_hWnd = CreateWindowExW(
            0, L"LuaUIControlsDemoNew", L"LuaUI Controls Demo - New Architecture",
            WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME,
            CW_USEDEFAULT, CW_USEDEFAULT, 900, 700,
            nullptr, nullptr, hInstance, this
        );

        if (!m_hWnd) return false;

        m_engine = CreateRenderEngine();
        if (!m_engine || !m_engine->Initialize()) {
            MessageBoxW(m_hWnd, L"Failed to initialize rendering engine", L"Error", MB_OK);
            return false;
        }

        RenderTargetDesc desc;
        desc.type = RenderTargetType::Window;
        desc.nativeHandle = m_hWnd;
        desc.width = 900;
        desc.height = 700;

        if (!m_engine->CreateRenderTarget(desc)) {
            MessageBoxW(m_hWnd, L"Failed to create render target", L"Error", MB_OK);
            return false;
        }

        CreateControls();

        ShowWindow(m_hWnd, nCmdShow);
        UpdateWindow(m_hWnd);
        return true;
    }

    int Run() {
        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return (int)msg.wParam;
    }

private:
    void CreateControls() {
        // 创建根面板 - 使用新StackPanel
        m_rootPanel = std::make_shared<StackPanel>();
        m_rootPanel->SetName("root");
        m_rootPanel->SetOrientation(StackPanel::Orientation::Vertical);
        
        // 标题
        auto title = std::make_shared<TextBlock>();
        title->SetText(L"🎨 LuaUI Controls Demo - New Architecture");
        title->SetFontSize(24);
        m_rootPanel->AddChild(title);
        
        // 副标题
        auto subtitle = std::make_shared<TextBlock>();
        subtitle->SetText(L"Using SOLID Architecture with Component Pattern");
        subtitle->SetFontSize(14);
        m_rootPanel->AddChild(subtitle);
        
        // 按钮区域
        auto buttonPanel = std::make_shared<StackPanel>();
        buttonPanel->SetOrientation(StackPanel::Orientation::Horizontal);
        buttonPanel->SetSpacing(10);
        
        auto btn1 = std::make_shared<Button>();
        buttonPanel->AddChild(btn1);
        
        auto btn2 = std::make_shared<Button>();
        buttonPanel->AddChild(btn2);
        
        m_rootPanel->AddChild(buttonPanel);
        
        // 文本框
        auto textBox = std::make_shared<TextBox>();
        m_rootPanel->AddChild(textBox);
        
        // 复选框
        auto checkBox = std::make_shared<CheckBox>();
        m_rootPanel->AddChild(checkBox);
        
        // 滑块
        auto slider = std::make_shared<Slider>();
        slider->SetValue(50);
        m_rootPanel->AddChild(slider);
        
        // 进度条
        m_progressBar = std::make_shared<ProgressBar>();
        m_progressBar->SetValue(50);
        m_rootPanel->AddChild(m_progressBar);
        
        // 形状展示
        auto shapesPanel = std::make_shared<StackPanel>();
        shapesPanel->SetOrientation(StackPanel::Orientation::Horizontal);
        shapesPanel->SetSpacing(10);
        
        auto rect = std::make_shared<luaui::controls::Rectangle>();
        if (auto* layout = rect->GetLayout()) {
            layout->SetWidth(60);
            layout->SetHeight(60);
        }
        rect->SetFill(Color::FromHex(0x4CAF50));
        shapesPanel->AddChild(rect);
        
        auto ellipse = std::make_shared<luaui::controls::Ellipse>();
        if (auto* layout = ellipse->GetLayout()) {
            layout->SetWidth(60);
            layout->SetHeight(60);
        }
        ellipse->SetFill(Color::FromHex(0x2196F3));
        shapesPanel->AddChild(ellipse);
        
        m_rootPanel->AddChild(shapesPanel);
        
        // 列表框
        auto listBox = std::make_shared<ListBox>();
        m_rootPanel->AddChild(listBox);
        
        // 状态文本
        m_statusText = std::make_shared<TextBlock>();
        m_statusText->SetText(L"Ready - New Architecture Demo");
        m_statusText->SetFontSize(12);
        m_rootPanel->AddChild(m_statusText);
    }

    void Render() {
        if (!m_engine->BeginFrame()) return;

        auto* context = m_engine->GetContext();
        if (!context) {
            m_engine->Present();
            return;
        }

        // 清空背景
        context->Clear(Color::White());

        // 测量和排列
        RECT rc;
        GetClientRect(m_hWnd, &rc);
        float width = static_cast<float>(rc.right - rc.left);
        float height = static_cast<float>(rc.bottom - rc.top);

        interfaces::LayoutConstraint constraint;
        constraint.available = Size(width, height);
        
        if (auto* layoutable = m_rootPanel->AsLayoutable()) {
            layoutable->Measure(constraint);
            layoutable->Arrange(Rect(0, 0, width, height));
        }
        
        // 渲染
        if (auto* renderable = m_rootPanel->AsRenderable()) {
            renderable->Render(context);
        }

        m_engine->Present();
    }

    void Cleanup() {
        m_rootPanel = nullptr;
        if (m_engine) {
            m_engine->Shutdown();
            m_engine = nullptr;
        }
    }

    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        ControlsDemoWindow* pThis = nullptr;

        if (message == WM_NCCREATE) {
            CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            pThis = reinterpret_cast<ControlsDemoWindow*>(pCreate->lpCreateParams);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
            pThis->m_hWnd = hWnd;
        } else {
            pThis = reinterpret_cast<ControlsDemoWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        }

        if (pThis) {
            switch (message) {
                case WM_PAINT: {
                    PAINTSTRUCT ps;
                    BeginPaint(hWnd, &ps);
                    pThis->Render();
                    EndPaint(hWnd, &ps);
                    return 0;
                }
                case WM_SIZE: {
                    int width = LOWORD(lParam);
                    int height = HIWORD(lParam);
                    if (pThis->m_engine) {
                        pThis->m_engine->ResizeRenderTarget(width, height);
                    }
                    InvalidateRect(hWnd, nullptr, FALSE);
                    return 0;
                }
                case WM_DESTROY:
                    PostQuitMessage(0);
                    return 0;
            }
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

private:
    HWND m_hWnd = nullptr;
    IRenderEnginePtr m_engine;
    std::shared_ptr<StackPanel> m_rootPanel;
    std::shared_ptr<TextBlock> m_statusText;
    std::shared_ptr<ProgressBar> m_progressBar;
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        MessageBoxW(nullptr, L"Failed to initialize COM", L"Error", MB_OK);
        return 1;
    }

    ControlsDemoWindow demo;
    if (!demo.Initialize(hInstance, nCmdShow)) {
        CoUninitialize();
        return 1;
    }

    int result = demo.Run();
    CoUninitialize();
    return result;
}
