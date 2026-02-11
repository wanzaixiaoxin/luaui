// LuaUI Controls Demo - 使用新架构API
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
#include <functional>

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
        
        // 设置根面板背景色
        if (auto* render = m_rootPanel->GetRender()) {
            render->SetBackground(Color::FromRGBA(245, 245, 245, 255));
        }
        
        // ========== 标题区域 ==========
        auto title = std::make_shared<TextBlock>();
        title->SetText(L"LuaUI Controls Demo - New Architecture");
        title->SetFontSize(24);
        title->SetForeground(Color::FromHex(0x333333));
        m_rootPanel->AddChild(title);
        
        auto subtitle = std::make_shared<TextBlock>();
        subtitle->SetText(L"Using Architecture with Component Pattern");
        subtitle->SetFontSize(14);
        subtitle->SetForeground(Color::FromHex(0x666666));
        m_rootPanel->AddChild(subtitle);
        
        // ========== 按钮区域 ==========
        auto buttonLabel = std::make_shared<TextBlock>();
        buttonLabel->SetText(L"Buttons:");
        buttonLabel->SetFontSize(12);
        buttonLabel->SetForeground(Color::FromHex(0x999999));
        m_rootPanel->AddChild(buttonLabel);
        
        auto buttonPanel = std::make_shared<StackPanel>();
        buttonPanel->SetOrientation(StackPanel::Orientation::Horizontal);
        buttonPanel->SetSpacing(10);
        
        // 默认按钮
        auto btn1 = std::make_shared<Button>();
        btn1->SetStateColors(
            Color::FromHex(0x2196F3),  // Normal
            Color::FromHex(0x1976D2),  // Hover
            Color::FromHex(0x0D47A1)   // Pressed
        );
        buttonPanel->AddChild(btn1);
        
        // 成功按钮
        auto btn2 = std::make_shared<Button>();
        btn2->SetStateColors(
            Color::FromHex(0x4CAF50),  // Normal
            Color::FromHex(0x388E3C),  // Hover
            Color::FromHex(0x1B5E20)   // Pressed
        );
        buttonPanel->AddChild(btn2);
        
        m_rootPanel->AddChild(buttonPanel);
        
        // ========== 输入区域 ==========
        auto inputLabel = std::make_shared<TextBlock>();
        inputLabel->SetText(L"Input Controls:");
        inputLabel->SetFontSize(12);
        inputLabel->SetForeground(Color::FromHex(0x999999));
        m_rootPanel->AddChild(inputLabel);
        
        // 文本框
        auto textBox = std::make_shared<TextBox>();
        m_rootPanel->AddChild(textBox);
        
        // 复选框
        auto checkBox = std::make_shared<CheckBox>();
        m_rootPanel->AddChild(checkBox);
        
        // ========== 滑块和进度条 ==========
        auto valueLabel = std::make_shared<TextBlock>();
        valueLabel->SetText(L"Value Controls:");
        valueLabel->SetFontSize(12);
        valueLabel->SetForeground(Color::FromHex(0x999999));
        m_rootPanel->AddChild(valueLabel);
        
        // 滑块
        auto slider = std::make_shared<Slider>();
        slider->SetValue(50);
        m_rootPanel->AddChild(slider);
        
        // 进度条
        m_progressBar = std::make_shared<ProgressBar>();
        m_progressBar->SetValue(50);
        m_rootPanel->AddChild(m_progressBar);
        
        // ========== Border 容器示例 ==========
        auto borderLabel = std::make_shared<TextBlock>();
        borderLabel->SetText(L"Border Container:");
        borderLabel->SetFontSize(12);
        borderLabel->SetForeground(Color::FromHex(0x999999));
        m_rootPanel->AddChild(borderLabel);
        
        auto border = std::make_shared<Border>();
        border->SetBackground(Color::White());
        border->SetBorderColor(Color::FromHex(0xCCCCCC));
        border->SetBorderThickness(2);
        if (auto* layout = border->GetLayout()) {
            layout->SetHeight(80);
        }
        
        // 在 Border 内添加内容
        auto borderContent = std::make_shared<TextBlock>();
        borderContent->SetText(L"Content inside Border");
        borderContent->SetFontSize(14);
        border->AddChild(borderContent);
        
        m_rootPanel->AddChild(border);
        
        // ========== 形状展示 ==========
        auto shapesLabel = std::make_shared<TextBlock>();
        shapesLabel->SetText(L"Shapes:");
        shapesLabel->SetFontSize(12);
        shapesLabel->SetForeground(Color::FromHex(0x999999));
        m_rootPanel->AddChild(shapesLabel);
        
        auto shapesPanel = std::make_shared<StackPanel>();
        shapesPanel->SetOrientation(StackPanel::Orientation::Horizontal);
        shapesPanel->SetSpacing(10);
        if (auto* layout = shapesPanel->GetLayout()) {
            layout->SetHeight(70);
        }
        
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
        
        // ========== 状态栏 ==========
        m_statusText = std::make_shared<TextBlock>();
        m_statusText->SetText(L"Ready - New Architecture Demo");
        m_statusText->SetFontSize(12);
        m_statusText->SetForeground(Color::FromHex(0x666666));
        m_rootPanel->AddChild(m_statusText);
    }

    void Render() {
        static int frameCount = 0;
        frameCount++;
        std::cout << "[Frame " << frameCount << "] Render started" << std::endl;
        
        if (!m_engine->BeginFrame()) {
            std::cout << "[Frame " << frameCount << "] BeginFrame failed" << std::endl;
            return;
        }

        auto* context = m_engine->GetContext();
        if (!context) {
            std::cout << "[Frame " << frameCount << "] Context is null" << std::endl;
            m_engine->Present();
            return;
        }

        // 清空背景
        context->Clear(Color::White());
        std::cout << "[Frame " << frameCount << "] Background cleared" << std::endl;

        // 测量和排列
        RECT rc;
        GetClientRect(m_hWnd, &rc);
        float width = static_cast<float>(rc.right - rc.left);
        float height = static_cast<float>(rc.bottom - rc.top);
        std::cout << "[Frame " << frameCount << "] Window size: " << width << "x" << height << std::endl;

        // 测量和排列 - 使用控件的布局系统
        interfaces::LayoutConstraint constraint;
        constraint.available = Size(width - 20, height - 20);
        
        std::cout << "[Frame " << frameCount << "] Starting measure and arrange..." << std::endl;
        
        // 只需要测量和排列根控件，Panel 会自动处理子控件
        if (auto* layoutable = m_rootPanel->AsLayoutable()) {
            layoutable->InvalidateMeasure();
            auto measured = layoutable->Measure(constraint);
            std::cout << "  [Measure] Root measured: " << measured.width << "x" << measured.height << std::endl;
            layoutable->Arrange(Rect(10, 10, width - 20, height - 20));
        }
        std::cout << "[Frame " << frameCount << "] Measure and arrange completed" << std::endl;
        
        // 渲染
        std::cout << "[Frame " << frameCount << "] Starting render..." << std::endl;
        // 检查 GetRender
        auto* renderComp = m_rootPanel->GetRender();
        std::cout << "[Frame " << frameCount << "] GetRender: " << (renderComp ? "not null" : "null") << std::endl;
        
        // 检查 AsRenderable
        auto* renderable = m_rootPanel->AsRenderable();
        std::cout << "[Frame " << frameCount << "] AsRenderable: " << (renderable ? "not null" : "null") << std::endl;
        
        if (renderable) {
            std::cout << "[Frame " << frameCount << "] Root panel renderable found" << std::endl;
            renderable->Render(context);
            std::cout << "[Frame " << frameCount << "] Root panel render completed" << std::endl;
        } else {
            std::cout << "[Frame " << frameCount << "] Root panel renderable NOT found" << std::endl;
        }

        m_engine->Present();
        std::cout << "[Frame " << frameCount << "] Present completed" << std::endl;
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

int main() {
    std::cout << "=== LuaUI Controls Demo Starting ===" << std::endl;
    
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize COM" << std::endl;
        return 1;
    }

    HINSTANCE hInstance = GetModuleHandle(nullptr);
    int nCmdShow = SW_SHOWDEFAULT;

    ControlsDemoWindow demo;
    if (!demo.Initialize(hInstance, nCmdShow)) {
        std::cerr << "Failed to initialize demo window" << std::endl;
        CoUninitialize();
        return 1;
    }

    int result = demo.Run();
    CoUninitialize();
    return result;
}
