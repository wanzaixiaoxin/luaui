#include "LuaAwareMvvmLoader.h"
#include "Logger.h"
#include "mvvm/MvvmXmlLoader.h"
#include "../utils/StringUtils.h"
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
    utils::Logger::InfoF("[LuaPropertyNotifier] NotifyPropertyChanged: '%s' (handlers=%zu)", 
        propertyName.c_str(), m_handlers.size());
    
    mvvm::PropertyChangedEventArgs args;
    args.propertyName = propertyName;
    
    for (const auto& handler : m_handlers) {
        handler(args);
    }
}

// ============================================================================
// 辅助函数：从当前栈顶表获取属性值（支持嵌套路径）
// 调用前：栈顶是一个表
// 返回后：栈顶是最终值，返回 true 表示成功
// ============================================================================
static bool GetNestedProperty(lua_State* L, const std::vector<std::string>& pathParts, int startIndex = 0) {
    if (pathParts.empty()) return false;
    
    // 获取当前栈顶的表
    if (!lua_istable(L, -1) && startIndex > 0) {
        return false;  // 中间路径不是表，无法继续
    }
    
    for (size_t i = startIndex; i < pathParts.size(); ++i) {
        const std::string& part = pathParts[i];
        
        // 处理数组索引，如 Users[0] 或 Users.0
        size_t bracketPos = part.find('[');
        std::string key = part;
        int arrayIndex = -1;
        
        if (bracketPos != std::string::npos) {
            key = part.substr(0, bracketPos);
            size_t endBracket = part.find(']', bracketPos);
            if (endBracket != std::string::npos) {
                try {
                    arrayIndex = std::stoi(part.substr(bracketPos + 1, endBracket - bracketPos - 1));
                } catch (...) {
                    // 解析失败，当作普通key处理
                }
            }
        }
        
        // 获取字段值
        if (!key.empty()) {
            lua_getfield(L, -1, key.c_str());
        } else {
            // 纯数组索引
            lua_pushnil(L);
        }
        
        // 处理数组索引
        if (arrayIndex >= 0 && lua_istable(L, -1)) {
            lua_rawgeti(L, -1, arrayIndex + 1);  // Lua数组是1-based
            lua_remove(L, -2);  // 移除表，保留值
        }
        
        // 如果不是最后一个，需要确保是表
        if (i < pathParts.size() - 1) {
            if (!lua_istable(L, -1)) {
                // 不是表，返回失败
                lua_pop(L, 1);  // 移除值
                return false;
            }
            // 继续循环，当前栈顶已经是下一个表
        } else {
            // 最后一个路径部分，返回值
            return true;
        }
    }
    
    return true;
}

