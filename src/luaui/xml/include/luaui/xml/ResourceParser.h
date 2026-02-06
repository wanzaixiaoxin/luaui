/**
 * @file ResourceParser.h
 * @brief 资源引用解析器
 * @details 解析XML中的资源引用（图片、图标等）
 * @version 1.0.0
 */

#ifndef LUAUI_RESOURCEPARSER_H
#define LUAUI_RESOURCEPARSER_H

#include <string>
#include <map>
#include <vector>
#include "xml/parser/XmlParser.h"

namespace LuaUI {
namespace Xml {

/**
 * @brief 资源类型
 */
enum ResourceType {
    RESOURCE_IMAGE,    ///< 图片资源
    RESOURCE_ICON,     ///< 图标资源
    RESOURCE_FONT,     ///< 字体资源
    RESOURCE_STYLE,    ///< 样式资源
    RESOURCE_THEME,    ///< 主题资源
    RESOURCE_UNKNOWN   ///< 未知资源
};

/**
 * @brief 资源引用信息
 */
struct ResourceReference {
    ResourceType type;         ///< 资源类型
    std::string name;          ///< 资源名称
    std::string path;          ///< 资源路径
    std::string elementId;      ///< 引用该资源的元素ID
    std::string attributeName; ///< 引用该资源的属性名
    
    /**
     * @brief 构造函数
     */
    ResourceReference()
        : type(RESOURCE_UNKNOWN)
    {
    }
    
    /**
     * @brief 构造函数
     * @param t 资源类型
     * @param n 资源名称
     * @param p 资源路径
     * @param elemId 元素ID
     * @param attrName 属性名
     */
    ResourceReference(ResourceType t, const std::string& n, const std::string& p,
                      const std::string& elemId = "", const std::string& attrName = "")
        : type(t)
        , name(n)
        , path(p)
        , elementId(elemId)
        , attributeName(attrName)
    {
    }
};

/**
 * @brief 资源引用解析器类
 */
class ResourceParser {
public:
    /**
     * @brief 构造函数
     */
    ResourceParser();
    
    /**
     * @brief 析构函数
     */
    ~ResourceParser();
    
    /**
     * @brief 解析资源引用
     * @param rootElement 根元素
     * @return 资源引用列表
     */
    std::vector<ResourceReference> parse(XmlElement* rootElement);
    
    /**
     * @brief 从XML文件解析资源引用
     * @param filepath XML文件路径
     * @return 资源引用列表
     */
    std::vector<ResourceReference> parseFile(const std::string& filepath);
    
    /**
     * @brief 从XML字符串解析资源引用
     * @param xmlContent XML内容
     * @return 资源引用列表
     */
    std::vector<ResourceReference> parseString(const std::string& xmlContent);
    
    /**
     * @brief 根据属性名判断资源类型
     * @param attributeName 属性名
     * @return 资源类型
     */
    static ResourceType getResourceType(const std::string& attributeName);
    
    /**
     * @brief 获取资源类型名称
     * @param type 资源类型
     * @return 类型名称
     */
    static std::string getResourceTypeName(ResourceType type);
    
private:
    std::map<std::string, ResourceType> m_attributeTypeMap; ///< 属性名到资源类型的映射
    
    /**
     * @brief 初始化属性类型映射表
     */
    void initAttributeTypeMap();
    
    /**
     * @brief 递归解析元素树中的资源引用
     * @param element 元素
     * @param references 资源引用列表
     */
    void parseElement(XmlElement* element, std::vector<ResourceReference>& references);
};

} // namespace Xml
} // namespace LuaUI

#endif // LUAUI_RESOURCEPARSER_H
