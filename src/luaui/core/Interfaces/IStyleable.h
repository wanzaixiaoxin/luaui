#pragma once

#include <memory>
#include <string>
#include <any>

namespace luaui {
namespace controls {
    class DependencyProperty;
    class Style;
    class ResourceDictionary;
}

namespace interfaces {

/**
 * @brief 样式化接口
 * 
 * 支持样式和资源字典的控件实现此接口
 */
class IStyleable {
public:
    virtual ~IStyleable() = default;

    // ========== 样式 ==========
    virtual std::shared_ptr<controls::Style> GetStyle() const = 0;
    virtual void SetStyle(std::shared_ptr<controls::Style> style) = 0;
    
    virtual void InvalidateStyle() = 0;
    virtual void UpdateStyle() = 0;

    // ========== 资源 ==========
    virtual std::shared_ptr<controls::ResourceDictionary> GetResources() = 0;
    virtual void SetResources(std::shared_ptr<controls::ResourceDictionary> resources) = 0;
    
    virtual std::any FindResource(const std::string& key) const = 0;
    
    template<typename T>
    T FindResource(const std::string& key) const {
        return std::any_cast<T>(FindResource(key));
    }

    // ========== 依赖属性 ==========
    virtual std::any GetValue(controls::DependencyProperty::Id propertyId) const = 0;
    virtual void SetValue(controls::DependencyProperty::Id propertyId, const std::any& value) = 0;
    virtual void ClearValue(controls::DependencyProperty::Id propertyId) = 0;
    virtual bool HasLocalValue(controls::DependencyProperty::Id propertyId) const = 0;
};

} // namespace interfaces
} // namespace luaui
