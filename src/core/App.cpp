/**
 * @file App.cpp
 * @brief 应用程序类实现
 */

#include "core/App.h"

namespace LuaUI {

App::App()
    : m_initialized(false)
    , m_exitCode(0)
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
    m_initialized = true;
    
    return true;
}

void App::shutdown() {
    if (!m_initialized) {
        return;
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
    
    // 主循环 - 将在实际实现中由MFC消息循环接管
    return m_exitCode;
}

void App::exit(int exitCode) {
    m_exitCode = exitCode;
    // 将在实际实现中调用MFC的退出函数
}

ILayoutEngine* App::getLayoutEngine() {
    // 将在实际实现中返回布局引擎实例
    return nullptr;
}

IScriptEngine* App::getScriptEngine() {
    // 将在实际实现中返回脚本引擎实例
    return nullptr;
}

} // namespace LuaUI
