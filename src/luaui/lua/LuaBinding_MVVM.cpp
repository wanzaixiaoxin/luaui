// Lua MVVM Binding - 支持 Lua 端触发属性变更通知

#include "LuaBinding_MVVM.h"
#include "LuaSandbox.h"
#include "Logger.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

namespace luaui {
namespace lua {

// Global registry for property notifiers (LuaState -> Notifier)
static std::unordered_map<lua_State*, std::weak_ptr<LuaPropertyNotifier>> g_notifiers;

void RegisterPropertyNotifier(lua_State* L, std::shared_ptr<LuaPropertyNotifier> notifier) {
    g_notifiers[L] = notifier;
}

void UnregisterPropertyNotifier(lua_State* L) {
    g_notifiers.erase(L);
}

// ============================================================================
// MVVM Binding Registration
// ============================================================================

void LuaBinding::RegisterMVVM(lua_State* L) {
    // 创建 Notify 全局函数
    // 用法: Notify("PropertyName") 或 Notify(ViewModel, "PropertyName")
    lua_pushcfunction(L, [](lua_State* L) -> int {
        const char* propertyName = nullptr;
        
        if (lua_gettop(L) >= 2 && lua_isstring(L, 2)) {
            // Notify(ViewModel, "PropertyName")
            propertyName = lua_tostring(L, 2);
        } else if (lua_gettop(L) >= 1 && lua_isstring(L, 1)) {
            // Notify("PropertyName") - 使用注册的通知器
            propertyName = lua_tostring(L, 1);
        }
        
        if (!propertyName) {
            luaL_error(L, "Notify expects a property name string");
            return 0;
        }
        
        // 查找已注册的 notifier
        auto it = g_notifiers.find(L);
        if (it != g_notifiers.end()) {
            if (auto notifier = it->second.lock()) {
                notifier->NotifyPropertyChanged(propertyName);
                utils::Logger::DebugF("[Lua MVVM] Property changed: %s", propertyName);
            }
        }
        
        return 0;
    });
    lua_setglobal(L, "Notify");
    
    // 创建 ViewModelBase 元表，提供 Notify 方法
    luaL_newmetatable(L, "LuaUI.ViewModelBase");
    
    // __index 表
    lua_newtable(L);
    
    // ViewModelBase:Notify(propertyName)
    lua_pushcfunction(L, [](lua_State* L) -> int {
        luaL_checktype(L, 1, LUA_TTABLE);  // self
        const char* propertyName = luaL_checkstring(L, 2);
        
        auto it = g_notifiers.find(L);
        if (it != g_notifiers.end()) {
            if (auto notifier = it->second.lock()) {
                notifier->NotifyPropertyChanged(propertyName);
            }
        }
        
        return 0;
    });
    lua_setfield(L, -2, "Notify");
    
    // 设置 __index
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
    
    utils::Logger::Info("[Lua MVVM] Binding registered");
}

} // namespace lua
} // namespace luaui
