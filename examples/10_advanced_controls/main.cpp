// Advanced Controls Demo - 使用新SOLID架构
// 展示高级控件功能

#include "Controls.h"
#include "IRenderContext.h"
#include "IRenderEngine.h"
#include <windows.h>
#include <windowsx.h>
#include <objbase.h>
#include <string>
#include <memory>

using namespace luaui;
using namespace luaui::controls;
using namespace luaui::rendering;

// 高级控件演示窗口
class AdvancedControlsDemo {
public:
    AdvancedControlsDemo() = default;
    ~AdvancedControlsDemo() { Cleanup(); }
    
    bool Initialize(HINSTANCE hInstance, int nCmdShow);
    int Run();
    
private:
    void CreateUI();
    void Render();
    void Cleanup();
    
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    
private:
    HWND m_hWnd = nullptr;
    IRenderEnginePtr m_engine;
    std::shared_ptr<StackPanel> m_rootPanel;
    std::shared_ptr<TextBlock> m_statusText;
    std::shared_ptr<Slider> m_colorSlider;
    std::shared_ptr<luaui::controls::Rectangle> m_colorRect;
};

bool AdvancedControlsDemo::Initialize(HINSTANCE hInstance, int nCmdShow) {
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = L"AdvancedControlsDemoNew";
    
    if (!RegisterClassExW(&wcex)) return false;
    
    m_hWnd = CreateWindowExW(
        0, L"AdvancedControlsDemoNew", L"Advanced Controls Demo - New Architecture",
        WS_OVERLAPPEDWINDOW,
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
    
    CreateUI();
    
    ShowWindow(m_hWnd, nCmdShow);
    UpdateWindow(m_hWnd);
    return true;
}

void AdvancedControlsDemo::CreateUI() {
    m_rootPanel = std::make_shared<StackPanel>();
    m_rootPanel->SetName("Root");
    m_rootPanel->SetOrientation(StackPanel::Orientation::Vertical);
    
    // 标题
    auto title = std::make_shared<TextBlock>();
    title->SetText(L"Advanced Controls Demo");
    title->SetFontSize(24);
    m_rootPanel->AddChild(title);
    
    // 说明
    auto desc = std::make_shared<TextBlock>();
    desc->SetText(L"Shape color control demo");
    desc->SetFontSize(14);
    m_rootPanel->AddChild(desc);
    
    // 颜色控制滑块
    auto sliderLabel = std::make_shared<TextBlock>();
    sliderLabel->SetText(L"Color Control");
    sliderLabel->SetFontSize(16);
    m_rootPanel->AddChild(sliderLabel);
    
    m_colorSlider = std::make_shared<Slider>();
    m_colorSlider->SetValue(0);
    m_colorSlider->ValueChanged.Add([this](Slider*, double value) {
        // 根据滑块值改变颜色
        float hue = static_cast<float>(value) / 100.0f;
        // 简化的HSV到RGB转换
        float r = std::abs(hue * 6.0f - 3.0f) - 1.0f;
        float g = 2.0f - std::abs(hue * 6.0f - 2.0f);
        float b = 2.0f - std::abs(hue * 6.0f - 4.0f);
        r = std::clamp(r, 0.0f, 1.0f);
        g = std::clamp(g, 0.0f, 1.0f);
        b = std::clamp(b, 0.0f, 1.0f);
        
        if (m_colorRect) {
            m_colorRect->SetFill(Color(r, g, b));
        }
    });
    m_rootPanel->AddChild(m_colorSlider);
    
    // 显示颜色的矩形
    m_colorRect = std::make_shared<luaui::controls::Rectangle>();
    if (auto* layout = m_colorRect->GetLayout()) {
        layout->SetWidth(200);
        layout->SetHeight(150);
    }
    m_colorRect->SetFill(Color::FromHex(0xFF0000));
    m_colorRect->SetStroke(Color::Black());
    m_colorRect->SetStrokeThickness(2);
    m_rootPanel->AddChild(m_colorRect);
    
    // 形状选择
    auto shapePanel = std::make_shared<StackPanel>();
    shapePanel->SetOrientation(StackPanel::Orientation::Horizontal);
    shapePanel->SetSpacing(10);
    
    auto rectBtn = std::make_shared<Button>();
    rectBtn->Click.Add([this](ControlBase*) {
        if (m_colorRect) {
            m_colorRect->SetRadiusX(0);
            m_colorRect->SetRadiusY(0);
        }
    });
    shapePanel->AddChild(rectBtn);
    
    auto roundRectBtn = std::make_shared<Button>();
    roundRectBtn->Click.Add([this](ControlBase*) {
        if (m_colorRect) {
            m_colorRect->SetRadiusX(20);
            m_colorRect->SetRadiusY(20);
        }
    });
    shapePanel->AddChild(roundRectBtn);
    
    m_rootPanel->AddChild(shapePanel);
    
    // 各种形状展示
    auto shapesTitle = std::make_shared<TextBlock>();
    shapesTitle->SetText(L"Shapes Gallery");
    shapesTitle->SetFontSize(16);
    m_rootPanel->AddChild(shapesTitle);
    
    auto shapesPanel = std::make_shared<StackPanel>();
    shapesPanel->SetOrientation(StackPanel::Orientation::Horizontal);
    shapesPanel->SetSpacing(15);
    
    // 矩形
    auto rect1 = std::make_shared<luaui::controls::Rectangle>();
    if (auto* layout = rect1->GetLayout()) {
        layout->SetWidth(60);
        layout->SetHeight(60);
    }
    rect1->SetFill(Color::FromHex(0xFF6B6B));
    shapesPanel->AddChild(rect1);
    
    // 圆角矩形
    auto rect2 = std::make_shared<luaui::controls::Rectangle>();
    if (auto* layout = rect2->GetLayout()) {
        layout->SetWidth(60);
        layout->SetHeight(60);
    }
    rect2->SetFill(Color::FromHex(0x4ECDC4));
    rect2->SetRadiusX(15);
    rect2->SetRadiusY(15);
    shapesPanel->AddChild(rect2);
    
    // 椭圆
    auto ellipse = std::make_shared<luaui::controls::Ellipse>();
    if (auto* layout = ellipse->GetLayout()) {
        layout->SetWidth(60);
        layout->SetHeight(60);
    }
    ellipse->SetFill(Color::FromHex(0x45B7D1));
    shapesPanel->AddChild(ellipse);
    
    m_rootPanel->AddChild(shapesPanel);
    
    // 状态文本
    m_statusText = std::make_shared<TextBlock>();
    m_statusText->SetText(L"Ready");
    m_statusText->SetFontSize(12);
    m_rootPanel->AddChild(m_statusText);
}

void AdvancedControlsDemo::Render() {
    if (!m_engine->BeginFrame()) return;
    
    auto* context = m_engine->GetContext();
    if (!context) {
        m_engine->Present();
        return;
    }
    
    context->Clear(Color::White());
    
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
    
    if (auto* renderable = m_rootPanel->AsRenderable()) {
        renderable->Render(context);
    }
    
    m_engine->Present();
}

void AdvancedControlsDemo::Cleanup() {
    m_rootPanel = nullptr;
    if (m_engine) {
        m_engine->Shutdown();
        m_engine = nullptr;
    }
}

LRESULT CALLBACK AdvancedControlsDemo::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    AdvancedControlsDemo* pThis = nullptr;
    
    if (message == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<AdvancedControlsDemo*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        pThis->m_hWnd = hWnd;
    } else {
        pThis = reinterpret_cast<AdvancedControlsDemo*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
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

int AdvancedControlsDemo::Run() {
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        MessageBoxW(nullptr, L"Failed to initialize COM", L"Error", MB_OK);
        return 1;
    }
    
    AdvancedControlsDemo demo;
    if (!demo.Initialize(hInstance, nCmdShow)) {
        CoUninitialize();
        return 1;
    }
    
    int result = demo.Run();
    CoUninitialize();
    return result;
}
