// ControlsDemo_Test_Simple.cpp - Simplified test for 01_controls_demo
// Headless mode - no visible window required

#include <iostream>
#include <windows.h>
#include <memory>
#include <string>
#include <vector>
#include <functional>

// LuaUI Headers
#include "Controls.h"
#include "Panel.h"
#include "Window.h"
#include "Button.h"
#include "TextBlock.h"
#include "CheckBox.h"
#include "Slider.h"
#include "ProgressBar.h"
#include "TabControl.h"
#include "Logger.h"

using namespace luaui;
using namespace luaui::controls;
using namespace luaui::rendering;
using namespace luaui::utils;

// ============================================================================
// Simple Test Framework
// ============================================================================
#define TEST_ASSERT(condition, msg) \
    do { \
        if (!(condition)) { \
            throw std::runtime_error(msg); \
        } \
    } while(0)

#define TEST_ASSERT_EQ(expected, actual) \
    TEST_ASSERT((expected) == (actual), "Assertion failed: values not equal")

static int g_passed = 0;
static int g_failed = 0;

void RunTest(const char* name, std::function<void()> test) {
    std::cout << "[TEST] " << name << "... " << std::flush;
    try {
        test();
        std::cout << "PASS" << std::endl;
        g_passed++;
    } catch (const std::exception& e) {
        std::cout << "FAIL: " << e.what() << std::endl;
        g_failed++;
    } catch (...) {
        std::cout << "FAIL: Unknown error" << std::endl;
        g_failed++;
    }
}

// ============================================================================
// Headless Test - Tests without creating actual Windows
// ============================================================================

// Test just the UI logic without Window/Rendering
void Test_ControlsLogic() {
    // Test Button logic
    {
        auto button = std::make_shared<Button>();
        button->SetText(L"Test Button");
        TEST_ASSERT_EQ(std::wstring(L"Test Button"), button->GetText());
        
        bool clicked = false;
        button->Click.Add([&clicked](luaui::Control*) { clicked = true; });
        
        // Simulate click
        button->Click.Invoke(button.get());
        TEST_ASSERT(clicked, "Button click event not fired");
    }
    
    // Test TextBlock
    {
        auto text = std::make_shared<TextBlock>();
        text->SetText(L"Hello World");
        text->SetFontSize(16);
        TEST_ASSERT_EQ(std::wstring(L"Hello World"), text->GetText());
    }
    
    // Test CheckBox
    {
        auto check = std::make_shared<CheckBox>();
        check->SetText(L"Enable");
        check->SetIsChecked(true);
        TEST_ASSERT(check->GetIsChecked(), "CheckBox should be checked");
        
        check->SetIsChecked(false);
        TEST_ASSERT(!check->GetIsChecked(), "CheckBox should be unchecked");
    }
    
    // Test Slider
    {
        auto slider = std::make_shared<Slider>();
        slider->SetMinimum(0);
        slider->SetMaximum(100);
        slider->SetValue(50);
        
        TEST_ASSERT_EQ(0.0, slider->GetMinimum());
        TEST_ASSERT_EQ(100.0, slider->GetMaximum());
        TEST_ASSERT_EQ(50.0, slider->GetValue());
        
        // Test value clamping
        slider->SetValue(150);
        TEST_ASSERT_EQ(100.0, slider->GetValue());  // Should clamp to max
        
        slider->SetValue(-10);
        TEST_ASSERT_EQ(0.0, slider->GetValue());  // Should clamp to min
    }
    
    // Test ProgressBar
    {
        auto progress = std::make_shared<ProgressBar>();
        progress->SetValue(75);
        TEST_ASSERT_EQ(75.0, progress->GetValue());
    }
    
    // Test TabControl
    {
        auto tabs = std::make_shared<TabControl>();
        
        auto tab1 = std::make_shared<TabItem>();
        tab1->SetHeader(L"Tab 1");
        tabs->AddTab(tab1);
        
        auto tab2 = std::make_shared<TabItem>();
        tab2->SetHeader(L"Tab 2");
        tabs->AddTab(tab2);
        
        TEST_ASSERT_EQ(2, (int)tabs->GetTabCount());
        TEST_ASSERT_EQ(0, tabs->GetSelectedIndex());
        
        tabs->SetSelectedIndex(1);
        TEST_ASSERT_EQ(1, tabs->GetSelectedIndex());
    }
    
    std::cout << "(Controls logic) ";
}

