// ControlsDemo_TestRunner.cpp - Standalone test runner for Controls Demo
// Can be compiled and run independently

#include <iostream>
#include <windows.h>
#include <memory>

// Include LuaUI headers
#include "Controls.h"
#include "Panel.h"
#include "Window.h"
#include "Button.h"
#include "TextBlock.h"
#include "TextBox.h"
#include "CheckBox.h"
#include "Slider.h"
#include "ProgressBar.h"
#include "ListBox.h"
#include "TabControl.h"
#include "Logger.h"

using namespace luaui;
using namespace luaui::controls;
using namespace luaui::rendering;
using namespace luaui::utils;

// ============================================================================
// Simple Test Framework
// ============================================================================
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            std::cerr << "[FAIL] " << message << " at line " << __LINE__ << std::endl; \
            return false; \
        } \
    } while(0)

#define TEST_ASSERT_EQ(expected, actual, message) \
    TEST_ASSERT((expected) == (actual), message)

struct TestResult {
    std::string name;
    bool passed;
    std::string error;
};

// ============================================================================
// Test Window Class (Exposes internals for testing)
// ============================================================================
class TestControlsWindow : public Window {
public:
    std::shared_ptr<TextBlock> m_statusText;
    std::shared_ptr<ProgressBar> m_progressBar;
    std::shared_ptr<Slider> m_slider;
    std::shared_ptr<TextBlock> m_sliderValue;
    std::shared_ptr<ListBox> m_listBox;
    std::shared_ptr<TabControl> m_mainTabs;
    
    // Test hooks
    Control* GetControlByName(const std::string& name) {
        return FindControlRecursive(m_root.get(), name);
    }
    
    void ClickButton(const std::string& name) {
        auto* btn = dynamic_cast<Button*>(GetControlByName(name));
        if (btn) {
            btn->Click.Fire(btn);
        }
    }
    
    void SetSliderValue(double value) {
        if (m_slider) {
            m_slider->SetValue(value);
        }
    }
    
    std::wstring GetStatusText() const {
        return m_statusText ? m_statusText->GetText() : L"";
    }
    
protected:
    void OnLoaded() override {
        // Create the same UI as ControlsShowcaseWindow
        auto root = std::make_shared<StackPanel>();
        root->SetName("rootPanel");
        
        // Title
        auto title = std::make_shared<TextBlock>();
        title->SetName("windowTitle");
        title->SetText(L"LuaUI Controls Showcase");
        title->SetFontSize(28);
        root->AddChild(title);
        
        // Main TabControl
        m_mainTabs = std::make_shared<TabControl>();
        m_mainTabs->SetName("mainTabs");
        m_mainTabs->SetTabHeight(36);
        
        // Basic Tab
        auto tabBasic = std::make_shared<TabItem>();
        tabBasic->SetHeader(L"Basic");
        tabBasic->SetContent(CreateBasicTab());
        m_mainTabs->AddTab(tabBasic);
        
        // Input Tab
        auto tabInput = std::make_shared<TabItem>();
        tabInput->SetHeader(L"Input");
        tabInput->SetContent(CreateInputTab());
        m_mainTabs->AddTab(tabInput);
        
        // Selection Tab
        auto tabSelect = std::make_shared<TabItem>();
        tabSelect->SetHeader(L"Selection");
        tabSelect->SetContent(CreateSelectionTab());
        m_mainTabs->AddTab(tabSelect);
        
        m_mainTabs->SetSelectedIndex(0);
        root->AddChild(m_mainTabs);
        
        // Status Panel
        auto statusPanel = std::make_shared<StackPanel>();
        m_statusText = std::make_shared<TextBlock>();
        m_statusText->SetName("statusText");
        m_statusText->SetText(L"Ready");
        statusPanel->AddChild(m_statusText);
        root->AddChild(statusPanel);
        
        SetRoot(root);
    }
    
private:
    Control* FindControlRecursive(Control* root, const std::string& name) {
        if (!root) return nullptr;
        if (root->GetName() == name) return root;
        
        // Search in children
        // Note: This requires Panel to expose children
        return nullptr;
    }
    
