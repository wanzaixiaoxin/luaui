/**
 * @file XmlParser.h
 * @brief XML解析器
 * @details 解析XML布局文件，提取控件和布局信息
 * @version 1.0.0
 */

#ifndef LUAUI_XMLPARSER_H
#define LUAUI_XMLPARSER_H

#include <string>
#include <vector>
#include <map>

// TinyXML2 forward declarations
namespace tinyxml2 {
    class XMLDocument;
    class XMLElement;
}

namespace LuaUI {
namespace Xml {

/**
 * @brief XML元素属性
 */
typedef std::map<std::string, std::string> XmlAttributes;

/**
 * @brief XML元素节点
 */
class XmlElement {
public:
    /**
     * @brief 构造函数
     */
    XmlElement();
    
    /**
     * @brief 析构函数
     */
    ~XmlElement();
    
    /**
     * @brief 获取元素类型（标签名）
     * @return 元素类型
     */
    std::string getType() const;
    
    /**
     * @brief 设置元素类型
     * @param type 元素类型
     */
    void setType(const std::string& type);
    
    /**
     * @brief 获取元素ID
     * @return 元素ID
     */
    std::string getId() const;
    
    /**
     * @brief 设置元素ID
     * @param id 元素ID
     */
    void setId(const std::string& id);
    
    /**
     * @brief 获取元素属性值
     * @param name 属性名
     * @return 属性值，如果不存在返回空字符串
     */
    std::string getAttribute(const std::string& name) const;
    
    /**
     * @brief 设置元素属性值
     * @param name 属性名
     * @param value 属性值
     */
    void setAttribute(const std::string& name, const std::string& value);
    
    /**
     * @brief 获取所有属性
     * @return 属性映射表
     */
    const XmlAttributes& getAttributes() const;
    
    /**
     * @brief 获取文本内容
     * @return 文本内容
     */
    std::string getText() const;
    
    /**
     * @brief 设置文本内容
     * @param text 文本内容
     */
    void setText(const std::string& text);
    
    /**
     * @brief 获取子元素数量
     * @return 子元素数量
     */
    size_t getChildCount() const;
    
    /**
     * @brief 获取所有子元素
     * @return 子元素列表
     */
    const std::vector<XmlElement*>& getChildren() const;
    
    /**
     * @brief 添加子元素
     * @param child 子元素指针（调用方不负责释放）
     */
    void addChild(XmlElement* child);
    
    /**
     * @brief 根据ID查找子元素
     * @param id 子元素ID
     * @return 子元素指针，如果不存在返回nullptr
     */
    XmlElement* findChildById(const std::string& id) const;
    
    /**
     * @brief 根据类型查找所有子元素
     * @param type 元素类型
     * @return 子元素列表
     */
    std::vector<XmlElement*> findChildrenByType(const std::string& type) const;
    
private:
    std::string m_type;                 ///< 元素类型
    std::string m_id;                   ///< 元素ID
    XmlAttributes m_attributes;        ///< 属性映射表
    std::string m_text;                ///< 文本内容
    std::vector<XmlElement*> m_children; ///< 子元素列表
};

/**
 * @brief XML解析器类
 * @details 解析XML布局文件，生成元素树
 */
class XmlParser {
public:
    /**
     * @brief 构造函数
     */
    XmlParser();
    
    /**
     * @brief 析构函数
     */
    ~XmlParser();
    
    /**
     * @brief 从文件解析XML
     * @param filepath XML文件路径
     * @return 根元素指针，解析失败返回nullptr
     */
    XmlElement* parseFile(const std::string& filepath);
    
    /**
     * @brief 从字符串解析XML
     * @param xmlContent XML内容字符串
     * @return 根元素指针，解析失败返回nullptr
     */
    XmlElement* parseString(const std::string& xmlContent);
    
    /**
     * @brief 获取最后一次错误信息
     * @return 错误信息字符串
     */
    std::string getLastError() const;
    
    /**
     * @brief 清除解析结果
     */
    void clear();
    
private:
    tinyxml2::XMLDocument* m_document;    ///< TinyXML2文档对象
    XmlElement* m_rootElement;           ///< 根元素
    std::string m_lastError;              ///< 最后一次错误信息
    
    /**
     * @brief 从TinyXML2元素转换为XmlElement
     * @param xmlElement TinyXML2元素指针
     * @return XmlElement指针
     */
    XmlElement* convertElement(tinyxml2::XMLElement* xmlElement);
    
    /**
     * @brief 递归解析元素树
     * @param xmlElement TinyXML2元素指针
     * @param parentElement 父XmlElement指针
     */
    void parseElementTree(tinyxml2::XMLElement* xmlElement, XmlElement* parentElement);
    
    /**
     * @brief 设置错误信息
     * @param error 错误信息
     */
    void setLastError(const std::string& error);
};

} // namespace Xml
} // namespace LuaUI

#endif // LUAUI_XMLPARSER_H
