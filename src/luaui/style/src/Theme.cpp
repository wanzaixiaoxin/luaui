#include "Theme.h"
#include <algorithm>

namespace luaui {
namespace controls {

// Theme 实现
Theme::Theme() = default;

void Theme::AddStyle(const std::string& key, Style::Ptr style) {
    if (style) {
        m_styles[key] = style;
    }
}

void Theme::RemoveStyle(const std::string& key) {
    m_styles.erase(key);
}

Style::Ptr Theme::GetStyle(const std::string& key) const {
    auto it = m_styles.find(key);
    if (it != m_styles.end()) {
        return it->second;
    }
    return nullptr;
}

void Theme::SetImplicitStyle(const std::type_index& type, Style::Ptr style) {
    if (style) {
        m_implicitStyles[type] = style;
    }
}

Style::Ptr Theme::GetImplicitStyle(const std::type_index& type) const {
    auto it = m_implicitStyles.find(type);
    if (it != m_implicitStyles.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::string> Theme::GetStyleKeys() const {
    std::vector<std::string> keys;
    keys.reserve(m_styles.size());
    for (const auto& [key, _] : m_styles) {
        keys.push_back(key);
    }
    return keys;
}

bool Theme::LoadFromFile(const std::string& path) {
    // TODO: 实现 XML 文件加载
    (void)path;
    return false;
}

bool Theme::LoadFromString(const std::string& xml) {
    // TODO: 实现 XML 字符串解析
    return ParseXml(xml);
}

bool Theme::ParseXml(const std::string& xml) {
    // TODO: 使用 TinyXML2 解析
    (void)xml;
    return false;
}

void Theme::MergeBaseTheme(const Theme& baseTheme) {
    // 合并资源
    m_resources.Merge(baseTheme.m_resources);
    
    // 合并非覆盖的样式
    for (const auto& [key, style] : baseTheme.m_styles) {
        if (m_styles.find(key) == m_styles.end()) {
            m_styles[key] = style;
        }
    }
    
    // 合并非覆盖的隐式样式
    for (const auto& [type, style] : baseTheme.m_implicitStyles) {
        if (m_implicitStyles.find(type) == m_implicitStyles.end()) {
            m_implicitStyles[type] = style;
        }
    }
}

void Theme::Clear() {
    m_name.clear();
    m_baseTheme.clear();
    m_version.clear();
    m_author.clear();
    m_description.clear();
    m_resources.Clear();
    m_styles.clear();
    m_implicitStyles.clear();
}

// ThemeManager 实现
ThemeManager& ThemeManager::GetInstance() {
    static ThemeManager instance;
    return instance;
}

bool ThemeManager::LoadTheme(const std::string& name, const std::string& path) {
    auto theme = std::make_unique<Theme>();
    if (!theme->LoadFromFile(path)) {
        return false;
    }
    
    // 如果有基础主题，先合并
    if (!theme->GetBaseTheme().empty()) {
        auto* baseTheme = GetTheme(theme->GetBaseTheme());
        if (baseTheme) {
            theme->MergeBaseTheme(*baseTheme);
        }
    }
    
    m_themes[name] = std::move(theme);
    return true;
}

void ThemeManager::UnloadTheme(const std::string& name) {
    // 不能卸载当前主题
    if (name == m_currentThemeName) {
        return;
    }
    
    m_themes.erase(name);
}

bool ThemeManager::HasTheme(const std::string& name) const {
    return m_themes.find(name) != m_themes.end();
}

void ThemeManager::SetCurrentTheme(const std::string& name) {
    auto it = m_themes.find(name);
    if (it == m_themes.end()) {
        return;
    }
    
    m_currentTheme = std::make_unique<Theme>(*it->second);
    m_currentThemeName = name;
    
    // 通知所有订阅者
    for (auto& callback : m_callbacks) {
        if (callback) {
            callback(name);
        }
    }
    
    // 刷新所有注册控件的样式
    InvalidateAllStyles();
}

Theme* ThemeManager::GetTheme(const std::string& name) {
    auto it = m_themes.find(name);
    if (it != m_themes.end()) {
        return it->second.get();
    }
    return nullptr;
}

std::vector<std::string> ThemeManager::GetAvailableThemes() const {
    std::vector<std::string> names;
    names.reserve(m_themes.size());
    for (const auto& [name, _] : m_themes) {
        names.push_back(name);
    }
    return names;
}

std::any ThemeManager::GetResource(const std::string& key) const {
    if (m_currentTheme) {
        try {
            return m_currentTheme->GetResources().Get(key);
        } catch (...) {
            // 资源未找到
        }
    }
    return std::any();
}

Style::Ptr ThemeManager::GetStyle(const std::string& key) const {
    if (m_currentTheme) {
        return m_currentTheme->GetStyle(key);
    }
    return nullptr;
}

Style::Ptr ThemeManager::GetImplicitStyle(const std::type_index& type) const {
    if (m_currentTheme) {
        return m_currentTheme->GetImplicitStyle(type);
    }
    return nullptr;
}

void ThemeManager::SubscribeThemeChanged(ThemeChangedCallback callback) {
    m_callbacks.push_back(std::move(callback));
}

void ThemeManager::InvalidateAllStyles() {
    // 通知所有注册的控件刷新样式
    for (auto* control : m_registeredControls) {
        if (control) {
            // TODO: 调用控件的样式刷新方法
            // control->InvalidateStyle();
        }
    }
}

void ThemeManager::RegisterControl(Control* control) {
    if (control && std::find(m_registeredControls.begin(), m_registeredControls.end(), control) 
                  == m_registeredControls.end()) {
        m_registeredControls.push_back(control);
    }
}

void ThemeManager::UnregisterControl(Control* control) {
    auto it = std::find(m_registeredControls.begin(), m_registeredControls.end(), control);
    if (it != m_registeredControls.end()) {
        m_registeredControls.erase(it);
    }
}

} // namespace controls
} // namespace luaui
