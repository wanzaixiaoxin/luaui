// LuaUI MVVM Demo - 03 (方案 C: 框架原生支持)
//
// 特性：
// - 只需 3 行核心代码启动
// - 自动绑定 XML Command 到 Lua 函数
// - 支持 {Binding Property} 和 Notify("Property")
// - TextBox 属性（Placeholder/IsReadOnly/IsPassword/MaxLength）完全通过 XML 声明

#include <luaui/core/Window.h>
#include <luaui/lua/LuaMvvmHost.h>
#include <luaui/utils/Logger.h>

#include <windows.h>

#include <csignal>
#include <cstdlib>
#include <exception>

namespace {

void HandleAbortSignal(int signalCode) {
    (void)signalCode;
    luaui::utils::Logger::Error("[Main] Abort signal received!");
    ::MessageBoxA(nullptr, "Application aborted unexpectedly", "Fatal Error", MB_ICONERROR);
    std::exit(3);
}

} // namespace

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
    try {
        ::signal(SIGABRT, HandleAbortSignal);

        luaui::utils::LoggerConfig config;
        config.consoleEnabled = true;
        config.consoleLevel = luaui::utils::LogLevel::Debug;
        config.fileEnabled = true;
        config.fileLevel = luaui::utils::LogLevel::Debug;
        config.logFilePath = "lua_mvvm_demo.log";
        config.createConsoleWindow = true;
        config.consoleWindowTitle = "LuaUI MVVM Debug Console";
        luaui::utils::Logger::Initialize(config);

        luaui::utils::Logger::Info("============================================");
        luaui::utils::Logger::Info("Starting LuaUI MVVM Demo...");
        luaui::utils::Logger::Info("============================================");

        luaui::utils::Logger::Info("[Main] Creating LuaMvvmHost...");
        luaui::lua::LuaMvvmHost host;

        luaui::utils::Logger::Info("[Main] Initializing host...");
        if (!host.Initialize(hInstance, "scripts/ViewModel.lua", "layouts/MainView.xml")) {
            luaui::utils::Logger::Error("[Main] Initialization failed!");
            ::MessageBoxA(
                nullptr,
                "Initialization failed. Check console for details.",
                "Error",
                MB_ICONERROR);
            return 1;
        }

        if (auto* window = host.GetWindow()) {
            window->SetExtendFrameIntoClientArea(true);
            ::SetWindowPos(
                window->GetHandle(),
                nullptr,
                0,
                0,
                980,
                860,
                SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        }

        luaui::utils::Logger::Info("[Main] Initialization successful, starting message loop...");

        const int result = host.Run();
        luaui::utils::Logger::InfoF("[Main] Message loop exited with code: %d", result);
        return result;
    } catch (const std::exception& e) {
        luaui::utils::Logger::ErrorF("[Main] C++ Exception: %s", e.what());
        ::MessageBoxA(nullptr, e.what(), "C++ Exception", MB_ICONERROR);
        return 2;
    } catch (...) {
        luaui::utils::Logger::Error("[Main] Unknown exception!");
        ::MessageBoxA(nullptr, "Unknown exception occurred", "Fatal Error", MB_ICONERROR);
        return 3;
    }
}
