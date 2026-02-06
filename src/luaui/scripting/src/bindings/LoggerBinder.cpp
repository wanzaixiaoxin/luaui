/**
 * @file LoggerBinder.cpp
 * @brief Lua日志绑定实现
 */

#include "lua/binding/LoggerBinder.h"
#include "utils/Logger.h"
#include <string>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

namespace LuaUI {
namespace Lua {
namespace Binding {

static int lua_log_debug(lua_State* L) {
    const char* msg = luaL_checkstring(L, 1);
    Utils::Logger::getInstance().setCategory("Lua");
    Utils::Logger::getInstance().debug(msg);
    return 0;
}

static int lua_log_info(lua_State* L) {
    const char* msg = luaL_checkstring(L, 1);
    Utils::Logger::getInstance().setCategory("Lua");
    Utils::Logger::getInstance().info(msg);
    return 0;
}

static int lua_log_warn(lua_State* L) {
    const char* msg = luaL_checkstring(L, 1);
    Utils::Logger::getInstance().setCategory("Lua");
    Utils::Logger::getInstance().warn(msg);
    return 0;
}

static int lua_log_error(lua_State* L) {
    const char* msg = luaL_checkstring(L, 1);
    Utils::Logger::getInstance().setCategory("Lua");
    Utils::Logger::getInstance().error(msg);
    return 0;
}

static int lua_log_fatal(lua_State* L) {
    const char* msg = luaL_checkstring(L, 1);
    Utils::Logger::getInstance().setCategory("Lua");
    Utils::Logger::getInstance().fatal(msg);
    return 0;
}

static int lua_log_setLevel(lua_State* L) {
    const char* levelStr = luaL_checkstring(L, 1);
    Utils::LogLevel level = Utils::Logger::stringToLevel(levelStr);
    Utils::Logger::getInstance().setLevel(level);
    return 0;
}

static int lua_log_enableConsole(lua_State* L) {
    bool enable = lua_toboolean(L, 1);
    Utils::Logger::getInstance().enableConsole(enable);
    return 0;
}

static int lua_log_enableFile(lua_State* L) {
    const char* filepath = luaL_checkstring(L, 1);
    size_t maxSize = static_cast<size_t>(luaL_optinteger(L, 2, 1024 * 1024));
    int maxFiles = static_cast<int>(luaL_optinteger(L, 3, 3));
    Utils::Logger::getInstance().enableFile(filepath, maxSize, maxFiles);
    return 0;
}

static int lua_log_disableFile(lua_State* L) {
    Utils::Logger::getInstance().disableFile();
    return 0;
}

void registerLogger(LuaBinder* binder) {
    if (!binder) {
        return;
    }

    lua_State* L = binder->getState();
    if (!L) {
        return;
    }

    lua_newtable(L);
    lua_setglobal(L, "Log");

    lua_getglobal(L, "Log");

    lua_pushcfunction(L, lua_log_debug);
    lua_setfield(L, -2, "debug");

    lua_pushcfunction(L, lua_log_info);
    lua_setfield(L, -2, "info");

    lua_pushcfunction(L, lua_log_warn);
    lua_setfield(L, -2, "warn");

    lua_pushcfunction(L, lua_log_error);
    lua_setfield(L, -2, "error");

    lua_pushcfunction(L, lua_log_fatal);
    lua_setfield(L, -2, "fatal");

    lua_pushcfunction(L, lua_log_setLevel);
    lua_setfield(L, -2, "setLevel");

    lua_pushcfunction(L, lua_log_enableConsole);
    lua_setfield(L, -2, "enableConsole");

    lua_pushcfunction(L, lua_log_enableFile);
    lua_setfield(L, -2, "enableFile");

    lua_pushcfunction(L, lua_log_disableFile);
    lua_setfield(L, -2, "disableFile");

    lua_pushinteger(L, static_cast<int>(Utils::LogLevel::LevelDebug));
    lua_setfield(L, -2, "DEBUG");

    lua_pushinteger(L, static_cast<int>(Utils::LogLevel::LevelInfo));
    lua_setfield(L, -2, "INFO");

    lua_pushinteger(L, static_cast<int>(Utils::LogLevel::LevelWarn));
    lua_setfield(L, -2, "WARN");

    lua_pushinteger(L, static_cast<int>(Utils::LogLevel::LevelError));
    lua_setfield(L, -2, "ERROR");

    lua_pushinteger(L, static_cast<int>(Utils::LogLevel::LevelFatal));
    lua_setfield(L, -2, "FATAL");

    lua_pop(L, 1);
}

} // namespace Binding
} // namespace Lua
} // namespace LuaUI
