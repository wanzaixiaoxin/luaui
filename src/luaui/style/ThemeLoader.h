#pragma once

#include "ResourceDictionary.h"
#include <string>

namespace luaui {
namespace controls {

/**
 * @brief 主题加载器：从 XML 文件加载主题配置
 * 
 * XML 格式:
 * @code
 * <?xml version="1.0" encoding="UTF-8"?>
 * <Theme Name="Light" IsDark="false">
 *   <Color Key="AccentColor" Value="#0078D7" />
 *   <Float Key="FontSize" Value="14.0" />
 *   <Thickness Key="ButtonPadding" Value="8,4,8,4" />
 *   <CornerRadius Key="ButtonRadius" Value="4" />
 * </Theme>
 * @endcode
 * 
 * 颜色格式: #RRGGBB 或 #AARRGGBB
 * Thickness 格式: "left,top,right,bottom" 或 "all"
 * CornerRadius 格式: "topLeft,topRight,bottomRight,bottomLeft" 或 "all"
 */
class ThemeLoader {
public:
    /**
     * @brief 从 XML 文件加载主题
     * @param filePath XML 文件路径
     * @return 加载的 ResourceDictionary，失败返回空字典
     */
    static ResourceDictionary LoadFromFile(const std::string& filePath);

    /**
     * @brief 从 XML 字符串加载主题
     * @param xmlContent XML 内容字符串
     * @return 加载的 ResourceDictionary，失败返回空字典
     */
    static ResourceDictionary LoadFromString(const std::string& xmlContent);

    /**
     * @brief 获取默认主题目录路径
     * 默认为可执行文件同级的 themes/ 目录
     */
    static std::string GetDefaultThemesDir();

    /**
     * @brief 按名称加载内置主题（从 themes/ 目录的 XML 文件加载）
     * @param name 主题名称（如 "Light"、"Dark"）
     * @return 对应的 ResourceDictionary；文件不存在返回空字典
     */
    static ResourceDictionary LoadBuiltinTheme(const std::string& name);
};

} // namespace controls
} // namespace luaui
