/**
 * @file BaseControl.cpp
 * @brief 控件基类实现
 */

#include "ui/controls/BaseControl.h"
#include <lua.hpp>

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

bool BaseControl::createFromXml(Xml::XmlElement* xmlElement, CWnd* parent) {
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

void BaseControl::bindLuaFunctions(lua_State* lua) {
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
    std::map<std::string, LuaFunctionRef>::iterator it = m_eventHandlers.find(eventName);
    if (it != m_eventHandlers.end()) {
        return callLuaHandler(it->second);
    }
    return false;
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
        const char* error = lua_tostring(handler.lua, -1);
        lua_pop(handler.lua, 1);
        return false;
    }
    
    return true;
}

} // namespace UI
} // namespace LuaUI
