// Animation Demo - Shows animation system usage
#include <windows.h>
#include "luaui/rendering/IRenderEngine.h"
#include "luaui/rendering/IAnimation.h"
#include "Logger.h"
#include <cmath>

using namespace luaui::rendering;
using namespace luaui::utils;

class AnimationDemo {
public:
    bool Initialize(HWND hwnd) {
        Logger::Info("Initializing animation demo...");
        
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
        
        // Create resources
        auto context = m_engine->GetContext();
        m_redBrush = context->CreateSolidColorBrush(Color(0.9f, 0.2f, 0.2f));
        m_greenBrush = context->CreateSolidColorBrush(Color(0.2f, 0.9f, 0.2f));
        m_blueBrush = context->CreateSolidColorBrush(Color(0.2f, 0.2f, 0.9f));
        m_yellowBrush = context->CreateSolidColorBrush(Color(0.9f, 0.9f, 0.2f));
        
        Logger::Info("Animation demo initialized successfully");
        return true;
    }
    
    void Shutdown() {
        Logger::Info("Shutting down animation demo...");
        m_engine->Shutdown();
    }
    
    void Update(float deltaTimeMs) {
        // Update animation values manually
        m_time += deltaTimeMs / 1000.0f; // Convert to seconds
        
        // Bounce animation (sine wave)
        m_bounceX = 375.0f + std::sin(m_time * 2.0f) * 300.0f;
        
        // Pulse animation
        m_pulseScale = 1.0f + std::sin(m_time * 4.0f) * 0.3f;
        
        // Fade animation
        m_fadeAlpha = 0.5f + std::sin(m_time * 3.0f) * 0.5f;
        
        // Rotate animation (using position)
        m_rotateOffset = std::sin(m_time * 2.5f) * 100.0f;
    }
    
    void Render() {
        auto context = m_engine->GetContext();
        
        m_engine->BeginFrame();
        context->Clear(Color(0.1f, 0.1f, 0.15f));
        
        // Draw animated rectangles
        float boxSize = 60.0f;
        
        // Bouncing red box (horizontal movement)
        context->FillRectangle(
            Rect(m_bounceX - boxSize/2, 100.0f, boxSize, boxSize), 
            m_redBrush.get()
        );
        
        // Rotating green box (simulated with position)
        float rotateX = 400.0f + m_rotateOffset;
        context->FillRectangle(
            Rect(rotateX - boxSize/2, 200.0f, boxSize, boxSize),
            m_greenBrush.get()
        );
        
        // Pulsing blue box
        float pulseSize = boxSize * m_pulseScale;
        float pulseX = 650.0f - pulseSize / 2.0f;
        float pulseY = 250.0f - pulseSize / 2.0f;
        context->FillRectangle(
            Rect(pulseX, pulseY, pulseSize, pulseSize),
            m_blueBrush.get()
        );
        
        // Fading yellow box
        auto fadeBrush = m_engine->GetContext()->CreateSolidColorBrush(
            Color(0.9f, 0.9f, 0.2f, m_fadeAlpha)
        );
        context->FillRectangle(
            Rect(350.0f, 400.0f, boxSize * 2.0f, boxSize),
            fadeBrush.get()
        );
        
        // Draw labels
        auto textFormat = context->CreateTextFormat(L"Arial", 16.0f);
        auto whiteBrush = context->CreateSolidColorBrush(Color::White());
        
        context->DrawTextString(L"Bounce (Sine)", textFormat.get(), Point(50.0f, 170.0f), whiteBrush.get());
        context->DrawTextString(L"Oscillate", textFormat.get(), Point(370.0f, 170.0f), whiteBrush.get());
        context->DrawTextString(L"Pulse (Sine)", textFormat.get(), Point(620.0f, 170.0f), whiteBrush.get());
        context->DrawTextString(L"Fade", textFormat.get(), Point(380.0f, 470.0f), whiteBrush.get());
        
        m_engine->Present();
    }
    
    void OnResize(int width, int height) {
        m_engine->ResizeRenderTarget(width, height);
    }
    
private:
    std::unique_ptr<IRenderEngine> m_engine;
    
    ISolidColorBrushPtr m_redBrush;
    ISolidColorBrushPtr m_greenBrush;
    ISolidColorBrushPtr m_blueBrush;
    ISolidColorBrushPtr m_yellowBrush;
    
    // Animation state
    float m_time = 0.0f;
    float m_bounceX = 375.0f;
    float m_rotateOffset = 0.0f;
    float m_pulseScale = 1.0f;
    float m_fadeAlpha = 1.0f;
};

static AnimationDemo g_demo;

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
    config.consoleWindowTitle = "Animation Demo - Debug Console";
    Logger::Initialize(config);
    Logger::Info("=== Animation Demo Starting ===");
    
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"AnimationDemo";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    
    if (!RegisterClassEx(&wc)) {
        Logger::Error("Failed to register window class");
        CoUninitialize();
        return 1;
    }
    
    HWND hwnd = CreateWindowEx(
        0, L"AnimationDemo", L"Animation Demo - Sine Wave Animations",
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
    
    // Message loop with animation timer
    MSG msg = {};
    auto lastTime = std::chrono::high_resolution_clock::now();
    
    while (true) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            // Calculate delta time
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaMs = std::chrono::duration<float, std::milli>(currentTime - lastTime).count();
            lastTime = currentTime;
            
            // Update animations
            g_demo.Update(deltaMs);
            
            // Render
            InvalidateRect(hwnd, nullptr, FALSE);
            
            // Cap at ~60 FPS
            Sleep(16);
        }
    }
    
    Logger::Shutdown();
    CoUninitialize();
    return (int)msg.wParam;
}
