/**
 * @file LuaUI.cpp
 * @brief LuaUI主实现文件
 */

#include "LuaUI.h"
#include "core/App.h"
#include "core/LuaState.h"
#include "core/ScriptEngine.h"
#include "core/Utils.h"
#include <cassert>

namespace LuaUI {

static App* g_app = nullptr;

bool Initialize() {
    if (g_app) {
        return true; // 已经初始化
    }
    
    g_app = new App();
    if (!g_app->initialize("LuaUI Application")) {
        delete g_app;
        g_app = nullptr;
        return false;
    }
    
    return true;
}

void Shutdown() {
    if (g_app) {
        g_app->shutdown();
        delete g_app;
        g_app = nullptr;
    }
}

int RunMainLoop() {
    if (g_app) {
        return g_app->run();
    }
    return -1;
}

ILayoutEngine* GetLayoutEngine() {
    if (g_app) {
        return g_app->getLayoutEngine();
    }
    return nullptr;
}

IScriptEngine* GetScriptEngine() {
    if (g_app) {
        return g_app->getScriptEngine();
    }
    return nullptr;
}

} // namespace LuaUI
