#include "Control.h"
#include "luaui/controls/layout.h"
#include "luaui/controls/Shapes.h"
#include "luaui/controls/Image.h"
#include "luaui/controls/Event.h"
#include "luaui/controls/FocusManager.h"
#include "luaui/controls/CheckableControls.h"
#include "luaui/controls/RangeControls.h"
#include "luaui/rendering/IRenderContext.h"
#include "luaui/rendering/IRenderEngine.h"
#include <windows.h>
#include <windowsx.h>
#include <objbase.h>  // For CoInitializeEx, CoUninitialize
#include <string>
#include <sstream>

using namespace luaui;
using namespace luaui::controls;
using namespace luaui::rendering;

// 全局状态显示文本
std::wstring g_statusText = L"Ready";

// 更新状态文本
void SetStatus(const std::wstring& text) {
    g_statusText = text;
    OutputDebugStringW((text + L"\n").c_str());
}

// Helper to track mouse state for button controls
class ButtonInputTracker {
public:
    void HandleMouseMove(const Point& pt, Control* root) {
        auto hitControl = root->HitTestPoint(pt);
        
        // Handle mouse leave
        if (m_hoveredControl && m_hoveredControl != hitControl) {
            if (auto btn = dynamic_cast<Button*>(m_hoveredControl.get())) {
                btn->OnMouseLeave();
            }
        }
        
        // Handle mouse enter
        if (hitControl && hitControl != m_hoveredControl) {
            if (auto btn = dynamic_cast<Button*>(hitControl.get())) {
                btn->OnMouseEnter();
            }
        }
        
        m_hoveredControl = hitControl;
    }
    
    void HandleMouseDown(const Point& pt, Control* root) {
        HandleMouseMove(pt, root);
        m_mouseDownControl = m_hoveredControl;
        
        if (auto btn = dynamic_cast<Button*>(m_hoveredControl.get())) {
            btn->OnMouseDown(pt);
        }
        
        // 处理 CheckBox
        if (auto cb = dynamic_cast<CheckBox*>(m_hoveredControl.get())) {
            cb->SetIsChecked(!cb->GetIsChecked());
            std::wstringstream ss;
            ss << L"CheckBox: " << (cb->GetIsChecked() ? L"Checked" : L"Unchecked");
            SetStatus(ss.str());
        }
        
        // 处理 RadioButton
        if (auto rb = dynamic_cast<RadioButton*>(m_hoveredControl.get())) {
            rb->SetIsSelected(true);
            SetStatus(L"RadioButton selected");
        }
        
        // 处理 Slider - 调用 OnMouseDown 开始拖拽
        if (auto slider = dynamic_cast<Slider*>(m_hoveredControl.get())) {
            m_draggingSlider = slider;
            slider->HandleMouseDown(pt);
        }
    }
    
    void HandleMouseUp(const Point& pt, Control* /*root*/) {
        if (auto btn = dynamic_cast<Button*>(m_hoveredControl.get())) {
            btn->OnMouseUp(pt);
        }
        
        // 通知 Slider 鼠标释放
        if (m_draggingSlider) {
            m_draggingSlider->HandleMouseUp(pt);
        }
        
        m_draggingSlider = nullptr;
        m_mouseDownControl = nullptr;
    }
    
    void HandleMouseMoveDrag(const Point& pt, Control* root) {
        if (m_draggingSlider) {
            // 直接传递 Point 给 Slider
            m_draggingSlider->HandleMouseMove(pt);
        } else {
            // 处理普通鼠标移动
            HandleMouseMove(pt, root);
        }
    }
    
