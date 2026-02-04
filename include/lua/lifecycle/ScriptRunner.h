/**
 * @file ScriptRunner.h
 * @brief 脚本执行器
 * @details 负责执行Lua脚本
 * @version 1.0.0
 */

#ifndef LUAUI_SCRIPTRUNNER_H
#define LUAUI_SCRIPTRUNNER_H

#include <string>
#include <lua.hpp>
#include "core/ScriptEngine.h"

namespace LuaUI {
namespace Lua {
namespace Lifecycle {

/**
 * @brief 执行结果
 */
struct ExecutionResult {
    bool success;         ///< 是否成功
    std::string error;     ///< 错误信息
    std::string output;    ///< 输出信息
    int returnValue;       ///< 返回值
    
    /**
     * @brief 构造函数
     */
    ExecutionResult()
        : success(true)
        , returnValue(0)
    {
    }
};

/**
 * @brief 脚本执行器类
 */
class ScriptRunner {
public:
    /**
     * @brief 构造函数
     * @param engine 脚本引擎
     */
    ScriptRunner(Core::ScriptEngine* engine);
    
    /**
     * @brief 析构函数
     */
    ~ScriptRunner();
    
    /**
     * @brief 执行Lua函数
     * @param funcName 函数名
     * @return 执行结果
     */
    ExecutionResult runFunction(const std::string& funcName);
    
    /**
     * @brief 执行带参数的Lua函数
     * @param funcName 函数名
     * @param args 参数列表
     * @return 执行结果
     */
    ExecutionResult runFunctionWithArgs(const std::string& funcName,
                                      const std::vector<std::string>& args);
    
    /**
     * @brief 执行Lua脚本字符串
     * @param scriptContent 脚本内容
     * @return 执行结果
     */
    ExecutionResult runString(const std::string& scriptContent);
    
    /**
     * @brief 执行Lua文件
     * @param filepath 文件路径
     * @return 执行结果
     */
    ExecutionResult runFile(const std::string& filepath);
    
    /**
     * @brief 设置超时时间（毫秒）
     * @param timeout 超时时间
     */
    void setTimeout(int timeout);
    
    /**
     * @brief 获取超时时间
     * @return 超时时间
     */
    int getTimeout() const;
    
    /**
     * @brief 捕获并返回print输出
     * @param capture 是否捕获
     */
    void captureOutput(bool capture);
    
    /**
     * @brief 获取捕获的输出
     * @return 输出内容
     */
    std::string getCapturedOutput() const;
    
    /**
     * @brief 清除捕获的输出
     */
    void clearCapturedOutput();
    
    /**
     * @brief 设置执行环境
     * @param tableName 环境表名
     */
    void setEnvironment(const std::string& tableName);
    
    /**
     * @brief 重置为默认执行环境
     */
    void resetEnvironment();

private:
    Core::ScriptEngine* m_engine;     ///< 脚本引擎
    int m_timeout;                   ///< 超时时间
    bool m_captureOutput;             ///< 是否捕获输出
    std::string m_capturedOutput;      ///< 捕获的输出
    std::string m_environmentTable;     ///< 环境表名
    
    /**
     * @brief 执行函数的内部实现
     * @param funcName 函数名
     * @param args 参数列表
     * @return 执行结果
     */
    ExecutionResult executeFunctionInternal(const std::string& funcName,
                                       const std::vector<std::string>* args);
    
    /**
     * @brief 检查是否超时
     * @param startTime 开始时间
     * @return 是否超时
     */
    bool checkTimeout(unsigned long startTime);
};

} // namespace Lifecycle
} // namespace Lua
} // namespace LuaUI

#endif // LUAUI_SCRIPTRUNNER_H
