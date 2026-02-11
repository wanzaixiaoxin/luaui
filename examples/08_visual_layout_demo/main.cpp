// Visual Layout Demo - 使用新SOLID架构
// 展示各种布局效果

#include "Controls.h"
#include "IRenderContext.h"
#include "IRenderEngine.h"
#include <windows.h>
#include <windowsx.h>
#include <objbase.h>
#include <string>
#include <vector>
#include <memory>

using namespace luaui;
using namespace luaui::controls;
using namespace luaui::rendering;

// 简化的彩色矩形控件
class ColorRect : public ControlBase {
public:
    ColorRect(const Color& fillColor, const Color& strokeColor = Color::Transparent())
        : m_fillColor(fillColor), m_strokeColor(strokeColor) {
        SetName("ColorRect");
        InitializeComponents();
    }
    
    std::string GetTypeName() const override { return "ColorRect"; }
    
    void SetFillColor(const Color& c) { m_fillColor = c; }
    void SetStrokeColor(const Color& c) { m_strokeColor = c; }
    void SetStrokeThickness(float t) { m_strokeThickness = t; }

protected:
    void InitializeComponents() override {
        ControlBase::InitializeComponents();
        if (auto* layout = GetLayout()) {
            layout->SetWidth(80);
            layout->SetHeight(60);
        }
    }

private:
    Color m_fillColor;
    Color m_strokeColor;
    float m_strokeThickness = 1.0f;
};

// 演示窗口
class VisualLayoutDemo {
public:
    VisualLayoutDemo() = default;
    ~VisualLayoutDemo() { Cleanup(); }
    
    bool Initialize(HINSTANCE hInstance, int nCmdShow);
    int Run();
    
private:
    void CreateDemo_VerticalStack();
    void CreateDemo_HorizontalStack();
    void CreateDemo_NestedStack();
    void CreateDemo_Shapes();
    void SwitchDemo(int demoIndex);
    void Render();
    void Cleanup();
    
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    
private:
    HWND m_hWnd = nullptr;
    IRenderEnginePtr m_engine;
    std::shared_ptr<StackPanel> m_rootPanel;
    int m_currentDemoIndex = 0;
    const wchar_t* m_demoNames[4] = {
        L"Vertical StackPanel",
        L"Horizontal StackPanel", 
        L"Nested StackPanels",
        L"Shapes"
    };
};

