#include "Control.h"
#include "luaui/rendering/IRenderContext.h"
#include "luaui/rendering/IRenderEngine.h"
#include <windows.h>
#include <string>

using namespace luaui;
using namespace luaui::controls;
using namespace luaui::rendering;

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
            0, L"LuaUIControlsDemo", L"LuaUI Controls Demo - Flat Structure",
            WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME,
            CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
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
        desc.width = 800;
        desc.height = 600;
        
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
        // Create root panel
        m_rootPanel = std::make_shared<StackPanel>();
        m_rootPanel->SetName("root");
        m_rootPanel->SetMargin(20, 20, 20, 20);
        m_rootPanel->SetSpacing(10);

        // Create title
        auto title = std::make_shared<TextBlock>();
        title->SetText(L"Controls Demo - Flat Directory Structure!");
        title->SetFontSize(24);
        title->SetForeground(Color::FromHex(0x333333));
        m_rootPanel->AddChild(title);

        // Create description
        auto desc = std::make_shared<TextBlock>();
        desc->SetText(L"Include path: src/luaui/controls/include/Control.h");
        desc->SetFontSize(14);
        desc->SetForeground(Color::FromHex(0x666666));
        m_rootPanel->AddChild(desc);

        // Create a border with content
        auto border = std::make_shared<Border>();
        border->SetBackground(Color::FromHex(0xF0F0F0));
        border->SetPadding(15, 15, 15, 15);

        auto borderContent = std::make_shared<TextBlock>();
        borderContent->SetText(L"Border Control with Content");
        borderContent->SetForeground(Color::FromHex(0x333333));
        border->SetContent(borderContent);
        m_rootPanel->AddChild(border);

        // Create button panel
        auto buttonPanel = std::make_shared<StackPanel>();
        buttonPanel->SetOrientation(StackPanel::Orientation::Horizontal);
        buttonPanel->SetSpacing(10);

        // Create button
        auto btn1 = std::make_shared<Button>();
        auto btn1Text = std::make_shared<TextBlock>();
        btn1Text->SetText(L"Click Me!");
        btn1->SetContent(btn1Text);
        btn1->AddClickHandler([](Control*) {
            OutputDebugStringA("Button clicked!\n");
        });
        buttonPanel->AddChild(btn1);

        m_rootPanel->AddChild(buttonPanel);

        // Canvas demonstration
        auto canvasTitle = std::make_shared<TextBlock>();
        canvasTitle->SetText(L"Canvas (Absolute Positioning):");
        canvasTitle->SetFontSize(14);
        m_rootPanel->AddChild(canvasTitle);

        auto canvas = std::make_shared<Canvas>();
        canvas->SetHeight(100);
        canvas->SetBackground(Color::FromHex(0xE8E8E8));

        auto canvasItem1 = std::make_shared<Border>();
        canvasItem1->SetBackground(Color::FromHex(0xFF6B6B));
        canvasItem1->SetWidth(50);
        canvasItem1->SetHeight(50);
        Canvas::SetLeft(canvasItem1.get(), 10);
        Canvas::SetTop(canvasItem1.get(), 10);
        canvas->AddChild(canvasItem1);

        auto canvasItem2 = std::make_shared<Border>();
        canvasItem2->SetBackground(Color::FromHex(0x4ECDC4));
        canvasItem2->SetWidth(50);
        canvasItem2->SetHeight(50);
        Canvas::SetLeft(canvasItem2.get(), 70);
        Canvas::SetTop(canvasItem2.get(), 10);
        canvas->AddChild(canvasItem2);

        auto canvasItem3 = std::make_shared<Border>();
        canvasItem3->SetBackground(Color::FromHex(0x45B7D1));
        canvasItem3->SetWidth(50);
        canvasItem3->SetHeight(50);
        Canvas::SetLeft(canvasItem3.get(), 130);
        Canvas::SetTop(canvasItem3.get(), 10);
        canvas->AddChild(canvasItem3);

        m_rootPanel->AddChild(canvas);
    }

    void Render() {
        if (!m_engine->BeginFrame()) return;

        // Get render context from engine
        auto* context = m_engine->GetContext();
        if (!context) {
            m_engine->Present();
            return;
        }

        // Clear background
        context->Clear(Color::White());

        // Measure and arrange
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
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    ControlsDemoWindow demo;
    if (!demo.Initialize(hInstance, nCmdShow)) {
        MessageBoxW(nullptr, L"Failed to initialize demo", L"Error", MB_OK);
        return 1;
    }
    return demo.Run();
}
