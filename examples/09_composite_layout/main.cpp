// Composite Layout Demo - 布局与普通控件复合示例
// 展示如何将布局容器与普通控件结合使用

#include "Control.h"
#include "layout.h"
#include "Shapes.h"
#include "CheckableControls.h"
#include "RangeControls.h"
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
    void OnButtonClick(const std::wstring& buttonName);
    void OnCheckBoxChanged(CheckBox* sender, bool isChecked);
    void OnSliderChanged(Slider* sender, double value);
    void UpdateStatus(const std::wstring& message);
    void Render();
    void Cleanup();
    
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    
private:
    HWND m_hWnd = nullptr;
    IRenderEnginePtr m_engine;
    
    // UI 根元素
    std::shared_ptr<StackPanel> m_rootPanel;
    
    // 状态栏文本
    std::shared_ptr<Panel> m_statusBar;
    std::wstring m_statusMessage = L"Ready";
    
    // 进度条
    std::shared_ptr<ProgressBar> m_progressBar;
    
    // 输入框
    std::shared_ptr<Panel> m_contentPanel;
};

bool CompositeLayoutDemo::Initialize(HINSTANCE hInstance, int nCmdShow) {
    // 注册窗口类
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = L"CompositeLayoutDemo";
    
    if (!RegisterClassExW(&wcex)) return false;
    
    // 创建窗口
    m_hWnd = CreateWindowExW(
        0, L"CompositeLayoutDemo", L"Composite Layout Demo - Layout + Controls",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 900, 700,
        nullptr, nullptr, hInstance, this
    );
    
    if (!m_hWnd) return false;
    
    // 初始化渲染引擎
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
    
    // 创建 UI
    CreateUI();
    
    ShowWindow(m_hWnd, nCmdShow);
    UpdateWindow(m_hWnd);
    return true;
}