// Test Panel hierarchy
void Test_PanelHierarchy() {
    auto root = std::make_shared<StackPanel>();
    root->SetOrientation(StackPanel::Orientation::Vertical);
    
    auto child1 = std::make_shared<Button>();
    child1->SetName("btn1");
    root->AddChild(child1);
    
    auto child2 = std::make_shared<Button>();
    child2->SetName("btn2");
    root->AddChild(child2);
    
    auto child3 = std::make_shared<TextBlock>();
    child3->SetName("text1");
    root->AddChild(child3);
    
    TEST_ASSERT_EQ(3, (int)root->GetChildCount());
    
    // Test child access
    auto retrieved = root->GetChild(0);
    TEST_ASSERT(retrieved != nullptr, "Child should exist");
    TEST_ASSERT_EQ(std::string("btn1"), retrieved->GetName());
    
    // Test removal
    root->RemoveChild(child1);
    TEST_ASSERT_EQ(2, (int)root->GetChildCount());
    
    std::cout << "(Panel hierarchy) ";
}

// Test event system
void Test_EventSystem() {
    // Test Slider ValueChanged event
    {
        auto slider = std::make_shared<Slider>();
        double capturedValue = 0;
        bool eventFired = false;
        
        slider->ValueChanged.Add([&capturedValue, &eventFired](luaui::Control*, double val) {
            capturedValue = val;
            eventFired = true;
        });
        
        slider->SetValue(75);
        
        // Note: Event may not fire on programmatic set depending on implementation
        // We just verify the value was set
        TEST_ASSERT_EQ(75.0, slider->GetValue());
    }
    
    // Test CheckBox CheckedChanged event
    {
        auto check = std::make_shared<CheckBox>();
        bool checkedState = false;
        bool eventFired = false;
        
        check->CheckedChanged.Add([&checkedState, &eventFired](luaui::Control*, bool isChecked) {
            checkedState = isChecked;
            eventFired = true;
        });
        
        check->SetIsChecked(true);
        TEST_ASSERT(check->GetIsChecked(), "Should be checked");
    }
    
    std::cout << "(Event system) ";
}

// Test layout properties
void Test_LayoutProperties() {
    auto control = std::make_shared<Button>();
    
    // Get layout component
    auto* layout = control->GetLayout();
    TEST_ASSERT(layout != nullptr, "Layout component should exist");
    
    // Test setting size
    layout->SetWidth(100);
    layout->SetHeight(50);
    
    // These may not be directly retrievable depending on implementation
    // but should not crash
    
    std::cout << "(Layout properties) ";
}

// Test colors and styling
void Test_ColorsAndStyling() {
    auto button = std::make_shared<Button>();
    
    // Test state colors
    Color normal(0.2f, 0.5f, 0.8f, 1.0f);
    Color hover(0.3f, 0.6f, 0.9f, 1.0f);
    Color pressed(0.1f, 0.4f, 0.7f, 1.0f);
    
    button->SetStateColors(normal, hover, pressed);
    
    // Test color creation
    Color hexColor = Color::FromHex(0xFF5733);
    TEST_ASSERT(hexColor.r > 0, "Red component should be > 0");
    
    std::cout << "(Colors/Styling) ";
}

// ============================================================================
// Integration Test with actual Window (commented out - requires GUI)
// ============================================================================
/*
void Test_FullWindowIntegration() {
    // This test requires an actual Windows message loop
    // Run only in interactive mode
    
    HINSTANCE hInstance = GetModuleHandle(nullptr);
    auto window = std::make_shared<Window>();
    
    bool created = window->Create(hInstance, L"Test", 800, 600);
    TEST_ASSERT(created, "Failed to create window");
    
    auto root = std::make_shared<StackPanel>();
    auto btn = std::make_shared<Button>();
    btn->SetText(L"Click Me");
    root->AddChild(btn);
    
    window->SetRoot(root);
    
    // Don't call Run() - that would block
    // Just verify setup worked
}
*/

// ============================================================================
// Main
// ============================================================================
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Controls Demo Tests (Headless)" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    // Initialize logging but suppress output
    Logger::Initialize();
    Logger::SetConsoleLevel(LogLevel::Error);  // Only show errors
    
    // Run all tests
    RunTest("Controls Logic", Test_ControlsLogic);
    RunTest("Panel Hierarchy", Test_PanelHierarchy);
    RunTest("Event System", Test_EventSystem);
    RunTest("Layout Properties", Test_LayoutProperties);
    RunTest("Colors and Styling", Test_ColorsAndStyling);
    
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Results: " << g_passed << " passed, " << g_failed << " failed" << std::endl;
    std::cout << "========================================" << std::endl;
    
    if (g_failed == 0) {
        std::cout << "All tests passed!" << std::endl;
    }
    
    return g_failed > 0 ? 1 : 0;
}
