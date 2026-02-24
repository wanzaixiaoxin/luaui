// Lua Logger Binding - Enhanced Lua to C++ Logger Bridge
// Provides comprehensive logging API for Lua scripts

#include "LuaSandbox.h"
#include "Logger.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

#include <sstream>

namespace luaui {
namespace lua {

// Helper to format Lua values for logging
static std::string FormatLuaValue(lua_State* L, int index) {
    int type = lua_type(L, index);
    switch (type) {
        case LUA_TNIL:
            return "nil";
        case LUA_TBOOLEAN:
            return lua_toboolean(L, index) ? "true" : "false";
        case LUA_TNUMBER: {
            lua_Number num = lua_tonumber(L, index);
            // Check if it's an integer
            if (num == (lua_Integer)num) {
                return std::to_string((lua_Integer)num);
            }
            return std::to_string(num);
        }
        case LUA_TSTRING:
            return std::string("\"") + lua_tostring(L, index) + "\"";
        case LUA_TTABLE:
            return "<table>";
        case LUA_TFUNCTION:
            return "<function>";
        case LUA_TUSERDATA:
            return "<userdata>";
        case LUA_TTHREAD:
            return "<thread>";
        default:
            return "<unknown>";
    }
}

// Format message with variable arguments
static std::string FormatLogMessage(lua_State* L, int startIndex) {
    int nargs = lua_gettop(L);
    if (nargs < startIndex) {
        return "";
    }
    
    // Single string argument - return as is
    if (nargs == startIndex && lua_isstring(L, startIndex)) {
        return lua_tostring(L, startIndex);
    }
    
    // Multiple arguments - format like print()
    std::ostringstream oss;
    for (int i = startIndex; i <= nargs; ++i) {
        if (i > startIndex) {
            oss << " ";
        }
        oss << FormatLuaValue(L, i);
    }
    return oss.str();
}

// ==================== Core Logging Functions ====================

void LuaBinding::RegisterLogger(lua_State* L) {
    // Create Log table with all logging functions
    lua_newtable(L);
    
    // ----- Basic logging functions -----
    
    // Log.trace(message, ...)
    lua_pushcfunction(L, [](lua_State* L) -> int {
        std::string msg = FormatLogMessage(L, 1);
        if (!msg.empty()) {
            luaui::utils::Logger::Trace("[Lua] " + msg);
        }
        return 0;
    });
    lua_setfield(L, -2, "trace");
    
    // Log.debug(message, ...)
    lua_pushcfunction(L, [](lua_State* L) -> int {
        std::string msg = FormatLogMessage(L, 1);
        if (!msg.empty()) {
            luaui::utils::Logger::Debug("[Lua] " + msg);
        }
        return 0;
    });
    lua_setfield(L, -2, "debug");
    
    // Log.info(message, ...)
    lua_pushcfunction(L, [](lua_State* L) -> int {
        std::string msg = FormatLogMessage(L, 1);
        if (!msg.empty()) {
            luaui::utils::Logger::Info("[Lua] " + msg);
        }
        return 0;
    });
    lua_setfield(L, -2, "info");
    
    // Log.warn(message, ...)
    lua_pushcfunction(L, [](lua_State* L) -> int {
        std::string msg = FormatLogMessage(L, 1);
        if (!msg.empty()) {
            luaui::utils::Logger::Warning("[Lua] " + msg);
        }
        return 0;
    });
    lua_setfield(L, -2, "warn");
    
    // Log.error(message, ...)
    lua_pushcfunction(L, [](lua_State* L) -> int {
        std::string msg = FormatLogMessage(L, 1);
        if (!msg.empty()) {
            luaui::utils::Logger::Error("[Lua] " + msg);
        }
        return 0;
    });
    lua_setfield(L, -2, "error");
    
    // Log.fatal(message, ...)
    lua_pushcfunction(L, [](lua_State* L) -> int {
        std::string msg = FormatLogMessage(L, 1);
        if (!msg.empty()) {
            luaui::utils::Logger::Fatal("[Lua] " + msg);
        }
        return 0;
    });
    lua_setfield(L, -2, "fatal");
    
    // ----- Formatted logging (printf-style) -----
    
    // Log.debugf(format, ...)
    lua_pushcfunction(L, [](lua_State* L) -> int {
        const char* format = luaL_checkstring(L, 1);
        int nargs = lua_gettop(L);
        
        // Simple format replacement for common specifiers
        std::string result = format;
        for (int i = 2; i <= nargs; ++i) {
            size_t pos = result.find_first_of("%");
            if (pos == std::string::npos) break;
            
            std::string replacement;
            if (pos + 1 < result.length()) {
                char spec = result[pos + 1];
                switch (spec) {
                    case 's':
                        replacement = luaL_checkstring(L, i);
                        break;
                    case 'd':
                    case 'i':
                        replacement = std::to_string((lua_Integer)luaL_checkinteger(L, i));
                        break;
                    case 'f':
                        replacement = std::to_string(luaL_checknumber(L, i));
                        break;
                    case 'b':
                        replacement = lua_toboolean(L, i) ? "true" : "false";
                        break;
                    case '%':
                        replacement = "%";
                        break;
                    default:
                        replacement = FormatLuaValue(L, i);
                        break;
                }
                result.replace(pos, 2, replacement);
            }
        }
        
        luaui::utils::Logger::Debug("[Lua] " + result);
        return 0;
    });
    lua_setfield(L, -2, "debugf");
    
    // Log.infof(format, ...)
    lua_pushcfunction(L, [](lua_State* L) -> int {
        const char* format = luaL_checkstring(L, 1);
        int nargs = lua_gettop(L);
        
        std::string result = format;
        for (int i = 2; i <= nargs; ++i) {
            size_t pos = result.find_first_of("%");
            if (pos == std::string::npos) break;
            
            std::string replacement;
            if (pos + 1 < result.length()) {
                char spec = result[pos + 1];
                switch (spec) {
                    case 's':
                        replacement = luaL_checkstring(L, i);
                        break;
                    case 'd':
                    case 'i':
                        replacement = std::to_string((lua_Integer)luaL_checkinteger(L, i));
                        break;
                    case 'f':
                        replacement = std::to_string(luaL_checknumber(L, i));
                        break;
                    case 'b':
                        replacement = lua_toboolean(L, i) ? "true" : "false";
                        break;
                    default:
                        replacement = FormatLuaValue(L, i);
                        break;
                }
                result.replace(pos, 2, replacement);
            }
        }
        
        luaui::utils::Logger::Info("[Lua] " + result);
        return 0;
    });
    lua_setfield(L, -2, "infof");
    
    // Log.warnf(format, ...)
    lua_pushcfunction(L, [](lua_State* L) -> int {
        const char* format = luaL_checkstring(L, 1);
        int nargs = lua_gettop(L);
        
        std::string result = format;
        for (int i = 2; i <= nargs; ++i) {
            size_t pos = result.find_first_of("%");
            if (pos == std::string::npos) break;
            
            std::string replacement;
            if (pos + 1 < result.length()) {
                char spec = result[pos + 1];
                switch (spec) {
                    case 's':
                        replacement = luaL_checkstring(L, i);
                        break;
                    case 'd':
                    case 'i':
                        replacement = std::to_string((lua_Integer)luaL_checkinteger(L, i));
                        break;
                    case 'f':
                        replacement = std::to_string(luaL_checknumber(L, i));
                        break;
                    case 'b':
                        replacement = lua_toboolean(L, i) ? "true" : "false";
                        break;
                    default:
                        replacement = FormatLuaValue(L, i);
                        break;
                }
                result.replace(pos, 2, replacement);
            }
        }
        
        luaui::utils::Logger::Warning("[Lua] " + result);
        return 0;
    });
    lua_setfield(L, -2, "warnf");
    
    // Log.errorf(format, ...)
    lua_pushcfunction(L, [](lua_State* L) -> int {
        const char* format = luaL_checkstring(L, 1);
        int nargs = lua_gettop(L);
        
        std::string result = format;
        for (int i = 2; i <= nargs; ++i) {
            size_t pos = result.find_first_of("%");
            if (pos == std::string::npos) break;
            
            std::string replacement;
            if (pos + 1 < result.length()) {
                char spec = result[pos + 1];
                switch (spec) {
                    case 's':
                        replacement = luaL_checkstring(L, i);
                        break;
                    case 'd':
                    case 'i':
                        replacement = std::to_string((lua_Integer)luaL_checkinteger(L, i));
                        break;
                    case 'f':
                        replacement = std::to_string(luaL_checknumber(L, i));
                        break;
                    case 'b':
                        replacement = lua_toboolean(L, i) ? "true" : "false";
                        break;
                    default:
                        replacement = FormatLuaValue(L, i);
                        break;
                }
                result.replace(pos, 2, replacement);
            }
        }
        
        luaui::utils::Logger::Error("[Lua] " + result);
        return 0;
    });
    lua_setfield(L, -2, "errorf");
    
    // ----- Log Level Management -----
    
    // Log.setLevel(level) - level: "trace", "debug", "info", "warn", "error", "fatal"
    lua_pushcfunction(L, [](lua_State* L) -> int {
        const char* levelStr = luaL_checkstring(L, 1);
        luaui::utils::LogLevel level;
        
        if (strcmp(levelStr, "trace") == 0) level = luaui::utils::LogLevel::Trace;
        else if (strcmp(levelStr, "debug") == 0) level = luaui::utils::LogLevel::Debug;
        else if (strcmp(levelStr, "info") == 0) level = luaui::utils::LogLevel::Info;
        else if (strcmp(levelStr, "warn") == 0) level = luaui::utils::LogLevel::Warning;
        else if (strcmp(levelStr, "error") == 0) level = luaui::utils::LogLevel::Error;
        else if (strcmp(levelStr, "fatal") == 0) level = luaui::utils::LogLevel::Fatal;
        else {
            luaL_error(L, "Invalid log level: %s", levelStr);
            return 0;
        }
        
        luaui::utils::Logger::SetConsoleLevel(level);
        return 0;
    });
    lua_setfield(L, -2, "setLevel");
    
    // Log.getLevel() -> string
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto logger = luaui::utils::Logger::Get();
        if (logger) {
            lua_pushstring(L, luaui::utils::LogLevelToString(logger->GetLevel()));
        } else {
            lua_pushstring(L, "UNKNOWN");
        }
        return 1;
    });
    lua_setfield(L, -2, "getLevel");
    
