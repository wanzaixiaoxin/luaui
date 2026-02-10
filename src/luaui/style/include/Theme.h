#pragma once
#include "ResourceDictionary.h"
#include "Style.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include <functional>

namespace luaui {
namespace controls {

// 主题类
class Theme {
public:
    Theme();
    ~Theme() = default;
    
    // 主题信息
    const std::string& GetName() const { return m_name; }
    void SetName(const std::string& name) { m_name = name; }
    
    const std::string& GetBaseTheme() const { return m_baseTheme; }
    void SetBaseTheme(const std::string& base) { m_baseTheme = base; }
    
    const std::string& GetVersion() const { return m_version; }
    void SetVersion(const std::string& version) { m_version = version; }
    
    const std::string& GetAuthor() const { return m_author; }
    void SetAuthor(const std::string& author) { m_author = author; }
    
    const std::string& GetDescription() const { return m_description; }
    void SetDescription(const std::string& desc) { m_description = desc; }
    
    // 资源字典
    ResourceDictionary& GetResources() { return m_resources; }
    const ResourceDictionary& GetResources() const { return m_resources; }
    
    // 样式管理
    void AddStyle(const std::string& key, Style::Ptr style);
    void RemoveStyle(const std::string& key);
    Style::Ptr GetStyle(const std::string& key) const;
    
    // 隐式样式（基于类型）
    void SetImplicitStyle(const std::type_index& type, Style::Ptr style);
    Style::Ptr GetImplicitStyle(const std::type_index& type) const;
    
    // 获取所有样式键
    std::vector<std::string> GetStyleKeys() const;
    
    // 加载主题
    bool LoadFromFile(const std::string& path);
    bool LoadFromString(const std::string& xml);
    
    // 合并基础主题
    void MergeBaseTheme(const Theme& baseTheme);
    
    // 清空
    void Clear();
    
private:
    std::string m_name;
    std::string m_baseTheme;
    std::string m_version;
    std::string m_author;
    std::string m_description;
    
    ResourceDictionary m_resources;
    std::unordered_map<std::string, Style::Ptr> m_styles;
    std::unordered_map<std::type_index, Style::Ptr> m_implicitStyles;
    
    // XML 解析辅助函数
    bool ParseXml(const std::string& xml);
};

// 主题管理器（单例）
class ThemeManager {
public:
    static ThemeManager& GetInstance();
    
    // 禁止拷贝
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;
    
    // 加载/卸载主题
    bool LoadTheme(const std::string& name, const std::string& path);
    void UnloadTheme(const std::string& name);
    bool HasTheme(const std::string& name) const;
    
    // 切换主题
    void SetCurrentTheme(const std::string& name);
    const std::string& GetCurrentThemeName() const { return m_currentThemeName; }
    Theme* GetCurrentTheme() { return m_currentTheme.get(); }
    const Theme* GetCurrentTheme() const { return m_currentTheme.get(); }
    
    // 获取主题
    Theme* GetTheme(const std::string& name);
    
    // 获取可用主题列表
    std::vector<std::string> GetAvailableThemes() const;
    
    // 获取资源（从当前主题）
    std::any GetResource(const std::string& key) const;
    template<typename T>
    T GetResource(const std::string& key) const {
        return std::any_cast<T>(GetResource(key));
    }
    
    // 获取样式（从当前主题）
    Style::Ptr GetStyle(const std::string& key) const;
    Style::Ptr GetImplicitStyle(const std::type_index& type) const;
    
    // 主题变更事件
    using ThemeChangedCallback = std::function<void(const std::string& newTheme)>;
    void SubscribeThemeChanged(ThemeChangedCallback callback);
    
    // 刷新所有控件的样式（在主题切换后调用）
    void InvalidateAllStyles();
    
    // 注册控件以接收样式更新
    void RegisterControl(Control* control);
    void UnregisterControl(Control* control);
    
private:
    ThemeManager() = default;
    ~ThemeManager() = default;
    
    std::unordered_map<std::string, std::unique_ptr<Theme>> m_themes;
    std::unique_ptr<Theme> m_currentTheme;
    std::string m_currentThemeName;
    std::vector<ThemeChangedCallback> m_callbacks;
    std::vector<Control*> m_registeredControls;
};

// 便捷函数
inline ThemeManager& GetThemeManager() {
    return ThemeManager::GetInstance();
}

} // namespace controls
} // namespace luaui
