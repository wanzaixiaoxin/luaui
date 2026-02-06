/**
 * @file XmlParser.cpp
 * @brief XML解析器实现
 */

#include "xml/parser/XmlParser.h"
#include "tinyxml2.h"
#include <algorithm>

namespace LuaUI {
namespace Xml {

// ========== XmlElement implementation ==========

XmlElement::XmlElement()
{
}

XmlElement::~XmlElement() {
    // 释放子元素
    for (size_t i = 0; i < m_children.size(); ++i) {
        delete m_children[i];
    }
    m_children.clear();
}

std::string XmlElement::getType() const {
    return m_type;
}

void XmlElement::setType(const std::string& type) {
    m_type = type;
}

std::string XmlElement::getId() const {
    return m_id;
}

void XmlElement::setId(const std::string& id) {
    m_id = id;
}

std::string XmlElement::getAttribute(const std::string& name) const {
    XmlAttributes::const_iterator it = m_attributes.find(name);
    if (it != m_attributes.end()) {
        return it->second;
    }
    return "";
}

void XmlElement::setAttribute(const std::string& name, const std::string& value) {
    m_attributes[name] = value;
}

const XmlAttributes& XmlElement::getAttributes() const {
    return m_attributes;
}

std::string XmlElement::getText() const {
    return m_text;
}

void XmlElement::setText(const std::string& text) {
    m_text = text;
}

size_t XmlElement::getChildCount() const {
    return m_children.size();
}

const std::vector<XmlElement*>& XmlElement::getChildren() const {
    return m_children;
}

void XmlElement::addChild(XmlElement* child) {
    if (child) {
        m_children.push_back(child);
    }
}

XmlElement* XmlElement::findChildById(const std::string& id) const {
    for (size_t i = 0; i < m_children.size(); ++i) {
        if (m_children[i]->getId() == id) {
            return m_children[i];
        }
        
        // 递归查找
        XmlElement* found = m_children[i]->findChildById(id);
        if (found) {
            return found;
        }
    }
    return nullptr;
}

std::vector<XmlElement*> XmlElement::findChildrenByType(const std::string& type) const {
    std::vector<XmlElement*> result;
    
    for (size_t i = 0; i < m_children.size(); ++i) {
        if (m_children[i]->getType() == type) {
            result.push_back(m_children[i]);
        }
        
        // 递归查找
        std::vector<XmlElement*> subChildren = m_children[i]->findChildrenByType(type);
        result.insert(result.end(), subChildren.begin(), subChildren.end());
    }
    
    return result;
}

// ========== XmlParser implementation ==========

XmlParser::XmlParser()
    : m_document(nullptr)
    , m_rootElement(nullptr)
{
}

XmlParser::~XmlParser() {
    clear();
}

XmlElement* XmlParser::parseFile(const std::string& filepath) {
    clear();
    
    m_document = new tinyxml2::XMLDocument();
    tinyxml2::XMLError result = m_document->LoadFile(filepath.c_str());
    
    if (result != tinyxml2::XML_SUCCESS) {
        setLastError("Failed to load XML file: " + filepath);
        return nullptr;
    }
    
    // 获取根元素
    tinyxml2::XMLElement* rootXml = m_document->RootElement();
    if (!rootXml) {
        setLastError("No root element found in XML file");
        return nullptr;
    }
    
    m_rootElement = convertElement(rootXml);
    return m_rootElement;
}

XmlElement* XmlParser::parseString(const std::string& xmlContent) {
    clear();
    
    m_document = new tinyxml2::XMLDocument();
    tinyxml2::XMLError result = m_document->Parse(xmlContent.c_str());
    
    if (result != tinyxml2::XML_SUCCESS) {
        setLastError("Failed to parse XML string");
        return nullptr;
    }
    
    // 获取根元素
    tinyxml2::XMLElement* rootXml = m_document->RootElement();
    if (!rootXml) {
        setLastError("No root element found in XML");
        return nullptr;
    }
    
    m_rootElement = convertElement(rootXml);
    return m_rootElement;
}

std::string XmlParser::getLastError() const {
    return m_lastError;
}

void XmlParser::clear() {
    if (m_rootElement) {
        delete m_rootElement;
        m_rootElement = nullptr;
    }
    
    if (m_document) {
        delete m_document;
        m_document = nullptr;
    }
    
    m_lastError.clear();
}

XmlElement* XmlParser::convertElement(tinyxml2::XMLElement* xmlElement) {
    if (!xmlElement) {
        return nullptr;
    }
    
    // 创建新的XmlElement
    XmlElement* element = new XmlElement();
    
    // 设置元素类型
    element->setType(xmlElement->Name());
    
    // 解析属性
    const tinyxml2::XMLAttribute* attr = xmlElement->FirstAttribute();
    while (attr) {
        element->setAttribute(attr->Name(), attr->Value());
        attr = attr->Next();
    }
    
    // 获取ID属性
    element->setId(element->getAttribute("id"));
    
    // 获取文本内容
    const char* text = xmlElement->GetText();
    if (text) {
        element->setText(text);
    }
    
    // 递归解析子元素
    parseElementTree(xmlElement, element);
    
    return element;
}

void XmlParser::parseElementTree(tinyxml2::XMLElement* xmlElement, XmlElement* parentElement) {
    if (!xmlElement || !parentElement) {
        return;
    }
    
    // 遍历子元素
    for (tinyxml2::XMLElement* childXml = xmlElement->FirstChildElement();
         childXml != nullptr;
         childXml = childXml->NextSiblingElement()) {
        
        XmlElement* childElement = convertElement(childXml);
        if (childElement) {
            parentElement->addChild(childElement);
        }
    }
}

void XmlParser::setLastError(const std::string& error) {
    m_lastError = error;
}

} // namespace Xml
} // namespace LuaUI
