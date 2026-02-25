// LuaUI Visual Tests - Console Version (for CI)
// Validates that test scripts and layouts can be loaded

#include "utils/Logger.h"
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>

struct TestCase {
    std::string name;
    std::string layoutFile;
    std::string scriptFile;
};

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "LuaUI Visual Test Suite (Console Check)" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    // Define test cases
    std::vector<TestCase> tests = {
        {"Test Runner", "TestRunner.xml", "TestRunnerViewModel.lua"},
        {"StackPanel Layout", "StackPanelTest.xml", "StackPanelTestViewModel.lua"},
        {"Grid Layout", "GridTest.xml", "GridTestViewModel.lua"}
    };
    
    // Check if files exist - use executable directory
    char buffer[MAX_PATH];
    GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    std::filesystem::path exePath(buffer);
    std::filesystem::path exeDir = exePath.parent_path();
    std::filesystem::path layoutsPath = exeDir / "test_layouts";
    std::filesystem::path scriptsPath = exeDir / "test_scripts";
    
    std::cout << "Checking test resources..." << std::endl;
    std::cout << "Layouts path: " << layoutsPath << std::endl;
    std::cout << "Scripts path: " << scriptsPath << std::endl;
    std::cout << std::endl;
    
    int passed = 0;
    int failed = 0;
    
    for (const auto& test : tests) {
        std::cout << "Test: " << test.name << std::endl;
        
        std::filesystem::path layoutFile = layoutsPath / test.layoutFile;
        std::filesystem::path scriptFile = scriptsPath / test.scriptFile;
        
        bool layoutExists = std::filesystem::exists(layoutFile);
        bool scriptExists = std::filesystem::exists(scriptFile);
        
        if (layoutExists) {
            std::cout << "  [OK] Layout: " << test.layoutFile << std::endl;
        } else {
            std::cout << "  [FAIL] Layout not found: " << layoutFile << std::endl;
        }
        
        if (scriptExists) {
            std::cout << "  [OK] Script: " << test.scriptFile << std::endl;
        } else {
            std::cout << "  [FAIL] Script not found: " << scriptFile << std::endl;
        }
        
        if (layoutExists && scriptExists) {
            passed++;
        } else {
            failed++;
        }
        
        std::cout << std::endl;
    }
    
    std::cout << "========================================" << std::endl;
    std::cout << "Results: " << passed << " passed, " << failed << " failed" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return failed > 0 ? 1 : 0;
}
