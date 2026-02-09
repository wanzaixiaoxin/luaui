// Offscreen Rendering Demo - Shows render-to-texture functionality
#include <windows.h>
#include <objbase.h>  // For COM functions
#include "luaui/rendering/IRenderEngine.h"
#include "luaui/rendering/IRenderTarget.h"
#include "Logger.h"

using namespace luaui::rendering;
using namespace luaui::utils;

class OffscreenDemo {
public:
    bool Initialize(HWND hwnd) {
        Logger::Info("Initializing offscreen rendering demo...");
        
        m_engine = CreateRenderEngine();
        if (!m_engine->Initialize(RenderAPI::Direct2D)) {
            Logger::Error("Failed to initialize render engine");
            return false;
        }
        
        RenderTargetDesc desc;
        desc.type = RenderTargetType::Window;
        desc.nativeHandle = hwnd;
        desc.width = 800;
        desc.height = 600;
        
        if (!m_engine->CreateRenderTarget(desc)) {
            Logger::Error("Failed to create render target");
            return false;
        }
        
        // Create offscreen render target (256x256)
        m_offscreenTarget = m_engine->CreateRenderTarget(256, 256, true);
        if (!m_offscreenTarget) {
            Logger::Error("Failed to create offscreen render target");
            return false;
        }
        Logger::Info("Offscreen render target created: 256x256");
        
        // Create brushes
        auto context = m_engine->GetContext();
        m_redBrush = context->CreateSolidColorBrush(Color(0.9f, 0.2f, 0.2f));
        m_greenBrush = context->CreateSolidColorBrush(Color(0.2f, 0.9f, 0.2f));
        m_blueBrush = context->CreateSolidColorBrush(Color(0.2f, 0.2f, 0.9f));
        m_whiteBrush = context->CreateSolidColorBrush(Color::White());
        
        // Create gradient for offscreen content
        m_gradientBrush = context->CreateLinearGradientBrush(
            Point(0, 0), Point(256, 256),
            {
                GradientStop(Color(1.0f, 0.0f, 0.0f), 0.0f),
                GradientStop(Color(0.0f, 1.0f, 0.0f), 0.5f),
                GradientStop(Color(0.0f, 0.0f, 1.0f), 1.0f)
            }
        );
        
        Logger::Info("Offscreen demo initialized");
        return true;
    }
    
    void Shutdown() {
        Logger::Info("Shutting down offscreen demo...");
        m_offscreenTarget.reset();
        m_engine->Shutdown();
    }
    
    void RenderToOffscreen() {
        if (!m_offscreenTarget) return;
        
        // Begin drawing to offscreen target
        if (!m_offscreenTarget->BeginDraw()) {
            Logger::Error("Failed to begin offscreen draw");
            return;
        }
        
        auto context = m_offscreenTarget->GetContext();
        
        // Clear with transparent background
        m_offscreenTarget->Clear(Color(0.0f, 0.0f, 0.0f, 0.0f));
        
        // Draw gradient background
        context->FillRectangle(Rect(0, 0, 256, 256), m_gradientBrush.get());
        
        // Draw some shapes
        context->FillCircle(Point(128, 128), 60.0f, m_whiteBrush.get());
        context->DrawCircle(Point(128, 128), 80.0f, m_blueBrush.get(), 4.0f);
        
        // Draw pattern
        for (int i = 0; i < 4; i++) {
            float x = 32.0f + i * 64.0f;
            context->FillRectangle(Rect(x, 200.0f, 32.0f, 32.0f), m_redBrush.get());
        }
        
        // End drawing
        if (!m_offscreenTarget->EndDraw()) {
            Logger::Error("Failed to end offscreen draw");
            return;
        }
        
        Logger::Debug("Offscreen rendering complete");
    }
    
    void SaveScreenshot() {
        if (!m_offscreenTarget) return;
        
        Logger::Info("Saving screenshot...");
        if (m_offscreenTarget->SaveToFile(L"offscreen_screenshot.png")) {
            Logger::Info("Screenshot saved: offscreen_screenshot.png");
        } else {
            Logger::Error("Failed to save screenshot");
        }
    }
    
