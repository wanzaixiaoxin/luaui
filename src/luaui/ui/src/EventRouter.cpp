/**
 * @file EventRouter.cpp
 * @brief 事件路由器实现
 */

#include "ui/events/EventRouter.h"

namespace LuaUI {
namespace Events {

EventRouter::EventRouter()
    : m_eventManager(nullptr)
{
}

EventRouter::~EventRouter() {
    m_windowToControlMap.clear();
    m_idToControlMap.clear();
}

EventRouter& EventRouter::instance() {
    static EventRouter router;
    return router;
}

bool EventRouter::registerControl(UI::BaseControl* control) {
    if (!control) {
        return false;
    }
    
    // 获取MFC窗口
    CWnd* window = control->getWindow();
    if (window) {
        m_windowToControlMap[window] = control;
    }
    
    // 按ID索引
    const std::string& controlId = control->getId();
    if (!controlId.empty()) {
        m_idToControlMap[controlId] = control;
    }
    
    return true;
}

bool EventRouter::unregisterControl(UI::BaseControl* control) {
    if (!control) {
        return false;
    }
    
    // 从MFC窗口映射中移除
    CWnd* window = control->getWindow();
    if (window) {
        m_windowToControlMap.erase(window);
    }
    
    // 从ID映射中移除
    const std::string& controlId = control->getId();
    if (!controlId.empty()) {
        m_idToControlMap.erase(controlId);
    }
    
    return true;
}

UI::BaseControl* EventRouter::findControl(CWnd* window) {
    std::map<CWnd*, UI::BaseControl*>::iterator it = m_windowToControlMap.find(window);
    if (it != m_windowToControlMap.end()) {
        return it->second;
    }
    return nullptr;
}

UI::BaseControl* EventRouter::findControl(const std::string& controlId) {
    std::map<std::string, UI::BaseControl*>::iterator it = m_idToControlMap.find(controlId);
    if (it != m_idToControlMap.end()) {
        return it->second;
    }
    return nullptr;
}

bool EventRouter::routeClick(CWnd* window) {
    UI::BaseControl* control = findControl(window);
    if (!control) {
        return false;
    }
    
    EventArgs args(EVENT_CLICK, control);
    return triggerEvent(control, EVENT_CLICK, &args);
}

bool EventRouter::routeDoubleClick(CWnd* window) {
    UI::BaseControl* control = findControl(window);
    if (!control) {
        return false;
    }
    
    EventArgs args(EVENT_DOUBLE_CLICK, control);
    return triggerEvent(control, EVENT_DOUBLE_CLICK, &args);
}

bool EventRouter::routeChanged(CWnd* window) {
    UI::BaseControl* control = findControl(window);
    if (!control) {
        return false;
    }
    
    EventArgs args(EVENT_CHANGED, control);
    return triggerEvent(control, EVENT_CHANGED, &args);
}

bool EventRouter::routeSelected(CWnd* window) {
    UI::BaseControl* control = findControl(window);
    if (!control) {
        return false;
    }
    
    EventArgs args(EVENT_SELECTED, control);
    return triggerEvent(control, EVENT_SELECTED, &args);
}

bool EventRouter::routeMouseMove(CWnd* window, int x, int y) {
    UI::BaseControl* control = findControl(window);
    if (!control) {
        return false;
    }
    
    MouseEventArgs args(EVENT_MOUSE_MOVE, control, x, y);
    return triggerEvent(control, EVENT_MOUSE_MOVE, &args);
}

bool EventRouter::routeKeyPress(CWnd* window, int keyCode) {
    UI::BaseControl* control = findControl(window);
    if (!control) {
        return false;
    }
    
    KeyEventArgs args(EVENT_KEY_PRESS, control, keyCode);
    return triggerEvent(control, EVENT_KEY_PRESS, &args);
}

bool EventRouter::routeWindowClose(CWnd* window) {
    UI::BaseControl* control = findControl(window);
    if (!control) {
        return false;
    }
    
    EventArgs args(EVENT_WINDOW_CLOSE, control);
    return triggerEvent(control, EVENT_WINDOW_CLOSE, &args);
}

bool EventRouter::routeWindowShow(CWnd* window) {
    UI::BaseControl* control = findControl(window);
    if (!control) {
        return false;
    }
    
    EventArgs args(EVENT_WINDOW_SHOW, control);
    return triggerEvent(control, EVENT_WINDOW_SHOW, &args);
}

bool EventRouter::routeWindowHide(CWnd* window) {
    UI::BaseControl* control = findControl(window);
    if (!control) {
        return false;
    }
    
    EventArgs args(EVENT_WINDOW_HIDE, control);
    return triggerEvent(control, EVENT_WINDOW_HIDE, &args);
}

void EventRouter::setEventManager(EventManager* manager) {
    m_eventManager = manager;
}

EventManager* EventRouter::getEventManager() {
    return m_eventManager;
}

bool EventRouter::triggerEvent(UI::BaseControl* control, EventType /*eventType*/, EventArgs* args) {
    if (m_eventManager) {
        return m_eventManager->fireEventByControl(control, args);
    }
    return false;
}

} // namespace Events
} // namespace LuaUI
