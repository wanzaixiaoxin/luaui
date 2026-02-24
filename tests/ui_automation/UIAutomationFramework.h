// LuaUI Automation Testing Framework
// A comprehensive UI automation testing solution for LuaUI framework

#pragma once

#include <luaui.h>
#include "mvvm/ViewModelBase.h"
#include <sstream>
#include "../TestFramework.h"
#include <functional>
#include <filesystem>
#include <vector>
#include <memory>
#include <string>

// Forward declarations
namespace luaui {
    class Window;
    class Control;
    namespace controls {
        class Panel;
    }
}

namespace luaui {
namespace test {
namespace automation {

// ============================================================================
// Test Configuration
// ============================================================================
struct AutomationConfig {
    // Screenshot settings
    std::filesystem::path screenshotBaselinePath = "tests/ui_automation/baselines";
    std::filesystem::path screenshotOutputPath = "tests/ui_automation/output";
    float pixelDiffThreshold = 0.01f;  // 1% pixel difference threshold
    
    // Timing settings
    int defaultTimeoutMs = 5000;       // Default wait timeout
    int animationWaitMs = 300;         // Wait time for animations
    int inputDelayMs = 50;             // Delay between inputs
    
    // Window settings
    int testWindowWidth = 800;
    int testWindowHeight = 600;
    bool headlessMode = true;          // Run without visible window (offscreen rendering)
};

// ============================================================================
// Pixel Comparison Result
// ============================================================================
struct PixelComparisonResult {
    bool matches = false;
    float diffPercentage = 0.0f;
    int diffPixelCount = 0;
    int totalPixels = 0;
    std::filesystem::path diffImagePath;  // Path to diff visualization
    
    std::string ToString() const {
        std::stringstream ss;
        ss << "PixelComparison: " << (matches ? "MATCH" : "MISMATCH")
           << " | Diff: " << diffPercentage * 100.0f << "%"
           << " | Pixels: " << diffPixelCount << "/" << totalPixels;
        return ss.str();
    }
};

// ============================================================================
// Screenshot Capture Interface
// ============================================================================
class IScreenshotCapture {
public:
    virtual ~IScreenshotCapture() = default;
    
    // Initialize the capture system
    virtual bool Initialize() { return true; }
    
    // Shutdown the capture system
    virtual void Shutdown() {}
    
    // Capture entire window
    virtual bool CaptureWindow(Window* window, const std::filesystem::path& outputPath) = 0;
    
    // Capture specific control region
    virtual bool CaptureControl(Control* control, const std::filesystem::path& outputPath) = 0;
    
    // Capture specific rectangle region
    virtual bool CaptureRegion(const luaui::rendering::Rect& region, const std::filesystem::path& outputPath) = 0;
    
    // Get pixel data for in-memory comparison
    virtual std::vector<uint8_t> GetPixelData(Window* window) = 0;
};

// ============================================================================
// Direct2D Offscreen Capture Implementation
// ============================================================================
class D2DOffscreenCapture : public IScreenshotCapture {
public:
    D2DOffscreenCapture();
    ~D2DOffscreenCapture();
    
    bool Initialize();
    void Shutdown();
    
    // IScreenshotCapture implementation
    bool CaptureWindow(Window* window, const std::filesystem::path& outputPath) override;
    bool CaptureControl(Control* control, const std::filesystem::path& outputPath) override;
    bool CaptureRegion(const luaui::rendering::Rect& region, const std::filesystem::path& outputPath) override;
    std::vector<uint8_t> GetPixelData(Window* window) override;
    
private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

// ============================================================================
// Input Simulation
// ============================================================================
class InputSimulator {
public:
    InputSimulator(Window* window);
    
    // Mouse actions
    void MouseMove(float x, float y);
    void MouseMoveToControl(Control* control);
    void MouseDown(int button = 0);  // 0=left, 1=right, 2=middle
    void MouseUp(int button = 0);
    void MouseClick(int button = 0);
    void MouseDoubleClick(int button = 0);
    void MouseWheel(int delta);
    
