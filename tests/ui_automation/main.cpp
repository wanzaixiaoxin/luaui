// main.cpp - Entry point for UI Automation Tests
// Simplified version for demonstration

#include <iostream>
#include <windows.h>

// Include all test files
#include "examples/ButtonClickTest.cpp"
#include "examples/LayoutTest.cpp"
#include "examples/VisualRegressionTest.cpp"
#include "examples/MVVM_BindingTest.cpp"

int main(int argc, char** argv) {
    std::cout << "========================================" << std::endl;
    std::cout << "LuaUI UI Automation Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Initialize COM for WIC
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize COM" << std::endl;
        return 1;
    }
    
    // Create output directories
    std::filesystem::create_directories("tests/ui_automation/output");
    std::filesystem::create_directories("tests/ui_automation/baselines");
    
    // Run all UI tests
    int result = luaui::test::automation::UIAutomationRunner::Instance().RunAll();
    
    CoUninitialize();
    
    return result;
}
