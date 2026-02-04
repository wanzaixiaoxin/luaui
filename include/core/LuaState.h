/**
 * @file LuaState.h
 * @brief Lua状态管理器
 * @details 管理Lua虚拟机的生命周期，提供脚本加载和执行功能
 * @version 1.0.0
 */

#ifndef LUAUI_LUASTATE_H
#define LUAUI_LUASTATE_H

#include <string>

// Forward declaration for lua_State
struct lua_State;

namespace LuaUI {
namespace Core {

/**
 * @brief Lua状态管理类
 * @details 封装Lua虚拟机，提供脚本加载和执行接口
 * 
 * 使用示例：
 * @code
 * LuaState lua;
 * lua.initialize();
 * lua.loadFile("script.lua");
 * lua.callFunction("onInit");
 * lua.shutdown();
 * @endcode
 */
class LuaState {
public:
    /**
     * @brief 构造函数
     */
    LuaState();
    
    /**
     * @brief 析构函数
     * @details 自动关闭Lua虚拟机
     */
    ~LuaState();
    
    /**
     * @brief 初始化Lua虚拟机
     * @return 成功返回true，失败返回false
     */
    bool initialize();
    
    /**
     * @brief 关闭Lua虚拟机
     * @details 清理所有资源
     */
    void shutdown();
    
    /**
     * @brief 检查Lua虚拟机是否已初始化
     * @return 已初始化返回true，否则返回false
     */
    bool isInitialized() const;
    
    /**
     * @brief 获取原生Lua状态指针
     * @return lua_State指针，如果未初始化返回nullptr
     */
    lua_State* getState() const;
    
    /**
     * @brief 加载并执行Lua脚本文件
     * @param filepath Lua文件路径
     * @return 成功返回true，失败返回false
     */
    bool loadFile(const std::string& filepath);
    
    /**
     * @brief 加载并执行Lua脚本字符串
     * @param script Lua脚本内容
     * @return 成功返回true，失败返回false
     */
    bool loadString(const std::string& script);
    
    /**
     * @brief 调用Lua函数
     * @param funcName 函数名称（格式：模块名.函数名 或 仅函数名）
     * @param nargs 传递给函数的参数个数
     * @param nresults 期望的返回值个数
     * @return 成功返回true，失败返回false
     */
    bool callFunction(const std::string& funcName, int nargs = 0, int nresults = 0);
    
    /**
     * @brief 注册C++函数到Lua
     * @param name 函数名称
     * @param func C++函数指针
     */
    void registerFunction(const std::string& name, lua_CFunction func);
    
    /**
     * @brief 执行Lua语句块（不加载）
     * @param script Lua脚本内容
     * @return 成功返回true，失败返回false
     */
    bool doString(const std::string& script);
    
    /**
     * @brief 执行Lua文件（不加载）
     * @param filepath Lua文件路径
     * @return 成功返回true，失败返回false
     */
    bool doFile(const std::string& filepath);
    
    /**
     * @brief 获取最后一次错误信息
     * @return 错误信息字符串
     */
    std::string getLastError() const;
    
private:
    lua_State* m_luaState;       ///< Lua虚拟机状态指针
    bool m_initialized;           ///< 是否已初始化标志
    std::string m_lastError;      ///< 最后一次错误信息
    
    /**
     * @brief 清理Lua栈
     */
    void clearStack();
    
    /**
     * @brief 设置错误信息
     * @param error 错误信息
     */
    void setLastError(const std::string& error);
};

} // namespace Core
} // namespace LuaUI

#endif // LUAUI_LUASTATE_H
