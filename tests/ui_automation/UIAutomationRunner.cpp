// UIAutomationRunner.cpp - Implementation of UI Automation Test Runner

#include "UIAutomationFramework.h"
#include "TestHooks.h"
#include "Window.h"
#include <windows.h>
#include <iostream>
#include <chrono>
#include <thread>

namespace luaui {
namespace test {
namespace automation {

// ============================================================================
// UITestScenario Implementation
// ============================================================================
UITestScenario::UITestScenario(const std::string& name) : m_name(name) {}

void UITestScenario::SetUp() {
    // Create test window
    m_window = CreateTestWindow();
    
    if (auto* tw = dynamic_cast<TestWindow*>(m_window)) {
        HINSTANCE hInstance = GetModuleHandle(nullptr);
        auto& config = UIAutomationRunner::Instance().GetConfig();
        
        if (!tw->Create(hInstance, L"UI Test Window", 
                        config.testWindowWidth, config.testWindowHeight)) {
            throw std::runtime_error("Failed to create test window");
        }
        
        // Don't show window in headless mode
        if (!config.headlessMode) {
            tw->Show(SW_SHOW);
        }
    }
    
    // Create helper objects
    m_input = std::make_unique<InputSimulator>(m_window);
    m_finder = std::make_unique<ControlFinder>(m_window);
    m_capture = std::make_unique<D2DOffscreenCapture>();
    m_capture->Initialize();
}

void UITestScenario::TearDown() {
    m_capture->Shutdown();
    m_capture.reset();
    m_finder.reset();
    m_input.reset();
    
    if (m_window) {
        delete m_window;
        m_window = nullptr;
    }
}

void UITestScenario::TakeScreenshot(const std::string& name) {
    if (!m_capture || !m_window) return;
    
    auto& config = UIAutomationRunner::Instance().GetConfig();
    auto path = config.screenshotOutputPath / (name + ".png");
    
    // Ensure directory exists
    std::filesystem::create_directories(path.parent_path());
    
    m_capture->CaptureWindow(m_window, path);
    
    std::cout << "[Screenshot] " << name << " -> " << path << std::endl;
}

void UITestScenario::Wait(int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void UITestScenario::WaitForRender() {
    if (auto* tw = dynamic_cast<TestWindow*>(m_window)) {
        tw->TestRender();
    }
}

void UITestScenario::WaitForLayout() {
    // Force layout update
    if (auto* tw = dynamic_cast<TestWindow*>(m_window)) {
        tw->TestUpdateLayout();
    }
    
    // Small delay for layout to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

void UITestScenario::WaitForAnimation() {
    auto& config = UIAutomationRunner::Instance().GetConfig();
    std::this_thread::sleep_for(std::chrono::milliseconds(config.animationWaitMs));
}

// ============================================================================
// UIAutomationRunner Implementation
// ============================================================================
UIAutomationRunner& UIAutomationRunner::Instance() {
    static UIAutomationRunner instance;
    return instance;
}

void UIAutomationRunner::Register(std::unique_ptr<UITestScenario> scenario) {
    m_scenarios.push_back(std::move(scenario));
}

int UIAutomationRunner::RunAll() {
    std::cout << "========================================" << std::endl;
    std::cout << "LuaUI Automation Test Runner" << std::endl;
    std::cout << "Running " << m_scenarios.size() << " test scenario(s)..." << std::endl;
    std::cout << "========================================" << std::endl;
    
    int passed = 0;
    int failed = 0;
    
    for (auto& scenario : m_scenarios) {
        std::cout << "\n[RUN] " << scenario->GetName() << "..." << std::endl;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        try {
            scenario->Run();
            passed++;
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "[PASS] " << scenario->GetName() << " (" << duration.count() << "ms)" << std::endl;
        } catch (const std::exception& e) {
            failed++;
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "[FAIL] " << scenario->GetName() << " (" << duration.count() << "ms)" << std::endl;
            std::cout << "       Error: " << e.what() << std::endl;
        }
    }
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "Results: " << passed << " passed, " << failed << " failed" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return failed > 0 ? 1 : 0;
}

int UIAutomationRunner::Run(const std::string& scenarioName) {
    for (auto& scenario : m_scenarios) {
        if (scenario->GetName() == scenarioName) {
            std::cout << "[RUN] " << scenarioName << "..." << std::endl;
            try {
                scenario->Run();
                std::cout << "[PASS] " << scenarioName << std::endl;
                return 0;
            } catch (const std::exception& e) {
                std::cout << "[FAIL] " << scenarioName << std::endl;
                std::cout << "       Error: " << e.what() << std::endl;
                return 1;
            }
        }
    }
    
    std::cerr << "Test scenario not found: " << scenarioName << std::endl;
    return 1;
}

void UIAutomationRunner::SetConfig(const AutomationConfig& config) {
    m_config = config;
}

const AutomationConfig& UIAutomationRunner::GetConfig() const {
    return m_config;
}

} // namespace automation
} // namespace test
} // namespace luaui

// ============================================================================
// Main entry point for UI automation tests
// ============================================================================
int main(int argc, char** argv) {
    using namespace luaui::test::automation;
    
    // Parse command line arguments
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--list") {
            // List all registered tests
            // Note: This won't work well with static registration
            // as tests may not be registered yet
            std::cout << "Use --run <test_name> to run a specific test" << std::endl;
            return 0;
        } else if (arg == "--run" && argc > 2) {
            return UIAutomationRunner::Instance().Run(argv[2]);
        } else if (arg == "--update-baselines") {
            // Update all baselines
            std::cout << "Updating baselines..." << std::endl;
            // Implementation would copy current outputs to baselines
            return 0;
        }
    }
    
    // Run all tests
    return UIAutomationRunner::Instance().RunAll();
}