    // ----- Utility Functions -----
    
    // Log.enable() / Log.disable()
    lua_pushcfunction(L, [](lua_State* L) -> int {
        (void)L;
        luaui::utils::Logger::EnableConsole(true);
        return 0;
    });
    lua_setfield(L, -2, "enable");
    
    lua_pushcfunction(L, [](lua_State* L) -> int {
        (void)L;
        luaui::utils::Logger::EnableConsole(false);
        return 0;
    });
    lua_setfield(L, -2, "disable");
    
    // Log.assert(condition, message) - log error if condition is false
    lua_pushcfunction(L, [](lua_State* L) -> int {
        bool condition = lua_toboolean(L, 1);
        if (!condition) {
            const char* message = luaL_optstring(L, 2, "Assertion failed");
            luaui::utils::Logger::Error("[Lua] ASSERT: " + std::string(message));
        }
        return 0;
    });
    lua_setfield(L, -2, "assert");
    
    // Log.table(t, name) - pretty print a table
    lua_pushcfunction(L, [](lua_State* L) -> int {
        luaL_checktype(L, 1, LUA_TTABLE);
        const char* name = luaL_optstring(L, 2, "table");
        
        std::ostringstream oss;
        oss << name << " = {";
        
        lua_pushnil(L);
        bool first = true;
        while (lua_next(L, 1) != 0) {
            if (!first) oss << ", ";
            first = false;
            
            oss << FormatLuaValue(L, -2) << "=" << FormatLuaValue(L, -1);
            lua_pop(L, 1);
        }
        
        oss << "}";
        luaui::utils::Logger::Debug("[Lua] " + oss.str());
        return 0;
    });
    lua_setfield(L, -2, "table");
    
    // ----- Log Level Constants -----
    
    lua_pushstring(L, "TRACE");
    lua_setfield(L, -2, "TRACE");
    
    lua_pushstring(L, "DEBUG");
    lua_setfield(L, -2, "DEBUG");
    
    lua_pushstring(L, "INFO");
    lua_setfield(L, -2, "INFO");
    
    lua_pushstring(L, "WARN");
    lua_setfield(L, -2, "WARN");
    
    lua_pushstring(L, "ERROR");
    lua_setfield(L, -2, "ERROR");
    
    lua_pushstring(L, "FATAL");
    lua_setfield(L, -2, "FATAL");
    
    // Set as global Log table
    lua_setglobal(L, "Log");
    
    // Also provide global print() function that redirects to Log.info
    lua_pushcfunction(L, [](lua_State* L) -> int {
        std::string msg = FormatLogMessage(L, 1);
        if (!msg.empty()) {
            luaui::utils::Logger::Info("[Lua] " + msg);
        }
        return 0;
    });
    lua_setglobal(L, "print");
}

} // namespace lua
} // namespace luaui
