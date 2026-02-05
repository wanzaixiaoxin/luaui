/**
 * @file App.cpp
 * @brief 应用程序类实现
 */

#include "core/App.h"
#include "ui/layout/LayoutEngine.h"
#include <winsock2.h>  // Must come before windows.h when using MFC
#include <windows.h>
#include <winbase.h>
#include <afxwin.h> // MFC support

namespace LuaUI {

App::App()
    : m_initialized(false)
    , m_exitCode(0)
    , m_layoutEngine(nullptr)
    , m_scriptEngine(nullptr)
{
}

App::~App() {
    shutdown();
}

bool App::initialize(const std::string& appTitle) {
    if (m_initialized) {
        return true;
    }
    
    m_appTitle = appTitle;
    
    // 初始化MFC
    if (!AfxWinInit(GetModuleHandle(NULL), NULL, ::GetCommandLine(), SW_SHOWDEFAULT)) {
        return false;
    }
    
    // 创建布局引擎
    m_layoutEngine = new Layout::LayoutEngine();
    
    // 创建脚本引擎
    m_scriptEngine = new Core::ScriptEngine();
    
    // 初始化脚本引擎
    if (!m_scriptEngine->initialize()) {
        delete m_scriptEngine;
        m_scriptEngine = nullptr;
        return false;
    }
    
    m_initialized = true;
    
    return true;
}

void App::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    // 清理引擎实例
    if (m_layoutEngine) {
        delete m_layoutEngine;
        m_layoutEngine = nullptr;
    }
    
    if (m_scriptEngine) {
        delete m_scriptEngine;
        m_scriptEngine = nullptr;
    }
    
    m_initialized = false;
}

bool App::isInitialized() const {
    return m_initialized;
}

int App::run() {
    if (!m_initialized) {
        return -1;
    }
    
    // 使用MFC的消息循环
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        // 检查是否是退出消息
        if (msg.message == WM_QUIT) {
            break;
        }
        
        // 处理消息
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return static_cast<int>(msg.wParam);
}

void App::exit(int exitCode) {
    m_exitCode = exitCode;
    // 将在实际实现中调用MFC的退出函数
}

ILayoutEngine* App::getLayoutEngine() {
    return m_layoutEngine;
}

IScriptEngine* App::getScriptEngine() {
    return static_cast<IScriptEngine*>(m_scriptEngine);
}

} // namespace LuaUI
