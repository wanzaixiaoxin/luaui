/**
 * @file EventManager.cpp
 * @brief 事件管理器实现
 */

#include "ui/events/EventManager.h"
#include <algorithm>

namespace LuaUI {
namespace Events {

EventManager::EventManager()
    : m_nextHandlerId(1)
{
}

EventManager::~EventManager() {
    m_handlers.clear();
    m_luaHandlers.clear();
}

int EventManager::registerHandler(const std::string& /*controlId*/, EventType /*eventType*/,
                                 EventHandler handler) {
    int handlerId = m_nextHandlerId++;
    m_handlers[handlerId] = handler;
    return handlerId;
}

bool EventManager::unregisterHandler(int handlerId) {
    std::map<int, EventHandler>::iterator it = m_handlers.find(handlerId);
    if (it != m_handlers.end()) {
        m_handlers.erase(it);
        return true;
    }
    return false;
}

void EventManager::unregisterControlHandlers(const std::string& controlId) {
    // 遍历并移除该控件的所有处理器
    std::vector<int> toRemove;
    
    for (std::map<int, LuaHandlerInfo>::iterator it = m_luaHandlers.begin();
         it != m_luaHandlers.end(); ++it) {
        if (it->second.controlId == controlId) {
            toRemove.push_back(it->first);
        }
    }
    
    for (size_t i = 0; i < toRemove.size(); ++i) {
        m_luaHandlers.erase(toRemove[i]);
    }
}

bool EventManager::fireEvent(const std::string& controlId, EventArgs* args) {
    bool handled = false;
    
    // 查找所有该控件的Lua处理器
    for (std::map<int, LuaHandlerInfo>::iterator it = m_luaHandlers.begin();
         it != m_luaHandlers.end(); ++it) {
        
        if (it->second.controlId == controlId && it->second.eventType == args->eventType) {
            if (callLuaHandler(it->second, args)) {
                handled = true;
            }
        }
    }
    
    return handled;
}

bool EventManager::fireEventByControl(UI::BaseControl* control, EventArgs* args) {
    if (!control) {
        return false;
    }
    
    return fireEvent(control->getId(), args);
}

int EventManager::registerLuaHandler(const std::string& controlId, EventType eventType,
                                    lua_State* lua, int funcRef) {
    int handlerId = m_nextHandlerId++;
    
    LuaHandlerInfo info;
    info.controlId = controlId;
    info.eventType = eventType;
    info.lua = lua;
    info.funcRef = funcRef;
    
    m_luaHandlers[handlerId] = info;
    
    return handlerId;
}

std::string EventManager::getEventTypeName(EventType type) {
    switch (type) {
        case EVENT_CLICK:          return "onClick";
        case EVENT_DOUBLE_CLICK:   return "onDoubleClick";
        case EVENT_CHANGED:        return "onChanged";
        case EVENT_SELECTED:       return "onSelected";
        case EVENT_HOVER:          return "onHover";
        case EVENT_FOCUS:          return "onFocus";
        case EVENT_BLUR:           return "onBlur";
        case EVENT_KEY_PRESS:      return "onKeyPress";
        case EVENT_KEY_RELEASE:    return "onKeyRelease";
        case EVENT_MOUSE_MOVE:     return "onMouseMove";
        case EVENT_SIZE_CHANGED:   return "onSizeChanged";
        case EVENT_POSITION_CHANGED:return "onPositionChanged";
        case EVENT_WINDOW_CLOSE:   return "onClose";
        case EVENT_WINDOW_SHOW:     return "onShow";
        case EVENT_WINDOW_HIDE:     return "onHide";
        default:                   return "onCustom";
    }
}

EventType EventManager::parseEventType(const std::string& typeName) {
    if (typeName == "onClick" || typeName == "click") {
        return EVENT_CLICK;
    } else if (typeName == "onDoubleClick" || typeName == "doubleClick") {
        return EVENT_DOUBLE_CLICK;
    } else if (typeName == "onChanged" || typeName == "changed") {
        return EVENT_CHANGED;
    } else if (typeName == "onSelected" || typeName == "selected") {
        return EVENT_SELECTED;
    } else if (typeName == "onHover" || typeName == "hover") {
        return EVENT_HOVER;
    } else if (typeName == "onFocus" || typeName == "focus") {
        return EVENT_FOCUS;
    } else if (typeName == "onBlur" || typeName == "blur") {
        return EVENT_BLUR;
    } else if (typeName == "onKeyPress" || typeName == "keyPress") {
        return EVENT_KEY_PRESS;
    } else if (typeName == "onMouseMove" || typeName == "mouseMove") {
        return EVENT_MOUSE_MOVE;
    } else if (typeName == "onClose" || typeName == "close") {
        return EVENT_WINDOW_CLOSE;
    } else if (typeName == "onShow" || typeName == "show") {
        return EVENT_WINDOW_SHOW;
    } else if (typeName == "onHide" || typeName == "hide") {
        return EVENT_WINDOW_HIDE;
    }
    return EVENT_CLICK; // 默认
}

bool EventManager::callLuaHandler(const LuaHandlerInfo& info, EventArgs* args) {
    if (!info.lua || info.funcRef < 0) {
        return false;
    }
    
    // 获取Lua函数
    lua_rawgeti(info.lua, LUA_REGISTRYINDEX, info.funcRef);
    
    // 压入事件源ID
    lua_pushstring(info.lua, args->eventSource ? args->eventSource->getId().c_str() : "");
    
    // 调用函数
    int result = lua_pcall(info.lua, 1, 0, 0);
    
    if (result != LUA_OK) {
        // 错误处理
        lua_pop(info.lua, 1);
        return false;
    }
    
    return true;
}

} // namespace Events
} // namespace LuaUI
