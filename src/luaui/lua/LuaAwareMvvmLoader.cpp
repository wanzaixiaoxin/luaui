#include "LuaAwareMvvmLoader.h"
#include "Logger.h"
#include "mvvm/MvvmXmlLoader.h"
#include <cstring>
#include <unordered_set>
#include <functional>

namespace luaui {
namespace lua {

// ============================================================================
// LuaPropertyNotifier Implementation
// ============================================================================

LuaPropertyNotifier::LuaPropertyNotifier(lua_State* L, const std::string& viewModelName)
    : m_L(L), m_viewModelName(viewModelName) {
    utils::Logger::DebugF("[LuaPropertyNotifier] Created for '%s'", viewModelName.c_str());
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

LuaPropertyNotifier::~LuaPropertyNotifier() {
    utils::Logger::DebugF("[LuaPropertyNotifier] Destroyed for '%s'", m_viewModelName.c_str());
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
    
    // Special handling: if IsFeatureEnabled changed, call UpdateStatus
    if (name == "IsFeatureEnabled") {
        utils::Logger::Debug("[Lua] IsFeatureEnabled changed, calling UpdateStatus");
        bool result = CallFunction("UpdateStatus");
        utils::Logger::DebugF("[Lua] UpdateStatus call result: %s", result ? "success" : "failed");
        NotifyPropertyChanged("FeatureStatusText");
    }
}

bool LuaPropertyNotifier::CallFunction(const std::string& name) {
    if (!m_L) {
        utils::Logger::Error("[Lua] CallFunction: Lua state is null");
        return false;
    }
    
    utils::Logger::DebugF("[Lua] CallFunction '%s' started", name.c_str());
    
    PushViewModel();
    if (!lua_istable(m_L, -1)) {
        utils::Logger::ErrorF("[Lua] CallFunction '%s': ViewModel is not a table", name.c_str());
        lua_pop(m_L, 1);
        return false;
    }
    
    lua_getfield(m_L, -1, name.c_str());
    if (!lua_isfunction(m_L, -1)) {
        utils::Logger::ErrorF("[Lua] CallFunction '%s': not a function (type=%s)", 
            name.c_str(), lua_typename(m_L, lua_type(m_L, -1)));
        lua_pop(m_L, 2);
        return false;
    }
    
    lua_pushvalue(m_L, -2);  // Push ViewModel as self
    int status = lua_pcall(m_L, 1, 0, 0);
    
    if (status != LUA_OK) {
        const char* error = lua_tostring(m_L, -1);
        utils::Logger::ErrorF("[Lua] Command '%s' error: %s", name.c_str(), error ? error : "unknown");
        lua_pop(m_L, 1);
    } else {
        utils::Logger::DebugF("[Lua] CallFunction '%s' completed successfully", name.c_str());
    }
    
    lua_pop(m_L, 1);
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

LuaAwareMvvmLoader::LuaAwareMvvmLoader() {
    utils::Logger::Debug("[LuaAwareMvvmLoader] Constructed");
}

LuaAwareMvvmLoader::~LuaAwareMvvmLoader() {
    utils::Logger::Debug("[LuaAwareMvvmLoader] Destroyed");
}

void LuaAwareMvvmLoader::SetLuaState(lua_State* L) {
    m_L = L;
    // Create notifier early so it can be registered before Load()
    if (L && !m_viewModelName.empty()) {
        m_notifier = std::make_shared<LuaPropertyNotifier>(L, m_viewModelName);
    }
}

void LuaAwareMvvmLoader::SetViewModelName(const std::string& name) {
    m_viewModelName = name;
    // Create notifier early if Lua state is already set
    if (m_L && !m_notifier) {
        m_notifier = std::make_shared<LuaPropertyNotifier>(m_L, m_viewModelName);
    }
}

std::shared_ptr<luaui::Control> LuaAwareMvvmLoader::Load(const std::string& filePath) {
    if (!m_L) {
        utils::Logger::Error("[LuaAwareMvvmLoader] Lua state not set");
        return nullptr;
    }
    
    // Create the property notifier
    if (!m_notifier) {
        m_notifier = std::make_shared<LuaPropertyNotifier>(m_L, m_viewModelName);
    }
    
    // Create base MVVM loader
    m_baseLoader = mvvm::CreateMvvmXmlLoader();
    
    // Set DataContext to our Lua-aware notifier
    m_baseLoader->SetDataContext(m_notifier);
    
    // Auto-bind Lua commands
    AutoBindCommands();
    
    // Load XML
    auto root = m_baseLoader->Load(filePath);
    
    if (root) {
        utils::Logger::Info("[LuaAwareMvvmLoader] Layout loaded with Lua bindings");
    }
    
    return root;
}

void LuaAwareMvvmLoader::AutoBindCommands() {
    utils::Logger::Info("[LuaAwareMvvmLoader] AutoBindCommands() started");
    
    if (!m_L) {
        utils::Logger::Error("[LuaAwareMvvmLoader] AutoBindCommands: Lua state is null");
        return;
    }
    if (!m_notifier) {
        utils::Logger::Error("[LuaAwareMvvmLoader] AutoBindCommands: notifier is null");
        return;
    }
    if (!m_baseLoader) {
        utils::Logger::Error("[LuaAwareMvvmLoader] AutoBindCommands: baseLoader is null");
        return;
    }
    
    utils::Logger::InfoF("[LuaAwareMvvmLoader] Looking for ViewModel: '%s'", m_viewModelName.c_str());
    
    // 获取 ViewModel
    lua_getglobal(m_L, m_viewModelName.c_str());
    if (!lua_istable(m_L, -1)) {
        utils::Logger::ErrorF("[LuaAwareMvvmLoader] ViewModel '%s' is not a table (type=%s)", 
            m_viewModelName.c_str(), lua_typename(m_L, lua_type(m_L, -1)));
        lua_pop(m_L, 1);
        return;
    }
    
    utils::Logger::Info("[LuaAwareMvvmLoader] ViewModel table found, discovering commands...");
    
    // 策略：通过名称直接查询已知命令模式
    // 因为 ViewModel 可能使用代理模式（__index 为函数），无法遍历
    const char* knownCommands[] = {
        // Counter buttons
        "IncrementCommand",
        "DecrementCommand", 
        "ResetCommand",
        // Feature toggle
        "ToggleCommand",
        // Items
        "AddItemCommand",
        "ClearItemsCommand",
        // Generic patterns
        "ClickCommand",
        "SubmitCommand",
        "SaveCommand",
        "DeleteCommand",
        "UpdateCommand",
        nullptr  // End marker
    };
    
    int commandCount = 0;
    
    // 方法1：尝试遍历原始表（非代理）
    lua_pushnil(m_L);
    while (lua_next(m_L, -2) != 0) {
        if (lua_type(m_L, -2) == LUA_TSTRING) {
            const char* key = lua_tostring(m_L, -2);
            if (lua_isfunction(m_L, -1)) {
                utils::Logger::DebugF("[LuaAwareMvvmLoader] Direct table function: '%s'", key);
                if (strstr(key, "Command") || strstr(key, "Handler")) {
                    utils::Logger::InfoF("[LuaAwareMvvmLoader] Registering command: '%s'", key);
                    RegisterLuaCommand(key);
                    commandCount++;
                }
            }
        }
        lua_pop(m_L, 1);
    }
    
    // 方法2：通过名称查询已知的命令
    for (int i = 0; knownCommands[i] != nullptr; i++) {
        const char* cmdName = knownCommands[i];
        
        // 查询 ViewModel[cmdName]
        lua_getfield(m_L, -1, cmdName);
        bool isFunction = lua_isfunction(m_L, -1);
        lua_pop(m_L, 1);
        
        if (isFunction) {
            utils::Logger::InfoF("[LuaAwareMvvmLoader] Found command by lookup: '%s'", cmdName);
            RegisterLuaCommand(cmdName);
            commandCount++;
        }
    }
    
    // 方法3：尝试扫描元表的 __index（如果是表的话）
    if (lua_getmetatable(m_L, -1)) {
        lua_getfield(m_L, -1, "__index");
        if (lua_istable(m_L, -1)) {
            utils::Logger::Info("[LuaAwareMvvmLoader] Scanning __index table...");
            lua_pushnil(m_L);
            while (lua_next(m_L, -2) != 0) {
                if (lua_type(m_L, -2) == LUA_TSTRING) {
                    const char* key = lua_tostring(m_L, -2);
                    if (lua_isfunction(m_L, -1)) {
                        if (strstr(key, "Command") || strstr(key, "Handler")) {
                            utils::Logger::InfoF("[LuaAwareMvvmLoader] Found in __index: '%s'", key);
                            RegisterLuaCommand(key);
                            commandCount++;
                        }
                    }
                }
                lua_pop(m_L, 1);
            }
        }
        lua_pop(m_L, 2);  // Pop __index and metatable
    }
    
    lua_pop(m_L, 1);  // Pop ViewModel
    
    utils::Logger::InfoF("[LuaAwareMvvmLoader] Command binding completed: %d commands registered", commandCount);
}

void LuaAwareMvvmLoader::RegisterLuaCommand(const std::string& commandName) {
    if (!m_baseLoader) {
        utils::Logger::ErrorF("[LuaAwareMvvmLoader] Cannot register command '%s': baseLoader is null", commandName.c_str());
        return;
    }
    
    utils::Logger::DebugF("[LuaAwareMvvmLoader] Registering click handler for command: '%s'", commandName.c_str());
    
    m_baseLoader->RegisterClickHandler(commandName, [this, commandName]() {
        utils::Logger::DebugF("[LuaAwareMvvmLoader] Command '%s' triggered, calling Lua function...", commandName.c_str());
        if (m_notifier) {
            bool result = m_notifier->CallFunction(commandName);
            if (!result) {
                utils::Logger::ErrorF("[LuaAwareMvvmLoader] Failed to execute command '%s'", commandName.c_str());
            }
        } else {
            utils::Logger::ErrorF("[LuaAwareMvvmLoader] Cannot execute command '%s': notifier is null", commandName.c_str());
        }
    });
}

} // namespace lua
} // namespace luaui
