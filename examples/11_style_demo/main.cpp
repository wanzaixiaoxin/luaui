// Style Demo - 使用新SOLID架构
// 展示控件样式和主题

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

// 样式演示窗口
class StyleDemo {
public:
    StyleDemo() = default;
    ~StyleDemo() { Cleanup(); }
    
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
};

bool StyleDemo::Initialize(HINSTANCE hInstance, int nCmdShow) {
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = L"StyleDemoNew";
    
    if (!RegisterClassExW(&wcex)) return false;
    
    m_hWnd = CreateWindowExW(
        0, L"StyleDemoNew", L"Style Demo - New Architecture",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
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
    desc.width = 800;
    desc.height = 600;
    
    if (!m_engine->CreateRenderTarget(desc)) {
        MessageBoxW(m_hWnd, L"Failed to create render target", L"Error", MB_OK);
        return false;
    }
    
    CreateUI();
    
    ShowWindow(m_hWnd, nCmdShow);
    UpdateWindow(m_hWnd);
    return true;
}

void StyleDemo::CreateUI() {
    m_rootPanel = std::make_shared<StackPanel>();
    m_rootPanel->SetName("Root");
    m_rootPanel->SetOrientation(StackPanel::Orientation::Vertical);
    
    // 标题
    auto title = std::make_shared<TextBlock>();
    title->SetText(L"Style Demo - New Architecture");
    title->SetFontSize(24);
    m_rootPanel->AddChild(title);
    
    // 不同颜色的按钮
    auto btnPanel = std::make_shared<StackPanel>();
    btnPanel->SetOrientation(StackPanel::Orientation::Horizontal);
    btnPanel->SetSpacing(10);
    
    // 默认按钮
    auto btn1 = std::make_shared<Button>();
    btnPanel->AddChild(btn1);
    
    // 蓝色按钮
    auto btn2 = std::make_shared<Button>();
    btn2->SetStateColors(
        Color::FromHex(0x2196F3),
        Color::FromHex(0x1976D2),
        Color::FromHex(0x0D47A1)
    );
    btnPanel->AddChild(btn2);
    
    // 绿色按钮
    auto btn3 = std::make_shared<Button>();
    btn3->SetStateColors(
        Color::FromHex(0x4CAF50),
        Color::FromHex(0x388E3C),
        Color::FromHex(0x1B5E20)
    );
    btnPanel->AddChild(btn3);
    
    // 红色按钮
    auto btn4 = std::make_shared<Button>();
    btn4->SetStateColors(
        Color::FromHex(0xF44336),
        Color::FromHex(0xD32F2F),
        Color::FromHex(0xB71C1C)
    );
    btnPanel->AddChild(btn4);
    
    m_rootPanel->AddChild(btnPanel);
    
    // 形状样式
    auto shapePanel = std::make_shared<StackPanel>();
    shapePanel->SetOrientation(StackPanel::Orientation::Horizontal);
    shapePanel->SetSpacing(15);
    
    auto rect1 = std::make_shared<luaui::controls::Rectangle>();
    if (auto* layout = rect1->GetLayout()) {
        layout->SetWidth(80);
        layout->SetHeight(80);
    }
    rect1->SetFill(Color::FromHex(0xFF6B6B));
    rect1->SetStroke(Color::FromHex(0x333333));
    rect1->SetStrokeThickness(3);
    shapePanel->AddChild(rect1);
    
    auto rect2 = std::make_shared<luaui::controls::Rectangle>();
    if (auto* layout = rect2->GetLayout()) {
        layout->SetWidth(80);
        layout->SetHeight(80);
    }
    rect2->SetFill(Color::FromHex(0x4ECDC4));
    rect2->SetRadiusX(15);
    rect2->SetRadiusY(15);
    shapePanel->AddChild(rect2);
    
    auto ellipse = std::make_shared<luaui::controls::Ellipse>();
    if (auto* layout = ellipse->GetLayout()) {
        layout->SetWidth(80);
        layout->SetHeight(80);
    }
    ellipse->SetFill(Color::FromHex(0x45B7D1));
    ellipse->SetStroke(Color::FromHex(0xFFFFFF));
    ellipse->SetStrokeThickness(4);
    shapePanel->AddChild(ellipse);
    
    m_rootPanel->AddChild(shapePanel);
    
    // 文本样式
    auto textPanel = std::make_shared<StackPanel>();
    textPanel->SetSpacing(5);
    
    auto text1 = std::make_shared<TextBlock>();
    text1->SetText(L"Small Text (12pt)");
    text1->SetFontSize(12);
    textPanel->AddChild(text1);
    
    auto text2 = std::make_shared<TextBlock>();
    text2->SetText(L"Normal Text (16pt)");
    text2->SetFontSize(16);
    textPanel->AddChild(text2);
    
    auto text3 = std::make_shared<TextBlock>();
    text3->SetText(L"Large Text (20pt)");
    text3->SetFontSize(20);
    textPanel->AddChild(text3);
    
    m_rootPanel->AddChild(textPanel);
}

void StyleDemo::Render() {
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

void StyleDemo::Cleanup() {
    m_rootPanel = nullptr;
    if (m_engine) {
        m_engine->Shutdown();
        m_engine = nullptr;
    }
}

LRESULT CALLBACK StyleDemo::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    StyleDemo* pThis = nullptr;
    
    if (message == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<StyleDemo*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        pThis->m_hWnd = hWnd;
    } else {
        pThis = reinterpret_cast<StyleDemo*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
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

int StyleDemo::Run() {
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
    
    StyleDemo demo;
    if (!demo.Initialize(hInstance, nCmdShow)) {
        CoUninitialize();
        return 1;
    }
    
    int result = demo.Run();
    CoUninitialize();
    return result;
}
