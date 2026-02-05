/**
 * @file LoggerBinder.h
 * @brief Lua日志绑定声明
 */

#ifndef LUAUI_LOGGER_BINDER_H
#define LUAUI_LOGGER_BINDER_H

#include "lua/binding/LuaBinder.h"

namespace LuaUI {
namespace Lua {
namespace Binding {

void registerLogger(LuaBinder* binder);

} // namespace Binding
} // namespace Lua
} // namespace LuaUI

#endif // LUAUI_LOGGER_BINDER_H
