#pragma once

#include "LuaAwareMvvmLoader.h"
#include <memory>
#include <unordered_map>

extern "C" {
struct lua_State;
}

namespace luaui {
namespace lua {

// 注册属性变更通知器（供 Lua 调用 Notify 时使用）
void RegisterPropertyNotifier(lua_State* L, std::shared_ptr<LuaPropertyNotifier> notifier);
void UnregisterPropertyNotifier(lua_State* L);

} // namespace lua
} // namespace luaui