void CompositeLayoutDemo::CreateUI() {
    // ========================================
    // 根布局：垂直 StackPanel
    // ========================================
    m_rootPanel = std::make_shared<StackPanel>();
    m_rootPanel->SetName("Root");
    m_rootPanel->SetOrientation(StackPanel::Orientation::Vertical);
    m_rootPanel->SetBackground(Color::FromHex(0xF0F0F0));
    
    // ========================================
    // 1. 标题栏（彩色背景 Border）
    // ========================================
    auto titleBorder = std::make_shared<Border>();
    titleBorder->SetName("TitleBar");
    titleBorder->SetHeight(60);
    titleBorder->SetBackground(Color::FromHex(0x2196F3)); // 蓝色背景
    titleBorder->SetPadding(20, 10, 20, 10);
    m_rootPanel->AddChild(titleBorder);
    
    // ========================================
    // 2. 工具栏（水平 StackPanel + 彩色矩形按钮）
    // ========================================
    auto toolbarPanel = std::make_shared<StackPanel>();
    toolbarPanel->SetName("Toolbar");
    toolbarPanel->SetOrientation(StackPanel::Orientation::Horizontal);
    toolbarPanel->SetSpacing(10);
    toolbarPanel->SetMargin(15, 15, 15, 0);
    toolbarPanel->SetHeight(50);
    toolbarPanel->SetBackground(Color::White());
    toolbarPanel->SetPadding(10, 5, 10, 5);
    
    // 工具栏按钮（使用彩色矩形模拟）
    struct ToolButton {
        const wchar_t* name;
        Color color;
    };
    ToolButton buttons[] = {
        { L"New", Color::FromHex(0x4CAF50) },
        { L"Open", Color::FromHex(0x2196F3) },
        { L"Save", Color::FromHex(0xFF9800) },
        { L"Delete", Color::FromHex(0xF44336) },
    };
    
    for (const auto& btn : buttons) {
        auto rect = std::make_shared<luaui::controls::Rectangle>();
        rect->SetWidth(80);
        rect->SetHeight(36);
        rect->SetFill(btn.color);
        rect->SetStroke(Color::FromHex(0x333333));
        rect->SetStrokeThickness(1);
        toolbarPanel->AddChild(rect);
    }
    
    m_rootPanel->AddChild(toolbarPanel);
    
    // ========================================
    // 3. 主内容区域（Grid - 左右分栏）
    // ========================================
    auto mainGrid = std::make_shared<Grid>();
    mainGrid->SetName("MainGrid");
    mainGrid->SetMargin(15, 15, 15, 15);
    mainGrid->SetBackground(Color::White());
    
    // 定义列：左侧固定 250px，右侧填充剩余空间
    mainGrid->AddColumn(GridLength::Pixel(250));
    mainGrid->AddColumn(GridLength::Star(1));
    
    // 定义行：标题 + 内容
    mainGrid->AddRow(GridLength::Auto());
    mainGrid->AddRow(GridLength::Star(1));
    
    // ---- 左侧面板：控制区 ----
    auto leftPanel = std::make_shared<StackPanel>();
    leftPanel->SetName("LeftPanel");
    leftPanel->SetSpacing(15);
    leftPanel->SetMargin(10, 10, 10, 10);
    Grid::SetRow(leftPanel.get(), 0);
    Grid::SetRowSpan(leftPanel.get(), 2);
    Grid::SetColumn(leftPanel.get(), 0);
    
    // 滑块区域
    auto sliderTitle = std::make_shared<luaui::controls::Rectangle>();
    sliderTitle->SetHeight(30);
    sliderTitle->SetFill(Color::FromHex(0xE3F2FD));
    sliderTitle->SetStroke(Color::FromHex(0x2196F3));
    leftPanel->AddChild(sliderTitle);
    
    auto slider = std::make_shared<Slider>();
    slider->SetHeight(30);
    slider->SetValue(50);
    m_progressBar = std::make_shared<ProgressBar>();
    m_progressBar->SetHeight(20);
    m_progressBar->SetValue(50);
    
    leftPanel->AddChild(slider);
    leftPanel->AddChild(m_progressBar);
    
    // 复选框区域（使用彩色矩形模拟）
    auto checkTitle = std::make_shared<luaui::controls::Rectangle>();
    checkTitle->SetHeight(30);
    checkTitle->SetFill(Color::FromHex(0xE8F5E9));
    checkTitle->SetStroke(Color::FromHex(0x4CAF50));
    leftPanel->AddChild(checkTitle);
    
    for (int i = 0; i < 3; i++) {
        auto checkItem = std::make_shared<StackPanel>();
        checkItem->SetOrientation(StackPanel::Orientation::Horizontal);
        checkItem->SetHeight(30);
        
        auto checkBox = std::make_shared<luaui::controls::Rectangle>();
        checkBox->SetWidth(20);
        checkBox->SetHeight(20);
        checkBox->SetFill(i == 0 ? Color::FromHex(0x4CAF50) : Color::White());
        checkBox->SetStroke(Color::FromHex(0x333333));
        
        auto label = std::make_shared<luaui::controls::Rectangle>();
        label->SetWidth(100);
        label->SetHeight(20);
        label->SetFill(Color::FromHex(0xEEEEEE));
        label->SetMargin(10, 0, 0, 0);
        
        checkItem->AddChild(checkBox);
        checkItem->AddChild(label);
        leftPanel->AddChild(checkItem);
    }
    
    mainGrid->AddChild(leftPanel);
    
    // ---- 右侧面板：内容显示区 ----
    auto rightHeader = std::make_shared<Border>();
    rightHeader->SetHeight(40);
    rightHeader->SetBackground(Color::FromHex(0xBBDEFB));
    rightHeader->SetPadding(10, 10, 10, 10);
    Grid::SetRow(rightHeader.get(), 0);
    Grid::SetColumn(rightHeader.get(), 1);
    mainGrid->AddChild(rightHeader);
    
    // 内容区域 - 使用 Canvas 绝对定位展示图形
    auto contentCanvas = std::make_shared<Canvas>();
    contentCanvas->SetName("ContentCanvas");
    contentCanvas->SetBackground(Color::FromHex(0xFAFAFA));
    Grid::SetRow(contentCanvas.get(), 1);
    Grid::SetColumn(contentCanvas.get(), 1);
    
    // 在 Canvas 上放置各种图形
    auto rect1 = std::make_shared<luaui::controls::Rectangle>();
    rect1->SetWidth(120);
    rect1->SetHeight(80);
    rect1->SetFill(Color::FromHex(0xFFCDD2));
    rect1->SetStroke(Color::FromHex(0xF44336));
    rect1->SetStrokeThickness(2);
    Canvas::SetLeft(rect1.get(), 30);
    Canvas::SetTop(rect1.get(), 30);
    contentCanvas->AddChild(rect1);
    
    auto ellipse1 = std::make_shared<luaui::controls::Ellipse>();
    ellipse1->SetWidth(100);
    ellipse1->SetHeight(100);
    ellipse1->SetFill(Color::FromHex(0xC8E6C9));
    ellipse1->SetStroke(Color::FromHex(0x4CAF50));
    ellipse1->SetStrokeThickness(2);
    Canvas::SetLeft(ellipse1.get(), 200);
    Canvas::SetTop(ellipse1.get(), 50);
    contentCanvas->AddChild(ellipse1);
    
    auto rect2 = std::make_shared<luaui::controls::Rectangle>();
    rect2->SetWidth(150);
    rect2->SetHeight(100);
    rect2->SetFill(Color::FromHex(0xBBDEFB));
    rect2->SetStroke(Color::FromHex(0x2196F3));
    rect2->SetStrokeThickness(2);
    Canvas::SetLeft(rect2.get(), 50);
    Canvas::SetTop(rect2.get(), 150);
    contentCanvas->AddChild(rect2);
    
    auto ellipse2 = std::make_shared<luaui::controls::Ellipse>();
    ellipse2->SetWidth(80);
    ellipse2->SetHeight(120);
    ellipse2->SetFill(Color::FromHex(0xFFE0B2));
    ellipse2->SetStroke(Color::FromHex(0xFF9800));
    ellipse2->SetStrokeThickness(2);
    Canvas::SetLeft(ellipse2.get(), 250);
    Canvas::SetTop(ellipse2.get(), 180);
    contentCanvas->AddChild(ellipse2);
    
    mainGrid->AddChild(contentCanvas);
    
    m_rootPanel->AddChild(mainGrid);
    
    // ========================================
    // 4. 状态栏（DockPanel）
    // ========================================
    auto statusDock = std::make_shared<DockPanel>();
    statusDock->SetName("StatusBar");
    statusDock->SetHeight(30);
    statusDock->SetBackground(Color::FromHex(0xEEEEEE));
    statusDock->SetLastChildFill(true);
    
    // 状态指示器（左侧）
    auto statusIndicator = std::make_shared<luaui::controls::Rectangle>();
    statusIndicator->SetWidth(60);
    statusIndicator->SetFill(Color::FromHex(0x4CAF50));
    DockPanel::SetDock(statusIndicator.get(), Dock::Left);
    statusDock->AddChild(statusIndicator);
    
    // 状态文本区域（填充）
    auto statusTextBg = std::make_shared<Border>();
    statusTextBg->SetBackground(Color::FromHex(0xE0E0E0));
    statusDock->AddChild(statusTextBg);
    
    m_rootPanel->AddChild(statusDock);
}

void CompositeLayoutDemo::UpdateStatus(const std::wstring& message) {
    m_statusMessage = message;
    InvalidateRect(m_hWnd, nullptr, FALSE);
}

void CompositeLayoutDemo::Render() {
    if (!m_engine->BeginFrame()) return;
    
    auto* context = m_engine->GetContext();
    if (!context) {
        m_engine->Present();
        return;
    }
    
    // 清空背景
    context->Clear(Color::White());
    
    // 获取窗口大小
    RECT rc;
    GetClientRect(m_hWnd, &rc);
    float width = static_cast<float>(rc.right - rc.left);
    float height = static_cast<float>(rc.bottom - rc.top);
    
    // 测量和排列
    m_rootPanel->Measure(Size(width, height));
    m_rootPanel->Arrange(Rect(0, 0, width, height));
    
    // 渲染
    m_rootPanel->Render(context);
    
    m_engine->Present();
}

void CompositeLayoutDemo::Cleanup() {
    m_rootPanel = nullptr;
    m_progressBar = nullptr;
    m_statusBar = nullptr;
    m_contentPanel = nullptr;
    
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
