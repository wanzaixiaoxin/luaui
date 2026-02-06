/**
 * @file LayoutValidator.cpp
 * @brief 布局验证器实现
 */

#include "xml/parser/LayoutValidator.h"
#include <algorithm>
#include <map>
#include <vector>
#include <string>

namespace LuaUI {
namespace Xml {

LayoutValidator::LayoutValidator() {
    initValidTypes();
}

LayoutValidator::~LayoutValidator() {
}

ValidationInfo LayoutValidator::validate(XmlElement* rootElement) {
    clearErrors();
    
    if (!rootElement) {
        return ValidationInfo(VALIDATION_ERROR_NO_ROOT, "Root element is null");
    }
    
    // 检查是否是窗口元素
    std::string type = rootElement->getType();
    if (type != "window") {
        return ValidationInfo(VALIDATION_ERROR_INVALID_TYPE, 
                              "Root element must be 'window'", 
                              rootElement->getId());
    }
    
    // 检查重复ID
    std::vector<std::string> duplicateIds = checkDuplicateIds(rootElement);
    for (size_t i = 0; i < duplicateIds.size(); ++i) {
        addError(VALIDATION_ERROR_DUPLICATE_ID, 
                 "Duplicate ID found: " + duplicateIds[i], 
                 duplicateIds[i]);
    }
    
    // 递归验证所有元素 - 内联实现
    if (rootElement) {
        // 验证元素类型
        std::string element_type = rootElement->getType();
        bool isValidType = false;
        for (size_t i = 0; i < m_validTypes.size(); ++i) {
            if (m_validTypes[i] == element_type) {
                isValidType = true;
                break;
            }
        }
        
        if (!isValidType) {
            addError(VALIDATION_ERROR_INVALID_TYPE, 
                     "Invalid control type: " + element_type, 
                     rootElement->getId());
        }
        
        // 验证元素属性
        std::string id = rootElement->getId();
        if (id.empty() && element_type == "window") {
            addError(VALIDATION_ERROR_MISSING_ATTR, 
                     "Element 'window' must have 'id' attribute");
        }
        
        // 递归验证子元素
        const std::vector<XmlElement*>& children = rootElement->getChildren();
        for (size_t i = 0; i < children.size(); ++i) {
            validateElementRecursiveInternal(children[i]);
        }
    }
    
    // 返回第一个错误（如果有）
    if (!m_errors.empty()) {
        return m_errors[0];
    }
    
    return ValidationInfo(VALIDATION_OK, "");
}

ValidationInfo LayoutValidator::validateFile(const std::string& filepath) {
    XmlParser parser;
    XmlElement* root = parser.parseFile(filepath);
    
    if (!root) {
        return ValidationInfo(VALIDATION_ERROR_NO_ROOT, 
                              parser.getLastError());
    }
    
    ValidationInfo result = validate(root);
    delete root;
    return result;
}

ValidationInfo LayoutValidator::validateString(const std::string& xmlContent) {
    XmlParser parser;
    XmlElement* root = parser.parseString(xmlContent);
    
    if (!root) {
        return ValidationInfo(VALIDATION_ERROR_NO_ROOT, 
                              parser.getLastError());
    }
    
    ValidationInfo result = validate(root);
    delete root;
    return result;
}

const std::vector<ValidationInfo>& LayoutValidator::getErrors() const {
    return m_errors;
}

void LayoutValidator::clearErrors() {
    m_errors.clear();
}

std::vector<std::string> LayoutValidator::checkDuplicateIds(XmlElement* rootElement) {
    std::vector<std::string> duplicates;
    std::map<std::string, int> idMap;
    
    checkDuplicateIdsRecursive(rootElement, idMap);
    
    // 收集重复的ID
    for (std::map<std::string, int>::iterator it = idMap.begin();
         it != idMap.end(); ++it) {
        if (it->second > 1) {
            duplicates.push_back(it->first);
        }
    }
    
    return duplicates;
}

void LayoutValidator::checkDuplicateIdsRecursive(XmlElement* element, 
                                                  std::map<std::string, int>& idMap) {
    if (!element) {
        return;
    }
    
    std::string id = element->getId();
    if (!id.empty()) {
        idMap[id]++;
    }
    
    // 递归检查子元素
    const std::vector<XmlElement*>& children = element->getChildren();
    for (size_t i = 0; i < children.size(); ++i) {
        checkDuplicateIdsRecursive(children[i], idMap);
    }
}

bool LayoutValidator::validateElementType(XmlElement* element) {
    if (!element) {
        return false;
    }
    
    std::string type = element->getType();
    
    // 检查类型是否有效
    for (size_t i = 0; i < m_validTypes.size(); ++i) {
        if (m_validTypes[i] == type) {
            return true;
        }
    }
    
    addError(VALIDATION_ERROR_INVALID_TYPE, 
             "Invalid control type: " + type, 
             element->getId());
    
    return false;
}

bool LayoutValidator::validateElementAttributes(XmlElement* element) {
    if (!element) {
        return false;
    }
    
    // 检查必需的ID属性
    std::string id = element->getId();
    if (id.empty()) {
        // 根元素必须有ID，其他元素可选
        if (element->getType() == "window") {
            addError(VALIDATION_ERROR_MISSING_ATTR, 
                     "Element 'window' must have 'id' attribute");
            return false;
        }
    }
    
    return true;
}

void LayoutValidator::addError(ValidationError error, const std::string& message, 
                              const std::string& elementId) {
    m_errors.push_back(ValidationInfo(error, message, elementId));
}



void LayoutValidator::initValidTypes() {
    // 基础控件
    m_validTypes.push_back("window");
    m_validTypes.push_back("button");
    m_validTypes.push_back("edit");
    m_validTypes.push_back("label");
    
    // 复杂控件
    m_validTypes.push_back("list");
    m_validTypes.push_back("tree");
    m_validTypes.push_back("grid");
    
    // 菜单系统
    m_validTypes.push_back("menu");
    m_validTypes.push_back("toolbar");
    m_validTypes.push_back("statusbar");
    
    // 容器
    m_validTypes.push_back("panel");
    m_validTypes.push_back("group");
    
    // 其他
    m_validTypes.push_back("image");
}

void LayoutValidator::validateElementRecursiveInternal(XmlElement* element) {
    if (!element) {
        return;
    }
    
    // 验证元素类型 - 内联实现
    std::string element_type = element->getType();
    bool isValidType = false;
    for (size_t i = 0; i < m_validTypes.size(); ++i) {
        if (m_validTypes[i] == element_type) {
            isValidType = true;
            break;
        }
    }
    
    if (!isValidType) {
        addError(VALIDATION_ERROR_INVALID_TYPE, 
                 "Invalid control type: " + element_type, 
                 element->getId());
    }
    
    // 验证元素属性 - 内联实现
    std::string id = element->getId();
    if (id.empty()) {
        // 根元素必须有ID，其他元素可选
        if (element_type == "window") {
            addError(VALIDATION_ERROR_MISSING_ATTR, 
                     "Element 'window' must have 'id' attribute");
        }
    }
    
    // 递归验证子元素
    const std::vector<XmlElement*>& children = element->getChildren();
    for (size_t i = 0; i < children.size(); ++i) {
        validateElementRecursiveInternal(children[i]);
    }
}

} // namespace Xml
} // namespace LuaUI
