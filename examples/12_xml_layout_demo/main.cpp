// 12_xml_layout_demo - XML Layout System with Code-Behind
// 演示从外部 XML 配置文件加载 UI，并通过代码后置处理事件

#include "MainWindow.h"
#include "Logger.h"
#include <objbase.h>
#include <windows.h>
#include <io.h>
#include <fcntl.h>

// 为 GUI 程序创建控制台窗口用于日志输出
void SetupConsole() {
    // 分配一个新的控制台
    AllocConsole();
    
    // 重定向 stdout/stderr 到控制台
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    
    // 设置控制台标题
    SetConsoleTitleW(L"XML Layout Demo - Logs");
    
    // 启用宽字符输出
    _setmode(_fileno(stdout), _O_TEXT);
    _setmode(_fileno(stderr), _O_TEXT);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    // 创建控制台窗口用于日志输出
    SetupConsole();
    
    // 初始化日志系统（控制台输出）
    luaui::utils::Logger::Initialize();
    luaui::utils::Logger::EnableConsole(true);
    luaui::utils::Logger::SetConsoleLevel(luaui::utils::LogLevel::Debug);
    luaui::utils::Logger::Info("=== XML Layout Demo Starting ===");

    // 初始化 COM（某些功能可能需要）
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        MessageBoxW(nullptr, L"Failed to initialize COM", L"Error", MB_OK);
        return 1;
    }

    // 创建并运行主窗口
    demo::MainWindow window;
    if (!window.Initialize(hInstance, nCmdShow)) {
        CoUninitialize();
        return 1;
    }

    int result = window.Run();
    CoUninitialize();
    return result;
}
