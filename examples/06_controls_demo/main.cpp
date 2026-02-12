// LuaUI Controls Demo - 使用新架构API
// 06_controls_demo的简化版本，展示新控件API

#include "Controls.h"
#include "IRenderContext.h"
#include "IRenderEngine.h"
#include "Control.h"
#include "Panel.h"
#include "Components/InputComponent.h"
#include "Logger.h"
#include <windows.h>
#include <windowsx.h>
#include <objbase.h>
#include <string>
#include <sstream>
#include <iostream>
#include <functional>
#include <iomanip>

using namespace luaui;
using namespace luaui::controls;
using namespace luaui::rendering;
using namespace luaui::utils;

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
        btn1->Click.Add([this](luaui::Control*) {
            m_statusText->SetText(L"Blue button clicked!");
            // 增加进度条值
            double newValue = m_progressBar->GetValue() + 10;
            if (newValue > 100) newValue = 0;
            m_progressBar->SetValue(newValue);
        });
        buttonPanel->AddChild(btn1);
        
        // 成功按钮
        auto btn2 = std::make_shared<Button>();
        btn2->SetStateColors(
            Color::FromHex(0x4CAF50),  // Normal
            Color::FromHex(0x388E3C),  // Hover
            Color::FromHex(0x1B5E20)   // Pressed
        );
        btn2->Click.Add([this](luaui::Control*) {
            m_statusText->SetText(L"Green button clicked!");
            // 减少进度条值
            double newValue = m_progressBar->GetValue() - 10;
            if (newValue < 0) newValue = 100;
            m_progressBar->SetValue(newValue);
        });
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
        checkBox->SetText(L"Enable Feature");
        checkBox->CheckedChanged.Add([this](CheckBox* sender, bool isChecked) {
            std::wstring text = isChecked ? L"CheckBox checked!" : L"CheckBox unchecked!";
            m_statusText->SetText(text);
        });
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
        slider->ValueChanged.Add([this](Slider* sender, double value) {
            // 滑块值变化时同步更新进度条
            m_progressBar->SetValue(value);
            // 更新状态栏显示当前值
            std::wstringstream ss;
            ss << L"Slider value: " << static_cast<int>(value);
            m_statusText->SetText(ss.str());
        });
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
        if (!m_engine->BeginFrame()) {
            return;
        }

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

        // 仅在需要时进行测量和排列（性能优化）
        if (m_layoutDirty) {
            if (auto* layoutable = m_rootPanel->AsLayoutable()) {
                interfaces::LayoutConstraint constraint;
                constraint.available = Size(width - 20, height - 20);
                layoutable->Measure(constraint);
                layoutable->Arrange(Rect(10, 10, width - 20, height - 20));
            }
            m_layoutDirty = false;
        }
        
        // 渲染
        if (auto* renderable = m_rootPanel->AsRenderable()) {
            renderable->Render(context);
        }

        m_engine->Present();
    }
    
    // 标记布局需要重新计算
    void InvalidateLayout() {
        m_layoutDirty = true;
        InvalidateRect(m_hWnd, nullptr, FALSE);
    }

    void Cleanup() {
        m_rootPanel = nullptr;
        if (m_engine) {
            m_engine->Shutdown();
            m_engine = nullptr;
        }
    }

    // 获取控件的全局位置
    void GetControlGlobalPosition(luaui::Control* control, float& outX, float& outY) {
        outX = 0;
        outY = 0;
        auto* current = control;
        while (current) {
            auto* render = current->GetRender();
            if (render) {
                outX += render->GetRenderRect().x;
                outY += render->GetRenderRect().y;
            }
            auto parent = current->GetParent();
            current = parent ? static_cast<luaui::Control*>(parent.get()) : nullptr;
        }
    }
    
    // 输入事件处理
    void HandleMouseMove(float x, float y) {
        // 如果有捕获的控件（拖动中），发送全局坐标
        if (m_capturedControl) {
            static int count = 0;
            if (++count % 10 == 0) {  // 每10次打印一次，避免刷屏
                Logger::TraceF("[Drag] %s at (%.1f,%.1f)", 
                    m_capturedControl->GetTypeName().c_str(), x, y);
            }
            if (auto* inputComp = m_capturedControl->GetInput()) {
                MouseEventArgs args{x, y, 0, false};
                inputComp->RaiseMouseMove(args);
            }
            // 拖动时需要持续重绘
            InvalidateRect(m_hWnd, nullptr, FALSE);
            return;
        }
        
        auto* control = HitTest(m_rootPanel.get(), x, y);
        
        // 处理 MouseLeave
        if (m_lastMouseOver && m_lastMouseOver != control) {
            if (auto* inputComp = m_lastMouseOver->GetInput()) {
                inputComp->RaiseMouseLeave();
                m_lastMouseOver = nullptr;
                InvalidateRect(m_hWnd, nullptr, FALSE);
            }
        }
        
        // 处理 MouseEnter/MouseMove
        if (control) {
            if (auto* inputComp = control->GetInput()) {
                if (m_lastMouseOver != control) {
                    inputComp->RaiseMouseEnter();
                    m_lastMouseOver = control;
                    InvalidateRect(m_hWnd, nullptr, FALSE);
                }
                MouseEventArgs args{x, y, 0, false};
                inputComp->RaiseMouseMove(args);
            }
        }
    }
    
    void HandleMouseDown(float x, float y, int button) {
        auto* control = HitTest(m_rootPanel.get(), x, y);
        
        Logger::TraceF("[MouseDown] Hit: %s at (%.1f,%.1f)", 
            control ? control->GetTypeName().c_str() : "null", x, y);
        
        if (control) {
            // 记录捕获的控件
            m_capturedControl = control;
            
            // 设置焦点
            if (auto* inputComp = control->GetInput()) {
                if (inputComp->GetIsFocusable() && !inputComp->GetIsFocused()) {
                    // 清除之前的焦点
                    if (m_focusedControl && m_focusedControl != control) {
                        if (auto* oldInputComp = m_focusedControl->GetInput()) {
                            oldInputComp->KillFocus();
                            oldInputComp->RaiseLostFocus();
                        }
                    }
                    // 设置新焦点
                    m_focusedControl = control;
                    inputComp->Focus();
                    inputComp->RaiseGotFocus();
                    InvalidateRect(m_hWnd, nullptr, FALSE);
                }
                
                MouseEventArgs args{x, y, button, false};
                inputComp->RaiseMouseDown(args);
                InvalidateRect(m_hWnd, nullptr, FALSE);
            }
        }
    }
    
    void HandleMouseUp(float x, float y, int button) {
        Logger::TraceF("[MouseUp] captured=%s", 
            m_capturedControl ? m_capturedControl->GetTypeName().c_str() : "null");
        
        // 如果有捕获的控件，先发送给它
        if (m_capturedControl) {
            if (auto* inputComp = m_capturedControl->GetInput()) {
                MouseEventArgs args{x, y, button, false};
                inputComp->RaiseMouseUp(args);
                // 如果鼠标在控件上，触发点击
                auto* hitControl = HitTest(m_rootPanel.get(), x, y);
                if (hitControl == m_capturedControl) {
                    inputComp->RaiseClick();
                }
            }
            m_capturedControl = nullptr;
            InvalidateRect(m_hWnd, nullptr, FALSE);
            return;
        }
        
        auto* control = HitTest(m_rootPanel.get(), x, y);
        if (control) {
            if (auto* inputComp = control->GetInput()) {
                MouseEventArgs args{x, y, button, false};
                inputComp->RaiseMouseUp(args);
                inputComp->RaiseClick();
                InvalidateRect(m_hWnd, nullptr, FALSE);
            }
        }
    }
    
    void HandleChar(wchar_t ch) {
        if (m_focusedControl) {
            if (auto* inputComp = m_focusedControl->GetInput()) {
                inputComp->RaiseChar(ch);
                InvalidateRect(m_hWnd, nullptr, FALSE);
            }
        }
    }
    
    void HandleKeyDown(int keyCode) {
        if (m_focusedControl) {
            if (auto* inputComp = m_focusedControl->GetInput()) {
                KeyEventArgs args{keyCode, false, false, false, false, false};
                inputComp->RaiseKeyDown(args);
                InvalidateRect(m_hWnd, nullptr, FALSE);
            }
        }
    }
    
    // 命中测试 - 查找鼠标位置下的控件
    luaui::Control* HitTest(luaui::Control* root, float x, float y) {
        if (!root) return nullptr;
        
        // 获取控件的渲染矩形（全局坐标）
        auto* render = root->GetRender();
        if (!render) return nullptr;
        
        const auto& rect = render->GetRenderRect();
        
        // 检查点是否在矩形内
        if (x >= rect.x && x < rect.x + rect.width &&
            y >= rect.y && y < rect.y + rect.height) {
            
            // 如果是 Panel，递归测试子控件
            if (auto* panel = dynamic_cast<luaui::controls::Panel*>(root)) {
                // 转换到 Panel 的本地坐标
                float localX = x - rect.x;
                float localY = y - rect.y;
                
                // 从后向前遍历子控件（后添加的在上面）
                for (int i = static_cast<int>(panel->GetChildCount()) - 1; i >= 0; --i) {
                    auto* child = static_cast<luaui::Control*>(panel->GetChild(i).get());
                    if (auto* result = HitTest(child, localX, localY)) {
                        return result;
                    }
                }
            }
            
            return root;
        }
        
        return nullptr;
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
                    pThis->InvalidateLayout();  // 窗口大小变化需要重新布局
                    return 0;
                }
                case WM_MOUSEMOVE: {
                    float x = static_cast<float>(GET_X_LPARAM(lParam));
                    float y = static_cast<float>(GET_Y_LPARAM(lParam));
                    pThis->HandleMouseMove(x, y);
                    return 0;
                }
                case WM_LBUTTONDOWN: {
                    float x = static_cast<float>(GET_X_LPARAM(lParam));
                    float y = static_cast<float>(GET_Y_LPARAM(lParam));
                    pThis->HandleMouseDown(x, y, 0);
                    SetCapture(hWnd);
                    return 0;
                }
                case WM_LBUTTONUP: {
                    float x = static_cast<float>(GET_X_LPARAM(lParam));
                    float y = static_cast<float>(GET_Y_LPARAM(lParam));
                    pThis->HandleMouseUp(x, y, 0);
                    ReleaseCapture();
                    return 0;
                }
                case WM_KEYDOWN: {
                    pThis->HandleKeyDown(static_cast<int>(wParam));
                    return 0;
                }
                case WM_CHAR: {
                    pThis->HandleChar(static_cast<wchar_t>(wParam));
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
    
    // 输入状态跟踪
    luaui::Control* m_lastMouseOver = nullptr;
    luaui::Control* m_focusedControl = nullptr;
    
    // 布局脏标记 - 避免每帧重测量
    bool m_layoutDirty = true;
    
    // 鼠标捕获的控件（用于拖动）
    luaui::Control* m_capturedControl = nullptr;
};

int main() {
    Logger::Initialize();
    Logger::Info("=== LuaUI Controls Demo Starting ===");
    
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        Logger::Error("Failed to initialize COM");
        return 1;
    }

    HINSTANCE hInstance = GetModuleHandle(nullptr);
    int nCmdShow = SW_SHOWDEFAULT;

    ControlsDemoWindow demo;
    if (!demo.Initialize(hInstance, nCmdShow)) {
        Logger::Error("Failed to initialize demo window");
        CoUninitialize();
        return 1;
    }

    int result = demo.Run();
    CoUninitialize();
    return result;
}
