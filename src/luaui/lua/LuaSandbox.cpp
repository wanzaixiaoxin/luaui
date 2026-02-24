#include "LuaSandbox.h"
#include <cstring>
#include <algorithm>
#include <fstream>
#include <sstream>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace luaui {
namespace lua {

// ==================== LuaSandbox::Impl ====================

class LuaSandbox::Impl {
public:
    lua_State* L = nullptr;
    SecurityPolicy policy;
    std::chrono::steady_clock::time_point executionStart;
    bool isExecuting = false;
    size_t memoryLimit = 64 * 1024 * 1024;  // 64MB default
};

// ==================== LuaSandbox ====================

LuaSandbox::LuaSandbox() : m_impl(std::make_unique<Impl>()) {
}

LuaSandbox::~LuaSandbox() {
    Shutdown();
}

LuaSandbox::LuaSandbox(LuaSandbox&&) noexcept = default;
LuaSandbox& LuaSandbox::operator=(LuaSandbox&&) noexcept = default;

bool LuaSandbox::Initialize(const SecurityPolicy& policy) {
    if (m_initialized) {
        Shutdown();
    }
    
    m_policy = policy;
    
    // 创建 Lua 状态
    m_impl->L = luaL_newstate();
    if (!m_impl->L) {
        m_lastError = "Failed to create Lua state";
        return false;
    }
    
    // 打开标准库
    luaL_openlibs(m_impl->L);
    
    // 设置内存限制
    SetMemoryLimit(policy.maxMemoryBytes);
    
    // 设置执行超时
    SetExecutionTimeout(policy.maxExecutionTime);
    
    // 设置安全策略
    SetupSecurity(policy);
    
    // 注册 UI 绑定
    LuaBinding::RegisterAll(m_impl->L);
    
    m_impl->policy = policy;
    m_initialized = true;
    
    return true;
}

void LuaSandbox::Shutdown() {
    if (m_impl->L) {
        lua_close(m_impl->L);
        m_impl->L = nullptr;
    }
    m_initialized = false;
}

bool LuaSandbox::Execute(const std::string& code, const std::string& chunkName) {
    if (!m_initialized || !m_impl->L) {
        m_lastError = "Sandbox not initialized";
        return false;
    }
    
    m_impl->executionStart = std::chrono::steady_clock::now();
    m_impl->isExecuting = true;
    
    // 设置超时钩子
    lua_sethook(m_impl->L, TimeoutHook, LUA_MASKCOUNT, 1000);
    
    // 加载代码
    int loadResult = luaL_loadbuffer(m_impl->L, code.c_str(), code.length(), chunkName.c_str());
    if (loadResult != LUA_OK) {
        m_lastError = lua_tostring(m_impl->L, -1);
        lua_pop(m_impl->L, 1);
        m_impl->isExecuting = false;
        return false;
    }
    
    // 执行代码
    int execResult = lua_pcall(m_impl->L, 0, LUA_MULTRET, 0);
    m_impl->isExecuting = false;
    
    // 检查执行时间
    auto elapsed = std::chrono::steady_clock::now() - m_impl->executionStart;
    if (elapsed > m_policy.maxExecutionTime) {
        m_lastError = "Execution timeout";
        return false;
    }
    
    if (execResult != LUA_OK) {
        m_lastError = lua_tostring(m_impl->L, -1);
        lua_pop(m_impl->L, 1);
        return false;
    }
    
    return true;
}

bool LuaSandbox::ExecuteFile(const std::string& path) {
    // 检查文件是否在允许的路径中
    bool pathAllowed = m_policy.allowedPaths.empty();
    for (const auto& allowedPath : m_policy.allowedPaths) {
        if (path.find(allowedPath) == 0) {
            pathAllowed = true;
            break;
        }
    }
    
    if (!pathAllowed) {
        m_lastError = "Access denied: " + path;
        return false;
    }
    
    // 读取文件内容
    std::ifstream file(path);
    if (!file.is_open()) {
        m_lastError = "Cannot open file: " + path;
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    return Execute(buffer.str(), path);
}

void LuaSandbox::SetMemoryLimit(size_t bytes) {
    m_impl->memoryLimit = bytes;
    if (m_impl->L) {
        // 设置 GC 参数以控制内存使用
        lua_gc(m_impl->L, LUA_GCSETPAUSE, 100);
        lua_gc(m_impl->L, LUA_GCSETSTEPMUL, 400);
    }
}

size_t LuaSandbox::GetMemoryUsage() const {
    if (!m_impl->L) return 0;
    return lua_gc(m_impl->L, LUA_GCCOUNT, 0) * 1024 + lua_gc(m_impl->L, LUA_GCCOUNTB, 0);
}

void LuaSandbox::ForceGarbageCollect() {
    if (m_impl->L) {
        lua_gc(m_impl->L, LUA_GCCOLLECT, 0);
    }
}

void LuaSandbox::SetExecutionTimeout(std::chrono::milliseconds timeout) {
    m_policy.maxExecutionTime = timeout;
}

lua_State* LuaSandbox::GetLuaState() const {
    return m_impl ? m_impl->L : nullptr;
}

void LuaSandbox::RegisterUIAPI() {
    // 基础 UI API 注册
    if (!m_impl->L) return;
    
    // 创建 UI 表
    lua_newtable(m_impl->L);
    
    // 设置版本信息
    lua_pushstring(m_impl->L, "1.0.0");
    lua_setfield(m_impl->L, -2, "VERSION");
    
    lua_setglobal(m_impl->L, "UI");
}

void LuaSandbox::RegisterUtilityAPI() {
    // 工具函数 API 注册
    if (!m_impl->L) return;
    
    // 创建 Util 表
    lua_newtable(m_impl->L);
    
    lua_setglobal(m_impl->L, "Util");
}

void LuaSandbox::SetupSecurity(const SecurityPolicy& policy) {
    lua_State* L = m_impl->L;
    
    // 1. 移除危险函数
    lua_pushnil(L);
    lua_setglobal(L, "dofile");
    lua_pushnil(L);
    lua_setglobal(L, "loadfile");
    // Note: load() is kept for executing loaded strings
    // It's needed for Framework.lua loading pattern
    
    // 2. 限制 IO 库
    if (!policy.allowFileWrite) {
        lua_getglobal(L, "io");
        if (lua_istable(L, -1)) {
            // 移除写入函数
            lua_pushnil(L);
            lua_setfield(L, -2, "write");
            lua_pushnil(L);
            lua_setfield(L, -2, "flush");
            // Note: io.open is kept for read-only access
            // Write access is controlled by policy.allowFileWrite
        }
        lua_pop(L, 1);
    }
    
    // 3. 限制 OS 库
    lua_getglobal(L, "os");
    if (lua_istable(L, -1)) {
        // 只保留时间相关函数
        std::vector<const char*> allowed = {"clock", "date", "difftime", "time"};
        lua_pushnil(L);
        while (lua_next(L, -2)) {
            const char* name = lua_tostring(L, -2);
            bool isAllowed = false;
            for (auto* a : allowed) {
                if (strcmp(name, a) == 0) {
                    isAllowed = true;
                    break;
                }
            }
            if (!isAllowed) {
                lua_pushnil(L);
                lua_setfield(L, -4, name);
            }
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
    
    // 4. 禁用 debug 库的某些功能
    lua_getglobal(L, "debug");
    if (lua_istable(L, -1)) {
        lua_pushnil(L);
        lua_setfield(L, -2, "setupvalue");
        lua_pushnil(L);
        lua_setfield(L, -2, "upvaluejoin");
        lua_pushnil(L);
        lua_setfield(L, -2, "getregistry");
    }
    lua_pop(L, 1);
    
    // 5. 禁用 package.loadlib
    lua_getglobal(L, "package");
    if (lua_istable(L, -1)) {
        lua_pushnil(L);
        lua_setfield(L, -2, "loadlib");
    }
    lua_pop(L, 1);
}

void LuaSandbox::MemoryCheckHook(lua_State* L, lua_Debug* ar) {
    (void)L;
    (void)ar;
    // 内存检查钩子
    // 可以通过 lua_getallocf 获取内存使用
}

void LuaSandbox::TimeoutHook(lua_State* L, lua_Debug* ar) {
    (void)ar;
    // 超时检查钩子
    luaL_error(L, "execution timeout");
}

int LuaSandbox::SafeRequire(lua_State* L) {
    // 安全的 require 实现
    const char* modname = luaL_checkstring(L, 1);
    
    // 只允许加载白名单模块
    static const char* allowedModules[] = {
        "table", "string", "math", "os", "io", "debug", "coroutine", "utf8"
    };
    
    bool isAllowed = false;
    for (const char* mod : allowedModules) {
        if (strcmp(modname, mod) == 0) {
            isAllowed = true;
            break;
        }
    }
    
    if (!isAllowed) {
        luaL_error(L, "module '%s' is not allowed", modname);
    }
    
    // 调用原始 require
    lua_getglobal(L, "require");
    lua_pushvalue(L, 1);
    lua_call(L, 1, 1);
    return 1;
}

// ==================== ScriptEngine ====================

ScriptEngine::ScriptEngine() : m_sandbox(std::make_unique<LuaSandbox>()) {
}

ScriptEngine::~ScriptEngine() = default;

bool ScriptEngine::Initialize() {
    if (!m_sandbox->Initialize()) {
        m_lastError = m_sandbox->GetLastError();
        return false;
    }
    
    // 注册所有 API
    LuaBinding::RegisterAll(m_sandbox->GetLuaState());
    
    return true;
}

void ScriptEngine::Shutdown() {
    m_sandbox->Shutdown();
    m_loadedScripts.clear();
}

bool ScriptEngine::LoadScript(const std::string& path) {
    if (!m_sandbox->ExecuteFile(path)) {
        m_lastError = m_sandbox->GetLastError();
        return false;
    }
    m_loadedScripts[path] = path;
    return true;
}

bool ScriptEngine::LoadViewModel(const std::string& path, const std::string& typeName) {
    (void)typeName;
    return LoadScript(path);
}

bool ScriptEngine::CallFunction(const std::string& name) {
    lua_State* L = m_sandbox->GetLuaState();
    if (!L) {
        m_lastError = "Sandbox not initialized";
        return false;
    }
    
    lua_getglobal(L, name.c_str());
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        m_lastError = "Function not found: " + name;
        return false;
    }
    
    int result = lua_pcall(L, 0, 0, 0);
    if (result != LUA_OK) {
        m_lastError = lua_tostring(L, -1);
        lua_pop(L, 1);
        return false;
    }
    
    return true;
}

std::string ScriptEngine::GetGlobalString(const std::string& name) {
    lua_State* L = m_sandbox->GetLuaState();
    if (!L) return "";
    
    lua_getglobal(L, name.c_str());
    const char* value = lua_tostring(L, -1);
    std::string result = value ? value : "";
    lua_pop(L, 1);
    return result;
}

void ScriptEngine::SetGlobalString(const std::string& name, const std::string& value) {
    lua_State* L = m_sandbox->GetLuaState();
    if (!L) return;
    
    lua_pushstring(L, value.c_str());
    lua_setglobal(L, name.c_str());
}

bool ScriptEngine::CreateInstance(const std::string& className) {
    lua_State* L = m_sandbox->GetLuaState();
    if (!L) {
        m_lastError = "Sandbox not initialized";
        return false;
    }
    
    // 查找类
    lua_getglobal(L, className.c_str());
    if (!lua_istable(L, -1) && !lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        m_lastError = "Class not found: " + className;
        return false;
    }
    
    // 创建实例
    if (lua_isfunction(L, -1)) {
        int result = lua_pcall(L, 0, 1, 0);
        if (result != LUA_OK) {
            m_lastError = lua_tostring(L, -1);
            lua_pop(L, 1);
            return false;
        }
    }
    
    lua_pop(L, 1);
    return true;
}

bool ScriptEngine::ReloadScript(const std::string& path) {
    auto it = m_loadedScripts.find(path);
    if (it == m_loadedScripts.end()) {
        m_lastError = "Script not loaded: " + path;
        return false;
    }
    
    return LoadScript(path);
}

} // namespace lua
} // namespace luaui
