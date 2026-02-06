/**
 * @file LifecycleManager.cpp
 * @brief 生命周期管理器实现
 */

#include "lua/lifecycle/LifecycleManager.h"

namespace LuaUI {
namespace Lua {
namespace Lifecycle {

LifecycleManager::LifecycleManager(Core::ScriptEngine* engine)
    : m_engine(engine)
    , m_scriptLoader(nullptr)
    , m_scriptRunner(nullptr)
    , m_lifecycleCallback(nullptr)
    , m_initialized(false)
{
    if (m_engine) {
        m_scriptLoader = new ScriptLoader(m_engine);
        m_scriptRunner = new ScriptRunner(m_engine);
    }
}

LifecycleManager::~LifecycleManager() {
    shutdown();
}

bool LifecycleManager::initialize() {
    if (m_initialized) {
        return true;
    }
    
    if (!m_engine || !m_scriptLoader || !m_scriptRunner) {
        return false;
    }
    
    m_initialized = true;
    return true;
}

void LifecycleManager::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    unloadAllScripts();
    
    if (m_scriptLoader) {
        delete m_scriptLoader;
        m_scriptLoader = nullptr;
    }
    
    if (m_scriptRunner) {
        delete m_scriptRunner;
        m_scriptRunner = nullptr;
    }
    
    m_scriptStages.clear();
    m_initialized = false;
}

void LifecycleManager::unloadAllScripts() {
    // Get all script names from the stages map
    std::vector<std::string> scriptNames;
    for (std::map<std::string, LifecycleStage>::iterator it = m_scriptStages.begin();
         it != m_scriptStages.end(); ++it) {
        scriptNames.push_back(it->first);
    }
    
    // Unload each script
    for (size_t i = 0; i < scriptNames.size(); ++i) {
        unloadScript(scriptNames[i]);
    }
    
    // Clear the stages map
    m_scriptStages.clear();
}

bool LifecycleManager::loadScript(const std::string& scriptName) {
    if (!m_initialized || !m_scriptLoader) {
        return false;
    }
    
    // 查找脚本文件
    std::string filepath = m_scriptLoader->findScript(scriptName);
    
    if (filepath.empty()) {
        return false;
    }
    
    // 加载脚本
    if (!m_scriptLoader->loadFile(filepath)) {
        return false;
    }
    
    // 触发初始化事件
    updateScriptStage(scriptName, STAGE_INIT);
    triggerInitEvent(scriptName);
    
    // 触发加载事件
    updateScriptStage(scriptName, STAGE_LOAD);
    triggerLoadEvent(scriptName);
    
    // 设置为就绪状态
    updateScriptStage(scriptName, STAGE_READY);
    
    // 触发生命周期回调
    triggerLifecycleEvent(STAGE_READY, scriptName);
    
    return true;
}

bool LifecycleManager::startScript(const std::string& scriptName) {
    std::map<std::string, LifecycleStage>::iterator it = m_scriptStages.find(scriptName);
    if (it == m_scriptStages.end() || it->second != STAGE_READY) {
        return false;
    }
    
    // 设置为运行状态
    updateScriptStage(scriptName, STAGE_RUNNING);
    
    // 触发生命周期回调
    triggerLifecycleEvent(STAGE_RUNNING, scriptName);
    
    return true;
}

bool LifecycleManager::pauseScript(const std::string& scriptName) {
    std::map<std::string, LifecycleStage>::iterator it = m_scriptStages.find(scriptName);
    if (it == m_scriptStages.end() || it->second != STAGE_RUNNING) {
        return false;
    }
    
    // 设置为暂停状态
    updateScriptStage(scriptName, STAGE_PAUSED);
    
    // 触发生命周期回调
    triggerLifecycleEvent(STAGE_PAUSED, scriptName);
    
    return true;
}

bool LifecycleManager::resumeScript(const std::string& scriptName) {
    std::map<std::string, LifecycleStage>::iterator it = m_scriptStages.find(scriptName);
    if (it == m_scriptStages.end() || it->second != STAGE_PAUSED) {
        return false;
    }
    
    // 设置为运行状态
    updateScriptStage(scriptName, STAGE_RUNNING);
    
    // 触发生命周期回调
    triggerLifecycleEvent(STAGE_RUNNING, scriptName);
    
    return true;
}

bool LifecycleManager::stopScript(const std::string& scriptName) {
    std::map<std::string, LifecycleStage>::iterator it = m_scriptStages.find(scriptName);
    if (it == m_scriptStages.end()) {
        return false;
    }
    
    LifecycleStage currentStage = it->second;
    
    // 只有运行或暂停状态才能停止
    if (currentStage != STAGE_RUNNING && currentStage != STAGE_PAUSED) {
        return false;
    }
    
    // 触发关闭事件
    updateScriptStage(scriptName, STAGE_CLOSING);
    triggerCloseEvent(scriptName);
    
    // 设置为关闭状态
    updateScriptStage(scriptName, STAGE_CLOSED);
    
    // 触发生命周期回调
    triggerLifecycleEvent(STAGE_CLOSED, scriptName);
    
    return true;
}

bool LifecycleManager::unloadScript(const std::string& scriptName) {
    std::map<std::string, LifecycleStage>::iterator it = m_scriptStages.find(scriptName);
    if (it == m_scriptStages.end()) {
        return false;
    }
    
    LifecycleStage currentStage = it->second;
    
    // 如果是运行或暂停状态，先停止
    if (currentStage == STAGE_RUNNING || currentStage == STAGE_PAUSED) {
        stopScript(scriptName);
    }
    
    // 卸载脚本
    if (m_scriptLoader) {
        m_scriptLoader->unload(scriptName);
    }
    
    // 从生命周期映射表中移除
    m_scriptStages.erase(it);
    
    return true;
}

LifecycleStage LifecycleManager::getScriptStage(const std::string& scriptName) const {
    std::map<std::string, LifecycleStage>::const_iterator it = m_scriptStages.find(scriptName);
    if (it != m_scriptStages.end()) {
        return it->second;
    }
    return STAGE_CLOSED;
}

std::map<std::string, LifecycleStage> LifecycleManager::getAllScriptStages() const {
    return m_scriptStages;
}

void LifecycleManager::setLifecycleCallback(LifecycleCallback callback) {
    m_lifecycleCallback = callback;
}

void LifecycleManager::triggerLifecycleEvent(LifecycleStage stage, const std::string& scriptName) {
    if (m_lifecycleCallback) {
        m_lifecycleCallback(stage, scriptName);
    }
}

ScriptLoader* LifecycleManager::getScriptLoader() {
    return m_scriptLoader;
}

ScriptRunner* LifecycleManager::getScriptRunner() {
    return m_scriptRunner;
}

void LifecycleManager::updateScriptStage(const std::string& scriptName, LifecycleStage stage) {
    m_scriptStages[scriptName] = stage;
}

bool LifecycleManager::triggerInitEvent(const std::string& scriptName) {
    if (!m_scriptRunner) {
        return false;
    }
    
    return m_scriptRunner->runFunction(scriptName + "_onInit").success;
}

bool LifecycleManager::triggerLoadEvent(const std::string& scriptName) {
    if (!m_scriptRunner) {
        return false;
    }
    
    return m_scriptRunner->runFunction(scriptName + "_onLoad").success;
}

bool LifecycleManager::triggerCloseEvent(const std::string& scriptName) {
    if (!m_scriptRunner) {
        return false;
    }
    
    return m_scriptRunner->runFunction(scriptName + "_onClose").success;
}

} // namespace Lifecycle
} // namespace Lua
} // namespace LuaUI
