#pragma once

#include <memory>
#include <unordered_map>
#include <typeindex>

namespace luaui {

// Forward declaration
class Control;

namespace components {

/**
 * @brief 组件基类
 * 
 * 用于实现组合模式，将 Control 的功能拆分为多个组件
 * 每个组件负责单一职责
 */
class Component {
public:
    Component(Control* owner) : m_owner(owner) {}
    virtual ~Component() = default;
    
    // 生命周期
    virtual void Initialize() {}
    virtual void Shutdown() {}
    
    // 访问宿主控件
    Control* GetOwner() { return m_owner; }
    const Control* GetOwner() const { return m_owner; }
    
protected:
    Control* m_owner;
};

/**
 * @brief 组件持有者
 * 
 * 管理组件的生命周期和访问
 */
class ComponentHolder {
public:
    template<typename T, typename... Args>
    T* AddComponent(Args&&... args) {
        static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = component.get();
        m_components[typeid(T)] = std::move(component);
        return ptr;
    }
    
    template<typename T>
    T* GetComponent() {
        auto it = m_components.find(typeid(T));
        if (it != m_components.end()) {
            return static_cast<T*>(it->second.get());
        }
        return nullptr;
    }
    
    template<typename T>
    const T* GetComponent() const {
        auto it = m_components.find(typeid(T));
        if (it != m_components.end()) {
            return static_cast<const T*>(it->second.get());
        }
        return nullptr;
    }
    
    template<typename T>
    bool HasComponent() const {
        return m_components.find(typeid(T)) != m_components.end();
    }
    
    // 获取所有组件（用于遍历）
    const std::unordered_map<std::type_index, std::unique_ptr<Component>>& GetComponents() const {
        return m_components;
    }
    
    void InitializeAll();
    void ShutdownAll();
    
private:
    std::unordered_map<std::type_index, std::unique_ptr<Component>> m_components;
};

} // namespace components
} // namespace luaui
