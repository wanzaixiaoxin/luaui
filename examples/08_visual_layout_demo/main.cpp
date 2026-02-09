// Visual Layout Demo - 可视化布局演示
// 展示各种布局面板的效果

#include "Control.h"
#include "layout.h"
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

// 彩色矩形控件 - 用于展示布局效果
class ColorRect : public Control {
public:
    ColorRect(const Color& fillColor, const Color& strokeColor = Color::Transparent())
        : m_fillColor(fillColor), m_strokeColor(strokeColor) {
        SetName("ColorRect");
    }
    
    std::string GetTypeName() const override { return "ColorRect"; }
    
    void SetFillColor(const Color& c) { m_fillColor = c; Invalidate(); }
    void SetStrokeColor(const Color& c) { m_strokeColor = c; Invalidate(); }
    void SetStrokeThickness(float t) { m_strokeThickness = t; Invalidate(); }
    
protected:
    Size MeasureOverride(const Size& availableSize) override {
        // 使用设定的宽高，如果没有则使用默认值
        float w = GetWidth() > 0 ? GetWidth() : 80;
        float h = GetHeight() > 0 ? GetHeight() : 60;
        
        // 如果可用空间有限制，则遵守
        if (availableSize.width > 0 && availableSize.width < std::numeric_limits<float>::max()) {
            w = std::min(w, availableSize.width);
        }
        if (availableSize.height > 0 && availableSize.height < std::numeric_limits<float>::max()) {
            h = std::min(h, availableSize.height);
        }
        
        return Size(w, h);
    }
    
    void RenderOverride(IRenderContext* context) override {
        Rect rect = GetRenderRect();
        
        // 填充
        if (m_fillColor.a > 0) {
            auto brush = context->CreateSolidColorBrush(m_fillColor);
            context->FillRectangle(rect, brush.get());
        }
        
        // 描边
        if (m_strokeThickness > 0 && m_strokeColor.a > 0) {
            auto brush = context->CreateSolidColorBrush(m_strokeColor);
            context->DrawRectangle(rect, brush.get(), m_strokeThickness);
        }
    }
    
private:
    Color m_fillColor;
    Color m_strokeColor;
    float m_strokeThickness = 1.0f;
};

// 带标题的面板
class TitledPanel : public Panel {
public:
    TitledPanel(const std::wstring& title) : m_title(title) {
        SetName("TitledPanel");
        SetBackground(Color::FromHex(0xF5F5F5));
        SetMargin(10, 10, 10, 10);
    }
    
    std::string GetTypeName() const override { return "TitledPanel"; }
    
protected:
    void RenderOverride(IRenderContext* context) override {
        // 背景
        Rect rect = GetRenderRect();
        auto bgBrush = context->CreateSolidColorBrush(m_background);
        context->FillRectangle(rect, bgBrush.get());
        
        // 边框
        auto borderBrush = context->CreateSolidColorBrush(Color::FromHex(0xCCCCCC));
        context->DrawRectangle(rect, borderBrush.get(), 1.0f);
        
        // 标题背景
        Rect titleRect(rect.x, rect.y, rect.width, 30);
        auto titleBgBrush = context->CreateSolidColorBrush(Color::FromHex(0xE0E0E0));
        context->FillRectangle(titleRect, titleBgBrush.get());
        
        // TODO: 绘制标题文字（需要文本渲染支持）
    }
    
private:
    std::wstring m_title;
};

// 演示窗口
class VisualLayoutDemo {
public:
    VisualLayoutDemo() = default;
    ~VisualLayoutDemo() { Cleanup(); }
    
    bool Initialize(HINSTANCE hInstance, int nCmdShow);
    int Run();
    
private:
    void CreateDemo_StackPanel();
    void CreateDemo_Grid();
    void CreateDemo_Canvas();
    void CreateDemo_DockPanel();
    void CreateDemo_WrapPanel();
    void SwitchDemo(int demoIndex);
    void Render();
    void Cleanup();
    
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    
private:
    HWND m_hWnd = nullptr;
    IRenderEnginePtr m_engine;
    std::shared_ptr<Panel> m_rootPanel;
    int m_currentDemoIndex = 0;
    const wchar_t* m_demoNames[5] = {
        L"StackPanel (Vertical)",
        L"Grid",
        L"Canvas",
        L"DockPanel",
        L"WrapPanel"
    };
};

