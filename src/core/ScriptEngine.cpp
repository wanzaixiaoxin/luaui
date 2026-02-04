/**
 * @file ScriptEngine.cpp
 * @brief 脚本引擎实现
 */

#include "core/ScriptEngine.h"
#include <algorithm>

namespace LuaUI {
namespace Core {

ScriptEngine::ScriptEngine()
    : m_initialized(false)
    , m_loadCallback(nullptr)
    , m_unloadCallback(nullptr)
{
}

ScriptEngine::~ScriptEngine() {
    shutdown();
}

bool ScriptEngine::initialize() {
    if (m_initialized) {
        return true;
    }
    
    // 初始化Lua状态
    if (!m_luaState.initialize()) {
        return false;
    }
    
    m_initialized = true;
    return true;
}

void ScriptEngine::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    unloadAllScripts();
    m_luaState.shutdown();
    m_initialized = false;
}

bool ScriptEngine::isInitialized() const {
    return m_initialized;
}

LuaState* ScriptEngine::getLuaState() {
    return &m_luaState;
}

bool ScriptEngine::loadScript(const std::string& scriptFile) {
    if (!m_initialized) {
        return false;
    }
    
    // 加载脚本文件
    if (!m_luaState.loadFile(scriptFile)) {
        return false;
    }
    
    // 记录已加载的脚本
    std::string scriptName = scriptFile;
    m_loadedScripts[scriptName] = true;
    
    // 触发加载回调
    triggerLoadCallback(scriptName);
    
    return true;
}

bool ScriptEngine::loadScriptString(const std::string& scriptContent) {
    if (!m_initialized) {
        return false;
    }
    
    return m_luaState.loadString(scriptContent);
}

bool ScriptEngine::executeScript(const std::string& scriptFile) {
    return loadScript(scriptFile);
}

bool ScriptEngine::executeString(const std::string& scriptContent) {
    return loadScriptString(scriptContent);
}

bool ScriptEngine::callFunction(const std::string& funcName) {
    if (!m_initialized) {
        return false;
    }
    
    return m_luaState.callFunction(funcName);
}

bool ScriptEngine::callFunctionWithArgs(const std::string& funcName, 
                                          const std::vector<std::string>& args) {
    if (!m_initialized) {
        return false;
    }
    
    // 获取函数
    int top = lua_gettop(m_luaState.getState());
    
    // 查找函数
    std::string::size_type dotPos = funcName.find('.');
    if (dotPos != std::string::npos) {
        std::string module = funcName.substr(0, dotPos);
        std::string func = funcName.substr(dotPos + 1);
        
        lua_getglobal(m_luaState.getState(), module.c_str());
        if (!lua_istable(m_luaState.getState(), -1)) {
            lua_settop(m_luaState.getState(), top);
            return false;
        }
        
        lua_getfield(m_luaState.getState(), -1, func.c_str());
        lua_remove(m_luaState.getState(), -2);
    } else {
        lua_getglobal(m_luaState.getState(), funcName.c_str());
    }
    
    if (!lua_isfunction(m_luaState.getState(), -1)) {
        lua_settop(m_luaState.getState(), top);
        return false;
    }
    
    // 压入参数
    for (size_t i = 0; i < args.size(); ++i) {
        lua_pushstring(m_luaState.getState(), args[i].c_str());
    }
    
    // 调用函数
    int result = lua_pcall(m_luaState.getState(), args.size(), 0, 0);
    if (result != LUA_OK) {
        std::string error = lua_tostring(m_luaState.getState(), -1);
        lua_pop(m_luaState.getState(), 1);
        return false;
    }
    
    return true;
}

void ScriptEngine::registerFunction(const std::string& name, lua_CFunction func) {
    m_luaState.registerFunction(name, func);
}

void ScriptEngine::setPackagePath(const std::string& path) {
    if (!m_initialized) {
        return;
    }
    
    lua_State* L = m_luaState.getState();
    lua_getglobal(L, "package");
    lua_pushstring(L, path.c_str());
    lua_setfield(L, -2, "path");
    lua_pop(L, 1);
}

void ScriptEngine::addPackagePath(const std::string& path) {
    if (!m_initialized) {
        return;
    }
    
    lua_State* L = m_luaState.getState();
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "path");
    const char* currentPath = lua_tostring(L, -1);
    std::string newPath = currentPath;
    newPath += ";";
    newPath += path;
    lua_pop(L, 1);
    lua_pushstring(L, newPath.c_str());
    lua_setfield(L, -2, "path");
    lua_pop(L, 1);
}

void ScriptEngine::setLoadCallback(ScriptLifecycleCallback callback) {
    m_loadCallback = callback;
}

void ScriptEngine::setUnloadCallback(ScriptLifecycleCallback callback) {
    m_unloadCallback = callback;
}

std::vector<std::string> ScriptEngine::getLoadedScripts() const {
    std::vector<std::string> scripts;
    for (std::map<std::string, bool>::const_iterator it = m_loadedScripts.begin();
         it != m_loadedScripts.end(); ++it) {
        scripts.push_back(it->first);
    }
    return scripts;
}

bool ScriptEngine::isScriptLoaded(const std::string& scriptName) const {
    return m_loadedScripts.find(scriptName) != m_loadedScripts.end();
}

bool ScriptEngine::unloadScript(const std::string& scriptName) {
    std::map<std::string, bool>::iterator it = m_loadedScripts.find(scriptName);
    if (it == m_loadedScripts.end()) {
        return false;
    }
    
    // 触发卸载回调
    triggerUnloadCallback(scriptName);
    
    m_loadedScripts.erase(it);
    return true;
}

void ScriptEngine::unloadAllScripts() {
    // 先触发所有卸载回调
    for (std::map<std::string, bool>::iterator it = m_loadedScripts.begin();
         it != m_loadedScripts.end(); ++it) {
        triggerUnloadCallback(it->first);
    }
    
    m_loadedScripts.clear();
}

bool ScriptEngine::reloadScript(const std::string& scriptName) {
    if (!isScriptLoaded(scriptName)) {
        return false;
    }
    
    // 卸载脚本
    unloadScript(scriptName);
    
    // 重新加载
    return loadScript(scriptName);
}

void ScriptEngine::triggerLoadCallback(const std::string& scriptName) {
    if (m_loadCallback) {
        m_loadCallback(scriptName);
    }
}

void ScriptEngine::triggerUnloadCallback(const std::string& scriptName) {
    if (m_unloadCallback) {
        m_unloadCallback(scriptName);
    }
}

} // namespace Core
} // namespace LuaUI