bool VisualLayoutDemo::Initialize(HINSTANCE hInstance, int nCmdShow) {
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = L"VisualLayoutDemoNew";
    
    if (!RegisterClassExW(&wcex)) return false;
    
    m_hWnd = CreateWindowExW(
        0, L"VisualLayoutDemoNew", L"Visual Layout Demo - New Architecture [Press 1-4 to switch]",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1024, 768,
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
    desc.width = 1024;
    desc.height = 768;
    
    if (!m_engine->CreateRenderTarget(desc)) {
        MessageBoxW(m_hWnd, L"Failed to create render target", L"Error", MB_OK);
        return false;
    }
    
    // 创建根面板
    m_rootPanel = std::make_shared<StackPanel>();
    m_rootPanel->SetName("Root");
    m_rootPanel->SetOrientation(StackPanel::Orientation::Vertical);
    
    // 加载初始演示
    SwitchDemo(0);
    
    ShowWindow(m_hWnd, nCmdShow);
    UpdateWindow(m_hWnd);
    return true;
}

void VisualLayoutDemo::CreateDemo_VerticalStack() {
    m_rootPanel->ClearChildren();
    
    auto panel = std::make_shared<StackPanel>();
    panel->SetName("VerticalStackDemo");
    panel->SetOrientation(StackPanel::Orientation::Vertical);
    panel->SetSpacing(10);
    
    // 添加彩色矩形
    Color colors[] = {
        Color::FromHex(0xFF6B6B),
        Color::FromHex(0x4ECDC4),
        Color::FromHex(0x45B7D1),
        Color::FromHex(0x96CEB4),
        Color::FromHex(0xFECA57),
    };
    
    for (int i = 0; i < 5; i++) {
        auto rect = std::make_shared<ColorRect>(colors[i]);
        if (auto* layout = rect->GetLayout()) {
            layout->SetWidth(200);
            layout->SetHeight(60);
        }
        panel->AddChild(rect);
    }
    
    m_rootPanel->AddChild(panel);
}

void VisualLayoutDemo::CreateDemo_HorizontalStack() {
    m_rootPanel->ClearChildren();
    
    auto panel = std::make_shared<StackPanel>();
    panel->SetName("HorizontalStackDemo");
    panel->SetOrientation(StackPanel::Orientation::Horizontal);
    panel->SetSpacing(15);
    
    Color colors[] = {
        Color::FromHex(0xFF6B6B),
        Color::FromHex(0x4ECDC4),
        Color::FromHex(0x45B7D1),
        Color::FromHex(0x96CEB4),
        Color::FromHex(0xFECA57),
    };
    
    for (int i = 0; i < 5; i++) {
        auto rect = std::make_shared<ColorRect>(colors[i]);
        if (auto* layout = rect->GetLayout()) {
            layout->SetWidth(100);
            layout->SetHeight(150);
        }
        panel->AddChild(rect);
    }
    
    m_rootPanel->AddChild(panel);
}

void VisualLayoutDemo::CreateDemo_NestedStack() {
    m_rootPanel->ClearChildren();
    
    auto outerPanel = std::make_shared<StackPanel>();
    outerPanel->SetName("NestedDemo");
    outerPanel->SetOrientation(StackPanel::Orientation::Vertical);
    outerPanel->SetSpacing(10);
    
    // Header
    auto header = std::make_shared<ColorRect>(Color::FromHex(0x333333));
    if (auto* layout = header->GetLayout()) {
        layout->SetWidth(600);
        layout->SetHeight(40);
    }
    outerPanel->AddChild(header);
    
    // Content - Horizontal panel
    auto contentPanel = std::make_shared<StackPanel>();
    contentPanel->SetName("Content");
    contentPanel->SetOrientation(StackPanel::Orientation::Horizontal);
    contentPanel->SetSpacing(5);
    
    auto left = std::make_shared<ColorRect>(Color::FromHex(0xFF6B6B));
    if (auto* layout = left->GetLayout()) {
        layout->SetWidth(150);
        layout->SetHeight(300);
    }
    contentPanel->AddChild(left);
    
    auto center = std::make_shared<ColorRect>(Color::FromHex(0x4ECDC4));
    if (auto* layout = center->GetLayout()) {
        layout->SetWidth(300);
        layout->SetHeight(300);
    }
    contentPanel->AddChild(center);
    
    auto right = std::make_shared<ColorRect>(Color::FromHex(0x45B7D1));
    if (auto* layout = right->GetLayout()) {
        layout->SetWidth(150);
        layout->SetHeight(300);
    }
    contentPanel->AddChild(right);
    
    outerPanel->AddChild(contentPanel);
    
    // Footer
    auto footer = std::make_shared<ColorRect>(Color::FromHex(0x666666));
    if (auto* layout = footer->GetLayout()) {
        layout->SetWidth(600);
        layout->SetHeight(40);
    }
    outerPanel->AddChild(footer);
    
    m_rootPanel->AddChild(outerPanel);
}

void VisualLayoutDemo::CreateDemo_Shapes() {
    m_rootPanel->ClearChildren();
    
    auto panel = std::make_shared<StackPanel>();
    panel->SetName("ShapesDemo");
    panel->SetOrientation(StackPanel::Orientation::Horizontal);
    panel->SetSpacing(20);
    
    // Rectangle
    auto rect = std::make_shared<luaui::controls::Rectangle>();
    if (auto* layout = rect->GetLayout()) {
        layout->SetWidth(100);
        layout->SetHeight(80);
    }
    rect->SetFill(Color::FromHex(0x4CAF50));
    rect->SetStroke(Color::FromHex(0x2E7D32));
    rect->SetStrokeThickness(2);
    panel->AddChild(rect);
    
    // Rounded Rectangle
    auto roundRect = std::make_shared<luaui::controls::Rectangle>();
    if (auto* layout = roundRect->GetLayout()) {
        layout->SetWidth(100);
        layout->SetHeight(80);
    }
    roundRect->SetFill(Color::FromHex(0x2196F3));
    roundRect->SetStroke(Color::FromHex(0x1565C0));
    roundRect->SetStrokeThickness(2);
    roundRect->SetRadiusX(10);
    roundRect->SetRadiusY(10);
    panel->AddChild(roundRect);
    
    // Ellipse
    auto ellipse = std::make_shared<luaui::controls::Ellipse>();
    if (auto* layout = ellipse->GetLayout()) {
        layout->SetWidth(100);
        layout->SetHeight(80);
    }
    ellipse->SetFill(Color::FromHex(0xFF5722));
    ellipse->SetStroke(Color::FromHex(0xD84315));
    ellipse->SetStrokeThickness(2);
    panel->AddChild(ellipse);
    
    m_rootPanel->AddChild(panel);
}

void VisualLayoutDemo::SwitchDemo(int demoIndex) {
    m_currentDemoIndex = demoIndex % 4;
    
    switch (m_currentDemoIndex) {
        case 0: CreateDemo_VerticalStack(); break;
        case 1: CreateDemo_HorizontalStack(); break;
        case 2: CreateDemo_NestedStack(); break;
        case 3: CreateDemo_Shapes(); break;
    }
    
    std::wstring title = std::wstring(L"Visual Layout Demo - ") + m_demoNames[m_currentDemoIndex] + L" [Press 1-4 to switch]";
    SetWindowTextW(m_hWnd, title.c_str());
    
    InvalidateRect(m_hWnd, nullptr, FALSE);
}

void VisualLayoutDemo::Render() {
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

void VisualLayoutDemo::Cleanup() {
    m_rootPanel = nullptr;
    if (m_engine) {
        m_engine->Shutdown();
        m_engine = nullptr;
    }
}

LRESULT CALLBACK VisualLayoutDemo::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    VisualLayoutDemo* pThis = nullptr;
    
    if (message == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<VisualLayoutDemo*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        pThis->m_hWnd = hWnd;
    } else {
        pThis = reinterpret_cast<VisualLayoutDemo*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
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
            case WM_KEYDOWN: {
                int key = static_cast<int>(wParam);
                if (key >= '1' && key <= '4') {
                    pThis->SwitchDemo(key - '1');
                }
                return 0;
            }
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
        }
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

int VisualLayoutDemo::Run() {
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
    
    VisualLayoutDemo demo;
    if (!demo.Initialize(hInstance, nCmdShow)) {
        CoUninitialize();
        return 1;
    }
    
    int result = demo.Run();
    CoUninitialize();
    return result;
}
