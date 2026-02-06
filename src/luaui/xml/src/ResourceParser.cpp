/**
 * @file ResourceParser.cpp
 * @brief 资源引用解析器实现
 */

#include "xml/parser/ResourceParser.h"
#include <algorithm>

namespace LuaUI {
namespace Xml {

ResourceParser::ResourceParser() {
    initAttributeTypeMap();
}

ResourceParser::~ResourceParser() {
}

std::vector<ResourceReference> ResourceParser::parse(XmlElement* rootElement) {
    std::vector<ResourceReference> references;
    
    if (!rootElement) {
        return references;
    }
    
    parseElement(rootElement, references);
    
    return references;
}

std::vector<ResourceReference> ResourceParser::parseFile(const std::string& filepath) {
    XmlParser parser;
    XmlElement* root = parser.parseFile(filepath);
    
    if (!root) {
        return std::vector<ResourceReference>();
    }
    
    std::vector<ResourceReference> references = parse(root);
    delete root;
    
    return references;
}

std::vector<ResourceReference> ResourceParser::parseString(const std::string& xmlContent) {
    XmlParser parser;
    XmlElement* root = parser.parseString(xmlContent);
    
    if (!root) {
        return std::vector<ResourceReference>();
    }
    
    std::vector<ResourceReference> references = parse(root);
    delete root;
    
    return references;
}

ResourceType ResourceParser::getResourceType(const std::string& attributeName) {
    ResourceParser parser;
    std::map<std::string, ResourceType>::const_iterator it = 
        parser.m_attributeTypeMap.find(attributeName);
    
    if (it != parser.m_attributeTypeMap.end()) {
        return it->second;
    }
    
    return RESOURCE_UNKNOWN;
}

std::string ResourceParser::getResourceTypeName(ResourceType type) {
    switch (type) {
        case RESOURCE_IMAGE:  return "Image";
        case RESOURCE_ICON:   return "Icon";
        case RESOURCE_FONT:   return "Font";
        case RESOURCE_STYLE:  return "Style";
        case RESOURCE_THEME:  return "Theme";
        default:              return "Unknown";
    }
}

void ResourceParser::initAttributeTypeMap() {
    // 图片相关属性
    m_attributeTypeMap["image"] = RESOURCE_IMAGE;
    m_attributeTypeMap["background"] = RESOURCE_IMAGE;
    m_attributeTypeMap["icon"] = RESOURCE_ICON;
    
    // 字体相关属性
    m_attributeTypeMap["font"] = RESOURCE_FONT;
    m_attributeTypeMap["fontName"] = RESOURCE_FONT;
    
    // 样式相关属性
    m_attributeTypeMap["style"] = RESOURCE_STYLE;
    m_attributeTypeMap["class"] = RESOURCE_STYLE;
    
    // 主题相关属性
    m_attributeTypeMap["theme"] = RESOURCE_THEME;
}

void ResourceParser::parseElement(XmlElement* element, 
                                   std::vector<ResourceReference>& references) {
    if (!element) {
        return;
    }
    
    std::string elementId = element->getId();
    
    // 遍历所有属性
    const XmlAttributes& attributes = element->getAttributes();
    for (XmlAttributes::const_iterator it = attributes.begin();
         it != attributes.end(); ++it) {
        
        std::string attrName = it->first;
        std::string attrValue = it->second;
        
        // 检查是否是资源引用
        ResourceType type = getResourceType(attrName);
        if (type != RESOURCE_UNKNOWN) {
            // 创建资源引用
            ResourceReference ref(type, attrName, attrValue, elementId, attrName);
            references.push_back(ref);
        }
    }
    
    // 递归解析子元素
    const std::vector<XmlElement*>& children = element->getChildren();
    for (size_t i = 0; i < children.size(); ++i) {
        parseElement(children[i], references);
    }
}

} // namespace Xml
} // namespace LuaUI
