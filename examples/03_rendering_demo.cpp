// Rendering Demo - Shows Direct2D rendering engine usage
#include <windows.h>
#include "luaui/rendering/IRenderEngine.h"
#include "luaui/rendering/IRenderContext.h"
#include <iostream>

using namespace luaui::rendering;

class RenderingDemo {
public:
    bool Initialize(HWND hwnd) {
        // Create render engine
        m_engine = CreateRenderEngine();
        if (!m_engine->Initialize(RenderAPI::Direct2D)) {
            std::cerr << "Failed to initialize render engine" << std::endl;
            return false;
        }
        
        // Create render target
        RenderTargetDesc desc;
        desc.type = RenderTargetType::Window;
        desc.nativeHandle = hwnd;
        desc.width = 800;
        desc.height = 600;
        
        if (!m_engine->CreateRenderTarget(desc)) {
            std::cerr << "Failed to create render target" << std::endl;
            return false;
        }
        
        // Create resources
        auto context = m_engine->GetContext();
        m_redBrush = context->CreateSolidColorBrush(Color(0.8f, 0.2f, 0.2f));
        m_greenBrush = context->CreateSolidColorBrush(Color(0.2f, 0.8f, 0.2f));
        m_blueBrush = context->CreateSolidColorBrush(Color(0.2f, 0.2f, 0.8f));
        m_gradientBrush = context->CreateLinearGradientBrush(
            Point(100, 100), Point(300, 300),
            {
                GradientStop(Color(1, 0, 0), 0.0f),
                GradientStop(Color(0, 1, 0), 0.5f),
                GradientStop(Color(0, 0, 1), 1.0f)
            }
        );
        
        return true;
    }
    
    void Shutdown() {
        m_engine->Shutdown();
    }
    
    void OnResize(int width, int height) {
        m_engine->ResizeRenderTarget(width, height);
    }
    
    void Render() {
        auto context = m_engine->GetContext();
        
        m_engine->BeginFrame();
        
        // Clear background
        context->Clear(Color(0.1f, 0.1f, 0.15f));
        
        // Draw rectangles
        context->FillRectangle(Rect(50, 50, 100, 100), m_redBrush.get());
        context->DrawRectangle(Rect(200, 50, 100, 100), m_greenBrush.get(), 3.0f);
        
        // Draw gradient
        context->FillRectangle(Rect(350, 50, 200, 200), m_gradientBrush.get());
        
        // Draw circles
        context->FillCircle(Point(150, 300), 50, m_blueBrush.get());
        context->DrawCircle(Point(350, 300), 80, m_redBrush.get(), 2.0f);
        
        // Draw ellipse
        context->FillEllipse(Point(550, 300), 100, 60, m_greenBrush.get());
        
        // Draw rounded rectangle
        context->FillRoundedRectangle(Rect(50, 400, 200, 100), CornerRadius(20), m_blueBrush.get());
        context->DrawRoundedRectangle(Rect(300, 400, 200, 100), CornerRadius(30), 
                                       m_redBrush.get(), 3.0f);
        
        // Draw lines
        context->DrawLine(Point(550, 400), Point(700, 500), m_greenBrush.get(), 2.0f);
        context->DrawLine(Point(550, 500), Point(700, 400), m_blueBrush.get(), 4.0f);
        
        m_engine->Present();
    }
    
private:
    std::unique_ptr<IRenderEngine> m_engine;
    ISolidColorBrushPtr m_redBrush;
    ISolidColorBrushPtr m_greenBrush;
    ISolidColorBrushPtr m_blueBrush;
    ILinearGradientBrushPtr m_gradientBrush;
};

// Global demo instance
static RenderingDemo g_demo;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        if (!g_demo.Initialize(hwnd)) {
            return -1;
        }
        return 0;
        
    case WM_DESTROY:
        g_demo.Shutdown();
        PostQuitMessage(0);
        return 0;
        
    case WM_SIZE:
        g_demo.OnResize(LOWORD(lParam), HIWORD(lParam));
        return 0;
        
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            g_demo.Render();
            EndPaint(hwnd, &ps);
        }
        return 0;
        
    case WM_ERASEBKGND:
        return 1; // Prevent flicker
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    // Register window class
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"LuaUIRenderingDemo";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr; // We handle background in render
    
    if (!RegisterClassEx(&wc)) {
        std::cerr << "Failed to register window class" << std::endl;
        return 1;
    }
    
    // Create window
    HWND hwnd = CreateWindowEx(
        0, L"LuaUIRenderingDemo", L"LuaUI Rendering Demo - Direct2D",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME, // Fixed size
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        nullptr, nullptr, hInstance, nullptr
    );
    
    if (!hwnd) {
        std::cerr << "Failed to create window" << std::endl;
        return 1;
    }
    
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    // Message loop
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}
