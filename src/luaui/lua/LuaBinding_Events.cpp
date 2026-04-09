// Lua Event Binding - Allows Lua functions as C++ event callbacks
// Bridges Lua functions to C++ event system (Control events, Button.Click, etc.)

#include "LuaSandbox.h"
#include "Window.h"
#include "Controls.h"
#include "Button.h"
#include "TextBox.h"
#include "Slider.h"
#include "CheckBox.h"
#include "Panel.h"
#include "Logger.h"
#include "../utils/StringUtils.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

#include <unordered_map>
#include <functional>
#include <memory>
#include <string>

namespace luaui {
namespace lua {

// ============================================================================
// Lua Callback Registry
// ============================================================================

// Stores Lua function references and manages their lifecycle
class LuaCallbackRegistry {
public:
    static LuaCallbackRegistry& Instance() {
        static LuaCallbackRegistry instance;
        return instance;
    }
    
    // Register a Lua function and return an ID
    int RegisterCallback(lua_State* L, int funcIndex) {
        if (!L || !lua_isfunction(L, funcIndex)) {
            return -1;
        }
        
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Get the registry for this Lua state
        auto& stateRegistry = m_registry[L];
        
        // Create reference to the function
        lua_pushvalue(L, funcIndex);  // Duplicate function at top of stack
        int ref = luaL_ref(L, LUA_REGISTRYINDEX);  // Store in registry
        
        stateRegistry[ref] = {L, ref};
        return ref;
    }
    
    // Unregister a callback
    void UnregisterCallback(lua_State* L, int ref) {
        if (ref < 0) return;
        
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto stateIt = m_registry.find(L);
        if (stateIt != m_registry.end()) {
            luaL_unref(L, LUA_REGISTRYINDEX, ref);
            stateIt->second.erase(ref);
        }
    }
    
    // Execute a registered callback
    bool ExecuteCallback(int ref, const std::vector<std::string>& args = {}) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Find the callback
        CallbackInfo* info = nullptr;
        for (auto& [state, registry] : m_registry) {
            auto it = registry.find(ref);
            if (it != registry.end()) {
                info = &it->second;
                break;
            }
        }
        
        if (!info || !info->L) {
            return false;
        }
        
        lua_State* L = info->L;
        
        // Push the function from registry
        lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
        
        if (!lua_isfunction(L, -1)) {
            lua_pop(L, 1);
            return false;
        }
        
        // Push arguments
        for (const auto& arg : args) {
            lua_pushstring(L, arg.c_str());
        }
        
        // Call the function
        int status = lua_pcall(L, static_cast<int>(args.size()), 0, 0);
        
        if (status != LUA_OK) {
            const char* error = lua_tostring(L, -1);
            luaui::utils::Logger::ErrorF("[Lua] Event callback error: %s", error ? error : "unknown");
            lua_pop(L, 1);
            return false;
        }
        
        return true;
    }
    
    // Execute callback with control pointer argument
    bool ExecuteCallbackWithControl(int ref, luaui::Control* control, const std::string& eventName) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        CallbackInfo* info = nullptr;
        for (auto& [state, registry] : m_registry) {
            auto it = registry.find(ref);
            if (it != registry.end()) {
                info = &it->second;
                break;
            }
        }
        
        if (!info || !info->L) {
            return false;
        }
        
        lua_State* L = info->L;
        
        // Push the function
        lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
        
        if (!lua_isfunction(L, -1)) {
            lua_pop(L, 1);
            return false;
        }
        
        // TODO: Push control userdata if available, otherwise nil
        (void)control;  // Unused for now
        lua_pushnil(L);  // Placeholder for control
        lua_pushstring(L, eventName.c_str());
        
        int status = lua_pcall(L, 2, 0, 0);
        
        if (status != LUA_OK) {
            const char* error = lua_tostring(L, -1);
            luaui::utils::Logger::ErrorF("[Lua] Event callback error: %s", error ? error : "unknown");
            lua_pop(L, 1);
            return false;
        }
        
        return true;
    }
    
    // Clean up all callbacks for a Lua state
    void CleanupState(lua_State* L) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_registry.find(L);
        if (it != m_registry.end()) {
            for (auto& [ref, info] : it->second) {
                luaL_unref(L, LUA_REGISTRYINDEX, ref);
            }
            m_registry.erase(it);
        }
    }
    