    // Keyboard actions
    void KeyDown(int keyCode);
    void KeyUp(int keyCode);
    void KeyPress(int keyCode);
    void SendText(const std::wstring& text);
    void SendText(const std::string& text);
    
    // Convenience methods
    void ClickAt(float x, float y);
    void ClickOn(Control* control);
    void Drag(float fromX, float fromY, float toX, float toY);
    
private:
    Window* m_window;
    float m_lastMouseX = 0;
    float m_lastMouseY = 0;
    
    void SendMouseMessage(UINT msg, WPARAM wParam, LPARAM lParam);
    void UpdateWindow();
};

// ============================================================================
// Control Finder - Find controls by various criteria
// ============================================================================
class ControlFinder {
public:
    ControlFinder(Window* window);
    
    // Find by automation ID
    Control* FindById(const std::string& id);
    Control* FindById(const std::wstring& id);
    
    // Find by control type
    template<typename T>
    T* FindByType() {
        return dynamic_cast<T*>(FindByTypeName(typeid(T).name()));
    }
    
    // Find by name/text
    Control* FindByName(const std::string& name);
    Control* FindByText(const std::wstring& text);
    
    // Find by position
    Control* FindAtPosition(float x, float y);
    
    // Find children of a panel
    std::vector<Control*> FindChildren(controls::Panel* panel);
    
    // Wait for control to appear/disappear
    Control* WaitForId(const std::string& id, int timeoutMs = 5000);
    bool WaitForIdToDisappear(const std::string& id, int timeoutMs = 5000);
    
private:
    Window* m_window;
    
    Control* FindRecursive(Control* root, const std::function<bool(Control*)>& predicate);
    Control* FindByTypeName(const char* typeName);
};

// ============================================================================
// UI Assertions
// ============================================================================
class UIAssert {
public:
    // Visibility assertions
    static void IsVisible(Control* control, const char* message = nullptr);
    static void IsNotVisible(Control* control, const char* message = nullptr);
    static void IsEnabled(Control* control, const char* message = nullptr);
    static void IsDisabled(Control* control, const char* message = nullptr);
    
    // Position/Size assertions
    static void HasPosition(Control* control, float x, float y, float tolerance = 1.0f);
    static void HasSize(Control* control, float width, float height, float tolerance = 1.0f);
    static void IsWithin(Control* control, const luaui::rendering::Rect& bounds);
    
    // Property assertions
    static void HasText(Control* control, const std::wstring& expected);
    static void ContainsText(Control* control, const std::wstring& substring);
    static void HasValue(Control* control, double expected, double tolerance = 0.01);
    
    // Layout assertions
    static void IsAligned(Control* control, Control* reference, const char* alignment); // "left", "right", "top", "bottom", "center"
    static void IsContainedIn(Control* child, Control* parent);
    static void DoesOverlap(Control* control1, Control* control2, bool shouldOverlap = true);
    
    // Rendering assertions
    static void MatchesBaseline(Window* window, const std::string& baselineName, 
                                 float threshold = 0.01f, IScreenshotCapture* capture = nullptr);
    static void MatchesBaseline(Control* control, const std::string& baselineName,
                                 float threshold = 0.01f, IScreenshotCapture* capture = nullptr);
};

// ============================================================================
// Layout Test Utilities
// ============================================================================
class LayoutTester {
public:
    struct LayoutViolation {
        std::string message;
        Control* control;
        luaui::rendering::Rect expected;
        luaui::rendering::Rect actual;
    };
    
    // Validate layout constraints
    static std::vector<LayoutViolation> ValidateLayout(Control* root);
    
    // Check for layout issues
    static bool HasClippingIssues(Control* control);
    static bool HasOverflow(controls::Panel* panel);
    static bool IsCropped(Control* control);
    
