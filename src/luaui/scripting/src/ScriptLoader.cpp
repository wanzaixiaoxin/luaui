/**
 * @file ScriptLoader.cpp
 * @brief 脚本加载器实现
 */

#include "lua/lifecycle/ScriptLoader.h"
#include "core/Utils.h"

namespace LuaUI {
namespace Lua {
namespace Lifecycle {

ScriptLoader::ScriptLoader(Core::ScriptEngine* engine)
    : m_engine(engine)
{
}

ScriptLoader::~ScriptLoader() {
    unloadAll();
}

bool ScriptLoader::loadFile(const std::string& filepath, const std::string& scriptName) {
    if (!m_engine) {
        setLastError("Script engine is null");
        return false;
    }
    
    // 读取文件内容
    std::string content = Utils::FileUtils::readFile(filepath);
    
    if (content.empty()) {
        setLastError("Failed to read file: " + filepath);
        return false;
    }
    
    // 确定脚本名称
    std::string name = scriptName.empty() ? 
        Utils::FileUtils::getFileName(filepath) : scriptName;
    
    // 移除扩展名
    std::string::size_type dotPos = name.find_last_of('.');
    if (dotPos != std::string::npos) {
        name = name.substr(0, dotPos);
    }
    
    // 使用引擎加载脚本
    if (!m_engine->loadScript(filepath)) {
        setLastError("Failed to load script into engine");
        return false;
    }
    
    // 记录脚本信息
    ScriptInfo info;
    info.name = name;
    info.path = filepath;
    info.content = content;
    info.loaded = true;
    info.refCount = 1;
    
    m_scripts[name] = info;
    
    return true;
}

bool ScriptLoader::loadString(const std::string& scriptContent, const std::string& scriptName) {
    if (!m_engine) {
        setLastError("Script engine is null");
        return false;
    }
    
    if (scriptName.empty()) {
        setLastError("Script name is required");
        return false;
    }
    
    // 使用引擎加载脚本字符串
    if (!m_engine->loadScriptString(scriptContent)) {
        setLastError("Failed to load script string into engine");
        return false;
    }
    
    // 记录脚本信息
    ScriptInfo info;
    info.name = scriptName;
    info.path = "";
    info.content = scriptContent;
    info.loaded = true;
    info.refCount = 1;
    
    m_scripts[scriptName] = info;
    
    return true;
}

bool ScriptLoader::reload(const std::string& scriptName) {
    std::map<std::string, ScriptInfo>::iterator it = m_scripts.find(scriptName);
    if (it == m_scripts.end()) {
        setLastError("Script not found: " + scriptName);
        return false;
    }
    
    const ScriptInfo& info = it->second;
    
    if (!info.path.empty()) {
        // 重新加载文件
        return loadFile(info.path, scriptName);
    } else {
        // 重新加载字符串
        return loadString(info.content, scriptName);
    }
}

bool ScriptLoader::unload(const std::string& scriptName) {
    std::map<std::string, ScriptInfo>::iterator it = m_scripts.find(scriptName);
    if (it == m_scripts.end()) {
        setLastError("Script not found: " + scriptName);
        return false;
    }
    
    it->second.refCount--;
    
    if (it->second.refCount <= 0) {
        m_scripts.erase(it);
    }
    
    return true;
}

void ScriptLoader::unloadAll() {
    m_scripts.clear();
}

bool ScriptLoader::isLoaded(const std::string& scriptName) const {
    std::map<std::string, ScriptInfo>::const_iterator it = m_scripts.find(scriptName);
    return (it != m_scripts.end() && it->second.loaded);
}

const ScriptInfo* ScriptLoader::getScriptInfo(const std::string& scriptName) const {
    std::map<std::string, ScriptInfo>::const_iterator it = m_scripts.find(scriptName);
    if (it != m_scripts.end()) {
        return &(it->second);
    }
    return nullptr;
}

std::vector<std::string> ScriptLoader::getLoadedScripts() const {
    std::vector<std::string> scripts;
    
    for (std::map<std::string, ScriptInfo>::const_iterator it = m_scripts.begin();
         it != m_scripts.end(); ++it) {
        if (it->second.loaded) {
            scripts.push_back(it->first);
        }
    }
    
    return scripts;
}

void ScriptLoader::setSearchPaths(const std::vector<std::string>& paths) {
    m_searchPaths = paths;
}

void ScriptLoader::addSearchPath(const std::string& path) {
    m_searchPaths.push_back(path);
}

std::string ScriptLoader::findScript(const std::string& scriptName) const {
    // 如果脚本名称包含路径，直接返回
    if (scriptName.find('/') != std::string::npos ||
        scriptName.find('\\') != std::string::npos) {
        return scriptName;
    }
    
    // 在搜索路径中查找
    for (size_t i = 0; i < m_searchPaths.size(); ++i) {
        std::string path = m_searchPaths[i];
        std::string filepath = path + "/" + scriptName;
        
        if (Utils::FileUtils::exists(filepath)) {
            return filepath;
        }
        
        // 尝试添加.lua扩展名
        filepath = path + "/" + scriptName + ".lua";
        if (Utils::FileUtils::exists(filepath)) {
            return filepath;
        }
    }
    
    // 未找到
    return "";
}

std::string ScriptLoader::readFile(const std::string& filepath) {
    return Utils::FileUtils::readFile(filepath);
}

void ScriptLoader::setLastError(const std::string& error) {
    m_lastError = error;
}

} // namespace Lifecycle
} // namespace Lua
} // namespace LuaUI
