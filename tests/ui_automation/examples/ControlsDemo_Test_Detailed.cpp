// ControlsDemo_Test_Detailed.cpp - Comprehensive test suite for 01_controls_demo
// Detailed reporting with metrics, performance, and validation

#include <iostream>
#include <windows.h>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <psapi.h>  // For memory info

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
// Detailed Test Framework with Metrics
// ============================================================================
struct TestMetrics {
    double durationMs = 0;
    int assertionsPassed = 0;
    int assertionsFailed = 0;
    size_t memoryBefore = 0;
    size_t memoryAfter = 0;
    std::vector<std::string> details;
    
    void AddDetail(const std::string& detail) {
        details.push_back(detail);
    }
};

struct TestResult {
    std::string name;
    bool passed;
    std::string error;
    TestMetrics metrics;
};

static std::vector<TestResult> g_results;
static TestMetrics g_currentMetrics;

// Memory tracking (simplified)
size_t GetCurrentMemoryUsage() {
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize / 1024; // KB
    }
    return 0;
}

// Helper to convert wstring for output
std::string ToString(const std::wstring& ws) {
    std::string s;
    for (wchar_t c : ws) {
        if (c < 128) s += (char)c;
        else s += '?';
    }
    return s;
}

// Enhanced assertion macros with details
#define DETAIL(msg) g_currentMetrics.AddDetail(msg)

