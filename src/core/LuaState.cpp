/**
 * @file LuaState.cpp
 * @brief Lua状态管理器实现
 */

#include "core/LuaState.h"
#include "lua/binding/LuaBinder.h"
#include "lua/binding/LoggerBinder.h"
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
#include <cassert>

namespace LuaUI {
namespace Core {

LuaState::LuaState()
    : m_luaState(nullptr)
    , m_initialized(false)
{
}

LuaState::~LuaState() {
    shutdown();
}

bool LuaState::initialize() {
    if (m_initialized) {
        return true;
    }
    
    // 创建Lua虚拟机
    m_luaState = luaL_newstate();
    if (!m_luaState) {
        setLastError("Failed to create Lua state");
        return false;
    }
    
    // 打开标准库
    luaL_openlibs(m_luaState);

    Lua::Binding::LuaBinder binder(m_luaState);
    Lua::Binding::registerLogger(&binder);

    m_initialized = true;
    return true;
}

void LuaState::shutdown() {
    if (m_luaState) {
        lua_close(m_luaState);
        m_luaState = nullptr;
    }
    m_initialized = false;
    m_lastError.clear();
}

bool LuaState::isInitialized() const {
    return m_initialized;
}

lua_State* LuaState::getState() const {
    return m_luaState;
}

bool LuaState::loadFile(const std::string& filepath) {
    if (!m_initialized) {
        setLastError("Lua state is not initialized");
        return false;
    }
    
    clearStack();
    
    // 加载Lua文件
    int result = luaL_loadfile(m_luaState, filepath.c_str());
    if (result != LUA_OK) {
        std::string error = lua_tostring(m_luaState, -1);
        setLastError("Failed to load file: " + error);
        lua_pop(m_luaState, 1);
        return false;
    }
    
    // 执行加载的脚本
    result = lua_pcall(m_luaState, 0, 0, 0);
    if (result != LUA_OK) {
        std::string error = lua_tostring(m_luaState, -1);
        setLastError("Failed to execute file: " + error);
        lua_pop(m_luaState, 1);
        return false;
    }
    
    return true;
}

bool LuaState::loadString(const std::string& script) {
    if (!m_initialized) {
        setLastError("Lua state is not initialized");
        return false;
    }
    
    clearStack();
    
    // 加载Lua字符串
    int result = luaL_loadstring(m_luaState, script.c_str());
    if (result != LUA_OK) {
        std::string error = lua_tostring(m_luaState, -1);
        setLastError("Failed to load string: " + error);
        lua_pop(m_luaState, 1);
        return false;
    }
    
    // 执行加载的脚本
    result = lua_pcall(m_luaState, 0, 0, 0);
    if (result != LUA_OK) {
        std::string error = lua_tostring(m_luaState, -1);
        setLastError("Failed to execute string: " + error);
        lua_pop(m_luaState, 1);
        return false;
    }
    
    return true;
}

bool LuaState::callFunction(const std::string& funcName, int nargs, int nresults) {
    if (!m_initialized) {
        setLastError("Lua state is not initialized");
        return false;
    }
    
    // 查找函数
    int top = lua_gettop(m_luaState);
    
    // 支持模块名.函数名格式
    std::string::size_type dotPos = funcName.find('.');
    if (dotPos != std::string::npos) {
        // 模块.函数格式
        std::string module = funcName.substr(0, dotPos);
        std::string func = funcName.substr(dotPos + 1);
        
        lua_getglobal(m_luaState, module.c_str());
        if (!lua_istable(m_luaState, -1)) {
            lua_settop(m_luaState, top);
            setLastError("Module not found: " + module);
            return false;
        }
        
        lua_getfield(m_luaState, -1, func.c_str());
        lua_remove(m_luaState, -2); // 移除模块表
    } else {
        // 仅函数名格式
        lua_getglobal(m_luaState, funcName.c_str());
    }
    
    if (!lua_isfunction(m_luaState, -1)) {
        lua_settop(m_luaState, top);
        setLastError("Function not found: " + funcName);
        return false;
    }
    
    // 调用函数
    int result = lua_pcall(m_luaState, nargs, nresults, 0);
    if (result != LUA_OK) {
        std::string error = lua_tostring(m_luaState, -1);
        setLastError("Failed to call function: " + error);
        lua_pop(m_luaState, 1);
        return false;
    }
    
    return true;
}

void LuaState::registerFunction(const std::string& name, lua_CFunction func) {
    if (!m_initialized) {
        return;
    }
    
    // 支持模块名.函数名格式
    std::string::size_type dotPos = name.find('.');
    if (dotPos != std::string::npos) {
        std::string module = name.substr(0, dotPos);
        std::string funcName = name.substr(dotPos + 1);
        
        lua_getglobal(m_luaState, module.c_str());
        if (!lua_istable(m_luaState, -1)) {
            lua_pop(m_luaState, 1);
            lua_newtable(m_luaState);
            lua_setglobal(m_luaState, module.c_str());
            lua_getglobal(m_luaState, module.c_str());
        }
        
        lua_pushcfunction(m_luaState, func);
        lua_setfield(m_luaState, -2, funcName.c_str());
        lua_pop(m_luaState, 1);
    } else {
        lua_register(m_luaState, name.c_str(), func);
    }
}

bool LuaState::doString(const std::string& script) {
    return loadString(script);
}

bool LuaState::doFile(const std::string& filepath) {
    return loadFile(filepath);
}

std::string LuaState::getLastError() const {
    return m_lastError;
}

void LuaState::clearStack() {
    if (m_luaState) {
        lua_settop(m_luaState, 0);
    }
}

void LuaState::setLastError(const std::string& error) {
    m_lastError = error;
}

} // namespace Core
} // namespace LuaUI