std::any LuaPropertyNotifier::GetPropertyValue(const std::string& name) const {
    if (!m_L) return {};
    
    // 分割路径
    auto pathParts = luaui::utils::StringUtils::SplitPath(name);
    if (pathParts.empty()) return {};
    
    PushViewModel();
    if (!lua_istable(m_L, -1)) {
        utils::Logger::WarningF("[Lua] GetPropertyValue '%s': ViewModel is not a table (type=%s)", 
            name.c_str(), lua_typename(m_L, lua_type(m_L, -1)));
        lua_pop(m_L, 1);
        return {};
    }
    
    std::any result;
    
    // 首先尝试直接获取（处理非代理表或原始表）
    if (pathParts.size() == 1) {
        // 单级属性，直接获取
        lua_getfield(m_L, -1, pathParts[0].c_str());
    } else {
        // 嵌套属性，使用递归获取
        if (!GetNestedProperty(m_L, pathParts, 0)) {
            lua_pop(m_L, 1);  // Pop ViewModel
            return result;
        }
    }
    
    // 检查是否找到值
    if (!lua_isnil(m_L, -1)) {
        // 直接找到了 - 优先检查数字类型（因为lua_isstring对数字也返回true）
        if (lua_isnumber(m_L, -1)) {
            result = lua_tonumber(m_L, -1);
            utils::Logger::DebugF("[Lua] GetPropertyValue '%s' = number %.1f", name.c_str(), std::any_cast<double>(result));
        } else if (lua_isstring(m_L, -1)) {
            result = std::string(lua_tostring(m_L, -1));
            utils::Logger::DebugF("[Lua] GetPropertyValue '%s' = string '%s'", name.c_str(), std::any_cast<std::string>(result).c_str());
        } else if (lua_isboolean(m_L, -1)) {
            result = static_cast<bool>(lua_toboolean(m_L, -1));
        }
        lua_pop(m_L, 2);  // Pop value + ViewModel
        return result;
    }
    lua_pop(m_L, 1);  // Pop nil
    
    // 尝试调用 __index 元方法（处理代理表）
    lua_getmetatable(m_L, -1);
    if (!lua_istable(m_L, -1)) {
        lua_pop(m_L, 1);  // Pop metatable (nil)
        lua_pop(m_L, 1);  // Pop ViewModel
        return result;
    }
    
    lua_getfield(m_L, -1, "__index");
    int indexType = lua_type(m_L, -1);
    
    if (indexType == LUA_TFUNCTION) {
        // __index 是函数，调用它: __index(viewModel, name)
        lua_pushvalue(m_L, -3);  // Push ViewModel (第一个参数 self)
        lua_pushstring(m_L, pathParts[0].c_str());  // Push 属性名 (第二个参数 key)
        
        utils::Logger::DebugF("[Lua] GetPropertyValue '%s': calling __index function", name.c_str());
        int status = lua_pcall(m_L, 2, 1, 0);  // 安全调用
        
        if (status == LUA_OK) {
            // 对于嵌套路径，需要继续获取
            if (pathParts.size() > 1 && lua_istable(m_L, -1)) {
                lua_remove(m_L, -2);  // 移除 metatable，栈: [ViewModel, value]
                lua_remove(m_L, -2);  // 移除 ViewModel，栈: [value]
                
                if (GetNestedProperty(m_L, pathParts, 1)) {
                    // 成功获取嵌套值
                    if (lua_isstring(m_L, -1)) {
                        result = std::string(lua_tostring(m_L, -1));
                    } else if (lua_isnumber(m_L, -1)) {
                        result = lua_tonumber(m_L, -1);
                    } else if (lua_isboolean(m_L, -1)) {
                        result = static_cast<bool>(lua_toboolean(m_L, -1));
                    }
                    lua_pop(m_L, 1);  // Pop value
                } else {
                    lua_pop(m_L, 1);  // Pop value (table or nil)
                }
                return result;
            }
            
            if (lua_isnumber(m_L, -1)) {
                result = lua_tonumber(m_L, -1);
            } else if (lua_isstring(m_L, -1)) {
                result = std::string(lua_tostring(m_L, -1));
            } else if (lua_isboolean(m_L, -1)) {
                result = static_cast<bool>(lua_toboolean(m_L, -1));
            }
            lua_pop(m_L, 1);  // Pop 返回值, 栈: [ViewModel, metatable]
        } else {
            const char* error = lua_tostring(m_L, -1);
            utils::Logger::ErrorF("[Lua] GetPropertyValue '%s': __index error: %s", 
                name.c_str(), error ? error : "unknown");
            lua_pop(m_L, 1);  // Pop 错误, 栈: [ViewModel, metatable]
        }
        lua_pop(m_L, 1);  // Pop metatable, 栈: [ViewModel]
    } else if (indexType == LUA_TTABLE) {
        // __index 是表，直接查找
        lua_getfield(m_L, -1, pathParts[0].c_str());
        
        // 对于嵌套路径，继续获取
        if (pathParts.size() > 1 && lua_istable(m_L, -1)) {
            lua_remove(m_L, -2);  // 移除 __index table
            lua_remove(m_L, -2);  // 移除 metatable
            lua_remove(m_L, -2);  // 移除 ViewModel
            
            if (GetNestedProperty(m_L, pathParts, 1)) {
                if (lua_isstring(m_L, -1)) {
                    result = std::string(lua_tostring(m_L, -1));
                } else if (lua_isnumber(m_L, -1)) {
                    result = lua_tonumber(m_L, -1);
                } else if (lua_isboolean(m_L, -1)) {
                    result = static_cast<bool>(lua_toboolean(m_L, -1));
                }
                lua_pop(m_L, 1);  // Pop value
            } else {
                lua_pop(m_L, 1);  // Pop value (table or nil)
            }
            return result;
        }
        
        if (lua_isstring(m_L, -1)) {
            result = std::string(lua_tostring(m_L, -1));
        } else if (lua_isnumber(m_L, -1)) {
            result = lua_tonumber(m_L, -1);
        } else if (lua_isboolean(m_L, -1)) {
            result = static_cast<bool>(lua_toboolean(m_L, -1));
        }
        lua_pop(m_L, 1);  // Pop value, 栈: [ViewModel, metatable, __index]
        lua_pop(m_L, 1);  // Pop __index table, 栈: [ViewModel, metatable]
        lua_pop(m_L, 1);  // Pop metatable, 栈: [ViewModel]
    } else {
        // __index 不是函数也不是表
        utils::Logger::DebugF("[Lua] GetPropertyValue '%s': __index is not function or table (type=%s)", 
            name.c_str(), lua_typename(m_L, indexType));
        lua_pop(m_L, 1);  // Pop __index, 栈: [ViewModel, metatable]
        lua_pop(m_L, 1);  // Pop metatable, 栈: [ViewModel]
    }
    
    lua_pop(m_L, 1);  // Pop ViewModel
    
    if (!result.has_value()) {
        utils::Logger::DebugF("[Lua] GetPropertyValue '%s': value not found", name.c_str());
    }
    
    return result;
}

