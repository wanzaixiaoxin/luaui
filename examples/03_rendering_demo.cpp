// Rendering Demo - Shows Direct2D rendering engine usage
#include <windows.h>
#include "luaui/rendering/IRenderEngine.h"
#include "luaui/rendering/IRenderContext.h"
#include "Logger.h"
#include <iostream>
#include <fstream>

using namespace luaui::rendering;
using namespace luaui::utils;

class RenderingDemo {
public:
    bool Initialize(HWND hwnd) {
        Logger::Info("Initializing rendering demo...");
        
        // Create render engine
        m_engine = CreateRenderEngine();
        if (!m_engine) {
            Logger::Error("Failed to create render engine");
            return false;
        }
        Logger::Info("Render engine created");
        
        if (!m_engine->Initialize(RenderAPI::Direct2D)) {
            Logger::Error("Failed to initialize render engine");
            return false;
        }
        Logger::Info("Render engine initialized");
        
        // Create render target
        RenderTargetDesc desc;
        desc.type = RenderTargetType::Window;
        desc.nativeHandle = hwnd;
        desc.width = 800;
        desc.height = 600;
        
        if (!m_engine->CreateRenderTarget(desc)) {
            Logger::Error("Failed to create render target");
            return false;
        }
        Logger::Info("Render target created");
        
        // Create resources
        auto context = m_engine->GetContext();
        if (!context) {
            Logger::Error("Failed to get render context");
            return false;
        }
        Logger::Info("Got render context");
        
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
        
        Logger::Info("Resources created successfully");
        return true;
    }
    
    void Shutdown() {
        Logger::Info("Shutting down demo...");
        m_engine->Shutdown();
        Logger::Info("Demo shutdown complete");
    }
    
    void OnResize(int width, int height) {
        Logger::InfoF("Window resized to %dx%d", width, height);
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
            Logger::Error("Failed to initialize demo in WM_CREATE");
            return -1;
        }
        return 0;
        
    case WM_DESTROY:
        Logger::Info("WM_DESTROY received");
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
    // Initialize COM for WIC
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        MessageBoxA(nullptr, "Failed to initialize COM", "Error", MB_OK);
        return 1;
    }
    
    // Initialize logger with both console and file output
    LoggerConfig config;
    config.consoleEnabled = true;       // Enable console output
    config.fileEnabled = true;          // Enable file output
    config.consoleLevel = LogLevel::Debug;   // Console shows all messages
    config.fileLevel = LogLevel::Info;       // File only shows Info and above
    config.logFilePath = "rendering_demo.log";
    config.consoleColored = true;       // Enable colored console output
    config.useStderr = false;           // Use stdout for console
    
    // For GUI applications: create a separate console window for debug output
    // This allows seeing logs even in GUI mode (WIN32 subsystem)
    config.createConsoleWindow = true;
    config.consoleWindowTitle = "LuaUI Rendering Demo - Debug Console";
    
    Logger::Initialize(config);
    Logger::Info("=== Rendering Demo Starting ===");
    
    // Demonstrate different log levels with colors
    Logger::Debug("This is a DEBUG message (gray)");
    Logger::Info("This is an INFO message (white)");
    Logger::Warning("This is a WARNING message (yellow)");
    Logger::Error("This is an ERROR message (red)");
    
    // Demonstrate formatted logging
    Logger::InfoF("Application started with PID: %d", GetCurrentProcessId());
    
    // The console window will automatically close when the main process exits
    // because ConsoleLogger owns it and will close it in its destructor
    
    // Register window class
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"LuaUIRenderingDemo";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr; // We handle background in render
    
    if (!RegisterClassEx(&wc)) {
        Logger::Error("Failed to register window class");
        CoUninitialize();
        return 1;
    }
    Logger::Info("Window class registered");
    
    // Create window
    HWND hwnd = CreateWindowEx(
        0, L"LuaUIRenderingDemo", L"LuaUI Rendering Demo - Direct2D",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME, // Fixed size
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        nullptr, nullptr, hInstance, nullptr
    );
    
    if (!hwnd) {
        DWORD error = GetLastError();
        Logger::ErrorF("Failed to create window, error: %lu", error);
        CoUninitialize();
        return 1;
    }
    Logger::Info("Window created successfully");
    
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    SetForegroundWindow(hwnd);
    SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
    Logger::Info("Window shown");
    
    // Message loop
    Logger::Info("Entering message loop");
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    Logger::Info("Message loop ended");
    
    // Shutdown
    Logger::Shutdown();
    CoUninitialize();
    
    return (int)msg.wParam;
}
