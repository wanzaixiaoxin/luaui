/**
 * @file BaseControl.cpp
 * @brief 控件基类实现
 */

#include "ui/controls/BaseControl.h"
#include "ui/events/EventManager.h"
#include "ui/events/LuaEventHandler.h"
#include "utils/Logger.h"

#include "core/LuaState.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

namespace LuaUI {
namespace UI {

BaseControl::BaseControl()
    : m_x(0)
    , m_y(0)
    , m_width(100)
    , m_height(30)
    , m_visible(true)
    , m_enabled(true)
{
}

BaseControl::~BaseControl() {
    // 清理事件处理器引用
    for (std::map<std::string, LuaFunctionRef>::iterator it = m_eventHandlers.begin();
         it != m_eventHandlers.end(); ++it) {
        if (it->second.lua && it->second.ref >= 0) {
            luaL_unref(it->second.lua, LUA_REGISTRYINDEX, it->second.ref);
        }
    }
    m_eventHandlers.clear();
}

bool BaseControl::createFromXml(Xml::XmlElement* xmlElement, CWnd* /*parent*/) {
    if (!xmlElement) {
        return false;
    }
    
    // 解析通用属性
    parseCommonAttributes(xmlElement);
    
    // 解析位置和大小
    parseGeometry(xmlElement);
    
    // 解析所有属性
    const Xml::XmlAttributes& attrs = xmlElement->getAttributes();
    for (Xml::XmlAttributes::const_iterator it = attrs.begin();
         it != attrs.end(); ++it) {
        m_properties[it->first] = it->second;
    }
    
    return true;
}

void BaseControl::bindLuaFunctions(lua_State* /*lua*/) {
    // 基类不绑定任何函数，由子类实现
}

const std::string& BaseControl::getId() const {
    return m_id;
}

void BaseControl::setId(const std::string& id) {
    m_id = id;
}

void BaseControl::setProperty(const std::string& name, const std::string& value) {
    m_properties[name] = value;
    
    // 处理特定属性
    if (name == "visible") {
        m_visible = (value == "true");
    } else if (name == "enabled") {
        m_enabled = (value == "true");
    }
}

std::string BaseControl::getProperty(const std::string& name) const {
    std::map<std::string, std::string>::const_iterator it = m_properties.find(name);
    if (it != m_properties.end()) {
        return it->second;
    }
    return "";
}

void BaseControl::setEventHandler(const std::string& eventName, const LuaFunctionRef& handler) {
    // 清除旧的处理器引用
    std::map<std::string, LuaFunctionRef>::iterator it = m_eventHandlers.find(eventName);
    if (it != m_eventHandlers.end() && it->second.lua && it->second.ref >= 0) {
        luaL_unref(it->second.lua, LUA_REGISTRYINDEX, it->second.ref);
    }
    
    // 设置新的处理器
    m_eventHandlers[eventName] = handler;
}

bool BaseControl::fireEvent(const std::string& eventName) {
    bool result = false;
    
    try {
        // 首先尝试使用旧的事件系统（为了向后兼容）
        std::map<std::string, LuaFunctionRef>::iterator it = m_eventHandlers.find(eventName);
        if (it != m_eventHandlers.end()) {
            result = callLuaHandler(it->second);
            
            // 如果旧系统处理成功，直接返回
            if (result) {
                return true;
            }
        }
        
        // 使用新的 EventManager 系统
        Events::EventArgs args;
        args.eventSource = this;
        args.eventType = Events::EventManager::parseEventType(eventName);
        
        Events::LuaEventHandler* luaEventHandler = Events::GetLuaEventHandler();
        if (luaEventHandler) {
            result = luaEventHandler->handleEvent(getId(), args.eventType, &args);
            
            // 记录事件处理结果
            LOG_S_DEBUG_CAT("BaseControl") << "Event " << eventName 
                                           << " fired for control " << getId() 
                                           << ", result: " << (result ? "success" : "failed");
        }
    }
    catch (const std::exception& e) {
        LOG_S_ERROR_CAT("BaseControl") << "Exception in fireEvent: " << e.what();
        result = false;
    }
    catch (...) {
        LOG_S_ERROR_CAT("BaseControl") << "Unknown exception in fireEvent";
        result = false;
    }
    
    return result;
}

void BaseControl::show() {
    m_visible = true;
}

void BaseControl::hide() {
    m_visible = false;
}

void BaseControl::enable() {
    m_enabled = true;
}

void BaseControl::disable() {
    m_enabled = false;
}

void BaseControl::setPosition(int x, int y) {
    m_x = x;
    m_y = y;
}

void BaseControl::setSize(int width, int height) {
    m_width = width;
    m_height = height;
}

void BaseControl::parseCommonAttributes(Xml::XmlElement* xmlElement) {
    if (!xmlElement) {
        return;
    }
    
    // 设置ID
    std::string id = xmlElement->getId();
    if (!id.empty()) {
        m_id = id;
    }
    
    // 解析可见性
    std::string visible = xmlElement->getAttribute("visible");
    if (!visible.empty()) {
        m_visible = (visible == "true");
    }
    
    // 解析启用状态
    std::string enabled = xmlElement->getAttribute("enabled");
    if (!enabled.empty()) {
        m_enabled = (enabled == "true");
    }
}

void BaseControl::parseGeometry(Xml::XmlElement* xmlElement) {
    if (!xmlElement) {
        return;
    }
    
    // 解析位置
    std::string x = xmlElement->getAttribute("x");
    std::string y = xmlElement->getAttribute("y");
    
    if (!x.empty()) {
        m_x = atoi(x.c_str());
    }
    if (!y.empty()) {
        m_y = atoi(y.c_str());
    }
    
    // 解析大小
    std::string width = xmlElement->getAttribute("width");
    std::string height = xmlElement->getAttribute("height");
    
    if (!width.empty()) {
        m_width = atoi(width.c_str());
    }
    if (!height.empty()) {
        m_height = atoi(height.c_str());
    }
}

bool BaseControl::callLuaHandler(const LuaFunctionRef& handler) {
    if (!handler.lua || handler.ref < 0) {
        return false;
    }
    
    // 获取Lua函数
    lua_rawgeti(handler.lua, LUA_REGISTRYINDEX, handler.ref);
    
    // 调用函数
    int result = lua_pcall(handler.lua, 0, 0, 0);
    
    if (result != LUA_OK) {
        // 错误处理
        lua_pop(handler.lua, 1);
        return false;
    }
    
    return true;
}

bool BaseControl::addChild(BaseControl* /*child*/) {
    // 默认实现：返回false，因为基础控件可能不支持子控件
    // 具体容器控件应该重写此方法
    return false;
}

BaseControl* BaseControl::findChildById(const std::string& /*id*/) {
    // 默认实现：返回nullptr，因为基础控件可能不支持子控件
    // 具体容器控件应该重写此方法
    return nullptr;
}

std::string BaseControl::getType() const {
    return "BaseControl";
}

CWnd* BaseControl::getWindow() {
    return nullptr; // BaseControl 本身不包含具体的 MFC 窗口
}

const CWnd* BaseControl::getWindow() const {
    return nullptr; // BaseControl 本身不包含具体的 MFC 窗口
}

} // namespace UI
} // namespace LuaUI