// ============================================================================
// 辅助函数：设置嵌套属性值
// ============================================================================
static bool SetNestedProperty(lua_State* L, const std::vector<std::string>& pathParts, int valueIndex) {
    if (pathParts.empty()) return false;
    
    // 遍历到倒数第二个路径部分，确保都是表
    for (size_t i = 0; i < pathParts.size() - 1; ++i) {
        const std::string& part = pathParts[i];
        
        // 处理数组索引
        size_t bracketPos = part.find('[');
        std::string key = part;
        int arrayIndex = -1;
        
        if (bracketPos != std::string::npos) {
            key = part.substr(0, bracketPos);
            size_t endBracket = part.find(']', bracketPos);
            if (endBracket != std::string::npos) {
                try {
                    arrayIndex = std::stoi(part.substr(bracketPos + 1, endBracket - bracketPos - 1));
                } catch (...) {}
            }
        }
        
        // 获取字段
        if (!key.empty()) {
            lua_getfield(L, -1, key.c_str());
        } else {
            lua_pushnil(L);
        }
        
        if (arrayIndex >= 0 && lua_istable(L, -1)) {
            lua_rawgeti(L, -1, arrayIndex + 1);
            lua_remove(L, -2);  // 移除外层表
        }
        
        if (!lua_istable(L, -1)) {
            // 路径不存在，无法设置
            lua_pop(L, 1);  // Pop nil/error value
            return false;
        }
        // 栈顶现在是下一个表，继续循环
    }
    
    // 设置最后一个路径部分
    const std::string& lastKey = pathParts.back();
    
    // 处理数组索引
    size_t bracketPos = lastKey.find('[');
    std::string key = lastKey;
    int arrayIndex = -1;
    
    if (bracketPos != std::string::npos) {
        key = lastKey.substr(0, bracketPos);
        size_t endBracket = lastKey.find(']', bracketPos);
        if (endBracket != std::string::npos) {
            try {
                arrayIndex = std::stoi(lastKey.substr(bracketPos + 1, endBracket - bracketPos - 1));
            } catch (...) {}
        }
    }
    
    // 复制值到栈顶
    lua_pushvalue(L, valueIndex);
    
    if (arrayIndex >= 0 && !key.empty()) {
        // 先获取数组表
        lua_getfield(L, -2, key.c_str());
        if (lua_istable(L, -1)) {
            lua_pushvalue(L, -2);  // 复制值
            lua_rawseti(L, -2, arrayIndex + 1);  // 设置数组元素
        }
        lua_pop(L, 1);  // Pop 数组表
        lua_pop(L, 1);  // Pop 值
    } else if (!key.empty()) {
        lua_setfield(L, -2, key.c_str());
    }
    
    return true;
}

