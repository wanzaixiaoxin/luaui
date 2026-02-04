/**
 * @file LuaEventHandler.cpp
 * @brief Lua事件处理器实现
 */

#include "ui/events/LuaEventHandler.h"
#include <cstring>

namespace LuaUI {
namespace Events {

static LuaEventHandler* g_luaEventHandler = nullptr;

LuaEventHandler::LuaEventHandler()
    : m_lua(nullptr)
    , m_eventManager(nullptr)
    , m_eventRouter(nullptr)
{
}

LuaEventHandler::~LuaEventHandler() {
    shutdown();
}

bool LuaEventHandler::initialize(lua_State* lua) {
    if (!lua) {
        return false;
    }
    
    m_lua = lua;
    m_eventManager = new EventManager();
    m_eventRouter = &EventRouter::instance();
    m_eventRouter->setEventManager(m_eventManager);
    
    // 注册全局函数
    registerGlobalFunctions(lua);
    
    return true;
}

void LuaEventHandler::shutdown() {
    clearAllEvents();
    
    if (m_eventManager) {
        delete m_eventManager;
        m_eventManager = nullptr;
    }
    
    m_lua = nullptr;
}

void LuaEventHandler::registerGlobalFunctions(lua_State* lua) {
    if (!lua) {
        return;
    }
    
    // 注册UI全局表
    lua_newtable(lua);
    
    // 注册函数
    lua_pushcfunction(lua, luaBindEvent);
    lua_setfield(lua, -2, "bindEvent");
    
    lua_pushcfunction(lua, luaUnbindEvent);
    lua_setfield(lua, -2, "unbindEvent");
    
    lua_pushcfunction(lua, luaFireEvent);
    lua_setfield(lua, -2, "fireEvent");
    
    // 设置为全局变量
    lua_setglobal(lua, "UI");
}

bool LuaEventHandler::bindControlEvent(const std::string& controlId,
                                     const std::string& eventName,
                                     lua_State* lua, int funcRef) {
    if (!lua || funcRef < 0) {
        return false;
    }
    
    // 解析事件类型
    EventType eventType = EventManager::parseEventType(eventName);
    
    // 获取Lua函数引用
    int ref = getLuaFunctionRef(lua, funcRef);
    
    // 注册到事件管理器
    int handlerId = m_eventManager->registerLuaHandler(controlId, eventType, lua, ref);
    
    // 保存绑定信息
    LuaFunctionRef funcRefInfo(lua, ref, handlerId);
    m_eventBindings[controlId][eventType] = funcRefInfo;
    
    return true;
}

bool LuaEventHandler::unbindControlEvent(const std::string& controlId, const std::string& eventName) {
    // 解析事件类型
    EventType eventType = EventManager::parseEventType(eventName);
    
    std::map<std::string, std::map<EventType, LuaFunctionRef>>::iterator it = 
        m_eventBindings.find(controlId);
    
    if (it != m_eventBindings.end()) {
        std::map<EventType, LuaFunctionRef>::iterator eventIt = it->second.find(eventType);
        if (eventIt != it->second.end()) {
            // 释放Lua引用
            releaseLuaFunctionRef(eventIt->second.lua, eventIt->second.ref);
            
            // 从事件管理器注销
            m_eventManager->unregisterHandler(eventIt->second.handlerId);
            
            // 从映射表移除
            it->second.erase(eventIt);
            
            return true;
        }
    }
    
    return false;
}

bool LuaEventHandler::handleEvent(const std::string& controlId, EventType /*eventType*/, EventArgs* args) {
    if (!args || !m_eventManager) {
        return false;
    }
    
    return m_eventManager->fireEvent(controlId, args);
}

void LuaEventHandler::clearControlEvents(const std::string& controlId) {
    m_eventManager->unregisterControlHandlers(controlId);
    
    std::map<std::string, std::map<EventType, LuaFunctionRef>>::iterator it = 
        m_eventBindings.find(controlId);
    
    if (it != m_eventBindings.end()) {
        // 释放所有Lua引用
        for (std::map<EventType, LuaFunctionRef>::iterator eventIt = it->second.begin();
             eventIt != it->second.end(); ++eventIt) {
            releaseLuaFunctionRef(eventIt->second.lua, eventIt->second.ref);
        }
        
        m_eventBindings.erase(it);
    }
}

void LuaEventHandler::clearAllEvents() {
    // 释放所有Lua引用
    for (std::map<std::string, std::map<EventType, LuaFunctionRef>>::iterator it = 
         m_eventBindings.begin(); it != m_eventBindings.end(); ++it) {
        
        for (std::map<EventType, LuaFunctionRef>::iterator eventIt = it->second.begin();
             eventIt != it->second.end(); ++eventIt) {
            releaseLuaFunctionRef(eventIt->second.lua, eventIt->second.ref);
        }
    }
    
    m_eventBindings.clear();
}

int LuaEventHandler::luaBindEvent(lua_State* L) {
    LuaEventHandler* handler = GetLuaEventHandler();
    if (!handler) {
        return 0;
    }
    
    // 参数检查
    if (lua_gettop(L) < 3) {
        lua_pushstring(L, "bindEvent requires controlId, eventName, and function");
        lua_error(L);
        return 0;
    }
    
    // 获取参数
    const char* controlId = lua_tostring(L, 1);
    const char* eventName = lua_tostring(L, 2);
    
    if (!lua_isfunction(L, 3)) {
        lua_pushstring(L, "Third argument must be a function");
        lua_error(L);
        return 0;
    }
    
    // 获取函数引用
    lua_pushvalue(L, 3);
    int funcRef = luaL_ref(L, LUA_REGISTRYINDEX);
    
    // 绑定事件
    bool result = handler->bindControlEvent(controlId, eventName, L, funcRef);
    
    lua_pushboolean(L, result ? 1 : 0);
    return 1;
}

int LuaEventHandler::luaUnbindEvent(lua_State* L) {
    LuaEventHandler* handler = GetLuaEventHandler();
    if (!handler) {
        return 0;
    }
    
    // 参数检查
    if (lua_gettop(L) < 2) {
        lua_pushstring(L, "unbindEvent requires controlId and eventName");
        lua_error(L);
        return 0;
    }
    
    // 获取参数
    const char* controlId = lua_tostring(L, 1);
    const char* eventName = lua_tostring(L, 2);
    
    // 解绑事件
    bool result = handler->unbindControlEvent(controlId, eventName);
    
    lua_pushboolean(L, result ? 1 : 0);
    return 1;
}

int LuaEventHandler::luaFireEvent(lua_State* L) {
    // TODO: 实现触发事件的Lua接口
    lua_pushboolean(L, 0);
    return 1;
}

int LuaEventHandler::getLuaFunctionRef(lua_State* L, int index) {
    if (!lua_isfunction(L, index)) {
        return -1;
    }
    
    lua_pushvalue(L, index);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    return ref;
}

void LuaEventHandler::releaseLuaFunctionRef(lua_State* lua, int ref) {
    if (lua && ref >= 0) {
        luaL_unref(lua, LUA_REGISTRYINDEX, ref);
    }
}

bool LuaEventHandler::callLuaFunction(const LuaFunctionRef& funcRef, EventArgs* args) {
    if (!funcRef.lua || funcRef.ref < 0) {
        return false;
    }
    
    // 获取Lua函数
    lua_rawgeti(funcRef.lua, LUA_REGISTRYINDEX, funcRef.ref);
    
    // 压入事件源ID
    lua_pushstring(funcRef.lua, args->eventSource ? args->eventSource->getId().c_str() : "");
    
    // 调用函数
    int result = lua_pcall(funcRef.lua, 1, 0, 0);
    
    if (result != LUA_OK) {
        // 错误处理
        lua_pop(funcRef.lua, 1);
        return false;
    }
    
    return true;
}

LuaEventHandler* GetLuaEventHandler() {
    if (!g_luaEventHandler) {
        g_luaEventHandler = new LuaEventHandler();
    }
    return g_luaEventHandler;
}

} // namespace Events
} // namespace LuaUI
