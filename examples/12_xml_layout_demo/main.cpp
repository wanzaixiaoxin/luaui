// XML Layout Demo - 使用新SOLID架构
// 展示XML布局解析（简化版本）

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

// XML布局演示窗口
class XmlLayoutDemo {
public:
    XmlLayoutDemo() = default;
    ~XmlLayoutDemo() { Cleanup(); }
    
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

bool XmlLayoutDemo::Initialize(HINSTANCE hInstance, int nCmdShow) {
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = L"XmlLayoutDemoNew";
    
    if (!RegisterClassExW(&wcex)) return false;
    
    m_hWnd = CreateWindowExW(
        0, L"XmlLayoutDemoNew", L"XML Layout Demo - New Architecture",
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

void XmlLayoutDemo::CreateUI() {
    m_rootPanel = std::make_shared<StackPanel>();
    m_rootPanel->SetName("Root");
    m_rootPanel->SetOrientation(StackPanel::Orientation::Vertical);
    
    // 标题
    auto title = std::make_shared<TextBlock>();
    title->SetText(L"XML Layout Demo - New Architecture");
    title->SetFontSize(20);
    m_rootPanel->AddChild(title);
    
    // 说明
    auto desc = std::make_shared<TextBlock>();
    desc->SetText(L"This demo shows programmatic UI creation using the new API");
    desc->SetFontSize(12);
    m_rootPanel->AddChild(desc);
    
    // 按钮区域
    auto btnPanel = std::make_shared<StackPanel>();
    btnPanel->SetOrientation(StackPanel::Orientation::Horizontal);
    btnPanel->SetSpacing(10);
    
    auto btn1 = std::make_shared<Button>();
    btnPanel->AddChild(btn1);
    
    auto btn2 = std::make_shared<Button>();
    btn2->SetStateColors(
        Color::FromHex(0x4CAF50),
        Color::FromHex(0x45A049),
        Color::FromHex(0x3D8B40)
    );
    btnPanel->AddChild(btn2);
    
    m_rootPanel->AddChild(btnPanel);
    
    // 滑块
    auto slider = std::make_shared<Slider>();
    slider->SetValue(50);
    m_rootPanel->AddChild(slider);
    
    // 复选框
    auto checkBox = std::make_shared<CheckBox>();
    checkBox->SetText(L"Enable Option");
    m_rootPanel->AddChild(checkBox);
    
    // 文本框
    auto textBox = std::make_shared<TextBox>();
    textBox->SetPlaceholder(L"Enter text...");
    m_rootPanel->AddChild(textBox);
    
    // 列表框
    auto listBox = std::make_shared<ListBox>();
    listBox->AddItem(L"Item 1");
    listBox->AddItem(L"Item 2");
    listBox->AddItem(L"Item 3");
    m_rootPanel->AddChild(listBox);
    
    // 形状
    auto shapePanel = std::make_shared<StackPanel>();
    shapePanel->SetOrientation(StackPanel::Orientation::Horizontal);
    shapePanel->SetSpacing(10);
    
    auto rect = std::make_shared<luaui::controls::Rectangle>();
    if (auto* layout = rect->GetLayout()) {
        layout->SetWidth(60);
        layout->SetHeight(60);
    }
    rect->SetFill(Color::FromHex(0x4CAF50));
    shapePanel->AddChild(rect);
    
    auto ellipse = std::make_shared<luaui::controls::Ellipse>();
    if (auto* layout = ellipse->GetLayout()) {
        layout->SetWidth(60);
        layout->SetHeight(60);
    }
    ellipse->SetFill(Color::FromHex(0x2196F3));
    shapePanel->AddChild(ellipse);
    
    m_rootPanel->AddChild(shapePanel);
}

void XmlLayoutDemo::Render() {
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

void XmlLayoutDemo::Cleanup() {
    m_rootPanel = nullptr;
    if (m_engine) {
        m_engine->Shutdown();
        m_engine = nullptr;
    }
}

LRESULT CALLBACK XmlLayoutDemo::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    XmlLayoutDemo* pThis = nullptr;
    
    if (message == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<XmlLayoutDemo*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        pThis->m_hWnd = hWnd;
    } else {
        pThis = reinterpret_cast<XmlLayoutDemo*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
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

int XmlLayoutDemo::Run() {
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
    
    XmlLayoutDemo demo;
    if (!demo.Initialize(hInstance, nCmdShow)) {
        CoUninitialize();
        return 1;
    }
    
    int result = demo.Run();
    CoUninitialize();
    return result;
}
