/**
 * @file ScriptRunner.cpp
 * @brief 脚本执行器实现
 */

#include "lua/lifecycle/ScriptRunner.h"
#include <ctime>

namespace LuaUI {
namespace Lua {
namespace Lifecycle {

ScriptRunner::ScriptRunner(Core::ScriptEngine* engine)
    : m_engine(engine)
    , m_timeout(0)
    , m_captureOutput(false)
{
}

ScriptRunner::~ScriptRunner() {
}

ExecutionResult ScriptRunner::runFunction(const std::string& funcName) {
    std::vector<std::string> args;
    return executeFunctionInternal(funcName, &args);
}

ExecutionResult ScriptRunner::runFunctionWithArgs(const std::string& funcName,
                                                 const std::vector<std::string>& args) {
    return executeFunctionInternal(funcName, &args);
}

ExecutionResult ScriptRunner::runString(const std::string& scriptContent) {
    ExecutionResult result;
    
    if (!m_engine) {
        result.success = false;
        result.error = "Script engine is null";
        return result;
    }
    
    // 记录开始时间
    unsigned long startTime = (unsigned long)time(nullptr);
    
    // 执行脚本
    bool execResult = m_engine->executeString(scriptContent);
    
    // 检查超时
    if (m_timeout > 0 && checkTimeout(startTime)) {
        result.success = false;
        result.error = "Script execution timeout";
        return result;
    }
    
    if (execResult) {
        result.success = true;
        result.output = m_captureOutput ? m_capturedOutput : "";
    } else {
        result.success = false;
        result.error = "Failed to execute script string";
    }
    
    return result;
}

ExecutionResult ScriptRunner::runFile(const std::string& filepath) {
    ExecutionResult result;
    
    if (!m_engine) {
        result.success = false;
        result.error = "Script engine is null";
        return result;
    }
    
    // 记录开始时间
    unsigned long startTime = (unsigned long)time(nullptr);
    
    // 执行脚本
    bool execResult = m_engine->executeScript(filepath);
    
    // 检查超时
    if (m_timeout > 0 && checkTimeout(startTime)) {
        result.success = false;
        result.error = "Script execution timeout";
        return result;
    }
    
    if (execResult) {
        result.success = true;
        result.output = m_captureOutput ? m_capturedOutput : "";
    } else {
        result.success = false;
        result.error = "Failed to execute script file";
    }
    
    return result;
}

void ScriptRunner::setTimeout(int timeout) {
    m_timeout = timeout;
}

int ScriptRunner::getTimeout() const {
    return m_timeout;
}

void ScriptRunner::captureOutput(bool capture) {
    m_captureOutput = capture;
    if (capture) {
        clearCapturedOutput();
    }
}

std::string ScriptRunner::getCapturedOutput() const {
    return m_capturedOutput;
}

void ScriptRunner::clearCapturedOutput() {
    m_capturedOutput.clear();
}

void ScriptRunner::setEnvironment(const std::string& tableName) {
    m_environmentTable = tableName;
}

void ScriptRunner::resetEnvironment() {
    m_environmentTable.clear();
}

ExecutionResult ScriptRunner::executeFunctionInternal(const std::string& funcName,
                                                      const std::vector<std::string>* args) {
    ExecutionResult result;
    
    if (!m_engine) {
        result.success = false;
        result.error = "Script engine is null";
        return result;
    }
    
    // 记录开始时间
    unsigned long startTime = (unsigned long)time(nullptr);
    
    bool execResult;
    if (args && !args->empty()) {
        execResult = m_engine->callFunctionWithArgs(funcName, *args);
    } else {
        execResult = m_engine->callFunction(funcName);
    }
    
    // 检查超时
    if (m_timeout > 0 && checkTimeout(startTime)) {
        result.success = false;
        result.error = "Script execution timeout";
        return result;
    }
    
    if (execResult) {
        result.success = true;
        result.output = m_captureOutput ? m_capturedOutput : "";
        result.returnValue = 0;
    } else {
        result.success = false;
        result.error = "Failed to execute function";
        result.returnValue = -1;
    }
    
    return result;
}

bool ScriptRunner::checkTimeout(unsigned long startTime) {
    unsigned long currentTime = (unsigned long)time(nullptr);
    unsigned long elapsed = currentTime - startTime;
    return (elapsed * 1000) >= (unsigned long)m_timeout;
}

} // namespace Lifecycle
} // namespace Lua
} // namespace LuaUI
