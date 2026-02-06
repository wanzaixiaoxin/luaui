/**
 * @file ThemeManager.h
 * @brief 主题管理器
 * @details 管理应用程序的主题
 * @version 1.0.0
 */

#ifndef LUAUI_THEMEMANAGER_H
#define LUAUI_THEMEMANAGER_H

#include <string>
#include <map>
#include <vector>
#include "resource/style/StyleManager.h"

namespace LuaUI {
namespace Resource {
namespace Theme {

/**
 * @brief 主题类
 */
class Theme {
public:
    /**
     * @brief 构造函数
     * @param name 主题名称
     */
    Theme(const std::string& name);
    
    /**
     * @brief 析构函数
     */
    ~Theme();
    
    /**
     * @brief 获取主题名称
     * @return 主题名称
     */
    std::string getName() const;
    
    /**
     * @brief 获取主题作者
     * @return 作者
     */
    std::string getAuthor() const;
    
    /**
     * @brief 设置主题作者
     * @param author 作者
     */
    void setAuthor(const std::string& author);
    
    /**
     * @brief 获取主题版本
     * @return 版本
     */
    std::string getVersion() const;
    
    /**
     * @brief 设置主题版本
     * @param version 版本
     */
    void setVersion(const std::string& version);
    
    /**
     * @brief 获取主题描述
     * @return 描述
     */
    std::string getDescription() const;
    
    /**
     * @brief 设置主题描述
     * @param description 描述
     */
    void setDescription(const std::string& description);
    
    /**
     * @brief 获取样式管理器
     * @return 样式管理器指针
     */
    Style::StyleManager* getStyleManager();
    
    /**
     * @brief 获取样式管理器（const版本）
     * @return 样式管理器指针
     */
    const Style::StyleManager* getStyleManager() const;
    
    /**
     * @brief 应用主题到控件
     * @param control 控件指针
     * @param styleName 样式名称
     * @return 成功返回true，失败返回false
     */
    bool applyTo(UI::BaseControl* control, const std::string& styleName);
    
    /**
     * @brief 加载主题配置文件
     * @param filepath 配置文件路径
     * @return 成功返回true，失败返回false
     */
    bool loadConfig(const std::string& filepath);
    
    /**
     * @brief 保存主题配置文件
     * @param filepath 配置文件路径
     * @return 成功返回true，失败返回false
     */
    bool saveConfig(const std::string& filepath);

private:
    std::string m_name;                  ///< 主题名称
    std::string m_author;                ///< 作者
    std::string m_version;               ///< 版本
    std::string m_description;            ///< 描述
    Style::StyleManager* m_styleManager; ///< 样式管理器
};

/**
 * @brief 主题管理器类
 */
class ThemeManager {
public:
    /**
     * @brief 构造函数
     */
    ThemeManager();
    
    /**
     * @brief 析构函数
     */
    ~ThemeManager();
    
    /**
     * @brief 加载主题
     * @param filepath 主题配置文件路径
     * @param themeName 主题名称（可选）
     * @return 主题指针，失败返回nullptr
     */
    Theme* loadTheme(const std::string& filepath, const std::string& themeName = "");
    
    /**
     * @brief 卸载主题
     * @param themeName 主题名称
     * @return 成功返回true，失败返回false
     */
    bool unloadTheme(const std::string& themeName);
    
    /**
     * @brief 获取当前主题
     * @return 当前主题指针
     */
    Theme* getCurrentTheme();
    
    /**
     * @brief 获取当前主题（const版本）
     * @return 当前主题指针
     */
    const Theme* getCurrentTheme() const;
    
    /**
     * @brief 设置当前主题
     * @param themeName 主题名称
     * @return 成功返回true，失败返回false
     */
    bool setCurrentTheme(const std::string& themeName);
    
    /**
     * @brief 获取主题
     * @param themeName 主题名称
     * @return 主题指针，如果不存在返回nullptr
     */
    Theme* getTheme(const std::string& themeName);
    
    /**
     * @brief 检查主题是否存在
     * @param themeName 主题名称
     * @return 存在返回true，否则返回false
     */
    bool hasTheme(const std::string& themeName) const;
    
    /**
     * @brief 获取所有主题名称
     * @return 主题名称列表
     */
    std::vector<std::string> getAllThemeNames() const;
    
    /**
     * @brief 创建主题
     * @param themeName 主题名称
     * @return 主题指针
     */
    Theme* createTheme(const std::string& themeName);
    
    /**
     * @brief 删除主题
     * @param themeName 主题名称
     * @return 成功返回true，失败返回false
     */
    bool deleteTheme(const std::string& themeName);
    
    /**
     * @brief 设置主题切换回调
     * @param callback 回调函数
     */
    typedef void (*ThemeChangeCallback)(Theme* oldTheme, Theme* newTheme);
    void setThemeChangeCallback(ThemeChangeCallback callback);
    
    /**
     * @brief 应用主题样式到控件
     * @param control 控件指针
     * @param styleName 样式名称
     * @return 成功返回true，失败返回false
     */
    bool applyCurrentTheme(UI::BaseControl* control, const std::string& styleName = "");

private:
    std::map<std::string, Theme*> m_themes;        ///< 主题映射表
    Theme* m_currentTheme;                           ///< 当前主题
    ThemeChangeCallback m_themeChangeCallback;           ///< 主题切换回调
};

} // namespace Theme
} // namespace Resource
} // namespace LuaUI

#endif // LUAUI_THEMEMANAGER_H
