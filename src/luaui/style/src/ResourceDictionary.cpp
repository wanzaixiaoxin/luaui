#include "ResourceDictionary.h"
#include <algorithm>
#include <cstddef>
#include <stdexcept>

namespace luaui {
namespace controls {

// ResourceReference 实现
std::any ResourceReference::Resolve(const ResourceDictionary* localResources, 
                                    const ResourceDictionary* themeResources) const {
    // 1. 先在本地资源中查找
    if (localResources && localResources->Contains(m_key)) {
        return localResources->Get(m_key);
    }
    
    // 2. 在主题资源中查找
    if (themeResources && themeResources->Contains(m_key)) {
        return themeResources->Get(m_key);
    }
    
    // 3. 未找到，返回空
    return std::any();
}

// ResourceDictionary 实现
void ResourceDictionary::Add(const std::string& key, const std::any& value) {
    bool existed = m_resources.find(key) != m_resources.end();
    m_resources[key] = value;
    
    if (existed) {
        NotifyResourceChanged(key);
    }
}

void ResourceDictionary::Remove(const std::string& key) {
    auto it = m_resources.find(key);
    if (it != m_resources.end()) {
        m_resources.erase(it);
        NotifyResourceChanged(key);
    }
}

bool ResourceDictionary::Contains(const std::string& key) const {
    if (m_resources.find(key) != m_resources.end()) {
        return true;
    }
    if (m_parent) {
        return m_parent->Contains(key);
    }
    return false;
}

std::any ResourceDictionary::Get(const std::string& key) const {
    auto it = m_resources.find(key);
    if (it != m_resources.end()) {
        // 如果值是 ResourceReference，需要解析它
        try {
            const ResourceReference& ref = std::any_cast<const ResourceReference&>(it->second);
            if (!ref.IsEmpty()) {
                return ref.Resolve(this, m_parent);
            }
        } catch (const std::bad_any_cast&) {
            // 不是 ResourceReference，直接返回值
        }
        return it->second;
    }
    
    // 从父字典查找
    if (m_parent) {
        return m_parent->Get(key);
    }
    
    throw std::runtime_error("Resource not found: " + key);
}

std::any ResourceDictionary::GetRaw(const std::string& key) const {
    auto it = m_resources.find(key);
    if (it != m_resources.end()) {
        return it->second;
    }
    
    if (m_parent) {
        return m_parent->GetRaw(key);
    }
    
    return std::any();
}

void ResourceDictionary::Merge(const ResourceDictionary& other) {
    for (const auto& [key, value] : other.m_resources) {
        // 只添加不存在的键，避免覆盖
        if (m_resources.find(key) == m_resources.end()) {
            m_resources[key] = value;
        }
    }
}

void ResourceDictionary::SubscribeResourceChanged(ResourceChangedCallback callback) {
    m_callbacks.push_back(std::move(callback));
}

void ResourceDictionary::NotifyResourceChanged(const std::string& key) {
    for (auto& callback : m_callbacks) {
        if (callback) {
            callback(key);
        }
    }
}

std::vector<std::string> ResourceDictionary::GetKeys() const {
    std::vector<std::string> keys;
    keys.reserve(m_resources.size());
    
    for (const auto& [key, _] : m_resources) {
        keys.push_back(key);
    }
    
    // 也包含父字典的键
    if (m_parent) {
        auto parentKeys = m_parent->GetKeys();
        for (const auto& key : parentKeys) {
            if (std::find(keys.begin(), keys.end(), key) == keys.end()) {
                keys.push_back(key);
            }
        }
    }
    
    return keys;
}

void ResourceDictionary::Clear() {
    auto keys = GetKeys();
    m_resources.clear();
    
    // 通知所有资源变更
    for (const auto& key : keys) {
        NotifyResourceChanged(key);
    }
}

} // namespace controls
} // namespace luaui
