/**
 * @file App.cpp
 * @brief 应用程序类实现
 */

#include "core/App.h"
#include "ui/layout/LayoutEngine.h"
#include "core/ScriptEngine.h"
#include <windows.h>
#include <winbase.h>

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
    
    // 创建布局引擎
    m_layoutEngine = new Layout::LayoutEngine();
    
    // 创建脚本引擎
    m_scriptEngine = new Core::ScriptEngine();
    
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
    
    // 在真实应用中，这里应该启动MFC消息循环
    // 但现在我们只是等待一段时间或直到用户关闭应用
    // 这里暂时使用一个简单的循环来避免立即退出
    
    MSG msg;
    while (true) {
        // 检查消息队列
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            // 没有消息时短暂休眠，避免CPU占用过高
            Sleep(10);
        }
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
    return m_scriptEngine;
}

} // namespace LuaUI
