// LuaUI Visual Tests - Lua MVVM Version
// Uses Lua + MVVM + XML architecture for all visual tests

#include "LuaMvvmTestHost.h"
#include "utils/Logger.h"
#include <iostream>
#include <windows.h>
#include <string>
#include <map>

std::string ResolveTestAlias(const std::string& input) {
    static const std::map<std::string, std::string> aliases = {
        {"grid", "Grid Layout"},
        {"stack", "StackPanel Layout"},
        {"canvas", "Canvas Layout"},
        {"dock", "DockPanel Layout"},
        {"wrap", "WrapPanel Layout"},
        {"scroll", "ScrollViewer Layout"},
        {"viewbox", "Viewbox Layout"},
        {"docklayout", "Dock Layout"}
    };

    // Case-insensitive lookup
    std::string lower = input;
    for (char& c : lower) {
        c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    }

    auto it = aliases.find(lower);
    return (it != aliases.end()) ? it->second : input;
}

void PrintUsage(const std::vector<luaui::test::LuaMvvmTestInfo>& tests) {
    std::cout << "Usage: visual_tests_lua_mvvm.exe [test_name]" << std::endl;
    std::cout << std::endl;
    std::cout << "Available tests:" << std::endl;
    for (const auto& test : tests) {
        std::cout << "  " << test.name << std::endl;
    }
    std::cout << std::endl;
    std::cout << "Shortcuts:" << std::endl;
    std::cout << "  grid       -> Grid Layout" << std::endl;
    std::cout << "  stack      -> StackPanel Layout" << std::endl;
    std::cout << "  canvas     -> Canvas Layout" << std::endl;
    std::cout << "  dock       -> DockPanel Layout" << std::endl;
    std::cout << "  wrap       -> WrapPanel Layout" << std::endl;
    std::cout << "  scroll     -> ScrollViewer Layout" << std::endl;
    std::cout << "  viewbox    -> Viewbox Layout" << std::endl;
    std::cout << "  docklayout -> Dock Layout" << std::endl;
    std::cout << std::endl;
    std::cout << "Use --list or --help to show this message." << std::endl;
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

        // 解析别名
        std::string resolvedName = ResolveTestAlias(testName);

        // 查找测试
        auto it = testMap.find(resolvedName);
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

    // 无参数：显示帮助信息
    std::cout << "No test name provided." << std::endl;
    std::cout << std::endl;
    PrintUsage(tests);
    return 1;
}