private:
    struct CallbackInfo {
        lua_State* L = nullptr;
        int ref = -1;
    };
    
    std::unordered_map<lua_State*, std::unordered_map<int, CallbackInfo>> m_registry;
    std::mutex m_mutex;
};

// ============================================================================
// Event Connection Class
// ============================================================================

// Represents a connection between a C++ event and a Lua callback
class LuaEventConnection {
public:
    using DisconnectFunc = std::function<void()>;
    
    LuaEventConnection() = default;
    
    LuaEventConnection(int callbackRef, DisconnectFunc disconnect)
        : m_callbackRef(callbackRef), m_disconnectFunc(disconnect) {}
    
    ~LuaEventConnection() {
        Disconnect();
    }
    
    // Disable copy
    LuaEventConnection(const LuaEventConnection&) = delete;
    LuaEventConnection& operator=(const LuaEventConnection&) = delete;
    
    // Enable move
    LuaEventConnection(LuaEventConnection&& other) noexcept {
        m_callbackRef = other.m_callbackRef;
        m_disconnectFunc = std::move(other.m_disconnectFunc);
        other.m_callbackRef = -1;
    }
    
    LuaEventConnection& operator=(LuaEventConnection&& other) noexcept {
        if (this != &other) {
            Disconnect();
            m_callbackRef = other.m_callbackRef;
            m_disconnectFunc = std::move(other.m_disconnectFunc);
            other.m_callbackRef = -1;
        }
        return *this;
    }
    
    void Disconnect() {
        if (m_disconnectFunc) {
            m_disconnectFunc();
            m_disconnectFunc = nullptr;
        }
        m_callbackRef = -1;
    }
    
    bool IsConnected() const {
        return m_callbackRef >= 0;
    }
    
private:
    int m_callbackRef = -1;
    DisconnectFunc m_disconnectFunc;
};

// ============================================================================
// Event Binding Registration
// ============================================================================

void LuaBinding::RegisterEvents(lua_State* L) {
    // Create Event table for managing connections
    lua_newtable(L);
    
    // Event.disconnect(connection) - disconnect an event connection
    lua_pushcfunction(L, [](lua_State* L) -> int {
        // Expect a connection userdata
        if (!lua_isuserdata(L, 1)) {
            luaL_error(L, "Expected connection object");
            return 0;
        }
        
        auto* conn = static_cast<LuaEventConnection**>(lua_touserdata(L, 1));
        if (conn && *conn) {
            (*conn)->Disconnect();
        }
        return 0;
    });
    lua_setfield(L, -2, "disconnect");
    
    lua_setglobal(L, "Event");
}

// ============================================================================
// Control Event Bindings
// ============================================================================

// Helper to register click event for Button
static int RegisterButtonClick(lua_State* L, luaui::controls::Button* button, int funcIndex) {
    int callbackRef = LuaCallbackRegistry::Instance().RegisterCallback(L, funcIndex);
    if (callbackRef < 0) {
        return -1;
    }
    
    // Create a C++ handler that calls the Lua callback
    auto handler = [callbackRef](luaui::Control*) {
        LuaCallbackRegistry::Instance().ExecuteCallback(callbackRef);
    };
    
    // Connect to the button's Click event
    button->Click.Add(handler);
    
    // Store reference to Lua state for cleanup
    return callbackRef;
}

// Register event-related functions for Button
void RegisterButtonEvents(lua_State* L) {
    // Extend Button metatable with onClick method
    luaL_getmetatable(L, "LuaUI.Button");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return;
    }
    
    lua_getfield(L, -1, "__index");
    
    // Button:onClick(callback)
    lua_pushcfunction(L, [](lua_State* L) -> int {
        // Get button from self
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::Button>*>(
            lua_touserdata(L, 1)
        );
        if (!ptr) {
            luaL_error(L, "Invalid button");
            return 0;
        }
        
        // Check callback function
        luaL_checktype(L, 2, LUA_TFUNCTION);
        
        int ref = RegisterButtonClick(L, ptr->get(), 2);
        if (ref < 0) {
            luaL_error(L, "Failed to register click handler");
            return 0;
        }
        
        // Return a connection object
        auto** conn = static_cast<LuaEventConnection**>(
            lua_newuserdata(L, sizeof(LuaEventConnection*))
        );
        
        // Create connection with cleanup
        *conn = new LuaEventConnection(ref, [L, ref]() {
            LuaCallbackRegistry::Instance().UnregisterCallback(L, ref);
        });
        
        // Set metatable for connection
        luaL_newmetatable(L, "LuaUI.EventConnection");
        lua_setmetatable(L, -2);
        
        return 1;
    });
    lua_setfield(L, -2, "onClick");
    
    lua_pop(L, 2);
}

