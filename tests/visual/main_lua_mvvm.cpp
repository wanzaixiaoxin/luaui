// LuaUI Visual Tests - Lua MVVM Version
// Uses Lua + MVVM + XML architecture for all visual tests

#include "LuaMvvmTestHost.h"
#include "utils/Logger.h"
#include <iostream>
#include <windows.h>

int main() {
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
    
    std::cout << "Starting visual test runner..." << std::endl;
    std::cout << std::endl;
    
    // 运行测试运行器
    return host.RunTestRunner();
}
