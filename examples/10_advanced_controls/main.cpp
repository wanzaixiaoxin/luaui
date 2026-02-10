// 10_advanced_controls - Advanced input controls demo
// Demonstrates TextBox, ComboBox, ListBox, TabControl, ScrollViewer

#include "Control.h"
#include "layout.h"
#include "IRenderContext.h"
#include "IRenderEngine.h"
#include "Event.h"
#include "FocusManager.h"
#include <windows.h>
#include <windowsx.h>
#include <objbase.h>
#include <string>
#include <sstream>

using namespace luaui;
using namespace luaui::controls;
using namespace luaui::rendering;

std::wstring g_statusText = L"Ready";

void SetStatus(const std::wstring& text) {
    g_statusText = text;
    OutputDebugStringW((text + L"\n").c_str());
}

class AdvancedControlsWindow {
public:
    bool Initialize(HINSTANCE hInstance, int nCmdShow) {
        WNDCLASSEXW wcex = {};
        wcex.cbSize = sizeof(WNDCLASSEXW);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WindowProc;
        wcex.hInstance = hInstance;
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszClassName = L"LuaUIAdvancedControls";

        if (!RegisterClassExW(&wcex)) return false;

        m_hWnd = CreateWindowExW(
            0, L"LuaUIAdvancedControls", L"LuaUI Advanced Controls Demo",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 1000, 700,
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
        desc.width = 1000;
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
        // Create root panel with padding
        auto rootPanel = std::make_shared<StackPanel>();
        rootPanel->SetName("rootPanel");
        rootPanel->SetMargin(20, 20, 20, 20);  // Larger margin from window edges
        rootPanel->SetSpacing(15);

        // ===== Title =====
        auto title = std::make_shared<TextBlock>();
        title->SetText(L"Advanced Controls Demo");
        title->SetFontSize(24);
        title->SetForeground(Color::FromHex(0x333333));
        rootPanel->AddChild(title);

        // ===== Text Input Section =====
        auto textSectionLabel = std::make_shared<TextBlock>();
        textSectionLabel->SetText(L"Text Input:");
        textSectionLabel->SetFontSize(14);
        textSectionLabel->SetForeground(Color::FromHex(0x666666));
        rootPanel->AddChild(textSectionLabel);

        // Single-line TextBox
        auto textBox = std::make_shared<TextBox>();
        textBox->SetWidth(400);
        textBox->SetHeight(32);
        textBox->SetPlaceholder(L"Enter text here...");
        textBox->SetTextChangedHandler([](TextBox* /*sender*/, const std::wstring& text) {
            std::wstringstream ss;
            ss << L"TextBox changed: " << text;
            SetStatus(ss.str());
        });
        rootPanel->AddChild(textBox);

        // Password TextBox
        auto pwdLabel = std::make_shared<TextBlock>();
        pwdLabel->SetText(L"Password:");
        pwdLabel->SetFontSize(12);
        pwdLabel->SetForeground(Color::FromHex(0x666666));
        pwdLabel->SetMargin(0, 10, 0, 0);
        rootPanel->AddChild(pwdLabel);

        auto pwdBox = std::make_shared<TextBox>();
        pwdBox->SetWidth(400);
        pwdBox->SetHeight(32);
        pwdBox->SetPlaceholder(L"Enter password...");
        pwdBox->SetIsPassword(true);
        rootPanel->AddChild(pwdBox);

        // ===== ListBox Section =====
        auto listSectionLabel = std::make_shared<TextBlock>();
        listSectionLabel->SetText(L"ListBox:");
        listSectionLabel->SetFontSize(14);
        listSectionLabel->SetForeground(Color::FromHex(0x666666));
        listSectionLabel->SetMargin(0, 10, 0, 0);
        rootPanel->AddChild(listSectionLabel);

        auto listBox = std::make_shared<ListBox>();
        listBox->SetWidth(400);
        listBox->SetHeight(150);
        listBox->AddItem(L"Item 1");
        listBox->AddItem(L"Item 2");
        listBox->AddItem(L"Item 3");
        listBox->AddItem(L"Item 4");
        listBox->AddItem(L"Item 5");
        rootPanel->AddChild(listBox);

        // ===== ScrollViewer Section =====
        auto scrollSectionLabel = std::make_shared<TextBlock>();
        scrollSectionLabel->SetText(L"ScrollViewer:");
        scrollSectionLabel->SetFontSize(14);
        scrollSectionLabel->SetForeground(Color::FromHex(0x666666));
        scrollSectionLabel->SetMargin(0, 10, 0, 0);
        rootPanel->AddChild(scrollSectionLabel);

        auto scrollViewer = std::make_shared<ScrollViewer>();
        scrollViewer->SetWidth(600);
        scrollViewer->SetHeight(200);

        // Create content for scrolling with padding
        auto scrollContent = std::make_shared<StackPanel>();
        scrollContent->SetSpacing(10);
        scrollContent->SetMargin(10, 10, 10, 10);  // Add padding inside scrollviewer
        
        for (int i = 0; i < 20; i++) {
            auto item = std::make_shared<TextBlock>();
            std::wstringstream ss;
            ss << L"Scroll item " << (i + 1) << L" - This is scrollable content";
            item->SetText(ss.str());
            item->SetFontSize(12);
            scrollContent->AddChild(item);
        }
        
        scrollViewer->SetContent(scrollContent);
        rootPanel->AddChild(scrollViewer);

        m_root = rootPanel;
    }

    void Render() {
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

        if (m_root) {
            m_root->Measure(Size(width, height));
            // Apply root control's margin to the arrange rect
            float marginLeft = m_root->GetMarginLeft();
            float marginTop = m_root->GetMarginTop();
            float marginRight = m_root->GetMarginRight();
            float marginBottom = m_root->GetMarginBottom();
            m_root->Arrange(Rect(
                marginLeft, 
                marginTop, 
                width - marginLeft - marginRight, 
                height - marginTop - marginBottom
            ));
            m_root->Render(context);
        }

        m_engine->Present();
    }

    void Cleanup() {
        m_root = nullptr;
        if (m_engine) {
            m_engine->Shutdown();
            m_engine = nullptr;
        }
    }

    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        AdvancedControlsWindow* pThis = nullptr;

        if (message == WM_NCCREATE) {
            CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            pThis = reinterpret_cast<AdvancedControlsWindow*>(pCreate->lpCreateParams);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
            pThis->m_hWnd = hWnd;
        } else {
            pThis = reinterpret_cast<AdvancedControlsWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
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
                    InvalidateRect(hWnd, nullptr, FALSE);
                    return 0;
                }
                case WM_LBUTTONDOWN: {
                    float x = static_cast<float>(GET_X_LPARAM(lParam));
                    float y = static_cast<float>(GET_Y_LPARAM(lParam));
                    
                    MouseEventArgs args(x, y);
                    if (pThis->m_root) {
                        auto hit = pThis->m_root->HitTestPoint(Point(x, y));
                        if (hit) {
                            hit->OnMouseDown(args);
                        }
                    }
                    InvalidateRect(hWnd, nullptr, FALSE);
                    return 0;
                }
                case WM_LBUTTONUP: {
                    InvalidateRect(hWnd, nullptr, FALSE);
                    return 0;
                }
                case WM_MOUSEWHEEL: {
                    int delta = GET_WHEEL_DELTA_WPARAM(wParam);
                    // GET_X_LPARAM/GET_Y_LPARAM return screen coordinates for WM_MOUSEWHEEL
                    POINT pt;
                    pt.x = GET_X_LPARAM(lParam);
                    pt.y = GET_Y_LPARAM(lParam);
                    ScreenToClient(hWnd, &pt);
                    
                    MouseEventArgs args(static_cast<float>(pt.x), static_cast<float>(pt.y));
                    args.Clicks = delta;
                    
                    if (pThis->m_root) {
                        auto hit = pThis->m_root->HitTestPoint(Point(args.Position.X, args.Position.Y));
                        if (hit) {
                            hit->OnMouseWheel(args);
                        }
                    }
                    InvalidateRect(hWnd, nullptr, FALSE);
                    return 0;
                }
                case WM_CHAR: {
                    wchar_t ch = static_cast<wchar_t>(wParam);
                    Control* focused = FocusManager::GetInstance().GetFocusedControl();
                    if (auto textBox = dynamic_cast<TextBox*>(focused)) {
                        textBox->OnChar(ch);
                        InvalidateRect(hWnd, nullptr, FALSE);
                    }
                    return 0;
                }
                case WM_KEYDOWN: {
                    int keyCode = static_cast<int>(wParam);
                    bool isRepeat = (lParam & 0x40000000) != 0;

                    KeyEventArgs args(keyCode, isRepeat);
                    args.Control = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
                    args.Shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
                    args.Alt = (GetKeyState(VK_MENU) & 0x8000) != 0;

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

                    Control* focused = FocusManager::GetInstance().GetFocusedControl();
                    if (focused) {
                        focused->OnKeyDown(args);
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
    ControlPtr m_root;
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        MessageBoxW(nullptr, L"Failed to initialize COM", L"Error", MB_OK);
        return 1;
    }

    AdvancedControlsWindow demo;
    if (!demo.Initialize(hInstance, nCmdShow)) {
        CoUninitialize();
        return 1;
    }

    int result = demo.Run();
    CoUninitialize();
    return result;
}
