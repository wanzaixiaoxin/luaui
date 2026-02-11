// Composite Layout Demo - 使用新SOLID架构
// 展示如何将布局容器与普通控件结合使用

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

// 复合布局演示窗口
class CompositeLayoutDemo {
public:
    CompositeLayoutDemo() = default;
    ~CompositeLayoutDemo() { Cleanup(); }
    
    bool Initialize(HINSTANCE hInstance, int nCmdShow);
    int Run();
    
private:
    void CreateUI();
    void UpdateStatus(const std::wstring& message);
    void Render();
    void Cleanup();
    
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    
private:
    HWND m_hWnd = nullptr;
    IRenderEnginePtr m_engine;
    std::shared_ptr<StackPanel> m_rootPanel;
    std::shared_ptr<TextBlock> m_statusText;
    std::shared_ptr<ProgressBar> m_progressBar;
};

bool CompositeLayoutDemo::Initialize(HINSTANCE hInstance, int nCmdShow) {
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = L"CompositeLayoutDemoNew";
    
    if (!RegisterClassExW(&wcex)) return false;
    
    m_hWnd = CreateWindowExW(
        0, L"CompositeLayoutDemoNew", L"Composite Layout Demo - New Architecture",
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

void CompositeLayoutDemo::CreateUI() {
    // 根面板 - 垂直布局
    m_rootPanel = std::make_shared<StackPanel>();
    m_rootPanel->SetName("Root");
    m_rootPanel->SetOrientation(StackPanel::Orientation::Vertical);
    
    // 标题
    auto title = std::make_shared<TextBlock>();
    title->SetText(L"Composite Layout Demo - New Architecture");
    title->SetFontSize(24);
    m_rootPanel->AddChild(title);
    
    // 按钮区域 - 水平布局
    auto buttonPanel = std::make_shared<StackPanel>();
    buttonPanel->SetOrientation(StackPanel::Orientation::Horizontal);
    buttonPanel->SetSpacing(10);
    
    auto btn1 = std::make_shared<Button>();
    btn1->Click.Add([this](Control*) {
        UpdateStatus(L"Button 1 clicked");
    });
    buttonPanel->AddChild(btn1);
    
    auto btn2 = std::make_shared<Button>();
    btn2->SetStateColors(
        Color::FromHex(0x4CAF50),
        Color::FromHex(0x45A049),
        Color::FromHex(0x3D8B40)
    );
    btn2->Click.Add([this](Control*) {
        UpdateStatus(L"Button 2 clicked");
    });
    buttonPanel->AddChild(btn2);
    
    auto btn3 = std::make_shared<Button>();
    btn3->Click.Add([this](Control*) {
        UpdateStatus(L"Button 3 clicked");
    });
    buttonPanel->AddChild(btn3);
    
    m_rootPanel->AddChild(buttonPanel);
    
    // 滑块和进度条区域
    auto sliderPanel = std::make_shared<StackPanel>();
    sliderPanel->SetSpacing(10);
    
    auto slider = std::make_shared<Slider>();
    slider->SetValue(50);
    slider->ValueChanged.Add([this](Slider*, double value) {
        if (m_progressBar) {
            m_progressBar->SetValue(value);
        }
        std::wstring msg = L"Slider value: " + std::to_wstring(static_cast<int>(value));
        UpdateStatus(msg);
    });
    sliderPanel->AddChild(slider);
    
    m_progressBar = std::make_shared<ProgressBar>();
    m_progressBar->SetValue(50);
    sliderPanel->AddChild(m_progressBar);
    
    m_rootPanel->AddChild(sliderPanel);
    
    // 复选框区域
    auto checkPanel = std::make_shared<StackPanel>();
    checkPanel->SetOrientation(StackPanel::Orientation::Horizontal);
    checkPanel->SetSpacing(20);
    
    auto cb1 = std::make_shared<CheckBox>();
    cb1->SetText(L"Option A");
    cb1->CheckedChanged.Add([this](CheckBox*, bool checked) {
        UpdateStatus(checked ? L"Option A enabled" : L"Option A disabled");
    });
    checkPanel->AddChild(cb1);
    
    auto cb2 = std::make_shared<CheckBox>();
    cb2->SetText(L"Option B");
    cb2->CheckedChanged.Add([this](CheckBox*, bool checked) {
        UpdateStatus(checked ? L"Option B enabled" : L"Option B disabled");
    });
    checkPanel->AddChild(cb2);
    
    m_rootPanel->AddChild(checkPanel);
    
    // 文本框
    auto textBox = std::make_shared<TextBox>();
    textBox->SetPlaceholder(L"Enter text here...");
    textBox->TextChanged.Add([this](TextBox*, const std::wstring& text) {
        if (!text.empty()) {
            UpdateStatus(L"Text: " + text);
        }
    });
    m_rootPanel->AddChild(textBox);
    
    // 列表框
    auto listBox = std::make_shared<ListBox>();
    listBox->AddItem(L"Item 1");
    listBox->AddItem(L"Item 2");
    listBox->AddItem(L"Item 3");
    listBox->AddItem(L"Item 4");
    listBox->SelectionChanged.Add([this](ListBox*, int index) {
        std::wstring msg = L"Selected item: " + std::to_wstring(index);
        UpdateStatus(msg);
    });
    m_rootPanel->AddChild(listBox);
    
    // 状态栏
    m_statusText = std::make_shared<TextBlock>();
    m_statusText->SetText(L"Ready");
    m_statusText->SetFontSize(12);
    m_rootPanel->AddChild(m_statusText);
}

void CompositeLayoutDemo::UpdateStatus(const std::wstring& message) {
    if (m_statusText) {
        m_statusText->SetText(message);
    }
    OutputDebugStringW((message + L"\n").c_str());
}

void CompositeLayoutDemo::Render() {
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

void CompositeLayoutDemo::Cleanup() {
    m_rootPanel = nullptr;
    if (m_engine) {
        m_engine->Shutdown();
        m_engine = nullptr;
    }
}

LRESULT CALLBACK CompositeLayoutDemo::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    CompositeLayoutDemo* pThis = nullptr;
    
    if (message == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<CompositeLayoutDemo*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        pThis->m_hWnd = hWnd;
    } else {
        pThis = reinterpret_cast<CompositeLayoutDemo*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
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

int CompositeLayoutDemo::Run() {
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
    
    CompositeLayoutDemo demo;
    if (!demo.Initialize(hInstance, nCmdShow)) {
        CoUninitialize();
        return 1;
    }
    
    int result = demo.Run();
    CoUninitialize();
    return result;
}
