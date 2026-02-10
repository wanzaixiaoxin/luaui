// 11_style_demo - Simplified Style Demo
// Demonstrates: Button with customizable state colors

#include "Control.h"
#include "layout.h"
#include "IRenderContext.h"
#include "IRenderEngine.h"
#include "Event.h"
#include <windows.h>
#include <windowsx.h>
#include <objbase.h>
#include <string>
#include <sstream>

using namespace luaui;
using namespace luaui::controls;
using namespace luaui::rendering;

class StyleDemoWindow {
public:
    bool Initialize(HINSTANCE hInstance, int nCmdShow) {
        WNDCLASSEXW wcex = {};
        wcex.cbSize = sizeof(WNDCLASSEXW);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WindowProc;
        wcex.hInstance = hInstance;
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszClassName = L"LuaUIStyleDemo";

        if (!RegisterClassExW(&wcex)) return false;

        m_hWnd = CreateWindowExW(
            0, L"LuaUIStyleDemo", L"\u6837\u5f0f\u7cfb\u7edf\u6f14\u793a",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 1000, 750,
            nullptr, nullptr, hInstance, this
        );

        if (!m_hWnd) return false;

        m_engine = CreateRenderEngine();
        if (!m_engine || !m_engine->Initialize()) {
            MessageBoxW(m_hWnd, L"\u6e32\u67d3\u5f15\u64ce\u521d\u59cb\u5316\u5931\u8d25", L"\u9519\u8bef", MB_OK);
            return false;
        }

        RenderTargetDesc desc;
        desc.type = RenderTargetType::Window;
        desc.nativeHandle = m_hWnd;
        desc.width = 1000;
        desc.height = 750;

        if (!m_engine->CreateRenderTarget(desc)) {
            MessageBoxW(m_hWnd, L"\u6e32\u67d3\u76ee\u6807\u521b\u5efa\u5931\u8d25", L"\u9519\u8bef", MB_OK);
            return false;
        }

        CreateDemo();

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
    void CreateDemo() {
        // ========================================
        // Create UI
        // ========================================
        auto root = std::make_shared<StackPanel>();
        root->SetMargin(20, 20, 20, 20);
        root->SetSpacing(10);
        root->SetBackground(Color::White());
        
        // Title
        auto title = CreateText(L"\u6837\u5f0f\u7cfb\u7edf\u6f14\u793a", 24, Color::FromHex(0x333333));
        root->AddChild(title);
        
        auto subtitle = CreateText(L"\u7b80\u5316\u8bbe\u8ba1\uff1a\u6309\u94ae\u81ea\u52a8\u5904\u7406\u60ac\u505c/\u6309\u4e0b\u6548\u679c", 12, Color::FromHex(0x666666));
        subtitle->SetOpacity(0.7f);
        root->AddChild(subtitle);
        
        // Section 1: Default Button (gray colors)
        AddSection(root, L"1. \u9ed8\u8ba4\u6309\u94ae\uff08\u7070\u8272\u7cfb\uff09");
        AddDesc(root, L"\u4e0d\u8bbe\u7f6e\u989c\u8272\uff0c\u4f7f\u7528\u9ed8\u8ba4\u7684\u7070\u8272\u60ac\u505c/\u6309\u4e0b\u6548\u679c");
        
        auto row1 = std::make_shared<StackPanel>();
        row1->SetOrientation(StackPanel::Orientation::Horizontal);
        row1->SetSpacing(10);
        
        auto defaultBtn1 = CreateButton(L"\u9ed8\u8ba4\u6309\u94ae 1");
        auto defaultBtn2 = CreateButton(L"\u9ed8\u8ba4\u6309\u94ae 2");
        
        row1->AddChild(defaultBtn1);
        row1->AddChild(defaultBtn2);
        root->AddChild(row1);
        
        // Section 2: Blue Theme Buttons
        AddSection(root, L"2. \u84dd\u8272\u4e3b\u9898\u6309\u94ae");
        AddDesc(root, L"\u4f7f\u7528 SetStateColors() \u8bbe\u7f6e\u81ea\u5b9a\u4e49\u72b6\u6001\u989c\u8272");
        
        auto row2 = std::make_shared<StackPanel>();
        row2->SetOrientation(StackPanel::Orientation::Horizontal);
        row2->SetSpacing(10);
        
        // Blue buttons
        auto blueBtn1 = CreateButton(L"\u84dd\u8272\u6309\u94ae");
        blueBtn1->SetStateColors(
            Color::FromHex(0x0078D4),   // Normal
            Color::FromHex(0x005A9E),   // Hover
            Color::FromHex(0x004578)    // Pressed
        );
        
        auto blueBtn2 = CreateButton(L"\u53e6\u4e00\u4e2a\u84dd\u8272");
        blueBtn2->SetStateColors(
            Color::FromHex(0x0078D4),
            Color::FromHex(0x005A9E),
            Color::FromHex(0x004578)
        );
        
        row2->AddChild(blueBtn1);
        row2->AddChild(blueBtn2);
        root->AddChild(row2);
        
        // Section 3: Danger (Red) Buttons
        AddSection(root, L"3. \u5371\u9669\uff08\u7ea2\u8272\uff09\u6309\u94ae");
        AddDesc(root, L"\u7ea2\u8272\u4e3b\u9898\u7684\u6309\u94ae\uff0c\u7528\u4e8e\u5371\u9669\u64cd\u4f5c");
        
        auto row3 = std::make_shared<StackPanel>();
        row3->SetOrientation(StackPanel::Orientation::Horizontal);
        row3->SetSpacing(10);
        
        auto dangerBtn = CreateButton(L"\u5220\u9664");
        dangerBtn->SetStateColors(
            Color::FromHex(0xD83B01),   // Normal
            Color::FromHex(0xA52C00),   // Hover
            Color::FromHex(0x7A1F00)    // Pressed
        );
        
        auto dangerBtn2 = CreateButton(L"\u6e05\u7a7a");
        dangerBtn2->SetStateColors(
            Color::FromHex(0xD83B01),
            Color::FromHex(0xA52C00),
            Color::FromHex(0x7A1F00)
        );
        
        row3->AddChild(dangerBtn);
        row3->AddChild(dangerBtn2);
        root->AddChild(row3);
        
        // Section 4: Success (Green) Buttons
        AddSection(root, L"4. \u6210\u529f\uff08\u7eff\u8272\uff09\u6309\u94ae");
        AddDesc(root, L"\u7eff\u8272\u4e3b\u9898\u7684\u6309\u94ae\uff0c\u7528\u4e8e\u786e\u8ba4\u64cd\u4f5c");
        
        auto row4 = std::make_shared<StackPanel>();
        row4->SetOrientation(StackPanel::Orientation::Horizontal);
        row4->SetSpacing(10);
        
        auto successBtn = CreateButton(L"\u4fdd\u5b58");
        successBtn->SetStateColors(
            Color::FromHex(0x107C10),   // Normal
            Color::FromHex(0x0B5E0B),   // Hover
            Color::FromHex(0x074007)    // Pressed
        );
        
        auto successBtn2 = CreateButton(L"\u63d0\u4ea4");
        successBtn2->SetStateColors(
            Color::FromHex(0x107C10),
            Color::FromHex(0x0B5E0B),
            Color::FromHex(0x074007)
        );
        
        row4->AddChild(successBtn);
        row4->AddChild(successBtn2);
        root->AddChild(row4);
        
        // Section 5: Dynamic Color Change
        AddSection(root, L"5. \u52a8\u6001\u5207\u6362\u989c\u8272");
        AddDesc(root, L"\u70b9\u51fb\u6309\u94ae\u5207\u6362\u4e3b\u9898\u989c\u8272");
        
        auto row5 = std::make_shared<StackPanel>();
        row5->SetOrientation(StackPanel::Orientation::Horizontal);
        row5->SetSpacing(10);
        
        auto dynamicBtn = CreateButton(L"\u70b9\u6211\u5207\u6362\u989c\u8272");
        auto isBlue = std::make_shared<bool>(true);
        dynamicBtn->AddClickHandler([dynamicBtn, isBlue](Control*) {
            if (*isBlue) {
                dynamicBtn->SetStateColors(
                    Color::FromHex(0x9C27B0),   // Purple
                    Color::FromHex(0x7B1FA2),
                    Color::FromHex(0x4A148C)
                );
            } else {
                dynamicBtn->SetStateColors(
                    Color::FromHex(0x0078D4),   // Blue
                    Color::FromHex(0x005A9E),
                    Color::FromHex(0x004578)
                );
            }
            *isBlue = !*isBlue;
        });
        
        row5->AddChild(dynamicBtn);
        root->AddChild(row5);
        
        m_root = root;
    }
    
    std::shared_ptr<Button> CreateButton(const std::wstring& text) {
        auto btn = std::make_shared<Button>();
        btn->SetPadding(20, 10, 20, 10);
        
        auto tb = std::make_shared<TextBlock>();
        tb->SetText(text);
        tb->SetForeground(Color::White());
        tb->SetFontSize(13);
        btn->SetContent(tb);
        
        return btn;
    }
    
    std::shared_ptr<TextBlock> CreateText(const std::wstring& text, int size, const Color& color) {
        auto tb = std::make_shared<TextBlock>();
        tb->SetText(text);
        tb->SetFontSize(size);
        tb->SetForeground(color);
        return tb;
    }
    
    void AddSection(std::shared_ptr<StackPanel> parent, const std::wstring& text) {
        auto tb = std::make_shared<TextBlock>();
        tb->SetText(text);
        tb->SetFontSize(16);
        tb->SetForeground(Color::FromHex(0x333333));
        tb->SetMargin(0, 20, 0, 5);
        parent->AddChild(tb);
    }
    
    void AddDesc(std::shared_ptr<StackPanel> parent, const std::wstring& text) {
        auto tb = std::make_shared<TextBlock>();
        tb->SetText(text);
        tb->SetFontSize(11);
        tb->SetForeground(Color::FromHex(0x666666));
        tb->SetOpacity(0.8f);
        tb->SetMargin(0, 0, 0, 10);
        parent->AddChild(tb);
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
            m_root->Arrange(Rect(0, 0, width, height));
            m_root->Render(context);
        }

        m_engine->Present();
    }

    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        StyleDemoWindow* pThis = nullptr;