    // Measure layout performance
    struct LayoutPerformance {
        double measureTimeMs;
        double arrangeTimeMs;
        int controlCount;
    };
    static LayoutPerformance MeasurePerformance(Window* window);
};

// ============================================================================
// MVVM Binding Test Utilities
// ============================================================================
class BindingTester {
public:
    // Test binding updates
    template<typename T>
    static void AssertBindingUpdate(Control* control, const std::string& propertyName,
                                     std::shared_ptr<luaui::mvvm::ViewModelBase> viewModel,
                                     const T& newValue);
    
    // Test TwoWay binding
    static void AssertTwoWayBinding(Control* control, const std::string& controlProperty,
                                     std::shared_ptr<luaui::mvvm::ViewModelBase> viewModel,
                                     const std::string& vmProperty);
    
    // Wait for binding to update
    static void WaitForBindingUpdate(int timeoutMs = 100);
    
    // Verify no binding errors
    static void AssertNoBindingErrors();
};

// ============================================================================
// Test Scenario Base Class
// ============================================================================
class UITestScenario {
public:
    UITestScenario(const std::string& name);
    virtual ~UITestScenario() = default;
    
    virtual void SetUp();
    virtual void TearDown();
    virtual void Run() = 0;
    
    // Helpers
    void TakeScreenshot(const std::string& name);
    void Wait(int milliseconds);
    void WaitForRender();
    void WaitForLayout();
    void WaitForAnimation();
    
    // Accessors
    Window* GetWindow() const { return m_window; }
    InputSimulator* GetInput() const { return m_input.get(); }
    ControlFinder* GetFinder() const { return m_finder.get(); }
    const std::string& GetName() const { return m_name; }
    
protected:
    std::string m_name;
    Window* m_window = nullptr;
    std::unique_ptr<InputSimulator> m_input;
    std::unique_ptr<ControlFinder> m_finder;
    std::unique_ptr<IScreenshotCapture> m_capture;
    
    virtual Window* CreateTestWindow() = 0;
};

// ============================================================================
// Test Runner for UI Tests
// ============================================================================
class UIAutomationRunner {
public:
    static UIAutomationRunner& Instance();
    
    void Register(std::unique_ptr<UITestScenario> scenario);
    int RunAll();
    int Run(const std::string& scenarioName);
    
    void SetConfig(const AutomationConfig& config);
    const AutomationConfig& GetConfig() const;
    
private:
    UIAutomationRunner() = default;
    std::vector<std::unique_ptr<UITestScenario>> m_scenarios;
    AutomationConfig m_config;
};

// ============================================================================
// Macros for UI Test Registration
// ============================================================================

#define UI_TEST(className) \
    struct className##Registrar { \
        className##Registrar() { \
            luaui::test::automation::UIAutomationRunner::Instance().Register( \
                std::make_unique<className>()); \
        } \
    } g_##className##Registrar; \

#define RUN_UI_TESTS() luaui::test::automation::UIAutomationRunner::Instance().RunAll()

// ============================================================================
// Convenience Macros for Assertions
// ============================================================================

#define ASSERT_VISIBLE(control) \
    luaui::test::automation::UIAssert::IsVisible(control, #control " should be visible")

#define ASSERT_NOT_VISIBLE(control) \
    luaui::test::automation::UIAssert::IsNotVisible(control, #control " should not be visible")

#define ASSERT_CONTROL_AT(control, x, y) \
    luaui::test::automation::UIAssert::HasPosition(control, x, y)

#define ASSERT_CONTROL_SIZE(control, w, h) \
    luaui::test::automation::UIAssert::HasSize(control, w, h)

#define ASSERT_MATCHES_BASELINE(window, name) \
    luaui::test::automation::UIAssert::MatchesBaseline(window, name)

#define ASSERT_TEXT_EQUALS(control, text) \
    luaui::test::automation::UIAssert::HasText(control, text)

} // namespace automation
} // namespace test
} // namespace luaui
