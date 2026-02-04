/**
 * @file App.h
 * @brief 应用程序类
 * @details LuaUI框架的应用程序入口和管理类
 * @version 1.0.0
 */

#ifndef LUAUI_APP_H
#define LUAUI_APP_H

#include "ILuaUI.h"
#include <string>

namespace LuaUI {

/**
 * @brief 应用程序类
 * @details 管理LuaUI框架的生命周期和初始化
 */
class App {
public:
    /**
     * @brief 构造函数
     */
    App();
    
    /**
     * @brief 析构函数
     */
    ~App();
    
    /**
     * @brief 初始化应用程序
     * @param appTitle 应用程序标题
     * @return 成功返回true，失败返回false
     */
    bool initialize(const std::string& appTitle = "LuaUI Application");
    
    /**
     * @brief 关闭应用程序
     */
    void shutdown();
    
    /**
     * @brief 检查是否已初始化
     * @return 已初始化返回true，否则返回false
     */
    bool isInitialized() const;
    
    /**
     * @brief 运行应用程序主循环
     * @return 应用程序退出码
     */
    int run();
    
    /**
     * @brief 退出应用程序
     * @param exitCode 退出码
     */
    void exit(int exitCode = 0);
    
    /**
     * @brief 获取布局引擎接口
     * @return 布局引擎指针
     */
    ILayoutEngine* getLayoutEngine();
    
    /**
     * @brief 获取脚本引擎接口
     * @return 脚本引擎指针
     */
    IScriptEngine* getScriptEngine();
    
private:
    bool m_initialized;      ///< 是否已初始化标志
    std::string m_appTitle;  ///< 应用程序标题
    int m_exitCode;           ///< 退出码
};

} // namespace LuaUI

#endif // LUAUI_APP_H