void LuaPropertyNotifier::SetPropertyValue(const std::string& name, const std::any& value) {
    if (!m_L) return;
    
    // 分割路径
    auto pathParts = luaui::utils::StringUtils::SplitPath(name);
    if (pathParts.empty()) return;
    
    PushViewModel();
    if (!lua_istable(m_L, -1)) {
        utils::Logger::ErrorF("[Lua] SetPropertyValue '%s': ViewModel is not a table", name.c_str());
        lua_pop(m_L, 1);
        return;
    }
    
    try {
        // 压入值
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
        
        utils::Logger::DebugF("[Lua] Setting property '%s' in ViewModel", name.c_str());
        
        if (pathParts.size() == 1) {
            // 单级属性
            lua_setfield(m_L, -2, pathParts[0].c_str());
        } else {
            // 嵌套属性
            if (SetNestedProperty(m_L, pathParts, -1)) {
                lua_pop(m_L, 1);  // Pop value
                utils::Logger::DebugF("[Lua] Property '%s' set successfully", name.c_str());
            } else {
                lua_pop(m_L, 1);  // Pop value
                lua_pop(m_L, 1);  // Pop ViewModel
                return;
            }
        }
        
        utils::Logger::DebugF("[Lua] Property '%s' set successfully", name.c_str());
    } catch (...) {
        // Ignore conversion errors
    }
    
    lua_pop(m_L, 1);
    NotifyPropertyChanged(name);
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
    // Don't create notifier here, wait until SetViewModelName is called
    // or Load() is called with the correct viewModelName
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
    
    //utils::Logger::InfoF("[LuaAwareMvvmLoader] Looking for ViewModel: '%s'", m_viewModelName.c_str());
    
    // 获取 ViewModel
    lua_getglobal(m_L, m_viewModelName.c_str());
    if (!lua_istable(m_L, -1)) {
        utils::Logger::ErrorF("[LuaAwareMvvmLoader] ViewModel '%s' is not a table (type=%s)", 
            m_viewModelName.c_str(), lua_typename(m_L, lua_type(m_L, -1)));
        lua_pop(m_L, 1);
        return;
    }
    
    //utils::Logger::Info("[LuaAwareMvvmLoader] ViewModel table found, discovering commands...");
    
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
        "RemoveItemCommand",
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
                //utils::Logger::DebugF("[LuaAwareMvvmLoader] Direct table function: '%s'", key);
                if (strstr(key, "Command") || strstr(key, "Handler")) {
                    //utils::Logger::InfoF("[LuaAwareMvvmLoader] Registering command: '%s'", key);
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
            //utils::Logger::InfoF("[LuaAwareMvvmLoader] Found command by lookup: '%s'", cmdName);
            RegisterLuaCommand(cmdName);
            commandCount++;
        }
    }
    
    // 方法3：尝试扫描元表的 __index（如果是表的话）
    if (lua_getmetatable(m_L, -1)) {
        lua_getfield(m_L, -1, "__index");
        if (lua_istable(m_L, -1)) {
            //utils::Logger::Info("[LuaAwareMvvmLoader] Scanning __index table...");
            lua_pushnil(m_L);
            while (lua_next(m_L, -2) != 0) {
                if (lua_type(m_L, -2) == LUA_TSTRING) {
                    const char* key = lua_tostring(m_L, -2);
                    if (lua_isfunction(m_L, -1)) {
                        if (strstr(key, "Command") || strstr(key, "Handler")) {
                            //utils::Logger::InfoF("[LuaAwareMvvmLoader] Found in __index: '%s'", key);
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
    
    //utils::Logger::InfoF("[LuaAwareMvvmLoader] Command binding completed: %d commands registered", commandCount);
}

void LuaAwareMvvmLoader::RegisterLuaCommand(const std::string& commandName) {
    if (!m_baseLoader) {
        utils::Logger::ErrorF("[LuaAwareMvvmLoader] Cannot register command '%s': baseLoader is null", commandName.c_str());
        return;
    }
    
    //utils::Logger::DebugF("[LuaAwareMvvmLoader] Registering click handler for command: '%s'", commandName.c_str());
    
    m_baseLoader->RegisterClickHandler(commandName, [this, commandName]() {
        //utils::Logger::DebugF("[LuaAwareMvvmLoader] Command '%s' triggered, calling Lua function...", commandName.c_str());
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
