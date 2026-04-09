#pragma once

#include "../rendering/Types.h"
#include "Style.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <functional>

namespace luaui {
namespace controls {

class Control;

using ThemeCallback = std::function<void()>;

class ResourceDictionary {
public:
    using Ptr = std::shared_ptr<ResourceDictionary>;

    void AddStyle(const std::string& key, Style::Ptr style) {
        m_styles[key] = style;
    }

    Style::Ptr GetStyle(const std::string& key) const {
        auto it = m_styles.find(key);
        return (it != m_styles.end()) ? it->second : nullptr;
    }

    void AddColor(const std::string& key, const rendering::Color& color) {
        m_colors[key] = color;
    }

    rendering::Color GetColor(const std::string& key) const {
        auto it = m_colors.find(key);
        return (it != m_colors.end()) ? it->second : rendering::Color::Transparent();
    }

    bool HasColor(const std::string& key) const {
        return m_colors.find(key) != m_colors.end();
    }

    void AddFloat(const std::string& key, float value) {
        m_floats[key] = value;
    }

    float GetFloat(const std::string& key, float fallback = 0.0f) const {
        auto it = m_floats.find(key);
        return (it != m_floats.end()) ? it->second : fallback;
    }

    bool HasFloat(const std::string& key) const {
        return m_floats.find(key) != m_floats.end();
    }

    void AddThickness(const std::string& key, const rendering::Thickness& value) {
        m_thickness[key] = value;
    }

    rendering::Thickness GetThickness(const std::string& key) const {
        auto it = m_thickness.find(key);
        return (it != m_thickness.end()) ? it->second : rendering::Thickness(0);
    }

    void AddCornerRadius(const std::string& key, const rendering::CornerRadius& value) {
        m_corners[key] = value;
    }

    rendering::CornerRadius GetCornerRadius(const std::string& key) const {
        auto it = m_corners.find(key);
        return (it != m_corners.end()) ? it->second : rendering::CornerRadius(0);
    }

    /** @brief 合并另一个字典（other 覆盖 this） */
    void Merge(const ResourceDictionary& other) {
        for (auto& [k, v] : other.m_styles) m_styles[k] = v;
        for (auto& [k, v] : other.m_colors) m_colors[k] = v;
        for (auto& [k, v] : other.m_floats) m_floats[k] = v;
        for (auto& [k, v] : other.m_thickness) m_thickness[k] = v;
        for (auto& [k, v] : other.m_corners) m_corners[k] = v;
    }

private:
    std::unordered_map<std::string, Style::Ptr> m_styles;
    std::unordered_map<std::string, rendering::Color> m_colors;
    std::unordered_map<std::string, float> m_floats;
    std::unordered_map<std::string, rendering::Thickness> m_thickness;
    std::unordered_map<std::string, rendering::CornerRadius> m_corners;
};

struct ResourceReference {
    std::string key;
    ResourceReference() = default;
    ResourceReference(const std::string& k) : key(k) {}
};

} // namespace controls
} // namespace luaui
