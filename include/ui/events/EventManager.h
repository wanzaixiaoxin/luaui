/**
 * @file EventManager.h
 * @brief 事件管理器
 * @details 管理控件的事件注册和触发
 * @version 1.0.0
 */

#ifndef LUAUI_EVENTMANAGER_H
#define LUAUI_EVENTMANAGER_H

#include <string>
#include <map>
#include <functional>
#include "ui/controls/BaseControl.h"
#include <lua.hpp>

namespace LuaUI {
namespace Events {

/**
 * @brief 事件类型
 */
enum EventType {
    EVENT_CLICK,          ///< 点击事件
    EVENT_DOUBLE_CLICK,   ///< 双击事件
    EVENT_CHANGED,        ///< 值改变事件
    EVENT_SELECTED,       ///< 选择事件
    EVENT_HOVER,          ///< 悬停事件
    EVENT_FOCUS,          ///< 获得焦点事件
    EVENT_BLUR,           ///< 失去焦点事件
    EVENT_KEY_PRESS,      ///< 键盘按下事件
    EVENT_KEY_RELEASE,    ///< 键盘释放事件
    EVENT_MOUSE_MOVE,     ///< 鼠标移动事件
    EVENT_SIZE_CHANGED,   ///< 大小改变事件
    EVENT_POSITION_CHANGED,///< 位置改变事件
    EVENT_WINDOW_CLOSE,   ///< 窗口关闭事件
    EVENT_WINDOW_SHOW,     ///< 窗口显示事件
    EVENT_WINDOW_HIDE,     ///< 窗口隐藏事件
    EVENT_CUSTOM          ///< 自定义事件
};

/**
 * @brief 事件参数基类
 */
class EventArgs {
public:
    /**
     * @brief 构造函数
     * @param type 事件类型
     * @param source 事件源控件
     */
    EventArgs(EventType type, UI::BaseControl* source)
        : eventType(type)
        , eventSource(source)
        , handled(false)
    {
    }
    
    virtual ~EventArgs() {}
    
    EventType eventType;          ///< 事件类型
    UI::BaseControl* eventSource; ///< 事件源
    bool handled;                 ///< 是否已处理
    
    /**
     * @brief 标记事件为已处理
     */
    void markHandled() {
        handled = true;
    }
};

/**
 * @brief 鼠标事件参数
 */
class MouseEventArgs : public EventArgs {
public:
    int x;          ///< X坐标
    int y;          ///< Y坐标
    int button;      ///< 鼠标按钮（0=左键，1=中键，2=右键）
    int delta;      ///< 滚轮增量
    
    /**
     * @brief 构造函数
     */
    MouseEventArgs(EventType type, UI::BaseControl* source, int mx, int my, int btn = 0)
        : EventArgs(type, source)
        , x(mx)
        , y(my)
        , button(btn)
        , delta(0)
    {
    }
};

/**
 * @brief 键盘事件参数
 */
class KeyEventArgs : public EventArgs {
public:
    int keyCode;    ///< 按键代码
    int repeatCount; ///< 重复次数
    bool shift;     ///< Shift键状态
    bool ctrl;      ///< Ctrl键状态
    bool alt;       ///< Alt键状态
    
    /**
     * @brief 构造函数
     */
    KeyEventArgs(EventType type, UI::BaseControl* source, int code)
        : EventArgs(type, source)
        , keyCode(code)
        , repeatCount(1)
        , shift(false)
        , ctrl(false)
        , alt(false)
    {
    }
};

/**
 * @brief 事件处理器函数类型
 */
typedef std::function<void(EventArgs*)> EventHandler;

/**
 * @brief 事件管理器类
 */
class EventManager {
public:
    /**
     * @brief 构造函数
     */
    EventManager();
    
    /**
     * @brief 析构函数
     */
    ~EventManager();
    
    /**
     * @brief 注册事件处理器
     * @param controlId 控件ID
     * @param eventType 事件类型
     * @param handler 事件处理器
     * @return 处理器ID
     */
    int registerHandler(const std::string& controlId, EventType eventType, 
                      EventHandler handler);
    
    /**
     * @brief 注销事件处理器
     * @param handlerId 处理器ID
     * @return 成功返回true，失败返回false
     */
    bool unregisterHandler(int handlerId);
    
    /**
     * @brief 注销控件的所有事件处理器
     * @param controlId 控件ID
     */
    void unregisterControlHandlers(const std::string& controlId);
    
    /**
     * @brief 触发事件
     * @param controlId 控件ID
     * @param args 事件参数
     * @return 成功返回true，失败返回false
     */
    bool fireEvent(const std::string& controlId, EventArgs* args);
    
    /**
     * @brief 触发事件（通过控件指针）
     * @param control 控件指针
     * @param args 事件参数
     * @return 成功返回true，失败返回false
     */
    bool fireEventByControl(UI::BaseControl* control, EventArgs* args);
    
    /**
     * @brief 注册Lua事件处理器
     * @param controlId 控件ID
     * @param eventType 事件类型
     * @param lua Lua状态
     * @param funcRef Lua函数引用
     * @return 处理器ID
     */
    int registerLuaHandler(const std::string& controlId, EventType eventType,
                          lua_State* lua, int funcRef);
    
    /**
     * @brief 获取事件类型名称
     * @param type 事件类型
     * @return 事件类型名称
     */
    static std::string getEventTypeName(EventType type);
    
    /**
     * @brief 从字符串解析事件类型
     * @param typeName 事件类型名称
     * @return 事件类型
     */
    static EventType parseEventType(const std::string& typeName);

private:
    int m_nextHandlerId;                    ///< 下一个处理器ID
    std::map<int, EventHandler> m_handlers; ///< C++处理器映射表
    
    /**
     * @brief Lua处理器信息
     */
    struct LuaHandlerInfo {
        std::string controlId;  ///< 控件ID
        EventType eventType;     ///< 事件类型
        lua_State* lua;        ///< Lua状态
        int funcRef;           ///< 函数引用
    };
    
    std::map<int, LuaHandlerInfo> m_luaHandlers; ///< Lua处理器映射表
    
    /**
     * @brief 调用Lua处理器
     * @param info 处理器信息
     * @param args 事件参数
     * @return 成功返回true，失败返回false
     */
    bool callLuaHandler(const LuaHandlerInfo& info, EventArgs* args);
};

} // namespace Events
} // namespace LuaUI

#endif // LUAUI_EVENTMANAGER_H
