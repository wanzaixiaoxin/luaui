/**
 * @file EventRouter.h
 * @brief 事件路由器
 * @details 负责将MFC事件路由到LuaUI框架
 * @version 1.0.0
 */

#ifndef LUAUI_EVENTROUTER_H
#define LUAUI_EVENTROUTER_H

#include <map>
#include <string>
#include "ui/events/EventManager.h"
#include "ui/controls/BaseControl.h"

// MFC forward declarations
class CWnd;
class CMessageMap;

namespace LuaUI {
namespace Events {

/**
 * @brief 事件路由器类
 * @details 单例模式，管理MFC事件到框架事件的映射
 */
class EventRouter {
public:
    /**
     * @brief 获取路由器单例
     * @return 路由器引用
     */
    static EventRouter& instance();
    
    /**
     * @brief 注册控件
     * @param control 控件指针
     * @return 成功返回true，失败返回false
     */
    bool registerControl(UI::BaseControl* control);
    
    /**
     * @brief 注销控件
     * @param control 控件指针
     * @return 成功返回true，失败返回false
     */
    bool unregisterControl(UI::BaseControl* control);
    
    /**
     * @brief 根据MFC窗口查找控件
     * @param window MFC窗口指针
     * @return 控件指针，如果不存在返回nullptr
     */
    UI::BaseControl* findControl(CWnd* window);
    
    /**
     * @brief 根据ID查找控件
     * @param controlId 控件ID
     * @return 控件指针，如果不存在返回nullptr
     */
    UI::BaseControl* findControl(const std::string& controlId);
    
    /**
     * @brief 路由点击事件
     * @param window MFC窗口
     * @return 成功返回true，失败返回false
     */
    bool routeClick(CWnd* window);
    
    /**
     * @brief 路由双击事件
     * @param window MFC窗口
     * @return 成功返回true，失败返回false
     */
    bool routeDoubleClick(CWnd* window);
    
    /**
     * @brief 路由值改变事件
     * @param window MFC窗口
     * @return 成功返回true，失败返回false
     */
    bool routeChanged(CWnd* window);
    
    /**
     * @brief 路由选择事件
     * @param window MFC窗口
     * @return 成功返回true，失败返回false
     */
    bool routeSelected(CWnd* window);
    
    /**
     * @brief 路由鼠标移动事件
     * @param window MFC窗口
     * @param x X坐标
     * @param y Y坐标
     * @return 成功返回true，失败返回false
     */
    bool routeMouseMove(CWnd* window, int x, int y);
    
    /**
     * @brief 路由键盘按下事件
     * @param window MFC窗口
     * @param keyCode 按键代码
     * @return 成功返回true，失败返回false
     */
    bool routeKeyPress(CWnd* window, int keyCode);
    
    /**
     * @brief 路由窗口关闭事件
     * @param window MFC窗口
     * @return 成功返回true，失败返回false
     */
    bool routeWindowClose(CWnd* window);
    
    /**
     * @brief 路由窗口显示事件
     * @param window MFC窗口
     * @return 成功返回true，失败返回false
     */
    bool routeWindowShow(CWnd* window);
    
    /**
     * @brief 路由窗口隐藏事件
     * @param window MFC窗口
     * @return 成功返回true，失败返回false
     */
    bool routeWindowHide(CWnd* window);
    
    /**
     * @brief 设置事件管理器
     * @param manager 事件管理器指针
     */
    void setEventManager(EventManager* manager);
    
    /**
     * @brief 获取事件管理器
     * @return 事件管理器指针
     */
    EventManager* getEventManager();

private:
    /**
     * @brief 私有构造函数（单例模式）
     */
    EventRouter();
    
    /**
     * @brief 私有析构函数
     */
    ~EventRouter();
    
    /**
     * @brief 禁止拷贝构造
     */
    EventRouter(const EventRouter&);
    
    /**
     * @brief 禁止赋值操作
     */
    EventRouter& operator=(const EventRouter&);
    
    std::map<CWnd*, UI::BaseControl*> m_windowToControlMap;  ///< MFC窗口到控件的映射
    std::map<std::string, UI::BaseControl*> m_idToControlMap; ///< ID到控件的映射
    EventManager* m_eventManager;                              ///< 事件管理器
    
    /**
     * @brief 触发事件
     * @param control 控件指针
     * @param eventType 事件类型
     * @param args 事件参数
     * @return 成功返回true，失败返回false
     */
    bool triggerEvent(UI::BaseControl* control, EventType eventType, EventArgs* args);
};

} // namespace Events
} // namespace LuaUI

#endif // LUAUI_EVENTROUTER_H
