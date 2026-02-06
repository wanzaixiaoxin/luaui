/**
 * @file LifecycleManager.h
 * @brief 生命周期管理器
 * @details 管理Lua脚本的完整生命周期
 * @version 1.0.0
 */

#ifndef LUAUI_LIFECYCLEMANAGER_H
#define LUAUI_LIFECYCLEMANAGER_H

#include <string>
#include <vector>
#include <map>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
#include "core/ScriptEngine.h"
#include "lua/lifecycle/ScriptLoader.h"
#include "lua/lifecycle/ScriptRunner.h"

namespace LuaUI {
namespace Lua {
namespace Lifecycle {

/**
 * @brief 生命周期阶段
 */
enum LifecycleStage {
    STAGE_INIT,           ///< 初始化阶段
    STAGE_LOAD,           ///< 加载阶段
    STAGE_READY,          ///< 就绪阶段
    STAGE_RUNNING,        ///< 运行阶段
    STAGE_PAUSED,         ///< 暂停阶段
    STAGE_CLOSING,        ///< 关闭阶段
    STAGE_CLOSED          ///< 已关闭
};

/**
 * @brief 生命周期事件回调
 */
typedef void (*LifecycleCallback)(LifecycleStage stage, const std::string& scriptName);

/**
 * @brief 生命周期管理器类
 */
class LifecycleManager {
public:
    /**
     * @brief 构造函数
     * @param engine 脚本引擎
     */
    LifecycleManager(Core::ScriptEngine* engine);
    
    /**
     * @brief 析构函数
     */
    ~LifecycleManager();
    
    /**
     * @brief 初始化生命周期管理器
     * @return 成功返回true，失败返回false
     */
    bool initialize();
    
    /**
     * @brief 关闭生命周期管理器
     */
    void shutdown();
    
    /**
     * @brief 加载并初始化脚本
     * @param scriptName 脚本名称
     * @return 成功返回true，失败返回false
     */
    bool loadScript(const std::string& scriptName);
    
    /**
     * @brief 启动脚本
     * @param scriptName 脚本名称
     * @return 成功返回true，失败返回false
     */
    bool startScript(const std::string& scriptName);
    
    /**
     * @brief 暂停脚本
     * @param scriptName 脚本名称
     * @return 成功返回true，失败返回false
     */
    bool pauseScript(const std::string& scriptName);
    
    /**
     * @brief 恢复脚本
     * @param scriptName 脚本名称
     * @return 成功返回true，失败返回false
     */
    bool resumeScript(const std::string& scriptName);
    
    /**
     * @brief 停止脚本
     * @param scriptName 脚本名称
     * @return 成功返回true，失败返回false
     */
    bool stopScript(const std::string& scriptName);
    
    /**
     * @brief 卸载脚本
     * @param scriptName 脚本名称
     * @return 成功返回true，失败返回false
     */
    bool unloadScript(const std::string& scriptName);
    
    /**
     * @brief 获取脚本的生命周期阶段
     * @param scriptName 脚本名称
     * @return 生命周期阶段
     */
    LifecycleStage getScriptStage(const std::string& scriptName) const;
    
    /**
     * @brief 获取所有脚本的生命周期信息
     * @return 脚本名称到阶段的映射表
     */
    std::map<std::string, LifecycleStage> getAllScriptStages() const;
    
    /**
     * @brief 设置生命周期回调
     * @param callback 回调函数
     */
    void setLifecycleCallback(LifecycleCallback callback);
    
    /**
     * @brief 触发生命周期事件
     * @param stage 生命周期阶段
     * @param scriptName 脚本名称
     */
    void triggerLifecycleEvent(LifecycleStage stage, const std::string& scriptName);
    
    /**
     * @brief 获取脚本加载器
     * @return 脚本加载器指针
     */
    ScriptLoader* getScriptLoader();
    
    /**
     * @brief 获取脚本执行器
     * @return 脚本执行器指针
     */
    ScriptRunner* getScriptRunner();
    
    /**
     * @brief 卸载所有脚本
     */
    void unloadAllScripts();

private:
    Core::ScriptEngine* m_engine;                    ///< 脚本引擎
    ScriptLoader* m_scriptLoader;                   ///< 脚本加载器
    ScriptRunner* m_scriptRunner;                   ///< 脚本执行器
    std::map<std::string, LifecycleStage> m_scriptStages; ///< 脚本生命周期阶段映射表
    LifecycleCallback m_lifecycleCallback;             ///< 生命周期回调
    bool m_initialized;                             ///< 是否已初始化
    
    /**
     * @brief 更新脚本的生命周期阶段
     * @param scriptName 脚本名称
     * @param stage 新阶段
     */
    void updateScriptStage(const std::string& scriptName, LifecycleStage stage);
    
    /**
     * @brief 触发初始化事件
     * @param scriptName 脚本名称
     * @return 成功返回true，失败返回false
     */
    bool triggerInitEvent(const std::string& scriptName);
    
    /**
     * @brief 触发加载事件
     * @param scriptName 脚本名称
     * @return 成功返回true，失败返回false
     */
    bool triggerLoadEvent(const std::string& scriptName);
    
    /**
     * @brief 触发关闭事件
     * @param scriptName 脚本名称
     * @return 成功返回true，失败返回false
     */
    bool triggerCloseEvent(const std::string& scriptName);
};

} // namespace Lifecycle
} // namespace Lua
} // namespace LuaUI

#endif // LUAUI_LIFECYCLEMANAGER_H
