/**
 * @file ControlBinder.h
 * @brief 控件绑定器
 * @details 将UI控件接口绑定到Lua
 * @version 1.0.0
 */

#ifndef LUAUI_CONTROLBINDER_H
#define LUAUI_CONTROLBINDER_H

#include <string>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
#include "ui/controls/BaseControl.h"

namespace LuaUI {
namespace Lua {
namespace Binding {

/**
 * @brief 控件绑定器类
 */
class ControlBinder {
public:
    /**
     * @brief 构造函数
     * @param lua Lua状态
     */
    ControlBinder(lua_State* lua);
    
    /**
     * @brief 析构函数
     */
    ~ControlBinder();
    
    /**
     * @brief 初始化控件绑定
     * @details 注册所有控件相关的Lua函数
     */
    void initialize();
    
    /**
     * @brief 注册控件到Lua
     * @param control 控件指针
     * @param name Lua中的名称
     */
    static void registerControl(UI::BaseControl* control, const std::string& name);
    
    /**
     * @brief 注销控件
     * @param name Lua中的名称
     */
    void unregisterControl(const std::string& name);
    
    /**
     * @brief 获取控件
     * @param name Lua中的名称
     * @return 控件指针
     */
    UI::BaseControl* getControl(const std::string& name);
    
private:
    lua_State* m_lua;  ///< Lua状态
    
    /**
     * @brief Lua函数：获取控件
     * @param L Lua状态
     * @return 返回值个数
     */
    static int luaGetControl(lua_State* L);
    
    /**
     * @brief Lua函数：设置控件属性
     * @param L Lua状态
     * @return 返回值个数
     */
    static int luaSetProperty(lua_State* L);
    
    /**
     * @brief Lua函数：获取控件属性
     * @param L Lua状态
     * @return 返回值个数
     */
    static int luaGetProperty(lua_State* L);
    
    /**
     * @brief Lua函数：显示控件
     * @param L Lua状态
     * @return 返回值个数
     */
    static int luaShow(lua_State* L);
    
    /**
     * @brief Lua函数：隐藏控件
     * @param L Lua状态
     * @return 返回值个数
     */
    static int luaHide(lua_State* L);
    
    /**
     * @brief Lua函数：启用控件
     * @param L Lua状态
     * @return 返回值个数
     */
    static int luaEnable(lua_State* L);
    
    /**
     * @brief Lua函数：禁用控件
     * @param L Lua状态
     * @return 返回值个数
     */
    static int luaDisable(lua_State* L);
    
    /**
     * @brief Lua函数：设置控件位置
     * @param L Lua状态
     * @return 返回值个数
     */
    static int luaSetPosition(lua_State* L);
    
    /**
     * @brief Lua函数：设置控件大小
     * @param L Lua状态
     * @return 返回值个数
     */
    static int luaSetSize(lua_State* L);
    
    /**
     * @brief Lua函数：设置控件文本
     * @param L Lua状态
     * @return 返回值个数
     */
    static int luaSetText(lua_State* L);
    
    /**
     * @brief Lua函数：获取控件文本
     * @param L Lua状态
     * @return 返回值个数
     */
    static int luaGetText(lua_State* L);

    /**
     * @brief Lua函数：退出应用程序
     * @param L Lua状态
     * @return 返回值个数
     */
    static int luaExit(lua_State* L);
};

} // namespace Binding
} // namespace Lua
} // namespace LuaUI

#endif // LUAUI_CONTROLBINDER_H