/**
 * @file ControlBinder.cpp
 * @brief Control Binder Implementation
 */

#include "lua/binding/ControlBinder.h"
#include "lua/binding/LuaBinder.h"
#include "ui/controls/BaseControl.h"
#include "utils/Logger.h"
#include <iostream>
#include <windows.h>
#include <map>

namespace LuaUI {
namespace Lua {
namespace Binding {

// Control mapping table
static std::map<std::string, UI::BaseControl*> g_controlMap;

ControlBinder::ControlBinder(lua_State* lua)
    : m_lua(lua)
{
}

ControlBinder::~ControlBinder() {
}

void ControlBinder::initialize() {
    if (!m_lua) {
        return;
    }
    
    // Create UI global table
    lua_newtable(m_lua);
    
    // Register functions
    lua_pushcfunction(m_lua, luaGetControl);
    lua_setfield(m_lua, -2, "getControl");
    
    lua_pushcfunction(m_lua, luaSetProperty);
    lua_setfield(m_lua, -2, "setProperty");
    
    lua_pushcfunction(m_lua, luaGetProperty);
    lua_setfield(m_lua, -2, "getProperty");
    
    lua_pushcfunction(m_lua, luaShow);
    lua_setfield(m_lua, -2, "show");
    
    lua_pushcfunction(m_lua, luaHide);
    lua_setfield(m_lua, -2, "hide");
    
    lua_pushcfunction(m_lua, luaEnable);
    lua_setfield(m_lua, -2, "enable");
    
    lua_pushcfunction(m_lua, luaDisable);
    lua_setfield(m_lua, -2, "disable");
    
    lua_pushcfunction(m_lua, luaSetPosition);
    lua_setfield(m_lua, -2, "setPosition");
    
    lua_pushcfunction(m_lua, luaSetSize);
    lua_setfield(m_lua, -2, "setSize");
    
    lua_pushcfunction(m_lua, luaSetText);
    lua_setfield(m_lua, -2, "setText");
    
    lua_pushcfunction(m_lua, luaGetText);
    lua_setfield(m_lua, -2, "getText");

    lua_pushcfunction(m_lua, luaExit);
    lua_setfield(m_lua, -2, "exit");

    // Set as global variable
    lua_setglobal(m_lua, "UI");
}

void ControlBinder::registerControl(UI::BaseControl* control, const std::string& name) {
    if (control && !name.empty()) {
        g_controlMap[name] = control;
        LOG_S_DEBUG_CAT("ControlBinder") << "Registered control to map: " << name << " (map size: " << g_controlMap.size() << ")";
    }
}

void ControlBinder::unregisterControl(const std::string& name) {
    std::map<std::string, UI::BaseControl*>::iterator it = g_controlMap.find(name);
    if (it != g_controlMap.end()) {
        g_controlMap.erase(it);
    }
}

UI::BaseControl* ControlBinder::getControl(const std::string& name) {
    std::map<std::string, UI::BaseControl*>::iterator it = g_controlMap.find(name);
    if (it != g_controlMap.end()) {
        return it->second;
    }
    return nullptr;
}

int ControlBinder::luaGetControl(lua_State* L) {
    // Handle both UI:getControl("id") and UI.getControl("id")
    int argIndex = 1;
    
    // If called with colon syntax, first param is UI table itself
    if (lua_gettop(L) >= 2 && lua_istable(L, 1)) {
        argIndex = 2;
    }
    
    if (lua_gettop(L) < argIndex || !lua_isstring(L, argIndex)) {
        lua_pushnil(L);
        return 1;
    }
    
    const char* controlId = lua_tostring(L, argIndex);
    
    if (!controlId) {
        lua_pushnil(L);
        return 1;
    }
    
    LOG_S_DEBUG_CAT("ControlBinder") << "luaGetControl: looking for '" << controlId << "' (map size: " << g_controlMap.size() << ")";
    
    UI::BaseControl* control = nullptr;
    
    std::map<std::string, UI::BaseControl*>::iterator it = g_controlMap.find(controlId);
    
    if (it != g_controlMap.end()) {
        control = it->second;
        LOG_S_DEBUG_CAT("ControlBinder") << "luaGetControl: found '" << controlId << "'";
    } else {
        LOG_S_DEBUG_CAT("ControlBinder") << "luaGetControl: '" << controlId << "' not found";
    }
    
    if (control) {
        lua_pushlightuserdata(L, control);
    } else {
        lua_pushnil(L);
    }
    
    return 1;
}

int ControlBinder::luaSetProperty(lua_State* L) {
    // Handle both UI:setProperty(id, name, value) and UI.setProperty(id, name, value)
    int argIndex = 1;
    
    // If called with colon syntax, first param is UI table itself
    if (lua_gettop(L) >= 4 && lua_istable(L, 1)) {
        argIndex = 2;
    }
    
    // params: controlId, name, value
    if (lua_gettop(L) < argIndex + 2) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    const char* controlId = lua_tostring(L, argIndex);
    const char* name = lua_tostring(L, argIndex + 1);
    const char* value = lua_tostring(L, argIndex + 2);
    
    if (!controlId || !name || !value) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    std::map<std::string, UI::BaseControl*>::iterator it = g_controlMap.find(controlId);
    
    bool result = false;
    
    if (it != g_controlMap.end()) {
        it->second->setProperty(name, value);
        result = true;
    }
    
    lua_pushboolean(L, result ? 1 : 0);
    return 1;
}

int ControlBinder::luaGetProperty(lua_State* L) {
    // Handle both UI:getProperty(id, name) and UI.getProperty(id, name)
    int argIndex = 1;
    
    // If called with colon syntax, first param is UI table itself
    if (lua_gettop(L) >= 3 && lua_istable(L, 1)) {
        argIndex = 2;
    }
    
    // params: controlId, name
    if (lua_gettop(L) < argIndex + 1) {
        lua_pushnil(L);
        return 1;
    }
    
    const char* controlId = lua_tostring(L, argIndex);
    const char* name = lua_tostring(L, argIndex + 1);
    
    std::map<std::string, UI::BaseControl*>::iterator it = g_controlMap.find(controlId);
    
    if (it != g_controlMap.end()) {
        std::string value = it->second->getProperty(name);
        lua_pushstring(L, value.c_str());
    } else {
        lua_pushnil(L);
    }
    
    return 1;
}

int ControlBinder::luaShow(lua_State* L) {
    // Handle both UI:show(id) and UI.show(id)
    int argIndex = 1;
    if (lua_gettop(L) >= 2 && lua_istable(L, 1)) {
        argIndex = 2;
    }
    
    // param: controlId
    if (lua_gettop(L) < argIndex) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    const char* controlId = lua_tostring(L, argIndex);
    
    std::map<std::string, UI::BaseControl*>::iterator it = g_controlMap.find(controlId);
    
    bool result = false;
    
    if (it != g_controlMap.end()) {
        it->second->show();
        result = true;
    }
    
    lua_pushboolean(L, result ? 1 : 0);
    return 1;
}

int ControlBinder::luaHide(lua_State* L) {
    // Handle both UI:hide(id) and UI.hide(id)
    int argIndex = 1;
    if (lua_gettop(L) >= 2 && lua_istable(L, 1)) {
        argIndex = 2;
    }
    
    // param: controlId
    if (lua_gettop(L) < argIndex) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    const char* controlId = lua_tostring(L, argIndex);
    
    std::map<std::string, UI::BaseControl*>::iterator it = g_controlMap.find(controlId);
    
    bool result = false;
    
    if (it != g_controlMap.end()) {
        it->second->hide();
        result = true;
    }
    
    lua_pushboolean(L, result ? 1 : 0);
    return 1;
}

int ControlBinder::luaEnable(lua_State* L) {
    // Handle both UI:enable(id) and UI.enable(id)
    int argIndex = 1;
    if (lua_gettop(L) >= 2 && lua_istable(L, 1)) {
        argIndex = 2;
    }
    
    // param: controlId
    if (lua_gettop(L) < argIndex) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    const char* controlId = lua_tostring(L, argIndex);
    
    std::map<std::string, UI::BaseControl*>::iterator it = g_controlMap.find(controlId);
    
    bool result = false;
    
    if (it != g_controlMap.end()) {
        it->second->enable();
        result = true;
    }
    
    lua_pushboolean(L, result ? 1 : 0);
    return 1;
}

int ControlBinder::luaDisable(lua_State* L) {
    // Handle both UI:disable(id) and UI.disable(id)
    int argIndex = 1;
    if (lua_gettop(L) >= 2 && lua_istable(L, 1)) {
        argIndex = 2;
    }
    
    // param: controlId
    if (lua_gettop(L) < argIndex) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    const char* controlId = lua_tostring(L, argIndex);
    
    std::map<std::string, UI::BaseControl*>::iterator it = g_controlMap.find(controlId);
    
    bool result = false;
    
    if (it != g_controlMap.end()) {
        it->second->disable();
        result = true;
    }
    
    lua_pushboolean(L, result ? 1 : 0);
    return 1;
}

int ControlBinder::luaSetPosition(lua_State* L) {
    // Handle both UI:setPosition(id, x, y) and UI.setPosition(id, x, y)
    int argIndex = 1;
    if (lua_gettop(L) >= 4 && lua_istable(L, 1)) {
        argIndex = 2;
    }
    
    // params: controlId, x, y
    if (lua_gettop(L) < argIndex + 2) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    const char* controlId = lua_tostring(L, argIndex);
    int x = (int)lua_tointeger(L, argIndex + 1);
    int y = (int)lua_tointeger(L, argIndex + 2);
    
    std::map<std::string, UI::BaseControl*>::iterator it = g_controlMap.find(controlId);
    
    bool result = false;
    
    if (it != g_controlMap.end()) {
        it->second->setPosition(x, y);
        result = true;
    }
    
    lua_pushboolean(L, result ? 1 : 0);
    return 1;
}

int ControlBinder::luaSetSize(lua_State* L) {
    // Handle both UI:setSize(id, width, height) and UI.setSize(id, width, height)
    int argIndex = 1;
    if (lua_gettop(L) >= 4 && lua_istable(L, 1)) {
        argIndex = 2;
    }
    
    // params: controlId, width, height
    if (lua_gettop(L) < argIndex + 2) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    const char* controlId = lua_tostring(L, argIndex);
    int width = (int)lua_tointeger(L, argIndex + 1);
    int height = (int)lua_tointeger(L, argIndex + 2);
    
    std::map<std::string, UI::BaseControl*>::iterator it = g_controlMap.find(controlId);
    
    bool result = false;
    
    if (it != g_controlMap.end()) {
        it->second->setSize(width, height);
        result = true;
    }
    
    lua_pushboolean(L, result ? 1 : 0);
    return 1;
}

int ControlBinder::luaSetText(lua_State* L) {
    // Handle both UI:setText(id, text) and UI.setText(id, text)
    int argIndex = 1;
    if (lua_gettop(L) >= 3 && lua_istable(L, 1)) {
        argIndex = 2;
    }
    
    // params: controlId, text
    if (lua_gettop(L) < argIndex + 1) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    const char* controlId = lua_tostring(L, argIndex);
    const char* text = lua_tostring(L, argIndex + 1);
    
    std::map<std::string, UI::BaseControl*>::iterator it = g_controlMap.find(controlId);
    
    bool result = false;
    
    if (it != g_controlMap.end()) {
        it->second->setProperty("text", text);
        result = true;
    }
    
    lua_pushboolean(L, result ? 1 : 0);
    return 1;
}

int ControlBinder::luaGetText(lua_State* L) {
    // Handle both UI:getText(id) and UI.getText(id)
    int argIndex = 1;
    if (lua_gettop(L) >= 2 && lua_istable(L, 1)) {
        argIndex = 2;
    }
    
    // param: controlId
    if (lua_gettop(L) < argIndex) {
        lua_pushnil(L);
        return 1;
    }
    
    const char* controlId = lua_tostring(L, argIndex);
    
    std::map<std::string, UI::BaseControl*>::iterator it = g_controlMap.find(controlId);
    
    if (it != g_controlMap.end()) {
        std::string text = it->second->getProperty("text");
        lua_pushstring(L, text.c_str());
    } else {
        lua_pushnil(L);
    }

    return 1;
}

int ControlBinder::luaExit(lua_State* /*L*/) {
    // Send WM_QUIT message to exit application
    PostQuitMessage(0);
    return 0;
}

} // namespace Binding
} // namespace Lua
} // namespace LuaUI