// Register Slider events
static int RegisterSliderValueChanged(lua_State* L, luaui::controls::Slider* slider, int funcIndex) {
    int callbackRef = LuaCallbackRegistry::Instance().RegisterCallback(L, funcIndex);
    if (callbackRef < 0) {
        return -1;
    }
    
    auto handler = [callbackRef](luaui::controls::Slider*, double value) {
        std::vector<std::string> args = {std::to_string(value)};
        LuaCallbackRegistry::Instance().ExecuteCallback(callbackRef, args);
    };
    
    slider->ValueChanged.Add(handler);
    return callbackRef;
}

void RegisterSliderEvents(lua_State* L) {
    luaL_getmetatable(L, "LuaUI.Slider");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return;
    }
    
    lua_getfield(L, -1, "__index");
    
    // Slider:onValueChanged(callback)
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::Slider>*>(
            lua_touserdata(L, 1)
        );
        if (!ptr) {
            luaL_error(L, "Invalid slider");
            return 0;
        }
        
        luaL_checktype(L, 2, LUA_TFUNCTION);
        
        int ref = RegisterSliderValueChanged(L, ptr->get(), 2);
        if (ref < 0) {
            luaL_error(L, "Failed to register value changed handler");
            return 0;
        }
        
        auto** conn = static_cast<LuaEventConnection**>(
            lua_newuserdata(L, sizeof(LuaEventConnection*))
        );
        
        *conn = new LuaEventConnection(ref, [L, ref]() {
            LuaCallbackRegistry::Instance().UnregisterCallback(L, ref);
        });
        
        luaL_newmetatable(L, "LuaUI.EventConnection");
        lua_setmetatable(L, -2);
        
        return 1;
    });
    lua_setfield(L, -2, "onValueChanged");
    
    lua_pop(L, 2);
}

// Register CheckBox events - 保留供将来使用
// static int RegisterCheckBoxChanged(lua_State* L, luaui::controls::CheckBox* checkbox, int funcIndex) {
//     int callbackRef = LuaCallbackRegistry::Instance().RegisterCallback(L, funcIndex);
//     if (callbackRef < 0) {
//         return -1;
//     }
//     
//     // Note: CheckBox doesn't have a direct event in current implementation
//     // This would need to be added to the CheckBox class
//     
//     return callbackRef;
// }

void RegisterCheckBoxEvents(lua_State* L) {
    luaL_getmetatable(L, "LuaUI.CheckBox");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return;
    }
    
    lua_getfield(L, -1, "__index");
    
    // CheckBox:onCheckedChanged(callback)
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::CheckBox>*>(
            lua_touserdata(L, 1)
        );
        if (!ptr) {
            luaL_error(L, "Invalid checkbox");
            return 0;
        }
        
        luaL_checktype(L, 2, LUA_TFUNCTION);
        
        // For now, just log that this would be connected
        // Full implementation would need CheckBox to have an IsCheckedChanged event
        luaui::utils::Logger::Warning("[Lua] CheckBox:onCheckedChanged not fully implemented yet");
        
        lua_pushnil(L);
        return 1;
    });
    lua_setfield(L, -2, "onCheckedChanged");
    
    lua_pop(L, 2);
}

// ============================================================================
// TextBox Event Bindings
// ============================================================================

static int RegisterTextBoxTextChanged(lua_State* L, luaui::controls::TextBox* textBox, int funcIndex) {
    int callbackRef = LuaCallbackRegistry::Instance().RegisterCallback(L, funcIndex);
    if (callbackRef < 0) {
        return -1;
    }
    
    auto handler = [callbackRef](luaui::controls::TextBox*, const std::wstring& text) {
        std::string narrow = WToUtf8(text);
        LuaCallbackRegistry::Instance().ExecuteCallback(callbackRef, {narrow});
    };
    
    textBox->TextChanged.Add(handler);
    return callbackRef;
}

