/**
 * @file EventBinder.cpp
 * @brief 事件绑定器实现
 */

#include "lua/binding/EventBinder.h"
#include "ui/events/LuaEventHandler.h"

namespace LuaUI {
namespace Lua {
namespace Binding {

EventBinder::EventBinder(lua_State* lua)
    : m_lua(lua)
{
}

EventBinder::~EventBinder() {
}

void EventBinder::initialize() {
    if (!m_lua) {
        return;
    }
    
    // 创建Events全局表
    lua_newtable(m_lua);
    
    // 注册函数
    lua_pushcfunction(m_lua, luaBindEvent);
    lua_setfield(m_lua, -2, "bind");
    
    lua_pushcfunction(m_lua, luaUnbindEvent);
    lua_setfield(m_lua, -2, "unbind");
    
    lua_pushcfunction(m_lua, luaFireEvent);
    lua_setfield(m_lua, -2, "fire");
    
    // 设置为全局变量
    lua_setglobal(m_lua, "Events");
}

int EventBinder::luaBindEvent(lua_State* L) {
    // 参数：controlId, eventName, function
    if (lua_gettop(L) < 3 || !lua_isfunction(L, 3)) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    const char* controlId = lua_tostring(L, 1);
    const char* eventName = lua_tostring(L, 2);
    
    // 获取Lua事件处理器
    Events::LuaEventHandler* handler = Events::GetLuaEventHandler();
    
    if (!handler) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    // 保存函数引用
    lua_pushvalue(L, 3);
    int funcRef = luaL_ref(L, LUA_REGISTRYINDEX);
    
    // 绑定事件
    bool result = handler->bindControlEvent(controlId, eventName, L, funcRef);
    
    lua_pushboolean(L, result ? 1 : 0);
    return 1;
}

int EventBinder::luaUnbindEvent(lua_State* L) {
    // 参数：controlId, eventName
    if (lua_gettop(L) < 2) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    const char* controlId = lua_tostring(L, 1);
    const char* eventName = lua_tostring(L, 2);
    
    // 获取Lua事件处理器
    Events::LuaEventHandler* handler = Events::GetLuaEventHandler();
    
    if (!handler) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    // 解绑事件
    bool result = handler->unbindControlEvent(controlId, eventName);
    
    lua_pushboolean(L, result ? 1 : 0);
    return 1;
}

int EventBinder::luaFireEvent(lua_State* L) {
    // 参数：controlId, eventName
    if (lua_gettop(L) < 2) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    const char* controlId = lua_tostring(L, 1);
    const char* eventName = lua_tostring(L, 2);
    
    // TODO: 实现触发事件的逻辑
    // 需要通过Lua事件处理器触发
    
    lua_pushboolean(L, 0);
    return 1;
}

int EventBinder::luaOnClick(lua_State* L) {
    // 参数：controlId, function
    if (lua_gettop(L) < 2 || !lua_isfunction(L, 2)) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    const char* controlId = lua_tostring(L, 1);
    
    // 获取Lua事件处理器
    Events::LuaEventHandler* handler = Events::GetLuaEventHandler();
    
    if (!handler) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    // 保存函数引用
    lua_pushvalue(L, 2);
    int funcRef = luaL_ref(L, LUA_REGISTRYINDEX);
    
    // 绑定点击事件
    bool result = handler->bindControlEvent(controlId, "onClick", L, funcRef);
    
    lua_pushboolean(L, result ? 1 : 0);
    return 1;
}

int EventBinder::luaOnDoubleClick(lua_State* L) {
    // 参数：controlId, function
    if (lua_gettop(L) < 2 || !lua_isfunction(L, 2)) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    const char* controlId = lua_tostring(L, 1);
    
    // 获取Lua事件处理器
    Events::LuaEventHandler* handler = Events::GetLuaEventHandler();
    
    if (!handler) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    // 保存函数引用
    lua_pushvalue(L, 2);
    int funcRef = luaL_ref(L, LUA_REGISTRYINDEX);
    
    // 绑定双击事件
    bool result = handler->bindControlEvent(controlId, "onDoubleClick", L, funcRef);
    
    lua_pushboolean(L, result ? 1 : 0);
    return 1;
}

int EventBinder::luaOnChanged(lua_State* L) {
    // 参数：controlId, function
    if (lua_gettop(L) < 2 || !lua_isfunction(L, 2)) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    const char* controlId = lua_tostring(L, 1);
    
    // 获取Lua事件处理器
    Events::LuaEventHandler* handler = Events::GetLuaEventHandler();
    
    if (!handler) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    // 保存函数引用
    lua_pushvalue(L, 2);
    int funcRef = luaL_ref(L, LUA_REGISTRYINDEX);
    
    // 绑定值改变事件
    bool result = handler->bindControlEvent(controlId, "onChanged", L, funcRef);
    
    lua_pushboolean(L, result ? 1 : 0);
    return 1;
}

} // namespace Binding
} // namespace Lua
} // namespace LuaUI