    std::shared_ptr<Panel> CreateBasicTab() {
        auto root = std::make_shared<StackPanel>();
        root->SetName("basicTab");
        
        auto btn1 = std::make_shared<Button>();
        btn1->SetName("defaultBtn");
        btn1->SetText(L"Default");
        btn1->Click.Add([this](auto*) { m_statusText->SetText(L"Default button clicked"); });
        root->AddChild(btn1);
        
        auto btn2 = std::make_shared<Button>();
        btn2->SetName("primaryBtn");
        btn2->SetText(L"Primary");
        btn2->Click.Add([this](auto*) { m_statusText->SetText(L"Primary button clicked"); });
        root->AddChild(btn2);
        
        return root;
    }
    
    std::shared_ptr<Panel> CreateInputTab() {
        auto root = std::make_shared<StackPanel>();
        root->SetName("inputTab");
        
        m_sliderValue = std::make_shared<TextBlock>();
        m_sliderValue->SetName("sliderValue");
        m_sliderValue->SetText(L"50%");
        root->AddChild(m_sliderValue);
        
        m_slider = std::make_shared<Slider>();
        m_slider->SetName("volumeSlider");
        m_slider->SetValue(50);
        m_slider->SetMinimum(0);
        m_slider->SetMaximum(100);
        m_slider->ValueChanged.Add([this](auto*, double v) {
            m_sliderValue->SetText(std::to_wstring((int)v) + L"%");
            if (m_progressBar) m_progressBar->SetValue(v);
        });
        root->AddChild(m_slider);
        
        m_progressBar = std::make_shared<ProgressBar>();
        m_progressBar->SetName("progressBar");
        m_progressBar->SetValue(50);
        root->AddChild(m_progressBar);
        
        return root;
    }
    
    std::shared_ptr<Panel> CreateSelectionTab() {
        auto root = std::make_shared<StackPanel>();
        root->SetName("selectionTab");
        
        auto chk1 = std::make_shared<CheckBox>();
        chk1->SetName("notifyCheck");
        chk1->SetText(L"Enable notifications");
        chk1->SetIsChecked(true);
        chk1->CheckedChanged.Add([this](auto*, bool c) {
            m_statusText->SetText(c ? L"Notifications enabled" : L"Notifications disabled");
        });
        root->AddChild(chk1);
        
        return root;
    }
};

// ============================================================================
// Individual Tests
// ============================================================================

bool Test_WindowCreation() {
    std::cout << "[TEST] Window Creation..." << std::endl;
    
    HINSTANCE hInstance = GetModuleHandle(nullptr);
    TestControlsWindow window;
    
    bool created = window.Create(hInstance, L"Test Window", 800, 600);
    TEST_ASSERT(created, "Failed to create window");
    
    // Verify title exists
    // This would need control access
    
    std::cout << "[PASS] Window Creation" << std::endl;
    return true;
}

bool Test_ButtonClicks() {
    std::cout << "[TEST] Button Clicks..." << std::endl;
    
    HINSTANCE hInstance = GetModuleHandle(nullptr);
    auto window = std::make_shared<TestControlsWindow>();
    
    bool created = window->Create(hInstance, L"Test", 800, 600);
    TEST_ASSERT(created, "Failed to create window");
    
    // Test default button click
    window->ClickButton("defaultBtn");
    TEST_ASSERT_EQ(std::wstring(L"Default button clicked"), window->GetStatusText(), 
                   "Status not updated after default button click");
    
    // Test primary button click
    window->ClickButton("primaryBtn");
    TEST_ASSERT_EQ(std::wstring(L"Primary button clicked"), window->GetStatusText(),
                   "Status not updated after primary button click");
    
    std::cout << "[PASS] Button Clicks" << std::endl;
    return true;
}