void RegisterTextBoxEvents(lua_State* L) {
    luaL_getmetatable(L, "LuaUI.TextBox");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return;
    }
    
    lua_getfield(L, -1, "__index");
    
    // TextBox:onTextChanged(callback)
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::TextBox>*>(
            lua_touserdata(L, 1)
        );
        if (!ptr) {
            luaL_error(L, "Invalid TextBox");
            return 0;
        }
        
        luaL_checktype(L, 2, LUA_TFUNCTION);
        
        int ref = RegisterTextBoxTextChanged(L, ptr->get(), 2);
        if (ref < 0) {
            luaL_error(L, "Failed to register TextChanged handler");
            return 0;
        }
        
        // Return a connection object
        auto** conn = static_cast<LuaEventConnection**>(
            lua_newuserdata(L, sizeof(LuaEventConnection*))
        );
        
        *conn = new LuaEventConnection(ref, [L, ref]() {
            LuaCallbackRegistry::Instance().UnregisterCallback(L, ref);
        });
        
        luaL_newmetatable(L, "LuaUI.EventConnection");
        lua_setmetatable(L, -2);
        
        return 1;
    });
    lua_setfield(L, -2, "onTextChanged");
    
    lua_pop(L, 2);
}

// ============================================================================
// Commands Binding
// ============================================================================

void LuaBinding::RegisterCommands(lua_State* L) {
    // Create Command table for MVVM-style commands
    lua_newtable(L);
    
    // Command.create(executeFunc, canExecuteFunc) -> command object
    lua_pushcfunction(L, [](lua_State* L) -> int {
        luaL_checktype(L, 1, LUA_TFUNCTION);  // execute
        
        int executeRef = LuaCallbackRegistry::Instance().RegisterCallback(L, 1);
        int canExecuteRef = -1;
        
        if (lua_gettop(L) >= 2 && lua_isfunction(L, 2)) {
            canExecuteRef = LuaCallbackRegistry::Instance().RegisterCallback(L, 2);
        }
        
        // Create command table
        lua_newtable(L);
        
        // Store references
        lua_pushinteger(L, executeRef);
        lua_setfield(L, -2, "_executeRef");
        
        if (canExecuteRef >= 0) {
            lua_pushinteger(L, canExecuteRef);
            lua_setfield(L, -2, "_canExecuteRef");
        }
        
        // execute() method
        lua_pushcfunction(L, [](lua_State* L) -> int {
            lua_getfield(L, 1, "_executeRef");
            int ref = static_cast<int>(lua_tointeger(L, -1));
            lua_pop(L, 1);
            
            LuaCallbackRegistry::Instance().ExecuteCallback(ref);
            return 0;
        });
        lua_setfield(L, -2, "execute");
        
        // canExecute() method
        lua_pushcfunction(L, [](lua_State* L) -> int {
            lua_getfield(L, 1, "_canExecuteRef");
            if (lua_isnil(L, -1)) {
                lua_pop(L, 1);
                lua_pushboolean(L, true);
                return 1;
            }
            
            // For now, always return true
            // Full implementation would call the canExecute function
            lua_pop(L, 1);
            lua_pushboolean(L, true);
            return 1;
        });
        lua_setfield(L, -2, "canExecute");
        
        return 1;
    });
    lua_setfield(L, -2, "create");
    
    lua_setglobal(L, "Command");
}

// ============================================================================
// Initialization
// ============================================================================

void InitializeEventBindings(lua_State* L) {
    // Register connection metatable
    luaL_newmetatable(L, "LuaUI.EventConnection");
    
    // __gc for cleanup
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto** conn = static_cast<LuaEventConnection**>(lua_touserdata(L, 1));
        if (conn && *conn) {
            delete *conn;
            *conn = nullptr;
        }
        return 0;
    });
    lua_setfield(L, -2, "__gc");
    
    // disconnect method
    lua_newtable(L);
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto** conn = static_cast<LuaEventConnection**>(lua_touserdata(L, 1));
        if (conn && *conn) {
            (*conn)->Disconnect();
        }
        return 0;
    });
    lua_setfield(L, -2, "disconnect");
    lua_setfield(L, -2, "__index");
    
    lua_pop(L, 1);
    
    // Register event bindings for each control type
    RegisterButtonEvents(L);
    RegisterTextBoxEvents(L);
    RegisterSliderEvents(L);
    RegisterCheckBoxEvents(L);
}

} // namespace lua
} // namespace luaui