    void Render() {
        // First, render to offscreen target
        RenderToOffscreen();
        
        // Get the offscreen bitmap
        IBitmapPtr offscreenBitmap = m_offscreenTarget->ToBitmap();
        if (!offscreenBitmap) {
            Logger::Error("Failed to get offscreen bitmap");
        }
        
        // Now render to main window
        auto context = m_engine->GetContext();
        
        m_engine->BeginFrame();
        context->Clear(Color(0.1f, 0.1f, 0.15f));
        
        // Draw main content
        context->FillRectangle(Rect(50, 50, 200, 200), m_redBrush.get());
        context->FillRectangle(Rect(300, 50, 200, 200), m_greenBrush.get());
        context->FillRectangle(Rect(550, 50, 200, 200), m_blueBrush.get());
        
        // Draw offscreen texture (if available)
        if (offscreenBitmap) {
            // Draw in multiple places to show it's a texture
            context->DrawBitmap(offscreenBitmap.get(), Rect(100, 300, 150, 150), 1.0f);
            context->DrawBitmap(offscreenBitmap.get(), Rect(300, 300, 200, 200), 0.8f);
            context->DrawBitmap(offscreenBitmap.get(), Rect(550, 300, 100, 100), 0.6f);
            
            // Draw with partial source rect
            context->DrawBitmap(
                offscreenBitmap.get(), 
                Rect(100, 520, 128, 64),
                Rect(64, 96, 128, 64),
                1.0f
            );
        }
        
        // Draw labels
        auto textFormat = context->CreateTextFormat(L"Arial", 14.0f);
        auto whiteBrush = context->CreateSolidColorBrush(Color::White());
        
        context->DrawTextString(L"Main Render", textFormat.get(), Point(100, 260), whiteBrush.get());
        context->DrawTextString(L"Offscreen Texture (various sizes)", textFormat.get(), Point(300, 520), whiteBrush.get());
        
        m_engine->Present();
    }
    
    void OnResize(int width, int height) {
        m_engine->ResizeRenderTarget(width, height);
    }
    
private:
    std::unique_ptr<IRenderEngine> m_engine;
    IRenderTargetPtr m_offscreenTarget;
    
    ISolidColorBrushPtr m_redBrush;
    ISolidColorBrushPtr m_greenBrush;
    ISolidColorBrushPtr m_blueBrush;
    ISolidColorBrushPtr m_whiteBrush;
    ILinearGradientBrushPtr m_gradientBrush;
};

static OffscreenDemo g_demo;

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
        
    case WM_PAINT: {
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);
        g_demo.Render();
        EndPaint(hwnd, &ps);
        return 0;
    }
    
    case WM_KEYDOWN:
        if (wParam == VK_SPACE) {
            // Save screenshot on spacebar
            g_demo.SaveScreenshot();
        }
        return 0;
        
    case WM_ERASEBKGND:
        return 1;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        return 1;
    }
    
    LoggerConfig config;
    config.consoleEnabled = true;
    config.createConsoleWindow = true;
    config.consoleWindowTitle = "Offscreen Demo - Debug Console";
    Logger::Initialize(config);
    Logger::Info("=== Offscreen Rendering Demo Starting ===");
    Logger::Info("Press SPACE to save screenshot");
    
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"OffscreenDemo";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    
    if (!RegisterClassEx(&wc)) {
        Logger::Error("Failed to register window class");
        CoUninitialize();
        return 1;
    }
    
    HWND hwnd = CreateWindowEx(
        0, L"OffscreenDemo", L"Offscreen Rendering Demo - Render to Texture",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        nullptr, nullptr, hInstance, nullptr
    );
    
    if (!hwnd) {
        Logger::Error("Failed to create window");
        CoUninitialize();
        return 1;
    }
    
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    Logger::Shutdown();
    CoUninitialize();
    return (int)msg.wParam;
}
