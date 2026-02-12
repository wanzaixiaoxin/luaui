// XML Layout Demo - 使用框架 Window 类
// 展示 XML 布局解析（简化版本）

#include "MainWindow.h"
#include "Logger.h"
#include <windows.h>
#include <iostream>

using namespace luaui::utils;

int main() {
    HINSTANCE hInstance = GetModuleHandle(nullptr);
    
    Logger::Initialize();
    Logger::SetConsoleLevel(LogLevel::Debug);
    
    try {
        MainWindow window;
        
        if (!window.Create(hInstance, L"XML Layout Demo", 900, 700)) {
            Logger::Error("Failed to create window");
            return 1;
        }
        
        window.Show(SW_SHOW);
        return window.Run();
        
    } catch (const std::exception& e) {
        Logger::ErrorF("Exception: %s", e.what());
        return 1;
    }
}
