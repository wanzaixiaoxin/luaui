#pragma once

#include "Style.h"
#include <unordered_map>
#include <string>
#include <memory>

namespace luaui {
namespace controls {

/**
 * @brief 资源字典（简化版）
 */
class ResourceDictionary {
public:
    using Ptr = std::shared_ptr<ResourceDictionary>;
    
    // 存储样式
    void AddStyle(const std::string& key, Style::Ptr style) {
        m_styles[key] = style;
    }
    
    Style::Ptr GetStyle(const std::string& key) const {
        auto it = m_styles.find(key);
        return (it != m_styles.end()) ? it->second : nullptr;
    }
    
    // 存储颜色
    void AddColor(const std::string& key, const rendering::Color& color) {
        m_colors[key] = color;
    }
    
    rendering::Color GetColor(const std::string& key) const {
        auto it = m_colors.find(key);
        return (it != m_colors.end()) ? it->second : rendering::Color::Transparent();
    }

private:
    std::unordered_map<std::string, Style::Ptr> m_styles;
    std::unordered_map<std::string, rendering::Color> m_colors;
};

/**
 * @brief 资源引用（简化版，用于 Setter 值）
 */
struct ResourceReference {
    std::string key;
    ResourceReference() = default;
    ResourceReference(const std::string& k) : key(k) {}
};

} // namespace controls
} // namespace luaui
