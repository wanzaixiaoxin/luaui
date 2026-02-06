/**
 * @file LuaEventHandler.h
 * @brief Lua事件处理器
 * @details 处理Lua脚本中的事件响应
 * @version 1.0.0
 */

#ifndef LUAUI_LUAEVENTHANDLER_H
#define LUAUI_LUAEVENTHANDLER_H

#include <string>
#include <map>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
#include "ui/events/EventManager.h"
#include "ui/events/EventRouter.h"

namespace LuaUI {
namespace Events {

/**
 * @brief Lua事件处理器类
 * @details 管理Lua脚本中注册的事件回调函数
 */
class LuaEventHandler {
public:
    /**
     * @brief 构造函数
     */
    LuaEventHandler();
    
    /**
     * @brief 析构函数
     */
    ~LuaEventHandler();
    
    /**
     * @brief 初始化Lua事件处理器
     * @param lua Lua状态
     * @return 成功返回true，失败返回false
     */
    bool initialize(lua_State* lua);
    
    /**
     * @brief 关闭Lua事件处理器
     */
    void shutdown();
    
    /**
     * @brief 注册Lua函数到全局命名空间
     * @param lua Lua状态
     */
    void registerGlobalFunctions(lua_State* lua);
    
    /**
     * @brief 绑定控件事件到Lua函数
     * @param controlId 控件ID
     * @param eventName 事件名称
     * @param lua Lua状态
     * @param funcRef Lua函数引用
     * @return 成功返回true，失败返回false
     */
    bool bindControlEvent(const std::string& controlId, const std::string& eventName,
                         lua_State* lua, int funcRef);
    
    /**
     * @brief 解绑控件事件
     * @param controlId 控件ID
     * @param eventName 事件名称
     * @return 成功返回true，失败返回false
     */
    bool unbindControlEvent(const std::string& controlId, const std::string& eventName);
    
    /**
     * @brief 触发Lua事件处理
     * @param controlId 控件ID
     * @param eventType 事件类型
     * @param args 事件参数
     * @return 成功返回true，失败返回false
     */
    bool handleEvent(const std::string& controlId, EventType eventType, EventArgs* args);
    
    /**
     * @brief 清除控件的所有事件绑定
     * @param controlId 控件ID
     */
    void clearControlEvents(const std::string& controlId);
    
    /**
     * @brief 清除所有事件绑定
     */
    void clearAllEvents();

private:
    lua_State* m_lua;                      ///< Lua状态
    EventManager* m_eventManager;           ///< 事件管理器
    EventRouter* m_eventRouter;             ///< 事件路由器
    
    /**
     * @brief Lua函数引用信息
     */
    struct LuaFunctionRef {
        lua_State* lua;     ///< Lua状态
        int ref;           ///< 引用ID
        int handlerId;      ///< 事件处理器ID
        
        /**
         * @brief 构造函数
         */
        LuaFunctionRef()
            : lua(nullptr)
            , ref(-1)
            , handlerId(-1)
        {
        }
        
        /**
         * @brief 构造函数
         */
        LuaFunctionRef(lua_State* L, int r, int hid)
            : lua(L)
            , ref(r)
            , handlerId(hid)
        {
        }
    };
    
    std::map<std::string, std::map<EventType, LuaFunctionRef>> m_eventBindings; ///< 事件绑定映射表
    
    /**
     * @brief 注册Lua全局函数：UI.bindEvent
     * @param L Lua状态
     * @return 返回值个数
     */
    static int luaBindEvent(lua_State* L);
    
    /**
     * @brief 注册Lua全局函数：UI.unbindEvent
     * @param L Lua状态
     * @return 返回值个数
     */
    static int luaUnbindEvent(lua_State* L);
    
    /**
     * @brief 注册Lua全局函数：UI.fireEvent
     * @param L Lua状态
     * @return 返回值个数
     */
    static int luaFireEvent(lua_State* L);
    
    /**
     * @brief 从Lua栈中获取Lua函数引用
     * @param L Lua状态
     * @param index 栈索引
     * @return 函数引用
     */
    int getLuaFunctionRef(lua_State* L, int index);
    
    /**
     * @brief 释放Lua函数引用
     * @param lua Lua状态
     * @param ref 引用ID
     */
    void releaseLuaFunctionRef(lua_State* lua, int ref);
    
    /**
     * @brief 调用Lua事件函数
     * @param funcRef 函数引用
     * @param args 事件参数
     * @return 成功返回true，失败返回false
     */
    bool callLuaFunction(const LuaFunctionRef& funcRef, EventArgs* args);
};

/**
 * @brief 获取全局Lua事件处理器实例
 * @return Lua事件处理器指针
 */
LuaEventHandler* GetLuaEventHandler();

} // namespace Events
} // namespace LuaUI

#endif // LUAUI_LUAEVENTHANDLER_H