        if (message == WM_NCCREATE) {
            CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            pThis = reinterpret_cast<StyleDemoWindow*>(pCreate->lpCreateParams);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
            pThis->m_hWnd = hWnd;
        } else {
            pThis = reinterpret_cast<StyleDemoWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
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
                    if (pThis->m_root) {
                        float x = static_cast<float>(GET_X_LPARAM(lParam));
                        float y = static_cast<float>(GET_Y_LPARAM(lParam));
                        
                        auto hit = pThis->m_root->HitTestPoint(Point(x, y));
                        
                        // Track previous hover target
                        static ControlPtr s_prevHover = nullptr;
                        
                        if (auto btn = std::dynamic_pointer_cast<Button>(hit)) {
                            if (btn != s_prevHover) {
                                if (s_prevHover) s_prevHover->OnMouseLeave();
                                btn->OnMouseEnter();
                                s_prevHover = btn;
                                InvalidateRect(hWnd, nullptr, FALSE);
                            }
                        } else {
                            if (s_prevHover) {
                                s_prevHover->OnMouseLeave();
                                s_prevHover = nullptr;
                                InvalidateRect(hWnd, nullptr, FALSE);
                            }
                        }
                    }
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
                            if (auto btn = std::dynamic_pointer_cast<Button>(hit)) {
                                btn->RaiseClick();
                            }
                        }
                    }
                    InvalidateRect(hWnd, nullptr, FALSE);
                    return 0;
                }
                case WM_LBUTTONUP: {
                    float x = static_cast<float>(GET_X_LPARAM(lParam));
                    float y = static_cast<float>(GET_Y_LPARAM(lParam));
                    
                    MouseEventArgs args(x, y);
                    if (pThis->m_root) {
                        auto hit = pThis->m_root->HitTestPoint(Point(x, y));
                        if (hit) {
                            hit->OnMouseUp(args);
                        }
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
    ControlPtr m_root;
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        MessageBoxW(nullptr, L"COM \u521d\u59cb\u5316\u5931\u8d25", L"\u9519\u8bef", MB_OK);
        return 1;
    }

    StyleDemoWindow demo;
    if (!demo.Initialize(hInstance, nCmdShow)) {
        CoUninitialize();
        return 1;
    }

    int result = demo.Run();
    CoUninitialize();
    return result;
}
