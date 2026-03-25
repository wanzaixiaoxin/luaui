// LuaUI MVVM Demo - 03 (方案 C: 框架原生支持)
// 
// 特性：
// - 只需 3 行核心代码启动
// - 自动绑定 XML Command 到 Lua 函数
// - 支持 {Binding Property} 和 Notify("Property")

#include <luaui/lua/LuaMvvmHost.h>
#include <luaui/core/Window.h>
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
    try {
        // 设置中止信号处理
        signal(SIGABRT, CustomAbortHandler);
        
        // 初始化日志系统
        utils::LoggerConfig config;
        config.consoleEnabled = true;
        config.consoleLevel = utils::LogLevel::Debug;
        config.fileEnabled = true;
        config.fileLevel = utils::LogLevel::Debug;
        config.logFilePath = "lua_mvvm_demo.log";
        config.createConsoleWindow = true;  // GUI程序需要创建控制台窗口才能看到输出
        config.consoleWindowTitle = "LuaUI MVVM Debug Console";
        utils::Logger::Initialize(config);
        
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

        if (auto* window = host.GetWindow()) {
            ::SetWindowPos(window->GetHandle(), nullptr, 0, 0, 980, 860,
                SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
        
        utils::Logger::Info("[Main] Initialization successful, starting message loop...");
        
        // 运行消息循环
        int result = host.Run();
        
        utils::Logger::InfoF("[Main] Message loop exited with code: %d", result);
        
        return result;
        
    } catch (const std::exception& e) {
        utils::Logger::ErrorF("[Main] C++ Exception: %s", e.what());
        MessageBoxA(NULL, e.what(), "C++ Exception", MB_ICONERROR);
        return 2;
    } catch (...) {
        utils::Logger::Error("[Main] Unknown exception!");
        MessageBoxA(NULL, "Unknown exception occurred", "Fatal Error", MB_ICONERROR);
        return 3;
    }
}
