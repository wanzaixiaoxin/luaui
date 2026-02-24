#include "LuaAwareMvvmLoader.h"
#include "Logger.h"
#include "mvvm/MvvmXmlLoader.h"
#include <cstring>

namespace luaui {
namespace lua {

// ============================================================================
// LuaPropertyNotifier Implementation
// ============================================================================

LuaPropertyNotifier::LuaPropertyNotifier(lua_State* L, const std::string& viewModelName)
    : m_L(L), m_viewModelName(viewModelName) {
}

void LuaPropertyNotifier::PushViewModel() const {
    if (!m_L) return;
    lua_getglobal(m_L, m_viewModelName.c_str());
}

void LuaPropertyNotifier::SubscribePropertyChanged(PropertyChangedHandler handler) {
    m_handlers.push_back(handler);
}

void LuaPropertyNotifier::UnsubscribePropertyChanged(PropertyChangedHandler handler) {
    auto it = std::find_if(m_handlers.begin(), m_handlers.end(),
        [&handler](const auto& h) { return h.target_type() == handler.target_type(); });
    if (it != m_handlers.end()) {
        m_handlers.erase(it);
    }
}

void LuaPropertyNotifier::NotifyPropertyChanged(const std::string& propertyName) {
    mvvm::PropertyChangedEventArgs args;
    args.propertyName = propertyName;
    
    for (const auto& handler : m_handlers) {
        handler(args);
    }
}

std::any LuaPropertyNotifier::GetPropertyValue(const std::string& name) const {
    if (!m_L) return {};
    
    PushViewModel();
    if (!lua_istable(m_L, -1)) {
        lua_pop(m_L, 1);
        return {};
    }
    
    lua_getfield(m_L, -1, name.c_str());
    std::any result;
    
    if (lua_isstring(m_L, -1)) {
        result = std::string(lua_tostring(m_L, -1));
    } else if (lua_isnumber(m_L, -1)) {
        result = lua_tonumber(m_L, -1);
    } else if (lua_isboolean(m_L, -1)) {
        result = static_cast<bool>(lua_toboolean(m_L, -1));
    }
    
    lua_pop(m_L, 2);
    return result;
}

void LuaPropertyNotifier::SetPropertyValue(const std::string& name, const std::any& value) {
    if (!m_L) return;
    
    PushViewModel();
    if (!lua_istable(m_L, -1)) {
        lua_pop(m_L, 1);
        return;
    }
    
    try {
        if (value.type() == typeid(std::string)) {
            lua_pushstring(m_L, std::any_cast<std::string>(value).c_str());
        } else if (value.type() == typeid(double)) {
            lua_pushnumber(m_L, std::any_cast<double>(value));
        } else if (value.type() == typeid(int)) {
            lua_pushinteger(m_L, std::any_cast<int>(value));
        } else if (value.type() == typeid(bool)) {
            lua_pushboolean(m_L, std::any_cast<bool>(value));
        } else {
            lua_pop(m_L, 1);
            return;
        }
        
        lua_setfield(m_L, -2, name.c_str());
    } catch (...) {
        // Ignore conversion errors
    }
    
    lua_pop(m_L, 1);
    NotifyPropertyChanged(name);
}

bool LuaPropertyNotifier::CallFunction(const std::string& name) {
    if (!m_L) {
        utils::Logger::ErrorF("[Lua] CallFunction '%s': Lua state is null", name.c_str());
        return false;
    }
    
    // 确保栈有足够的空间
    lua_checkstack(m_L, 3);
    
    PushViewModel();
    if (!lua_istable(m_L, -1)) {
        utils::Logger::ErrorF("[Lua] CallFunction '%s': ViewModel is not a table", name.c_str());
        lua_pop(m_L, 1);
        return false;
    }
    
    lua_getfield(m_L, -1, name.c_str());
    if (!lua_isfunction(m_L, -1)) {
        lua_pop(m_L, 2);
        return false;
    }
    
    // 调用前保存栈顶位置
    int topBefore = lua_gettop(m_L);
    
    lua_pushvalue(m_L, -2);  // Push ViewModel as self
    int status = lua_pcall(m_L, 1, 0, 0);
    
    if (status != LUA_OK) {
        const char* error = lua_tostring(m_L, -1);
        utils::Logger::ErrorF("[Lua] Command '%s' error: %s", name.c_str(), error ? error : "unknown");
        lua_pop(m_L, 1);  // Pop error message
    }
    
    // 清理栈：保留 ViewModel，移除其他
    lua_settop(m_L, topBefore - 1);
    return status == LUA_OK;
}

bool LuaPropertyNotifier::HasProperty(const std::string& name) const {
    if (!m_L) return false;
    
    PushViewModel();
    if (!lua_istable(m_L, -1)) {
        lua_pop(m_L, 1);
        return false;
    }
    
    lua_getfield(m_L, -1, name.c_str());
    bool exists = !lua_isnil(m_L, -1);
    lua_pop(m_L, 2);
    return exists;
}

bool LuaPropertyNotifier::HasFunction(const std::string& name) const {
    if (!m_L) return false;
    
    PushViewModel();
    if (!lua_istable(m_L, -1)) {
        lua_pop(m_L, 1);
        return false;
    }
    
    lua_getfield(m_L, -1, name.c_str());
    bool isFunc = lua_isfunction(m_L, -1);
    lua_pop(m_L, 2);
    return isFunc;
}

// ============================================================================
// LuaAwareMvvmLoader Implementation
// ============================================================================

LuaAwareMvvmLoader::LuaAwareMvvmLoader() = default;
LuaAwareMvvmLoader::~LuaAwareMvvmLoader() = default;

void LuaAwareMvvmLoader::SetLuaState(lua_State* L) {
    m_L = L;
}

void LuaAwareMvvmLoader::SetViewModelName(const std::string& name) {
    m_viewModelName = name;
}

std::shared_ptr<luaui::Control> LuaAwareMvvmLoader::Load(const std::string& filePath) {
    if (!m_L) {
        utils::Logger::Error("[LuaAwareMvvmLoader] Lua state not set");
        return nullptr;
    }
    
    m_notifier = std::make_shared<LuaPropertyNotifier>(m_L, m_viewModelName);
    m_baseLoader = mvvm::CreateMvvmXmlLoader();
    m_baseLoader->SetDataContext(m_notifier);
    
    AutoBindCommands();
    
    auto root = m_baseLoader->Load(filePath);
    
    if (root) {
        utils::Logger::Info("[LuaAwareMvvmLoader] Layout loaded with Lua bindings");
    }
    
    return root;
}

void LuaAwareMvvmLoader::AutoBindCommands() {
    if (!m_L || !m_notifier || !m_baseLoader) return;
    
    lua_getglobal(m_L, m_viewModelName.c_str());
    if (!lua_istable(m_L, -1)) {
        lua_pop(m_L, 1);
        return;
    }
    
    int commandCount = 0;
    
    lua_pushnil(m_L);
    while (lua_next(m_L, -2) != 0) {
        if (lua_type(m_L, -2) == LUA_TSTRING) {
            const char* key = lua_tostring(m_L, -2);
            
            if (lua_isfunction(m_L, -1)) {
                if (strstr(key, "Command") || strstr(key, "Handler")) {
                    RegisterLuaCommand(key);
                    commandCount++;
                }
            }
        }
        lua_pop(m_L, 1);
    }
    
    lua_pop(m_L, 1);
    
    utils::Logger::InfoF("[LuaAwareMvvmLoader] Auto-bound %d commands", commandCount);
}

void LuaAwareMvvmLoader::RegisterLuaCommand(const std::string& commandName) {
    if (!m_baseLoader) return;
    
    m_baseLoader->RegisterClickHandler(commandName, [this, commandName]() {
        if (m_notifier) {
            m_notifier->CallFunction(commandName);
        }
    });
}

} // namespace lua
} // namespace luaui