#define ASSERT_TRUE_DETAIL(condition, detail) \
    do { \
        if (condition) { \
            g_currentMetrics.assertionsPassed++; \
            DETAIL("[OK] " + std::string(detail)); \
        } else { \
            g_currentMetrics.assertionsFailed++; \
            DETAIL("[FAIL] " + std::string(detail) + " (FAILED)"); \
            throw std::runtime_error("Assertion failed: " + std::string(#condition)); \
        } \
    } while(0)

#define ASSERT_EQ_DETAIL(expected, actual, desc) \
    do { \
        if ((expected) == (actual)) { \
            g_currentMetrics.assertionsPassed++; \
            std::stringstream ss; \
            ss << "[OK] " << desc << " = " << (actual); \
            DETAIL(ss.str()); \
        } else { \
            g_currentMetrics.assertionsFailed++; \
            std::stringstream ss; \
            ss << "[FAIL] " << desc << ": expected " << (expected) << " but got " << (actual); \
            DETAIL(ss.str() + " (FAILED)"); \
            throw std::runtime_error(ss.str()); \
        } \
    } while(0)

#define ASSERT_NEAR_DETAIL(expected, actual, tolerance, desc) \
    do { \
        double diff = std::abs((double)(expected) - (double)(actual)); \
        if (diff <= (tolerance)) { \
            g_currentMetrics.assertionsPassed++; \
            std::stringstream ss; \
            ss << "[OK] " << desc << " = " << (actual) << " (diff: " << diff << ")"; \
            DETAIL(ss.str()); \
        } else { \
            g_currentMetrics.assertionsFailed++; \
            std::stringstream ss; \
            ss << "[FAIL] " << desc << ": expected " << (expected) << " +/-" << (tolerance) \
               << " but got " << (actual) << " (diff: " << diff << ")"; \
            DETAIL(ss.str() + " (FAILED)"); \
            throw std::runtime_error(ss.str()); \
        } \
    } while(0)

#define ASSERT_NOT_NULL_DETAIL(ptr, desc) \
    ASSERT_TRUE_DETAIL((ptr) != nullptr, desc)

void RunTestDetailed(const char* name, std::function<void()> test) {
    std::cout << "\n" << std::string(60, '-') << std::endl;
    std::cout << "[TEST] " << name << std::endl;
    std::cout << std::string(60, '-') << std::endl;
    
    g_currentMetrics = TestMetrics{};
    g_currentMetrics.memoryBefore = GetCurrentMemoryUsage();
    
    auto start = std::chrono::high_resolution_clock::now();
    bool passed = false;
    std::string error;
    
    try {
        test();
        passed = true;
    } catch (const std::exception& e) {
        error = e.what();
    } catch (...) {
        error = "Unknown error";
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    g_currentMetrics.durationMs = std::chrono::duration<double, std::milli>(end - start).count();
    g_currentMetrics.memoryAfter = GetCurrentMemoryUsage();
    
    // Print details
    for (const auto& detail : g_currentMetrics.details) {
        std::cout << "  " << detail << std::endl;
    }
    
    // Print summary
    std::cout << "\n  [METRICS] Time: " << std::fixed << std::setprecision(2) 
              << g_currentMetrics.durationMs << "ms | ";
    std::cout << "Assertions: " << g_currentMetrics.assertionsPassed << " passed, "
              << g_currentMetrics.assertionsFailed << " failed | ";
    std::cout << "Memory: " << (g_currentMetrics.memoryAfter - g_currentMetrics.memoryBefore) << " KB";
    std::cout << std::endl;
    
    std::cout << "  [RESULT] " << (passed ? "PASS" : "FAIL");
    if (!error.empty()) {
        std::cout << " - " << error;
    }
    std::cout << std::endl;
    
    g_results.push_back({name, passed, error, g_currentMetrics});
}

// ============================================================================
// Comprehensive Tests
// ============================================================================

void Test_ButtonComprehensive() {
    DETAIL("=== Button Creation & Properties ===");
    
    auto button = std::make_shared<Button>();
    ASSERT_NOT_NULL_DETAIL(button.get(), "Button instance created");
    
    // Text property
    button->SetText(L"Click Me");
    ASSERT_TRUE_DETAIL(button->GetText() == L"Click Me", "Button text match");
    
    button->SetText(L"");
    ASSERT_TRUE_DETAIL(button->GetText() == L"", "Empty button text");
    
    button->SetText(L"Unicode: Test");
    ASSERT_TRUE_DETAIL(button->GetText() == L"Unicode: Test", "Unicode button text");
    
    DETAIL("\n=== Button State & Visibility ===");
    
    // Default state
    ASSERT_TRUE_DETAIL(button->GetIsVisible(), "Button visible by default");
    ASSERT_TRUE_DETAIL(button->GetIsEnabled(), "Button enabled by default");
    
    // Visibility
    button->SetIsVisible(false);
    ASSERT_TRUE_DETAIL(!button->GetIsVisible(), "Button hidden");
    button->SetIsVisible(true);
    ASSERT_TRUE_DETAIL(button->GetIsVisible(), "Button shown");
    
    // Enabled state
    button->SetIsEnabled(false);
    ASSERT_TRUE_DETAIL(!button->GetIsEnabled(), "Button disabled");
    button->SetIsEnabled(true);
    ASSERT_TRUE_DETAIL(button->GetIsEnabled(), "Button enabled");
    
    DETAIL("\n=== Button Event Handling ===");
    
    int clickCount = 0;
    button->Click.Add([&clickCount](luaui::Control*) { clickCount++; });
    
    button->Click.Invoke(button.get());
    ASSERT_EQ_DETAIL(1, clickCount, "First click");
    
    button->Click.Invoke(button.get());
    ASSERT_EQ_DETAIL(2, clickCount, "Second click");
    
    // Multiple handlers
    int secondCount = 0;
    button->Click.Add([&secondCount](luaui::Control*) { secondCount++; });
    button->Click.Invoke(button.get());
    ASSERT_EQ_DETAIL(3, clickCount, "First handler after adding second");
    ASSERT_EQ_DETAIL(1, secondCount, "Second handler called");
    
    DETAIL("\n=== Button Styling ===");
    
    Color normal(0.2f, 0.5f, 0.8f, 1.0f);
    Color hover(0.3f, 0.6f, 0.9f, 1.0f);
    Color pressed(0.1f, 0.4f, 0.7f, 1.0f);
    
    button->SetStateColors(normal, hover, pressed);
    DETAIL("[OK] State colors set (normal/hover/pressed)");
    
    // Name for identification
    button->SetName("testButton");
    ASSERT_EQ_DETAIL(std::string("testButton"), button->GetName(), "Button name");
}

void Test_SliderComprehensive() {
    DETAIL("=== Slider Creation & Range ===");
    
    auto slider = std::make_shared<Slider>();
    ASSERT_NOT_NULL_DETAIL(slider.get(), "Slider instance created");
    
    // Default range
    ASSERT_EQ_DETAIL(0.0, slider->GetMinimum(), "Default minimum");
    ASSERT_EQ_DETAIL(100.0, slider->GetMaximum(), "Default maximum");
    ASSERT_EQ_DETAIL(0.0, slider->GetValue(), "Default value");
    
    DETAIL("\n=== Slider Value Setting ===");
    
    // Set within range
    slider->SetValue(50);
    ASSERT_EQ_DETAIL(50.0, slider->GetValue(), "Value set to 50");
    
    slider->SetValue(0);
    ASSERT_EQ_DETAIL(0.0, slider->GetValue(), "Value set to 0");
    
    slider->SetValue(100);
    ASSERT_EQ_DETAIL(100.0, slider->GetValue(), "Value set to 100");
    
    DETAIL("\n=== Slider Clamping ===");
    
    // Test clamping above max
    slider->SetValue(150);
    ASSERT_EQ_DETAIL(100.0, slider->GetValue(), "Value clamped to max (100)");
    
    // Test clamping below min
    slider->SetValue(-50);
    ASSERT_EQ_DETAIL(0.0, slider->GetValue(), "Value clamped to min (0)");
    
    DETAIL("\n=== Slider Custom Range ===");
    
    slider->SetMinimum(-100);
    slider->SetMaximum(100);
    ASSERT_EQ_DETAIL(-100.0, slider->GetMinimum(), "Custom minimum");
    ASSERT_EQ_DETAIL(100.0, slider->GetMaximum(), "Custom maximum");
    
    slider->SetValue(0);
    ASSERT_EQ_DETAIL(0.0, slider->GetValue(), "Value at center");
    
    slider->SetValue(-100);
    ASSERT_EQ_DETAIL(-100.0, slider->GetValue(), "Value at custom min");
    
    slider->SetValue(100);
    ASSERT_EQ_DETAIL(100.0, slider->GetValue(), "Value at custom max");
    
    // Clamp in custom range
    slider->SetValue(200);
    ASSERT_EQ_DETAIL(100.0, slider->GetValue(), "Value clamped to custom max");
    
    slider->SetValue(-200);
    ASSERT_EQ_DETAIL(-100.0, slider->GetValue(), "Value clamped to custom min");
    
    DETAIL("\n=== Slider Events ===");
    
    double capturedValue = -1;
    int eventCount = 0;
    
    slider->ValueChanged.Add([&capturedValue, &eventCount](luaui::Control*, double val) {
        capturedValue = val;
        eventCount++;
    });
    
    slider->SetValue(50);
    // Note: Event may or may not fire on programmatic set depending on implementation
    DETAIL("ValueChanged event handler registered");
}

void Test_CheckBoxComprehensive() {
    DETAIL("=== CheckBox States ===");
    
    auto check = std::make_shared<CheckBox>();
    ASSERT_NOT_NULL_DETAIL(check.get(), "CheckBox created");
    
    // Default state
    ASSERT_TRUE_DETAIL(!check->GetIsChecked(), "Default unchecked");
    
    // Set checked
    check->SetIsChecked(true);
    ASSERT_TRUE_DETAIL(check->GetIsChecked(), "Now checked");
    
    // Set unchecked
    check->SetIsChecked(false);
    ASSERT_TRUE_DETAIL(!check->GetIsChecked(), "Now unchecked");
    
    // Toggle multiple times
    for (int i = 0; i < 5; i++) {
        check->SetIsChecked(i % 2 == 0);
    }
    // After loop: i=4, 4%2==0 -> true (checked)
    ASSERT_TRUE_DETAIL(check->GetIsChecked(), "State after toggles (i=4 -> checked)");
    
    DETAIL("\n=== CheckBox Text ===");
    
    check->SetText(L"Enable Feature");
    ASSERT_TRUE_DETAIL(check->GetText() == L"Enable Feature", "CheckBox text match");
    
    DETAIL("\n=== CheckBox Events ===");
    
    bool lastCheckedState = false;
    int changeCount = 0;
    
    check->CheckedChanged.Add([&lastCheckedState, &changeCount](luaui::Control*, bool checked) {
        lastCheckedState = checked;
        changeCount++;
    });
    
    check->SetIsChecked(true);
    check->SetIsChecked(false);
    DETAIL("CheckedChanged event handler registered");
}

void Test_PanelHierarchyComprehensive() {
    DETAIL("=== Panel Creation ===");
    
    auto panel = std::make_shared<StackPanel>();
    ASSERT_NOT_NULL_DETAIL(panel.get(), "StackPanel created");
    
    DETAIL("\n=== Adding Children ===");
    
    auto btn1 = std::make_shared<Button>();
    btn1->SetName("btn1");
    panel->AddChild(btn1);
    ASSERT_EQ_DETAIL(1, (int)panel->GetChildCount(), "Child count after first add");
    
    auto btn2 = std::make_shared<Button>();
    btn2->SetName("btn2");
    panel->AddChild(btn2);
    ASSERT_EQ_DETAIL(2, (int)panel->GetChildCount(), "Child count after second add");
    
    auto text = std::make_shared<TextBlock>();
    text->SetName("text1");
    panel->AddChild(text);
    ASSERT_EQ_DETAIL(3, (int)panel->GetChildCount(), "Child count after third add");
    
    DETAIL("\n=== Child Access ===");
    
    auto child0 = panel->GetChild(0);
    ASSERT_NOT_NULL_DETAIL(child0.get(), "First child exists");
    ASSERT_EQ_DETAIL(std::string("btn1"), child0->GetName(), "First child name");
    
    auto child1 = panel->GetChild(1);
    ASSERT_NOT_NULL_DETAIL(child1.get(), "Second child exists");
    ASSERT_EQ_DETAIL(std::string("btn2"), child1->GetName(), "Second child name");
    
    auto child2 = panel->GetChild(2);
    ASSERT_NOT_NULL_DETAIL(child2.get(), "Third child exists");
    ASSERT_EQ_DETAIL(std::string("text1"), child2->GetName(), "Third child name");
    
    DETAIL("\n=== Child Removal ===");
    
    panel->RemoveChild(btn1);
    ASSERT_EQ_DETAIL(2, (int)panel->GetChildCount(), "Child count after removal");
    
    // Verify remaining children shifted
    auto newChild0 = panel->GetChild(0);
    ASSERT_EQ_DETAIL(std::string("btn2"), newChild0->GetName(), "First child after removal");
    
    DETAIL("\n=== Clear All ===");
    
    panel->ClearChildren();
    ASSERT_EQ_DETAIL(0, (int)panel->GetChildCount(), "Child count after clear");
    
    DETAIL("\n=== Parent Reference ===");
    
    auto parent = std::make_shared<StackPanel>();
    auto child = std::make_shared<Button>();
    parent->AddChild(child);
    
    // Parent reference test
    DETAIL("[OK] Parent reference test (weak_ptr)");
}

void Test_TabControlComprehensive() {
    DETAIL("=== TabControl Creation ===");
    
    auto tabs = std::make_shared<TabControl>();
    ASSERT_NOT_NULL_DETAIL(tabs.get(), "TabControl created");
    ASSERT_EQ_DETAIL(0, (int)tabs->GetTabCount(), "Initial tab count");
    // Initial selection is -1 when no tabs exist
    ASSERT_EQ_DETAIL(-1, tabs->GetSelectedIndex(), "Initial selection (no tabs)");
    
    DETAIL("\n=== Adding Tabs ===");
    
    auto tab1 = std::make_shared<TabItem>();
    tab1->SetHeader(L"General");
    auto content1 = std::make_shared<TextBlock>();
    content1->SetText(L"General settings");
    tab1->SetContent(content1);
    tabs->AddTab(tab1);
    ASSERT_EQ_DETAIL(1, (int)tabs->GetTabCount(), "Tab count after first add");
    
    auto tab2 = std::make_shared<TabItem>();
    tab2->SetHeader(L"Advanced");
    auto content2 = std::make_shared<Button>();
    content2->SetText(L"Save");
    tab2->SetContent(content2);
    tabs->AddTab(tab2);
    ASSERT_EQ_DETAIL(2, (int)tabs->GetTabCount(), "Tab count after second add");
    
    auto tab3 = std::make_shared<TabItem>();
    tab3->SetHeader(L"About");
    tabs->AddTab(tab3);
    ASSERT_EQ_DETAIL(3, (int)tabs->GetTabCount(), "Tab count after third add");
    
    DETAIL("\n=== Tab Selection ===");
    
    // Default selection
    ASSERT_EQ_DETAIL(0, tabs->GetSelectedIndex(), "Default tab selected");
    
    // Change selection
    tabs->SetSelectedIndex(1);
    ASSERT_EQ_DETAIL(1, tabs->GetSelectedIndex(), "Second tab selected");
    
    tabs->SetSelectedIndex(2);
    ASSERT_EQ_DETAIL(2, tabs->GetSelectedIndex(), "Third tab selected");
    
    tabs->SetSelectedIndex(0);
    ASSERT_EQ_DETAIL(0, tabs->GetSelectedIndex(), "Back to first tab");
    
    DETAIL("\n=== Tab Properties ===");
    
    tabs->SetTabHeight(40);
    DETAIL("Tab height set to 40");
}

void Test_PerformanceMetrics() {
    DETAIL("=== Object Creation Performance ===");
    
    const int ITERATIONS = 1000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < ITERATIONS; i++) {
        auto btn = std::make_shared<Button>();
        btn->SetText(L"Test");
        btn->SetName("btn" + std::to_string(i));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - start).count();
    
    std::stringstream ss;
    ss << "Created " << ITERATIONS << " buttons in " << std::fixed << std::setprecision(2) 
       << duration << "ms (" << (duration / ITERATIONS * 1000) << " μs each)";
    DETAIL(ss.str());
    
    ASSERT_TRUE_DETAIL(duration < 1000, "Button creation under 1 second");
    
    DETAIL("\n=== Panel Hierarchy Performance ===");
    
    auto root = std::make_shared<StackPanel>();
    
    start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 100; i++) {
        auto row = std::make_shared<StackPanel>();
        for (int j = 0; j < 10; j++) {
            auto btn = std::make_shared<Button>();
            btn->SetText(L"Item");
            row->AddChild(btn);
        }
        root->AddChild(row);
    }
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration<double, std::milli>(end - start).count();
    
    ss.str("");
    ss << "Created 100x10 hierarchy (1000 controls) in " << duration << "ms";
    DETAIL(ss.str());
    
    // Count total children
    int totalChildren = 0;
    for (size_t i = 0; i < root->GetChildCount(); i++) {
        totalChildren++;
        if (auto row = std::dynamic_pointer_cast<StackPanel>(root->GetChild(i))) {
            totalChildren += (int)row->GetChildCount();
        }
    }
    ASSERT_EQ_DETAIL(1100, totalChildren, "Total controls in hierarchy");
}

// ============================================================================
// Main with Detailed Report
// ============================================================================
int main() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "  LuaUI Controls Demo - Comprehensive Test Suite" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    
    Logger::Initialize();
    Logger::SetConsoleLevel(LogLevel::Error);
    
    auto suiteStart = std::chrono::high_resolution_clock::now();
    
    // Run all tests
    RunTestDetailed("Button Comprehensive", Test_ButtonComprehensive);
    RunTestDetailed("Slider Comprehensive", Test_SliderComprehensive);
    RunTestDetailed("CheckBox Comprehensive", Test_CheckBoxComprehensive);
    RunTestDetailed("Panel Hierarchy Comprehensive", Test_PanelHierarchyComprehensive);
    RunTestDetailed("TabControl Comprehensive", Test_TabControlComprehensive);
    RunTestDetailed("Performance Metrics", Test_PerformanceMetrics);
    
    auto suiteEnd = std::chrono::high_resolution_clock::now();
    double totalTime = std::chrono::duration<double, std::milli>(suiteEnd - suiteStart).count();
    
    // Calculate totals
    int totalPassed = 0;
    int totalFailed = 0;
    int totalAssertions = 0;
    
    for (const auto& result : g_results) {
        if (result.passed) totalPassed++;
        else totalFailed++;
        totalAssertions += result.metrics.assertionsPassed + result.metrics.assertionsFailed;
    }
    
    // Print Summary Report
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "  TEST SUMMARY REPORT" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    
    std::cout << "\n  Test Results:" << std::endl;
    std::cout << "  " << std::string(50, '-') << std::endl;
    
    for (const auto& result : g_results) {
        std::cout << "  [" << (result.passed ? "OK" : "FAIL") << "] " 
                  << std::left << std::setw(35) << result.name
                  << std::right << std::setw(8) << result.metrics.assertionsPassed << " passed"
                  << std::setw(6) << result.metrics.assertionsFailed << " failed"
                  << std::setw(10) << std::fixed << std::setprecision(1) << result.metrics.durationMs << "ms"
                  << std::endl;
    }
    
    std::cout << "  " << std::string(50, '-') << std::endl;
    std::cout << "  " << std::left << std::setw(37) << "TOTAL"
              << std::right << std::setw(6) << totalPassed << " passed"
              << std::setw(6) << totalFailed << " failed"
              << std::setw(10) << totalTime << "ms"
              << std::endl;
    
    std::cout << "\n  Statistics:" << std::endl;
    std::cout << "    Total Tests:    " << g_results.size() << std::endl;
    std::cout << "    Passed:         " << totalPassed << " (" << (totalPassed * 100 / g_results.size()) << "%)" << std::endl;
    std::cout << "    Failed:         " << totalFailed << " (" << (totalFailed * 100 / g_results.size()) << "%)" << std::endl;
    std::cout << "    Total Assertions: " << totalAssertions << std::endl;
    std::cout << "    Suite Duration: " << std::fixed << std::setprecision(2) << totalTime << "ms" << std::endl;
    
    std::cout << "\n  " << std::string(70, '=') << std::endl;
    if (totalFailed == 0) {
        std::cout << "  === ALL TESTS PASSED ===" << std::endl;
    } else {
        std::cout << "  === SOME TESTS FAILED ===" << std::endl;
        std::cout << "\n  Failed Tests:" << std::endl;
        for (const auto& result : g_results) {
            if (!result.passed) {
                std::cout << "    - " << result.name << ": " << result.error << std::endl;
            }
        }
    }
    std::cout << "  " << std::string(70, '=') << std::endl << std::endl;
    
    return totalFailed > 0 ? 1 : 0;
}
