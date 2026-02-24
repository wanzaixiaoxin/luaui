// MVVM Data Binding Demo
// 展示LuaUI的MVVM数据绑定功能

#include "MainWindow.h"
#include <Logger.h>
#include <windows.h>

int main() {
    HINSTANCE hInstance = GetModuleHandle(nullptr);
    
    luaui::utils::Logger::Initialize();
    luaui::utils::Logger::SetConsoleLevel(luaui::utils::LogLevel::Debug);
    
    try {
        MainWindow window;
        
        if (!window.Create(hInstance, L"MVVM Data Binding Demo", 900, 700)) {
            luaui::utils::Logger::Error("Failed to create window");
            return 1;
        }
        
        window.Show(SW_SHOW);
        return window.Run();
        
    } catch (const std::exception& e) {
        luaui::utils::Logger::ErrorF("Exception: %s", e.what());
        return 1;
    }
}
