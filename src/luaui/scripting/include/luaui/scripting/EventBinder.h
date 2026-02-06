/**
 * @file EventBinder.h
 * @brief 事件绑定器
 * @details 将事件系统绑定到Lua
 * @version 1.0.0
 */

#ifndef LUAUI_EVENTBINDER_H
#define LUAUI_EVENTBINDER_H

#include <string>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
#include "ui/events/EventManager.h"

namespace LuaUI {
namespace Lua {
namespace Binding {

/**
 * @brief 事件绑定器类
 */
class EventBinder {
public:
    /**
     * @brief 构造函数
     * @param lua Lua状态
     */
    EventBinder(lua_State* lua);
    
    /**
     * @brief 析构函数
     */
    ~EventBinder();
    
    /**
     * @brief 初始化事件绑定
     * @details 注册所有事件相关的Lua函数
     */
    void initialize();
    
private:
    lua_State* m_lua;  ///< Lua状态
    
    /**
     * @brief Lua函数：绑定事件
     * @param L Lua状态
     * @return 返回值个数
     */
    static int luaBindEvent(lua_State* L);
    
    /**
     * @brief Lua函数：解绑事件
     * @param L Lua状态
     * @return 返回值个数
     */
    static int luaUnbindEvent(lua_State* L);
    
    /**
     * @brief Lua函数：触发事件
     * @param L Lua状态
     * @return 返回值个数
     */
    static int luaFireEvent(lua_State* L);
    
    /**
     * @brief Lua函数：绑定点击事件（快捷方式）
     * @param L Lua状态
     * @return 返回值个数
     */
    static int luaOnClick(lua_State* L);
    
    /**
     * @brief Lua函数：绑定双击事件（快捷方式）
     * @param L Lua状态
     * @return 返回值个数
     */
    static int luaOnDoubleClick(lua_State* L);
    
    /**
     * @brief Lua函数：绑定值改变事件（快捷方式）
     * @param L Lua状态
     * @return 返回值个数
     */
    static int luaOnChanged(lua_State* L);
};

} // namespace Binding
} // namespace Lua
} // namespace LuaUI

#endif // LUAUI_EVENTBINDER_H
