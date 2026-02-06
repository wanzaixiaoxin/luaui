/**
 * @file ScriptEngine.h
 * @brief 脚本引擎核心
 * @details 提供脚本加载、执行和生命周期管理功能
 * @version 1.0.0
 */

#ifndef LUAUI_SCRIPTENGINE_H
#define LUAUI_SCRIPTENGINE_H

#include <string>
#include <vector>
#include <map>
#include "LuaState.h"
#include "ILuaUI.h"
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

namespace LuaUI {
namespace Core {

/**
 * @brief 脚本生命周期回调函数类型
 */
typedef void (*ScriptLifecycleCallback)(const std::string& scriptName);

/**
 * @brief 脚本引擎类
 * @details 殡理Lua脚本的加载、执行和生命周期
 */
class ScriptEngine : public IScriptEngine {
public:
    /**
     * @brief 构造函数
     */
    ScriptEngine();
    
    /**
     * @brief 析构函数
     */
    ~ScriptEngine();
    
    /**
     * @brief 初始化脚本引擎
     * @return 成功返回true，失败返回false
     */
    bool initialize();
    
    /**
     * @brief 关闭脚本引擎
     */
    void shutdown();
    
    /**
     * @brief 检查是否已初始化
     * @return 已初始化返回true，否则返回false
     */
    bool isInitialized() const;
    
    /**
     * @brief 获取Lua状态管理器
     * @return LuaState指针
     */
    LuaState* getLuaState();
    
    /**
     * @brief 加载脚本文件
     * @param scriptFile Lua文件路径
     * @return 成功返回true，失败返回false
     */
    bool loadScript(const std::string& scriptFile);
    
    /**
     * @brief 加载脚本字符串
     * @param scriptContent 脚本内容
     * @return 成功返回true，失败返回false
     */
    bool loadScriptString(const std::string& scriptContent);
    
    /**
     * @brief 执行脚本文件
     * @param scriptFile Lua文件路径
     * @return 成功返回true，失败返回false
     */
    bool executeScript(const std::string& scriptFile);
    
    /**
     * @brief 执行脚本字符串
     * @param scriptContent 脚本内容
     * @return 成功返回true，失败返回false
     */
    bool executeString(const std::string& scriptContent);
    
    /**
     * @brief 调用Lua函数
     * @param funcName 函数名称
     * @return 成功返回true，失败返回false
     */
    bool callFunction(const std::string& funcName);
    
    /**
     * @brief 调用带参数的Lua函数
     * @param funcName 函数名称
     * @param args 参数列表
     * @return 成功返回true，失败返回false
     */
    bool callFunctionWithArgs(const std::string& funcName, const std::vector<std::string>& args);

    /**
     * @brief 获取最后一次错误信息
     * @return 错误信息字符串
     */
    std::string getLastError() const;
    
    /**
     * @brief 注册C++函数到Lua全局命名空间
     * @param name 函数名称
     * @param func C++函数指针
     */
    void registerFunction(const std::string& name, lua_CFunction func);
    
    /**
     * @brief 注册C++函数到Lua模块
     * @param tableName 模块名
     * @param funcName 函数名称
     * @param func C++函数指针
     */
    void registerFunctionToTable(const std::string& tableName, const std::string& funcName, lua_CFunction func);
    
    /**
     * @brief 设置脚本路径搜索路径
     * @param path 搜索路径（Lua的package.path格式）
     */
    void setPackagePath(const std::string& path);
    
    /**
     * @brief 添加脚本路径搜索路径
     * @param path 要添加的路径
     */
    void addPackagePath(const std::string& path);
    
    /**
     * @brief 设置脚本加载回调
     * @param callback 加载回调函数
     */
    void setLoadCallback(ScriptLifecycleCallback callback);
    
    /**
     * @brief 设置脚本卸载回调
     * @param callback 卸载回调函数
     */
    void setUnloadCallback(ScriptLifecycleCallback callback);
    
    /**
     * @brief 获取已加载的脚本列表
     * @return 脚本文件名列表
     */
    std::vector<std::string> getLoadedScripts() const;
    
    /**
     * @brief 检查脚本是否已加载
     * @param scriptName 脚本名称
     * @return 已加载返回true，否则返回false
     */
    bool isScriptLoaded(const std::string& scriptName) const;
    
    /**
     * @brief 卸载脚本
     * @param scriptName 脚本名称
     * @return 成功返回true，失败返回false
     */
    bool unloadScript(const std::string& scriptName);
    
    /**
     * @brief 卸载所有脚本
     */
    void unloadAllScripts();
    
    /**
     * @brief 重新加载脚本
     * @param scriptName 脚本名称
     * @return 成功返回true，失败返回false
     */
    bool reloadScript(const std::string& scriptName);

private:
    LuaState m_luaState;                          ///< Lua状态管理器
    bool m_initialized;                           ///< 是否已初始化标志
    std::map<std::string, bool> m_loadedScripts;  ///< 已加载脚本列表
    ScriptLifecycleCallback m_loadCallback;        ///< 加载回调函数
    ScriptLifecycleCallback m_unloadCallback;       ///< 卸载回调函数
    
    /**
     * @brief 触发加载回调
     * @param scriptName 脚本名称
     */
    void triggerLoadCallback(const std::string& scriptName);
    
    /**
     * @brief 触发卸载回调
     * @param scriptName 脚本名称
     */
    void triggerUnloadCallback(const std::string& scriptName);
};

} // namespace Core
} // namespace LuaUI

#endif // LUAUI_SCRIPTENGINE_H
