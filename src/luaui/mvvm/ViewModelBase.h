#pragma once

#include "IBindable.h"
#include <vector>
#include <algorithm>

namespace luaui {
namespace mvvm {

// ============================================================================
// ViewModelBase - ViewModel基类
// 提供属性变更通知机制
// ============================================================================
class ViewModelBase : public INotifyPropertyChanged {
public:
    ViewModelBase();
    virtual ~ViewModelBase() = default;
    
    // 禁止拷贝（避免绑定混乱）
    ViewModelBase(const ViewModelBase&) = delete;
    ViewModelBase& operator=(const ViewModelBase&) = delete;
    
    // 允许移动
    ViewModelBase(ViewModelBase&&) = default;
    ViewModelBase& operator=(ViewModelBase&&) = default;
    
    // INotifyPropertyChanged 实现
    void SubscribePropertyChanged(PropertyChangedHandler handler) override;
    void UnsubscribePropertyChanged(PropertyChangedHandler handler) override;
    void NotifyPropertyChanged(const std::string& propertyName) override;
    
    // 属性值获取/设置（供绑定引擎使用）
    std::any GetPropertyValue(const std::string& propertyName) const override;
    void SetPropertyValue(const std::string& propertyName, const std::any& value) override;
    
    // 批量更新模式（减少通知次数）
    void BeginUpdate();
    void EndUpdate();
    bool IsUpdating() const;
    
protected:
    // 设置属性值并触发通知（辅助宏的底层实现）
    template<typename T>
    bool SetProperty(T& storage, const T& value, const std::string& propertyName) {
        if (storage == value) {
            return false;
        }
        
        T oldValue = storage;
        storage = value;
        
        if (IsUpdating()) {
            m_hasPendingChanges = true;
        } else {
            PropertyChangedEventArgs args{propertyName, oldValue, value};
            for (auto& handler : m_handlers) {
                if (handler) {
                    handler(args);
                }
            }
        }
        return true;
    }
    
    // 注册属性 getter（子类在构造函数中调用）
    template<typename T>
    void RegisterPropertyGetter(const std::string& name, std::function<T()> getter) {
        m_getters[name] = [getter]() -> std::any {
            return getter();
        };
    }
    
    // 注册属性 setter（子类在构造函数中调用，可选）
    template<typename T>
    void RegisterPropertySetter(const std::string& name, std::function<void(const T&)> setter) {
        m_setters[name] = [setter](const std::any& value) {
            try {
                setter(std::any_cast<T>(value));
            } catch (...) {
                // 类型不匹配，忽略
            }
        };
    }
    
private:
    std::vector<PropertyChangedHandler> m_handlers;
    int m_updateCount = 0;
    bool m_hasPendingChanges = false;
    
    // 属性访问器存储
    std::unordered_map<std::string, std::function<std::any()>> m_getters;
    std::unordered_map<std::string, std::function<void(const std::any&)>> m_setters;
};

// ============================================================================
// 便捷宏 - 用于声明属性
// 示例: BINDABLE_PROPERTY(std::string, UserName)
// ============================================================================
#define BINDABLE_PROPERTY(type, name) \
private: \
    type m_##name; \
public: \
    type Get##name() const { return m_##name; } \
    void Set##name(const type& value) { \
        if (SetProperty(m_##name, value, #name)) { \
            On##name##Changed(); \
        } \
    } \
protected: \
    virtual void On##name##Changed() {}

} // namespace mvvm
} // namespace luaui