bool VisualLayoutDemo::Initialize(HINSTANCE hInstance, int nCmdShow) {
    // 注册窗口类
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = L"VisualLayoutDemo";
    
    if (!RegisterClassExW(&wcex)) return false;
    
    // 创建窗口
    m_hWnd = CreateWindowExW(
        0, L"VisualLayoutDemo", L"Visual Layout Demo - 按数字键1-5切换布局",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1024, 768,
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
    desc.width = 1024;
    desc.height = 768;
    
    if (!m_engine->CreateRenderTarget(desc)) {
        MessageBoxW(m_hWnd, L"Failed to create render target", L"Error", MB_OK);
        return false;
    }
    
    // 创建根面板
    m_rootPanel = std::make_shared<StackPanel>();
    m_rootPanel->SetName("Root");
    m_rootPanel->SetBackground(Color::White());
    
    // 加载初始演示
    SwitchDemo(0);
    
    ShowWindow(m_hWnd, nCmdShow);
    UpdateWindow(m_hWnd);
    return true;
}

void VisualLayoutDemo::CreateDemo_StackPanel() {
    m_rootPanel->ClearChildren();
    
    auto panel = std::make_shared<StackPanel>();
    panel->SetName("StackPanelDemo");
    panel->SetOrientation(StackPanel::Orientation::Vertical);
    panel->SetSpacing(10);
    panel->SetMargin(20, 20, 20, 20);
    panel->SetBackground(Color::FromHex(0xFAFAFA));
    
    // 添加彩色矩形
    Color colors[] = {
        Color::FromHex(0xFF6B6B),  // 红
        Color::FromHex(0x4ECDC4),  // 青
        Color::FromHex(0x45B7D1),  // 蓝
        Color::FromHex(0x96CEB4),  // 绿
        Color::FromHex(0xFECA57),  // 黄
    };
    
    for (int i = 0; i < 5; i++) {
        auto rect = std::make_shared<ColorRect>(colors[i], Color::FromHex(0x333333));
        rect->SetWidth(200);
        rect->SetHeight(60);
        rect->SetStrokeThickness(2);
        panel->AddChild(rect);
    }
    
    m_rootPanel->AddChild(panel);
}

void VisualLayoutDemo::CreateDemo_Grid() {
    m_rootPanel->ClearChildren();
    
    auto grid = std::make_shared<Grid>();
    grid->SetName("GridDemo");
    grid->SetMargin(20, 20, 20, 20);
    grid->SetBackground(Color::FromHex(0xFAFAFA));
    
    // 定义 3 列
    grid->AddColumn(GridLength::Star(1));
    grid->AddColumn(GridLength::Star(1));
    grid->AddColumn(GridLength::Star(1));
    
    // 定义 2 行
    grid->AddRow(GridLength::Star(1));
    grid->AddRow(GridLength::Star(1));
    
    // 6 个单元格
    Color colors[] = {
        Color::FromHex(0xFF6B6B),
        Color::FromHex(0x4ECDC4),
        Color::FromHex(0x45B7D1),
        Color::FromHex(0x96CEB4),
        Color::FromHex(0xFECA57),
        Color::FromHex(0xDDA0DD),
    };
    
    for (int i = 0; i < 6; i++) {
        auto cell = std::make_shared<ColorRect>(colors[i], Color::FromHex(0x333333));
        cell->SetStrokeThickness(2);
        Grid::SetRow(cell.get(), i / 3);
        Grid::SetColumn(cell.get(), i % 3);
        grid->AddChild(cell);
    }
    
    m_rootPanel->AddChild(grid);
}

void VisualLayoutDemo::CreateDemo_Canvas() {
    m_rootPanel->ClearChildren();
    
    auto canvas = std::make_shared<Canvas>();
    canvas->SetName("CanvasDemo");
    canvas->SetMargin(20, 20, 20, 20);
    canvas->SetBackground(Color::FromHex(0xFAFAFA));
    
    // 添加绝对定位的矩形
    auto r1 = std::make_shared<ColorRect>(Color::FromHex(0xFF6B6B), Color::FromHex(0x333333));
    r1->SetWidth(120);
    r1->SetHeight(80);
    Canvas::SetLeft(r1.get(), 50);
    Canvas::SetTop(r1.get(), 50);
    canvas->AddChild(r1);
    
    auto r2 = std::make_shared<ColorRect>(Color::FromHex(0x4ECDC4), Color::FromHex(0x333333));
    r2->SetWidth(100);
    r2->SetHeight(100);
    Canvas::SetLeft(r2.get(), 200);
    Canvas::SetTop(r2.get(), 100);
    canvas->AddChild(r2);
    
    auto r3 = std::make_shared<ColorRect>(Color::FromHex(0x45B7D1), Color::FromHex(0x333333));
    r3->SetWidth(150);
    r3->SetHeight(60);
    Canvas::SetLeft(r3.get(), 400);
    Canvas::SetTop(r3.get(), 80);
    canvas->AddChild(r3);
    
    auto r4 = std::make_shared<ColorRect>(Color::FromHex(0x96CEB4), Color::FromHex(0x333333));
    r4->SetWidth(80);
    r4->SetHeight(120);
    Canvas::SetLeft(r4.get(), 100);
    Canvas::SetTop(r4.get(), 250);
    canvas->AddChild(r4);
    
    auto r5 = std::make_shared<ColorRect>(Color::FromHex(0xFECA57), Color::FromHex(0x333333));
    r5->SetWidth(200);
    r5->SetHeight(100);
    Canvas::SetLeft(r5.get(), 350);
    Canvas::SetTop(r5.get(), 300);
    canvas->AddChild(r5);
    
    m_rootPanel->AddChild(canvas);
}

void VisualLayoutDemo::CreateDemo_DockPanel() {
    m_rootPanel->ClearChildren();
    
    auto dock = std::make_shared<DockPanel>();
    dock->SetName("DockPanelDemo");
    dock->SetMargin(20, 20, 20, 20);
    dock->SetBackground(Color::FromHex(0xFAFAFA));
    dock->SetLastChildFill(true);
    
    // 顶部
    auto top = std::make_shared<ColorRect>(Color::FromHex(0xFF6B6B), Color::FromHex(0x333333));
    top->SetHeight(60);
    DockPanel::SetDock(top.get(), Dock::Top);
    dock->AddChild(top);
    
    // 左侧
    auto left = std::make_shared<ColorRect>(Color::FromHex(0x4ECDC4), Color::FromHex(0x333333));
    left->SetWidth(120);
    DockPanel::SetDock(left.get(), Dock::Left);
    dock->AddChild(left);
    
    // 右侧
    auto right = std::make_shared<ColorRect>(Color::FromHex(0x45B7D1), Color::FromHex(0x333333));
    right->SetWidth(120);
    DockPanel::SetDock(right.get(), Dock::Right);
    dock->AddChild(right);
    
    // 底部
    auto bottom = std::make_shared<ColorRect>(Color::FromHex(0x96CEB4), Color::FromHex(0x333333));
    bottom->SetHeight(60);
    DockPanel::SetDock(bottom.get(), Dock::Bottom);
    dock->AddChild(bottom);
    
    // 中间填充
    auto fill = std::make_shared<ColorRect>(Color::FromHex(0xFECA57), Color::FromHex(0x333333));
    dock->AddChild(fill);
    
    m_rootPanel->AddChild(dock);
}

void VisualLayoutDemo::CreateDemo_WrapPanel() {
    m_rootPanel->ClearChildren();
    
    auto wrap = std::make_shared<WrapPanel>();
    wrap->SetName("WrapPanelDemo");
    wrap->SetOrientation(WrapPanel::Orientation::Horizontal);
    wrap->SetMargin(20, 20, 20, 20);
    wrap->SetBackground(Color::FromHex(0xFAFAFA));
    
    // 添加多个矩形，会自动换行
    Color colors[] = {
        Color::FromHex(0xFF6B6B),
        Color::FromHex(0x4ECDC4),
        Color::FromHex(0x45B7D1),
        Color::FromHex(0x96CEB4),
        Color::FromHex(0xFECA57),
        Color::FromHex(0xDDA0DD),
        Color::FromHex(0xFF9F43),
        Color::FromHex(0x10AC84),
    };
    
    for (int i = 0; i < 12; i++) {
        auto rect = std::make_shared<ColorRect>(colors[i % 8], Color::FromHex(0x333333));
        rect->SetWidth(120);
        rect->SetHeight(80);
        wrap->AddChild(rect);
    }
    
    m_rootPanel->AddChild(wrap);
}

void VisualLayoutDemo::SwitchDemo(int demoIndex) {
    m_currentDemoIndex = demoIndex % 5;
    
    switch (m_currentDemoIndex) {
        case 0: CreateDemo_StackPanel(); break;
        case 1: CreateDemo_Grid(); break;
        case 2: CreateDemo_Canvas(); break;
        case 3: CreateDemo_DockPanel(); break;
        case 4: CreateDemo_WrapPanel(); break;
    }
    
    // 更新窗口标题
    std::wstring title = std::wstring(L"Visual Layout Demo - ") + m_demoNames[m_currentDemoIndex] + L" [Press 1-5 to switch]";
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
                if (key >= '1' && key <= '5') {
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
