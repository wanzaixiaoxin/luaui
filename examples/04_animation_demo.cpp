// Animation Demo - Shows animation system usage
#include <windows.h>
#include <objbase.h>  // For COM functions
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
        
        // Create animation timeline
        m_timeline = CreateAnimationTimeline();
        
        // Create brushes
        auto context = m_engine->GetContext();
        m_redBrush = context->CreateSolidColorBrush(Color(0.9f, 0.2f, 0.2f));
        m_greenBrush = context->CreateSolidColorBrush(Color(0.2f, 0.9f, 0.2f));
        m_blueBrush = context->CreateSolidColorBrush(Color(0.2f, 0.2f, 0.9f));
        m_yellowBrush = context->CreateSolidColorBrush(Color(0.9f, 0.9f, 0.2f));
        m_whiteBrush = context->CreateSolidColorBrush(Color::White());
        
        // Setup animations
        SetupAnimations();
        
        Logger::Info("Animation demo initialized successfully");
        return true;
    }
    
    void Shutdown() {
        Logger::Info("Shutting down animation demo...");
        m_timeline.reset();
        m_engine->Shutdown();
    }
    
    void Update(float deltaTimeMs) {
        if (m_timeline) {
            m_timeline->Update(deltaTimeMs);
        }
    }
    
    void Render() {
        auto context = m_engine->GetContext();
        
        m_engine->BeginFrame();
        context->Clear(Color(0.1f, 0.1f, 0.15f));
        
        // Draw animated rectangles
        float boxSize = 60.0f;
        
        // Bouncing red box
        context->FillRectangle(
            Rect(m_bounceX - boxSize/2, 100.0f, boxSize, boxSize), 
            m_redBrush.get()
        );
        
        // Oscillating green box
        float oscillateX = 400.0f + m_oscillateOffset;
        context->FillRectangle(
            Rect(oscillateX - boxSize/2, 200.0f, boxSize, boxSize),
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
            Rect(350.0f - boxSize, 400.0f, boxSize * 2.0f, boxSize),
            fadeBrush.get()
        );
        
        // Draw labels
        auto textFormat = context->CreateTextFormat(L"Arial", 16.0f);
        
        context->DrawTextString(L"Bounce (Quad)", textFormat.get(), Point(50.0f, 170.0f), m_whiteBrush.get());
        context->DrawTextString(L"Elastic", textFormat.get(), Point(370.0f, 170.0f), m_whiteBrush.get());
        context->DrawTextString(L"Pulse (Sine)", textFormat.get(), Point(620.0f, 170.0f), m_whiteBrush.get());
        context->DrawTextString(L"Fade (Loop)", textFormat.get(), Point(360.0f, 470.0f), m_whiteBrush.get());
        
        // Draw instructions
        context->DrawTextString(L"Press SPACE to pause/resume, R to restart", textFormat.get(), Point(250.0f, 550.0f), m_whiteBrush.get());
        
        m_engine->Present();
    }
    
    void OnResize(int width, int height) {
        m_engine->ResizeRenderTarget(width, height);
    }
    
    void TogglePause() {
        static bool paused = false;
        paused = !paused;
        if (paused) {
            m_timeline->PauseAll();
            Logger::Info("Animations paused");
        } else {
            m_timeline->ResumeAll();
            Logger::Info("Animations resumed");
        }
    }
    
    void Restart() {
        SetupAnimations();
        Logger::Info("Animations restarted");
    }
    
private:
    void SetupAnimations() {
        // Clear existing animations
        m_timeline = CreateAnimationTimeline();
        
        // Bounce animation (left to right)
        auto bounceAnim = m_timeline->CreateAnimation();
        bounceAnim->SetDuration(2000.0f);  // 2 seconds
        bounceAnim->SetEasing(Easing::QuadInOut);
        bounceAnim->SetIterations(-1);  // Infinite
        bounceAnim->SetDirection(AnimationDirection::Alternate);
        bounceAnim->SetStartValue(AnimationValue(50.0f));
        bounceAnim->SetEndValue(AnimationValue(750.0f));
        bounceAnim->SetUpdateCallback([this](const AnimationValue& v) {
            m_bounceX = v.AsFloat();
        });
        bounceAnim->Play();
        m_timeline->Add(std::move(bounceAnim));
        
        // Oscillate animation (elastic)
        auto oscillateAnim = m_timeline->CreateAnimation();
        oscillateAnim->SetDuration(1500.0f);
        oscillateAnim->SetEasing(Easing::ElasticOut);
        oscillateAnim->SetIterations(-1);
        oscillateAnim->SetDirection(AnimationDirection::Alternate);
        oscillateAnim->SetStartValue(AnimationValue(-100.0f));
        oscillateAnim->SetEndValue(AnimationValue(100.0f));
        oscillateAnim->SetUpdateCallback([this](const AnimationValue& v) {
            m_oscillateOffset = v.AsFloat();
        });
        oscillateAnim->Play();
        m_timeline->Add(std::move(oscillateAnim));
        
        // Pulse animation (sine wave)
        auto pulseAnim = m_timeline->CreateAnimation();
        pulseAnim->SetDuration(1000.0f);
        pulseAnim->SetEasing(Easing::SineInOut);
        pulseAnim->SetIterations(-1);
        pulseAnim->SetDirection(AnimationDirection::Alternate);
        pulseAnim->SetStartValue(AnimationValue(0.5f));
        pulseAnim->SetEndValue(AnimationValue(1.5f));
        pulseAnim->SetUpdateCallback([this](const AnimationValue& v) {
            m_pulseScale = v.AsFloat();
        });
        pulseAnim->Play();
        m_timeline->Add(std::move(pulseAnim));
        
        // Fade animation (loop)
        auto fadeAnim = m_timeline->CreateAnimation();
        fadeAnim->SetDuration(2000.0f);
        fadeAnim->SetEasing(Easing::QuadInOut);
        fadeAnim->SetIterations(-1);
        fadeAnim->SetDirection(AnimationDirection::Alternate);
        fadeAnim->SetStartValue(AnimationValue(0.2f));
        fadeAnim->SetEndValue(AnimationValue(1.0f));
        fadeAnim->SetUpdateCallback([this](const AnimationValue& v) {
            m_fadeAlpha = v.AsFloat();
        });
        fadeAnim->Play();
        m_timeline->Add(std::move(fadeAnim));
    }
    
    std::unique_ptr<IRenderEngine> m_engine;
    IAnimationTimelinePtr m_timeline;
    
    ISolidColorBrushPtr m_redBrush;
    ISolidColorBrushPtr m_greenBrush;
    ISolidColorBrushPtr m_blueBrush;
    ISolidColorBrushPtr m_yellowBrush;
    ISolidColorBrushPtr m_whiteBrush;
    
    // Animated values
    float m_bounceX = 50.0f;
    float m_oscillateOffset = 0.0f;
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
        
    case WM_KEYDOWN:
        if (wParam == VK_SPACE) {
            g_demo.TogglePause();
        } else if (wParam == 'R') {
            g_demo.Restart();
        }
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
    Logger::Info("Controls: SPACE = pause/resume, R = restart");
    
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
        0, L"AnimationDemo", L"Animation Demo - Press SPACE to pause/resume, R to restart",
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
            UpdateWindow(hwnd);
            
            // Cap at ~60 FPS
            Sleep(16);
        }
    }
    
    Logger::Shutdown();
    CoUninitialize();
    return (int)msg.wParam;
}
