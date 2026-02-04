/**
 * @file ScriptLoader.h
 * @brief 脚本加载器
 * @details 负责加载Lua脚本文件
 * @version 1.0.0
 */

#ifndef LUAUI_SCRIPTLOADER_H
#define LUAUI_SCRIPTLOADER_H

#include <string>
#include <vector>
#include <lua.hpp>
#include "core/ScriptEngine.h"

namespace LuaUI {
namespace Lua {
namespace Lifecycle {

/**
 * @brief 脚本信息
 */
struct ScriptInfo {
    std::string name;       ///< 脚本名称
    std::string path;       ///< 脚本路径
    std::string content;    ///< 脚本内容
    bool loaded;           ///< 是否已加载
    int refCount;          ///< 引用计数
    
    /**
     * @brief 构造函数
     */
    ScriptInfo()
        : loaded(false)
        , refCount(0)
    {
    }
};

/**
 * @brief 脚本加载器类
 */
class ScriptLoader {
public:
    /**
     * @brief 构造函数
     * @param engine 脚本引擎
     */
    ScriptLoader(Core::ScriptEngine* engine);
    
    /**
     * @brief 析构函数
     */
    ~ScriptLoader();
    
    /**
     * @brief 加载脚本文件
     * @param filepath 文件路径
     * @param scriptName 脚本名称（可选）
     * @return 成功返回true，失败返回false
     */
    bool loadFile(const std::string& filepath, const std::string& scriptName = "");
    
    /**
     * @brief 加载脚本字符串
     * @param scriptContent 脚本内容
     * @param scriptName 脚本名称
     * @return 成功返回true，失败返回false
     */
    bool loadString(const std::string& scriptContent, const std::string& scriptName);
    
    /**
     * @brief 重新加载脚本
     * @param scriptName 脚本名称
     * @return 成功返回true，失败返回false
     */
    bool reload(const std::string& scriptName);
    
    /**
     * @brief 卸载脚本
     * @param scriptName 脚本名称
     * @return 成功返回true，失败返回false
     */
    bool unload(const std::string& scriptName);
    
    /**
     * @brief 卸载所有脚本
     */
    void unloadAll();
    
    /**
     * @brief 检查脚本是否已加载
     * @param scriptName 脚本名称
     * @return 已加载返回true，否则返回false
     */
    bool isLoaded(const std::string& scriptName) const;
    
    /**
     * @brief 获取脚本信息
     * @param scriptName 脚本名称
     * @return 脚本信息指针，如果不存在返回nullptr
     */
    const ScriptInfo* getScriptInfo(const std::string& scriptName) const;
    
    /**
     * @brief 获取所有已加载的脚本列表
     * @return 脚本名称列表
     */
    std::vector<std::string> getLoadedScripts() const;
    
    /**
     * @brief 设置搜索路径
     * @param paths 搜索路径列表
     */
    void setSearchPaths(const std::vector<std::string>& paths);
    
    /**
     * @brief 添加搜索路径
     * @param path 路径
     */
    void addSearchPath(const std::string& path);
    
    /**
     * @brief 查找脚本文件
     * @param scriptName 脚本名称
     * @return 脚本文件路径，如果未找到返回空字符串
     */
    std::string findScript(const std::string& scriptName) const;
    
    /**
     * @brief 获取最后一次错误信息
     * @return 错误信息
     */
    std::string getLastError() const;

private:
    Core::ScriptEngine* m_engine;            ///< 脚本引擎
    std::map<std::string, ScriptInfo> m_scripts; ///< 脚本映射表
    std::vector<std::string> m_searchPaths;  ///< 搜索路径列表
    std::string m_lastError;                   ///< 最后一次错误信息
    
    /**
     * @brief 读取文件内容
     * @param filepath 文件路径
     * @return 文件内容，失败返回空字符串
     */
    std::string readFile(const std::string& filepath);
    
    /**
     * @brief 设置错误信息
     * @param error 错误信息
     */
    void setLastError(const std::string& error);
};

} // namespace Lifecycle
} // namespace Lua
} // namespace LuaUI

#endif // LUAUI_SCRIPTLOADER_H
