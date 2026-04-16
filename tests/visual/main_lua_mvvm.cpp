// LuaUI Visual Tests - Lua MVVM Version
// Uses Lua + MVVM + XML architecture for all visual tests

#include "LuaMvvmTestHost.h"
#include "utils/Logger.h"
#include <iostream>
#include <windows.h>
#include <string>
#include <map>

void PrintUsage(const std::vector<luaui::test::LuaMvvmTestInfo>& tests) {
    std::cout << "Usage: visual_tests_lua_mvvm.exe [test_name]" << std::endl;
    std::cout << std::endl;
    std::cout << "Available tests:" << std::endl;
    for (const auto& test : tests) {
        std::cout << "  " << test.name << std::endl;
    }
    std::cout << std::endl;
    std::cout << "Run without arguments to start the test runner." << std::endl;
}

int main(int argc, char* argv[]) {
    // 控制台输出
    std::cout << "========================================" << std::endl;
    std::cout << "LuaUI Visual Test Suite (Lua MVVM)" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Architecture: Lua + MVVM + XML" << std::endl;
    std::cout << "- View: XML Layouts" << std::endl;
    std::cout << "- ViewModel: Lua Scripts" << std::endl;
    std::cout << "- Binding: Two-way MVVM" << std::endl;
    std::cout << std::endl;
    
    luaui::test::LuaMvvmTestHost host;
    
    // 获取 HINSTANCE
    HINSTANCE hInstance = GetModuleHandle(nullptr);
    
    // 初始化
    std::cout << "Initializing Lua MVVM host..." << std::endl;
    if (!host.Initialize(hInstance)) {
        std::cerr << "Failed to initialize!" << std::endl;
        return 1;
    }
    
    // 获取可用测试列表
    auto tests = host.GetAvailableTests();
    
    // 构建测试名称映射
    std::map<std::string, luaui::test::LuaMvvmTestInfo> testMap;
    for (const auto& test : tests) {
        testMap[test.name] = test;
    }
    
    // 检查命令行参数
    if (argc > 1) {
        std::string testName = argv[1];
        
        // 特殊参数：--list 或 --help
        if (testName == "--list" || testName == "--help" || testName == "-h") {
            PrintUsage(tests);
            return 0;
        }
        
        // 查找测试
        auto it = testMap.find(testName);
        if (it != testMap.end()) {
            const auto& test = it->second;
            std::cout << "Running test: " << test.name << std::endl;
            std::cout << "  Description: " << test.description << std::endl;
            std::cout << "  Layout: " << test.layoutFile << std::endl;
            std::cout << "  Script: " << test.scriptFile << std::endl;
            std::cout << std::endl;
            
            return host.RunTest(test.layoutFile, test.scriptFile, test.viewModelName);
        } else {
            std::cerr << "Unknown test: " << testName << std::endl;
            std::cout << std::endl;
            PrintUsage(tests);
            return 1;
        }
    }
    
    // 无参数：运行测试运行器
    std::cout << "Starting visual test runner..." << std::endl;
    std::cout << std::endl;
    std::cout << "Tip: Use 'visual_tests_lua_mvvm.exe --list' to see available tests" << std::endl;
    std::cout << "     Use 'visual_tests_lua_mvvm.exe \"ScrollViewer Layout\"' to run a specific test" << std::endl;
    std::cout << std::endl;
    
    return host.RunTestRunner();
}