bool Test_SliderValueChange() {
    std::cout << "[TEST] Slider Value Change..." << std::endl;
    
    HINSTANCE hInstance = GetModuleHandle(nullptr);
    auto window = std::make_shared<TestControlsWindow>();
    
    bool created = window->Create(hInstance, L"Test", 800, 600);
    TEST_ASSERT(created, "Failed to create window");
    
    // Switch to Input tab
    if (window->m_mainTabs) {
        window->m_mainTabs->SetSelectedIndex(1);
    }
    
    // Test slider value change
    window->SetSliderValue(75);
    
    // Verify slider value
    if (window->m_slider) {
        TEST_ASSERT_EQ(75.0, window->m_slider->GetValue(), "Slider value not set correctly");
    }
    
    // Verify progress bar synced
    if (window->m_progressBar) {
        TEST_ASSERT_EQ(75.0, window->m_progressBar->GetValue(), "ProgressBar not synced with slider");
    }
    
    // Verify label updated
    if (window->m_sliderValue) {
        TEST_ASSERT_EQ(std::wstring(L"75%"), window->m_sliderValue->GetText(),
                       "Slider value label not updated");
    }
    
    std::cout << "[PASS] Slider Value Change" << std::endl;
    return true;
}

bool Test_TabNavigation() {
    std::cout << "[TEST] Tab Navigation..." << std::endl;
    
    HINSTANCE hInstance = GetModuleHandle(nullptr);
    auto window = std::make_shared<TestControlsWindow>();
    
    bool created = window->Create(hInstance, L"Test", 800, 600);
    TEST_ASSERT(created, "Failed to create window");
    
    TEST_ASSERT(window->m_mainTabs != nullptr, "TabControl not created");
    
    // Test switching tabs
    window->m_mainTabs->SetSelectedIndex(0);
    TEST_ASSERT_EQ(0, window->m_mainTabs->GetSelectedIndex(), "Failed to select Basic tab");
    
    window->m_mainTabs->SetSelectedIndex(1);
    TEST_ASSERT_EQ(1, window->m_mainTabs->GetSelectedIndex(), "Failed to select Input tab");
    
    window->m_mainTabs->SetSelectedIndex(2);
    TEST_ASSERT_EQ(2, window->m_mainTabs->GetSelectedIndex(), "Failed to select Selection tab");
    
    std::cout << "[PASS] Tab Navigation" << std::endl;
    return true;
}

// ============================================================================
// Main Test Runner
// ============================================================================
int main(int argc, char** argv) {
    std::cout << "========================================" << std::endl;
    std::cout << "Controls Demo Automated Tests" << std::endl;
    std::cout << "========================================" << std::endl;
    
    Logger::Initialize();
    Logger::SetConsoleLevel(LogLevel::Info);
    
    std::vector<TestResult> results;
    
    // Run all tests
    auto runTest = [&](const std::string& name, auto testFunc) {
        std::cout << "\n--- Running: " << name << " ---" << std::endl;
        try {
            bool passed = testFunc();
            results.push_back({name, passed, passed ? "" : "Test failed"});
        } catch (const std::exception& e) {
            results.push_back({name, false, e.what()});
        }
    };
    
    runTest("Window Creation", Test_WindowCreation);
    runTest("Button Clicks", Test_ButtonClicks);
    runTest("Slider Value Change", Test_SliderValueChange);
    runTest("Tab Navigation", Test_TabNavigation);
    
    // Print summary
    std::cout << "\n========================================" << std::endl;
    std::cout << "Test Results:" << std::endl;
    std::cout << "========================================" << std::endl;
    
    int passed = 0;
    int failed = 0;
    
    for (const auto& result : results) {
        if (result.passed) {
            std::cout << "[PASS] " << result.name << std::endl;
            passed++;
        } else {
            std::cout << "[FAIL] " << result.name << ": " << result.error << std::endl;
            failed++;
        }
    }
    
    std::cout << "\nTotal: " << passed << " passed, " << failed << " failed" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return failed > 0 ? 1 : 0;
}