    bool IsDragging() const {
        return m_draggingSlider != nullptr;
    }

private:
    ControlPtr m_hoveredControl;
    ControlPtr m_mouseDownControl;
    Slider* m_draggingSlider = nullptr;
};

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
        wcex.lpszClassName = L"LuaUIControlsDemo";
        
        if (!RegisterClassExW(&wcex)) return false;

        m_hWnd = CreateWindowExW(
            0, L"LuaUIControlsDemo", L"LuaUI Controls Demo - Complete",
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

        // Create render target for window
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
        // 创建根滚动面板（模拟）
        m_rootPanel = std::make_shared<StackPanel>();
        m_rootPanel->SetName("root");
        m_rootPanel->SetMargin(15, 15, 15, 15);
        m_rootPanel->SetSpacing(12);

        // ===== 标题区域 =====
        auto title = std::make_shared<TextBlock>();
        title->SetText(L"🎨 LuaUI Controls Demo");
        title->SetFontSize(28);
        title->SetForeground(Color::FromHex(0x1a1a1a));
        m_rootPanel->AddChild(title);
        
        // 副标题
        auto subtitle = std::make_shared<TextBlock>();
        subtitle->SetText(L"Press Tab to navigate, Space to activate");
        subtitle->SetFontSize(12);
        subtitle->SetForeground(Color::FromHex(0x666666));
        m_rootPanel->AddChild(subtitle);

        // ===== 按钮区域 =====
        auto btnSectionTitle = CreateSectionTitle(L"Buttons");
        m_rootPanel->AddChild(btnSectionTitle);
        
        auto buttonPanel = std::make_shared<StackPanel>();
        buttonPanel->SetOrientation(StackPanel::Orientation::Horizontal);
        buttonPanel->SetSpacing(12);
        
        // 普通按钮
        auto btn1 = std::make_shared<Button>();
        auto btn1Text = std::make_shared<TextBlock>();
        btn1Text->SetText(L"Click Me!");
        btn1->SetContent(btn1Text);
        btn1->AddClickHandler([](Control*) {
            SetStatus(L"Button clicked!");
        });
        buttonPanel->AddChild(btn1);
        
        // 带边框的按钮
        auto btn2 = std::make_shared<Button>();
        auto btn2Text = std::make_shared<TextBlock>();
        btn2Text->SetText(L"Another Button");
        btn2->SetContent(btn2Text);
        btn2->SetBackground(Color::FromHex(0x4CAF50));
        btn2->AddClickHandler([](Control*) {
            SetStatus(L"Green button clicked!");
        });
        buttonPanel->AddChild(btn2);
        
        m_rootPanel->AddChild(buttonPanel);

        // ===== 复选框和单选按钮区域 =====
        auto checkSectionTitle = CreateSectionTitle(L"CheckBox & RadioButton");
        m_rootPanel->AddChild(checkSectionTitle);
        
        auto checkPanel = std::make_shared<StackPanel>();
        checkPanel->SetOrientation(StackPanel::Orientation::Horizontal);
        checkPanel->SetSpacing(30);
        
        // 复选框组
        auto checkGroup = std::make_shared<StackPanel>();
        checkGroup->SetSpacing(8);
        
        auto cb1 = std::make_shared<CheckBox>();
        cb1->SetText(L"Enable Feature A");
        cb1->SetCheckChangedHandler([](CheckBox* sender, bool isChecked) {
            std::wstringstream ss;
            ss << L"Feature A: " << (isChecked ? L"ON" : L"OFF");
            SetStatus(ss.str());
        });
        checkGroup->AddChild(cb1);
        
        auto cb2 = std::make_shared<CheckBox>();
        cb2->SetText(L"Enable Feature B");
        cb2->SetIsChecked(true);
        checkGroup->AddChild(cb2);
        
        auto cb3 = std::make_shared<CheckBox>();
        cb3->SetText(L"Enable Feature C");
        checkGroup->AddChild(cb3);
        
        checkPanel->AddChild(checkGroup);
        
        // 单选按钮组
        auto radioGroup = std::make_shared<StackPanel>();
        radioGroup->SetSpacing(8);
        
        auto rb1 = std::make_shared<RadioButton>();
        rb1->SetText(L"Option 1");
        rb1->SetIsSelected(true);
        radioGroup->AddChild(rb1);
        
        auto rb2 = std::make_shared<RadioButton>();
        rb2->SetText(L"Option 2");
        radioGroup->AddChild(rb2);
        
        auto rb3 = std::make_shared<RadioButton>();
        rb3->SetText(L"Option 3");
        radioGroup->AddChild(rb3);
        
        checkPanel->AddChild(radioGroup);
        m_rootPanel->AddChild(checkPanel);

        // ===== 滑块和进度条区域 =====
        auto rangeSectionTitle = CreateSectionTitle(L"Slider & ProgressBar");
        m_rootPanel->AddChild(rangeSectionTitle);
        
        auto rangePanel = std::make_shared<StackPanel>();
        rangePanel->SetSpacing(15);
        
        // 水平滑块
        auto slider1 = std::make_shared<Slider>();
        slider1->SetWidth(300);
        slider1->SetValue(50);
        
        // 设置重绘回调，实现拖拽时实时更新
        HWND hwnd = m_hWnd;
        slider1->SetRedrawCallback([hwnd]() {
            InvalidateRect(hwnd, nullptr, FALSE);
        });
        
        slider1->SetValueChangedHandler([this](Slider* sender, double value) {
            // 更新进度条
            if (m_progressBar) {
                m_progressBar->SetValue(value);
            }
        });
        rangePanel->AddChild(slider1);
        
        // 进度条
        m_progressBar = std::make_shared<ProgressBar>();
        m_progressBar->SetValue(50);
        rangePanel->AddChild(m_progressBar);
        
        m_rootPanel->AddChild(rangePanel);

        // ===== 图形区域 =====
        auto shapesSectionTitle = CreateSectionTitle(L"Shapes (Rectangle & Ellipse)");
        m_rootPanel->AddChild(shapesSectionTitle);
        
        auto shapesPanel = std::make_shared<StackPanel>();
        shapesPanel->SetOrientation(StackPanel::Orientation::Horizontal);
        shapesPanel->SetSpacing(20);
        shapesPanel->SetHeight(80);
        
        // 圆角矩形
        auto rect = std::make_shared<luaui::controls::Rectangle>();
        rect->SetWidth(80);
        rect->SetHeight(60);
        rect->SetFill(Color::FromHex(0x4CAF50));
        rect->SetStroke(Color::FromHex(0x2E7D32));
        rect->SetStrokeThickness(2);
        rect->SetRadiusX(10);
        rect->SetRadiusY(10);
        shapesPanel->AddChild(rect);
        
        // 椭圆
        auto ellipse = std::make_shared<luaui::controls::Ellipse>();
        ellipse->SetWidth(80);
        ellipse->SetHeight(60);
        ellipse->SetFill(Color::FromHex(0x2196F3));
        ellipse->SetStroke(Color::FromHex(0x1565C0));
        ellipse->SetStrokeThickness(2);
        shapesPanel->AddChild(ellipse);
        
        // 带边框矩形
        auto simpleRect = std::make_shared<luaui::controls::Rectangle>();
        simpleRect->SetWidth(80);
        simpleRect->SetHeight(60);
        simpleRect->SetFill(Color::FromHex(0xFF5722));
        simpleRect->SetStroke(Color::FromHex(0xD84315));
        simpleRect->SetStrokeThickness(3);
        shapesPanel->AddChild(simpleRect);
        
        m_rootPanel->AddChild(shapesPanel);

        // ===== Canvas 区域 =====
        auto canvasSectionTitle = CreateSectionTitle(L"Canvas (Absolute Positioning)");
        m_rootPanel->AddChild(canvasSectionTitle);

        auto canvas = std::make_shared<Canvas>();
        canvas->SetHeight(100);
        canvas->SetBackground(Color::FromHex(0xF5F5F5));

        auto canvasItem1 = std::make_shared<Border>();
        canvasItem1->SetBackground(Color::FromHex(0xFF6B6B));
        canvasItem1->SetWidth(50);
        canvasItem1->SetHeight(50);
        Canvas::SetLeft(canvasItem1.get(), 10);
        Canvas::SetTop(canvasItem1.get(), 25);
        canvas->AddChild(canvasItem1);

        auto canvasItem2 = std::make_shared<Border>();
        canvasItem2->SetBackground(Color::FromHex(0x4ECDC4));
        canvasItem2->SetWidth(50);
        canvasItem2->SetHeight(50);
        Canvas::SetLeft(canvasItem2.get(), 70);
        Canvas::SetTop(canvasItem2.get(), 25);
        canvas->AddChild(canvasItem2);

        auto canvasItem3 = std::make_shared<Border>();
        canvasItem3->SetBackground(Color::FromHex(0x45B7D1));
        canvasItem3->SetWidth(50);
        canvasItem3->SetHeight(50);
        Canvas::SetLeft(canvasItem3.get(), 130);
        Canvas::SetTop(canvasItem3.get(), 25);
        canvas->AddChild(canvasItem3);

        m_rootPanel->AddChild(canvas);
        
        // ===== 状态栏 =====
        auto statusBorder = std::make_shared<Border>();
        statusBorder->SetBackground(Color::FromHex(0xF0F0F0));
        statusBorder->SetPadding(10, 5, 10, 5);
        statusBorder->SetBorderThickness(1);
        
        m_statusText = std::make_shared<TextBlock>();
        m_statusText->SetText(L"Ready");
        m_statusText->SetFontSize(12);
        m_statusText->SetForeground(Color::FromHex(0x333333));
        statusBorder->SetContent(m_statusText);
        
        m_rootPanel->AddChild(statusBorder);
    }
    
    std::shared_ptr<TextBlock> CreateSectionTitle(const std::wstring& text) {
        auto title = std::make_shared<TextBlock>();
        title->SetText(text);
        title->SetFontSize(14);
        title->SetForeground(Color::FromHex(0x0078D4));
        return title;
    }

    void Render() {
        if (!m_engine->BeginFrame()) return;

        auto* context = m_engine->GetContext();
        if (!context) {
            m_engine->Present();
            return;
        }

        // 更新状态文本
        if (m_statusText) {
            m_statusText->SetText(g_statusText);
        }

        // 清空背景
        context->Clear(Color::White());

        // 测量和排列
        RECT rc;
        GetClientRect(m_hWnd, &rc);
        float width = static_cast<float>(rc.right - rc.left);
        float height = static_cast<float>(rc.bottom - rc.top);

        m_rootPanel->Measure(Size(width, height));
        m_rootPanel->Arrange(Rect(0, 0, width, height));
        m_rootPanel->Render(context);

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
                case WM_MOUSEMOVE: {
                    float x = static_cast<float>(GET_X_LPARAM(lParam));
                    float y = static_cast<float>(GET_Y_LPARAM(lParam));
                    pThis->m_inputTracker.HandleMouseMoveDrag(Point(x, y), pThis->m_rootPanel.get());
                    // Slider 会通过回调自动触发重绘，但悬停效果需要这里重绘
                    InvalidateRect(hWnd, nullptr, FALSE);
                    return 0;
                }
                case WM_LBUTTONDOWN: {
                    float x = static_cast<float>(GET_X_LPARAM(lParam));
                    float y = static_cast<float>(GET_Y_LPARAM(lParam));
                    pThis->m_inputTracker.HandleMouseDown(Point(x, y), pThis->m_rootPanel.get());
                    InvalidateRect(hWnd, nullptr, FALSE);
                    return 0;
                }
                case WM_LBUTTONUP: {
                    float x = static_cast<float>(GET_X_LPARAM(lParam));
                    float y = static_cast<float>(GET_Y_LPARAM(lParam));
                    pThis->m_inputTracker.HandleMouseUp(Point(x, y), pThis->m_rootPanel.get());
                    InvalidateRect(hWnd, nullptr, FALSE);
                    return 0;
                }
                case WM_KEYDOWN: {
                    int keyCode = static_cast<int>(wParam);
                    bool isRepeat = (lParam & 0x40000000) != 0;
                    
                    KeyEventArgs args(keyCode, isRepeat);
                    args.Control = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
                    args.Shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
                    args.Alt = (GetKeyState(VK_MENU) & 0x8000) != 0;
                    
                    // Tab 键导航
                    if (keyCode == VK_TAB) {
                        if (args.Shift) {
                            FocusManager::GetInstance().MoveFocusPrevious();
                        } else {
                            FocusManager::GetInstance().MoveFocusNext();
                        }
                        SetStatus(L"Tab navigation");
                        InvalidateRect(hWnd, nullptr, FALSE);
                        return 0;
                    }
                    
                    // Space 键激活
                    if (keyCode == VK_SPACE) {
                        Control* focused = FocusManager::GetInstance().GetFocusedControl();
                        if (auto btn = dynamic_cast<Button*>(focused)) {
                            btn->RaiseClick();
                        }
                    }
                    
                    Control* focused = FocusManager::GetInstance().GetFocusedControl();
                    if (focused) {
                        EventRouter::RaiseEvent(focused, Events::KeyDown, args);
                        if (args.Handled) {
                            InvalidateRect(hWnd, nullptr, FALSE);
                            return 0;
                        }
                    }
                    break;
                }
                case WM_KEYUP: {
                    int keyCode = static_cast<int>(wParam);
                    KeyEventArgs args(keyCode, false);
                    args.Control = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
                    args.Shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
                    args.Alt = (GetKeyState(VK_MENU) & 0x8000) != 0;
                    
                    Control* focused = FocusManager::GetInstance().GetFocusedControl();
                    if (focused) {
                        EventRouter::RaiseEvent(focused, Events::KeyUp, args);
                        if (args.Handled) {
                            InvalidateRect(hWnd, nullptr, FALSE);
                            return 0;
                        }
                    }
                    break;
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
    ButtonInputTracker m_inputTracker;
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
