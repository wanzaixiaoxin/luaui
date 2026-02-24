#pragma once

#include "lua_wrapper.h"
#include <functional>
#include <memory>
#include <chrono>
#include <string>
#include <vector>

namespace luaui {
namespace lua {

// ==================== Lua沙箱 ====================

class LuaSandbox {
public:
    struct SecurityPolicy {
        size_t maxMemoryBytes = 64 * 1024 * 1024;  // 64MB
        std::chrono::milliseconds maxExecutionTime = std::chrono::seconds(30);
        bool allowFileWrite = false;
        bool allowNetwork = false;
        bool allowLoadLibrary = false;
        std::vector<std::string> allowedPaths;  // 允许访问的目录
    };
    
    LuaSandbox();
    ~LuaSandbox();
    
    // 禁止拷贝，允许移动
    LuaSandbox(const LuaSandbox&) = delete;
    LuaSandbox& operator=(const LuaSandbox&) = delete;
    LuaSandbox(LuaSandbox&&) noexcept;
    LuaSandbox& operator=(LuaSandbox&&) noexcept;
    
    // 初始化
    bool Initialize(const SecurityPolicy& policy = SecurityPolicy());
    void Shutdown();
    
    // 安全执行
    bool Execute(const std::string& code, const std::string& chunkName = "[string]");
    bool ExecuteFile(const std::string& path);
    
    // 内存管理
    void SetMemoryLimit(size_t bytes);
    size_t GetMemoryUsage() const;
    void ForceGarbageCollect();
    
    // 执行时间限制
    void SetExecutionTimeout(std::chrono::milliseconds timeout);
    
    // 获取底层状态
    lua_State* GetLuaState() const;
    
    // 注册API到Lua
    void RegisterUIAPI();
    void RegisterUtilityAPI();
    
    // 检查是否已初始化
    bool IsInitialized() const { return m_initialized; }
    
    // 获取最后错误
    const std::string& GetLastError() const { return m_lastError; }
    
private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
    
    bool m_initialized = false;
    std::string m_lastError;
    SecurityPolicy m_policy;
    
    // 安全检查钩子
    static void MemoryCheckHook(lua_State* L, lua_Debug* ar);
    static void TimeoutHook(lua_State* L, lua_Debug* ar);
    
    // 安全设置
    void SetupSecurity(const SecurityPolicy& policy);
    static int SafeRequire(lua_State* L);
};

// ==================== 脚本引擎 ====================

class ScriptEngine {
public:
    ScriptEngine();
    ~ScriptEngine();
    
    bool Initialize();
    void Shutdown();
    
    // 加载脚本
    bool LoadScript(const std::string& path);
    bool LoadViewModel(const std::string& path, const std::string& typeName);
    
    // 执行函数
    bool CallFunction(const std::string& name);
    
    // 获取/设置全局变量
    std::string GetGlobalString(const std::string& name);
    void SetGlobalString(const std::string& name, const std::string& value);
    
    // 创建实例
    bool CreateInstance(const std::string& className);
    
    // 热重载
    bool ReloadScript(const std::string& path);
    
    // 获取沙箱
    LuaSandbox* GetSandbox() const { return m_sandbox.get(); }
    
private:
    std::unique_ptr<LuaSandbox> m_sandbox;
    std::unordered_map<std::string, std::string> m_loadedScripts;
    std::string m_lastError;
};

// ==================== Lua绑定器 ====================

class LuaBinding {
public:
    // 注册所有绑定
    static void RegisterAll(lua_State* L);
    
    // UI元素绑定
    static void RegisterUIElements(lua_State* L);
    static void RegisterWindow(lua_State* L);
    static void RegisterButton(lua_State* L);
    static void RegisterTextBlock(lua_State* L);
    static void RegisterTextBox(lua_State* L);
    static void RegisterPanel(lua_State* L);
    static void RegisterGrid(lua_State* L);
    static void RegisterStackPanel(lua_State* L);
    static void RegisterCheckBox(lua_State* L);
    static void RegisterSlider(lua_State* L);
    static void RegisterProgressBar(lua_State* L);
    static void RegisterImage(lua_State* L);
    static void RegisterListBox(lua_State* L);
    static void RegisterComboBox(lua_State* L);
    static void RegisterTabControl(lua_State* L);
    static void RegisterTreeView(lua_State* L);
    static void RegisterDataGrid(lua_State* L);
    static void RegisterMenu(lua_State* L);
    static void RegisterToolBar(lua_State* L);
    static void RegisterStatusBar(lua_State* L);
    static void RegisterDialog(lua_State* L);
    static void RegisterShapes(lua_State* L);
    
    // 属性与事件绑定
    static void RegisterProperties(lua_State* L);
    static void RegisterEvents(lua_State* L);
    static void RegisterCommands(lua_State* L);
    static void RegisterBindings(lua_State* L);
    
    // 动画绑定
    static void RegisterAnimations(lua_State* L);
    
    // 资源绑定
    static void RegisterResources(lua_State* L);
    
    // 对话框绑定
    static void RegisterDialogs(lua_State* L);
    
    // 工具绑定
    static void RegisterLogger(lua_State* L);
    static void RegisterStorage(lua_State* L);
    static void RegisterTask(lua_State* L);
    
    // MVVM 绑定
    static void RegisterMVVM(lua_State* L);
    
    // 全局UI对象
    static void RegisterUIGlobal(lua_State* L);
    
    // 暴露Window给Lua (用于设置根控件) - window passed as void* to avoid header dependency
    static void ExposeWindow(lua_State* L, void* window);
};

} // namespace lua
} // namespace luaui
