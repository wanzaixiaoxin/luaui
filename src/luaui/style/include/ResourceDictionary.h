#pragma once
#include "Types.h"
#include <unordered_map>
#include <string>
#include <any>
#include <memory>
#include <vector>
#include <functional>

namespace luaui {
namespace controls {

// 前向声明
class ResourceDictionary;

// 资源引用类型
enum class ResourceLookupMode {
    Static,     // 一次性查找
    Dynamic     // 动态查找，支持运行时变更
};

// 资源引用（支持延迟解析）
class ResourceReference {
public:
    ResourceReference() = default;
    ResourceReference(const std::string& key, ResourceLookupMode mode = ResourceLookupMode::Static)
        : m_key(key), m_mode(mode) {}
    
    const std::string& GetKey() const { return m_key; }
    ResourceLookupMode GetMode() const { return m_mode; }
    
    bool IsEmpty() const { return m_key.empty(); }
    
    // 解析引用为实际值
    std::any Resolve(const ResourceDictionary* localResources, 
                     const ResourceDictionary* themeResources) const;
    
private:
    std::string m_key;
    ResourceLookupMode m_mode = ResourceLookupMode::Static;
};

// 资源字典
class ResourceDictionary : public std::enable_shared_from_this<ResourceDictionary> {
public:
    using Ptr = std::shared_ptr<ResourceDictionary>;
    using ConstPtr = std::shared_ptr<const ResourceDictionary>;
    using ResourceChangedCallback = std::function<void(const std::string& key)>;
    
    ResourceDictionary() = default;
    virtual ~ResourceDictionary() = default;
    
    // 资源管理
    void Add(const std::string& key, const std::any& value);
    void Remove(const std::string& key);
    bool Contains(const std::string& key) const;
    std::any Get(const std::string& key) const;
    
    // 类型安全获取
    template<typename T>
    T Get(const std::string& key) const {
        return std::any_cast<T>(Get(key));
    }
    
    template<typename T>
    T GetOrDefault(const std::string& key, const T& defaultValue) const {
        auto it = m_resources.find(key);
        if (it != m_resources.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (...) {
                return defaultValue;
            }
        }
        // 尝试从父字典获取
        if (m_parent) {
            return m_parent->GetOrDefault(key, defaultValue);
        }
        return defaultValue;
    }
    
    // 获取原始值（可能是 ResourceReference）
    std::any GetRaw(const std::string& key) const;
    
    // 合并另一个字典（用于主题继承）
    void Merge(const ResourceDictionary& other);
    
    // 父字典（支持资源查找链）
    void SetParent(ResourceDictionary* parent) { m_parent = parent; }
    ResourceDictionary* GetParent() const { return m_parent; }
    
    // 资源变更通知
    void SubscribeResourceChanged(ResourceChangedCallback callback);
    void NotifyResourceChanged(const std::string& key);
    
    // 获取所有资源键
    std::vector<std::string> GetKeys() const;
    
    // 清空资源
    void Clear();
    
    // 资源数量
    size_t GetCount() const { return m_resources.size(); }
    
private:
    std::unordered_map<std::string, std::any> m_resources;
    ResourceDictionary* m_parent = nullptr;
    std::vector<ResourceChangedCallback> m_callbacks;
};

// 资源键帮助函数
inline ResourceReference StaticResource(const std::string& key) {
    return ResourceReference(key, ResourceLookupMode::Static);
}

inline ResourceReference DynamicResource(const std::string& key) {
    return ResourceReference(key, ResourceLookupMode::Dynamic);
}

} // namespace controls
} // namespace luaui
