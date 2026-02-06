/**
 * @file LuaBinder.cpp
 * @brief Lua绑定器实现
 */

#include "lua/binding/LuaBinder.h"
#include "utils/Logger.h"
#include <iostream>
#include <sstream>

namespace LuaUI {
namespace Lua {
namespace Binding {

LuaBinder::LuaBinder(lua_State* lua)
    : m_lua(lua)
{
}

LuaBinder::~LuaBinder() {
}

lua_State* LuaBinder::getState() const {
    return m_lua;
}

void LuaBinder::registerFunction(const std::string& name, lua_CFunction func) {
    if (!m_lua || !func) {
        return;
    }
    
    lua_register(m_lua, name.c_str(), func);
}

void LuaBinder::registerFunctionToTable(const std::string& tableName,
                                       const std::string& funcName,
                                       lua_CFunction func) {
    if (!m_lua || !func) {
        return;
    }
    
    // 确保表存在
    ensureTable(tableName);
    
    // 获取表
    lua_getglobal(m_lua, tableName.c_str());
    
    // 注册函数
    lua_pushcfunction(m_lua, func);
    lua_setfield(m_lua, -2, funcName.c_str());
    
    // 弹出表
    lua_pop(m_lua, 1);
}

void LuaBinder::registerFunctionToSubTable(const std::string& tableName,
                                         const std::string& subTableName,
                                         const std::string& funcName,
                                         lua_CFunction func) {
    if (!m_lua || !func) {
        return;
    }
    
    // 确保主表存在
    ensureTable(tableName);
    
    // 获取主表
    lua_getglobal(m_lua, tableName.c_str());
    
    // 确保子表存在
    lua_getfield(m_lua, -1, subTableName.c_str());
    if (!lua_istable(m_lua, -1)) {
        lua_pop(m_lua, 1);
        lua_newtable(m_lua);
        lua_setfield(m_lua, -2, subTableName.c_str());
        lua_getfield(m_lua, -1, subTableName.c_str());
    }
    
    // 注册函数
    lua_pushcfunction(m_lua, func);
    lua_setfield(m_lua, -2, funcName.c_str());
    
    // 弹出子表和主表
    lua_pop(m_lua, 2);
}

void LuaBinder::registerTable(const std::string& tableName) {
    if (!m_lua) {
        return;
    }
    
    ensureTable(tableName);
}

void LuaBinder::setGlobalInt(const std::string& name, int value) {
    if (!m_lua) {
        return;
    }
    
    lua_pushinteger(m_lua, value);
    lua_setglobal(m_lua, name.c_str());
}

void LuaBinder::setGlobalString(const std::string& name, const std::string& value) {
    if (!m_lua) {
        return;
    }
    
    lua_pushstring(m_lua, value.c_str());
    lua_setglobal(m_lua, name.c_str());
}

void LuaBinder::setGlobalBool(const std::string& name, bool value) {
    if (!m_lua) {
        return;
    }
    
    lua_pushboolean(m_lua, value ? 1 : 0);
    lua_setglobal(m_lua, name.c_str());
}

void LuaBinder::setTableInt(const std::string& tableName, const std::string& fieldName, int value) {
    if (!m_lua) {
        return;
    }

    lua_getglobal(m_lua, tableName.c_str());
    if (!lua_istable(m_lua, -1)) {
        lua_pop(m_lua, 1);
        return;
    }

    lua_pushinteger(m_lua, value);
    lua_setfield(m_lua, -2, fieldName.c_str());
    lua_pop(m_lua, 1);
}

void LuaBinder::setTableString(const std::string& tableName, const std::string& fieldName, const std::string& value) {
    if (!m_lua) {
        return;
    }

    lua_getglobal(m_lua, tableName.c_str());
    if (!lua_istable(m_lua, -1)) {
        lua_pop(m_lua, 1);
        return;
    }

    lua_pushstring(m_lua, value.c_str());
    lua_setfield(m_lua, -2, fieldName.c_str());
    lua_pop(m_lua, 1);
}

int LuaBinder::getGlobalInt(const std::string& name, int defaultValue) {
    if (!m_lua) {
        return defaultValue;
    }
    
    lua_getglobal(m_lua, name.c_str());
    int result = defaultValue;
    
    if (lua_isinteger(m_lua, -1)) {
        result = (int)lua_tointeger(m_lua, -1);
    }
    
    lua_pop(m_lua, 1);
    return result;
}

std::string LuaBinder::getGlobalString(const std::string& name, const std::string& defaultValue) {
    if (!m_lua) {
        return defaultValue;
    }
    
    lua_getglobal(m_lua, name.c_str());
    std::string result = defaultValue;
    
    if (lua_isstring(m_lua, -1)) {
        result = lua_tostring(m_lua, -1);
    }
    
    lua_pop(m_lua, 1);
    return result;
}

bool LuaBinder::getGlobalBool(const std::string& name, bool defaultValue) {
    if (!m_lua) {
        return defaultValue;
    }
    
    lua_getglobal(m_lua, name.c_str());
    bool result = defaultValue;
    
    if (lua_isboolean(m_lua, -1)) {
        result = (lua_toboolean(m_lua, -1) != 0);
    }
    
    lua_pop(m_lua, 1);
    return result;
}

bool LuaBinder::callFunction(const std::string& funcName, int nargs, int nresults) {
    if (!m_lua) {
        return false;
    }
    
    lua_getglobal(m_lua, funcName.c_str());
    
    if (!lua_isfunction(m_lua, -1)) {
        lua_pop(m_lua, 1);
        return false;
    }
    
    // 调用函数
    int result = lua_pcall(m_lua, nargs, nresults, 0);
    
    if (result != LUA_OK) {
        // 错误处理
        const char* error = lua_tostring(m_lua, -1);
        LOG_S_ERROR_CAT("LuaBinder") << "Lua error: " << error;
        lua_pop(m_lua, 1);
        return false;
    }
    
    return true;
}

bool LuaBinder::doString(const std::string& script) {
    if (!m_lua) {
        return false;
    }
    
    int result = luaL_dostring(m_lua, script.c_str());
    
    if (result != LUA_OK) {
        const char* error = lua_tostring(m_lua, -1);
        LOG_S_ERROR_CAT("LuaBinder") << "Lua error: " << error;
        lua_pop(m_lua, 1);
        return false;
    }
    
    return true;
}

bool LuaBinder::doFile(const std::string& filename) {
    if (!m_lua) {
        return false;
    }
    
    int result = luaL_dofile(m_lua, filename.c_str());
    
    if (result != LUA_OK) {
        const char* error = lua_tostring(m_lua, -1);
        LOG_S_ERROR_CAT("LuaBinder") << "Lua error: " << error;
        lua_pop(m_lua, 1);
        return false;
    }
    
    return true;
}

std::string LuaBinder::popString() {
    if (!m_lua || lua_gettop(m_lua) < 1) {
        return "";
    }
    
    std::string result = "";
    
    if (lua_isstring(m_lua, -1)) {
        result = lua_tostring(m_lua, -1);
    }
    
    lua_pop(m_lua, 1);
    return result;
}

int LuaBinder::popInt() {
    if (!m_lua || lua_gettop(m_lua) < 1) {
        return 0;
    }
    
    int result = 0;
    
    if (lua_isinteger(m_lua, -1)) {
        result = (int)lua_tointeger(m_lua, -1);
    }
    
    lua_pop(m_lua, 1);
    return result;
}

bool LuaBinder::popBool() {
    if (!m_lua || lua_gettop(m_lua) < 1) {
        return false;
    }
    
    bool result = false;
    
    if (lua_isboolean(m_lua, -1)) {
        result = (lua_toboolean(m_lua, -1) != 0);
    }
    
    lua_pop(m_lua, 1);
    return result;
}

void LuaBinder::pushInt(int value) {
    if (!m_lua) {
        return;
    }
    
    lua_pushinteger(m_lua, value);
}

void LuaBinder::pushString(const std::string& value) {
    if (!m_lua) {
        return;
    }
    
    lua_pushstring(m_lua, value.c_str());
}

void LuaBinder::pushBool(bool value) {
    if (!m_lua) {
        return;
    }
    
    lua_pushboolean(m_lua, value ? 1 : 0);
}

void LuaBinder::pushNil() {
    if (!m_lua) {
        return;
    }
    
    lua_pushnil(m_lua);
}

void LuaBinder::pushNewTable() {
    if (!m_lua) {
        return;
    }
    
    lua_newtable(m_lua);
}

void LuaBinder::setField(const std::string& key) {
    if (!m_lua || lua_gettop(m_lua) < 2) {
        return;
    }
    
    lua_setfield(m_lua, -2, key.c_str());
}

int LuaBinder::getField(const std::string& key) {
    if (!m_lua || lua_gettop(m_lua) < 1) {
        return LUA_TNIL;
    }
    
    lua_getfield(m_lua, -1, key.c_str());
    int type = lua_type(m_lua, -1);
    lua_remove(m_lua, -2); // 移除原来的表
    
    return type;
}

void LuaBinder::printStack() {
    if (!m_lua) {
        return;
    }
    
    int top = lua_gettop(m_lua);
    LOG_DEBUG_CAT("LuaBinder", "=== Lua Stack ===");
    
    for (int i = 1; i <= top; ++i) {
        int type = lua_type(m_lua, i);
        std::stringstream ss;
        ss << i << ": " << lua_typename(m_lua, type);
        
        switch (type) {
            case LUA_TSTRING:
                ss << " (" << lua_tostring(m_lua, i) << ")";
                break;
            case LUA_TNUMBER:
                ss << " (" << lua_tonumber(m_lua, i) << ")";
                break;
            case LUA_TBOOLEAN:
                ss << " (" << (lua_toboolean(m_lua, i) ? "true" : "false") << ")";
                break;
        }
        
        LOG_DEBUG_CAT("LuaBinder", ss.str());
    }
    
    LOG_DEBUG_CAT("LuaBinder", "==================");
}

void LuaBinder::ensureTable(const std::string& tableName) {
    lua_getglobal(m_lua, tableName.c_str());
    
    if (!lua_istable(m_lua, -1)) {
        lua_pop(m_lua, 1);
        lua_newtable(m_lua);
        lua_setglobal(m_lua, tableName.c_str());
    } else {
        lua_pop(m_lua, 1);
    }
}

} // namespace Binding
} // namespace Lua
} // namespace LuaUI
