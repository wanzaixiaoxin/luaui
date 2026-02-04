/**
 * @file StyleManager.h
 * @brief 样式管理器
 * @details 管理控件的样式
 * @version 1.0.0
 */

#ifndef LUAUI_STYLEMANAGER_H
#define LUAUI_STYLEMANAGER_H

#include <string>
#include <map>
#include <vector>
#include "ui/controls/BaseControl.h"

namespace LuaUI {
namespace Resource {
namespace Style {

/**
 * @brief 样式属性类型
 */
enum StylePropertyType {
    PROPERTY_COLOR,      ///< 颜色属性
    PROPERTY_FONT,       ///< 字体属性
    PROPERTY_SIZE,       ///< 尺寸属性
    PROPERTY_MARGIN,     ///< 边距属性
    PROPERTY_PADDING,    ///< 内边距属性
    PROPERTY_BORDER,     ///< 边框属性
    PROPERTY_BACKGROUND,///< 背景属性
    PROPERTY_CUSTOM      ///< 自定义属性
};

/**
 * @brief 颜色值
 */
struct ColorValue {
    unsigned char r;  ///< 红色分量
    unsigned char g;  ///< 绿色分量
    unsigned char b;  ///< 蓝色分量
    unsigned char a;  ///< 透明度
    
    /**
     * @brief 构造函数
     */
    ColorValue()
        : r(0)
        , g(0)
        , b(0)
        , a(255)
    {
    }
    
    /**
     * @brief 构造函数
     */
    ColorValue(unsigned char red, unsigned char green, unsigned char blue, 
               unsigned char alpha = 255)
        : r(red)
        , g(green)
        , b(blue)
        , a(alpha)
    {
    }
    
    /**
     * @brief 转换为RGB值
     * @return RGB值
     */
    unsigned int toRGB() const {
        return (r << 16) | (g << 8) | b;
    }
    
    /**
     * @brief 转换为RGBA值
     * @return RGBA值
     */
    unsigned int toRGBA() const {
        return (a << 24) | (r << 16) | (g << 8) | b;
    }
    
    /**
     * @brief 从RGB值创建
     * @param rgb RGB值
     * @return 颜色值
     */
    static ColorValue fromRGB(unsigned int rgb);
    
    /**
     * @brief 从RGBA值创建
     * @param rgba RGBA值
     * @return 颜色值
     */
    static ColorValue fromRGBA(unsigned int rgba);
    
    /**
     * @brief 从十六进制字符串解析
     * @param hexString 十六进制字符串（例如："#FF0000"）
     * @return 颜色值
     */
    static ColorValue fromHexString(const std::string& hexString);
    
    /**
     * @brief 转换为十六进制字符串
     * @return 十六进制字符串
     */
    std::string toHexString() const;
};

/**
 * @brief 样式属性值
 */
struct StyleValue {
    StylePropertyType type;   ///< 属性类型
    std::string stringValue;   ///< 字符串值
    int intValue;            ///< 整数值
    ColorValue colorValue;   ///< 颜色值
    
    /**
     * @brief 构造函数
     */
    StyleValue()
        : type(PROPERTY_CUSTOM)
        , intValue(0)
    {
    }
};

/**
 * @brief 样式类
 */
class Style {
public:
    /**
     * @brief 构造函数
     * @param name 样式名称
     */
    Style(const std::string& name);
    
    /**
     * @brief 析构函数
     */
    ~Style();
    
    /**
     * @brief 获取样式名称
     * @return 样式名称
     */
    std::string getName() const;
    
    /**
     * @brief 设置属性
     * @param name 属性名
     * @param value 属性值
     */
    void setProperty(const std::string& name, const StyleValue& value);
    
    /**
     * @brief 获取属性
     * @param name 属性名
     * @return 属性值
     */
    StyleValue getProperty(const std::string& name) const;
    
    /**
     * @brief 检查是否存在属性
     * @param name 属性名
     * @return 存在返回true，否则返回false
     */
    bool hasProperty(const std::string& name) const;
    
    /**
     * @brief 设置父样式（继承）
     * @param style 父样式指针
     */
    void setParent(Style* style);
    
    /**
     * @brief 获取父样式
     * @return 父样式指针
     */
    Style* getParent() const;
    
    /**
     * @brief 应用样式到控件
     * @param control 控件指针
     */
    void apply(UI::BaseControl* control);

private:
    std::string m_name;                              ///< 样式名称
    std::map<std::string, StyleValue> m_properties;   ///< 属性映射表
    Style* m_parent;                                 ///< 父样式
};

/**
 * @brief 样式管理器类
 */
class StyleManager {
public:
    /**
     * @brief 构造函数
     */
    StyleManager();
    
    /**
     * @brief 析构函数
     */
    ~StyleManager();
    
    /**
     * @brief 创建样式
     * @param name 样式名称
     * @return 样式指针
     */
    Style* createStyle(const std::string& name);
    
    /**
     * @brief 删除样式
     * @param name 样式名称
     * @return 成功返回true，失败返回false
     */
    bool deleteStyle(const std::string& name);
    
    /**
     * @brief 获取样式
     * @param name 样式名称
     * @return 样式指针，如果不存在返回nullptr
     */
    Style* getStyle(const std::string& name);
    
    /**
     * @brief 检查样式是否存在
     * @param name 样式名称
     * @return 存在返回true，否则返回false
     */
    bool hasStyle(const std::string& name) const;
    
    /**
     * @brief 加载样式表
     * @param filepath 样式表文件路径
     * @return 成功返回true，失败返回false
     */
    bool loadStyleSheet(const std::string& filepath);
    
    /**
     * @brief 加载样式字符串
     * @param styleContent 样式内容
     * @return 成功返回true，失败返回false
     */
    bool loadStyleString(const std::string& styleContent);
    
    /**
     * @brief 应用样式到控件
     * @param control 控件指针
     * @param styleName 样式名称
     * @return 成功返回true，失败返回false
     */
    bool applyStyle(UI::BaseControl* control, const std::string& styleName);
    
    /**
     * @brief 获取所有样式名称
     * @return 样式名称列表
     */
    std::vector<std::string> getAllStyleNames() const;
    
    /**
     * @brief 清除所有样式
     */
    void clearAllStyles();

private:
    std::map<std::string, Style*> m_styles;  ///< 样式映射表
    
    /**
     * @brief 解析CSS样式字符串
     * @param styleContent CSS内容
     * @return 成功返回true，失败返回false
     */
    bool parseCSS(const std::string& styleContent);
    
    /**
     * @brief 解析样式规则
     * @param ruleText 规则文本
     */
    void parseStyleRule(const std::string& ruleText);
};

} // namespace Style
} // namespace Resource
} // namespace LuaUI

#endif // LUAUI_STYLEMANAGER_H
