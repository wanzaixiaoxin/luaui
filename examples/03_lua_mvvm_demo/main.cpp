// LuaUI MVVM Demo - 03 (方案 C: 框架原生支持)
// 
// 特性：
// - 只需 3 行核心代码启动
// - 自动绑定 XML Command 到 Lua 函数
// - 支持 {Binding Property} 和 Notify("Property")

#include <luaui/lua/LuaMvvmHost.h>
#include <luaui/utils/Logger.h>
#include <windows.h>
#include <iostream>
#include <exception>
#include <csignal>

using namespace luaui;
using namespace luaui::lua;

// 自定义崩溃处理
void CustomAbortHandler(int signal) {
    utils::Logger::Error("[Main] Abort signal received!");
    MessageBoxA(NULL, "Application aborted unexpectedly", "Fatal Error", MB_ICONERROR);
    exit(3);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
    // 先分配控制台并设置输出
    AllocConsole();
    FILE* dummy;
    freopen_s(&dummy, "CONOUT$", "w", stdout);
    
    try {
        // 设置中止信号处理
        signal(SIGABRT, CustomAbortHandler);
        
        utils::Logger::Info("============================================");
        utils::Logger::Info("Starting LuaUI MVVM Demo...");
        utils::Logger::Info("============================================");
        
        // 一键启动：初始化 + 加载 Lua + 加载 XML + 自动绑定
        utils::Logger::Info("[Main] Creating LuaMvvmHost...");
        LuaMvvmHost host;
        
        utils::Logger::Info("[Main] Initializing host...");
        if (!host.Initialize(hInstance, 
                            "scripts/ViewModel.lua", 
                            "layouts/MainView.xml")) {
            utils::Logger::Error("[Main] Initialization failed!");
            MessageBoxA(NULL, "Initialization failed. Check console for details.", "Error", MB_ICONERROR);
            return 1;
        }
        
        utils::Logger::Info("[Main] Initialization successful, starting message loop...");
        
        // 运行消息循环
        int result = host.Run();
        
        utils::Logger::InfoF("[Main] Message loop exited with code: %d", result);
        
        // 保持控制台窗口显示结果
        printf("\nPress any key to exit...\n");
        system("pause > nul");
        
        return result;
        
    } catch (const std::exception& e) {
        utils::Logger::ErrorF("[Main] C++ Exception: %s", e.what());
        MessageBoxA(NULL, e.what(), "C++ Exception", MB_ICONERROR);
        system("pause");
        return 2;
    } catch (...) {
        utils::Logger::Error("[Main] Unknown exception!");
        MessageBoxA(NULL, "Unknown exception occurred", "Fatal Error", MB_ICONERROR);
        system("pause");
        return 3;
    }
}
