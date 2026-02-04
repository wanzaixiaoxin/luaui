/**
 * @file LayoutValidator.h
 * @brief 布局验证器
 * @details 验证XML布局的正确性和完整性
 * @version 1.0.0
 */

#ifndef LUAUI_LAYOUTVALIDATOR_H
#define LUAUI_LAYOUTVALIDATOR_H

#include <string>
#include <vector>
#include "xml/parser/XmlParser.h"

namespace LuaUI {
namespace Xml {

/**
 * @brief 验证错误类型
 */
enum ValidationError {
    VALIDATION_OK,                  ///< 验证通过
    VALIDATION_ERROR_NO_ROOT,       ///< 缺少根元素
    VALIDATION_ERROR_DUPLICATE_ID,  ///< 重复的ID
    VALIDATION_ERROR_INVALID_TYPE,  ///< 无效的控件类型
    VALIDATION_ERROR_INVALID_ATTR,  ///< 无效的属性
    VALIDATION_ERROR_MISSING_ATTR,  ///< 缺少必需属性
    VALIDATION_ERROR_INVALID_LAYOUT ///< 无效的布局
};

/**
 * @brief 验证错误信息
 */
struct ValidationInfo {
    ValidationError error;   ///< 错误类型
    std::string message;     ///< 错误消息
    std::string elementId;   ///< 出错元素ID
    
    /**
     * @brief 构造函数
     */
    ValidationInfo()
        : error(VALIDATION_OK)
    {
    }
    
    /**
     * @brief 构造函数
     * @param err 错误类型
     * @param msg 错误消息
     * @param elemId 元素ID
     */
    ValidationInfo(ValidationError err, const std::string& msg, const std::string& elemId = "")
        : error(err)
        , message(msg)
        , elementId(elemId)
    {
    }
};

/**
 * @brief 布局验证器类
 */
class LayoutValidator {
public:
    /**
     * @brief 构造函数
     */
    LayoutValidator();
    
    /**
     * @brief 析构函数
     */
    ~LayoutValidator();
    
    /**
     * @brief 验证布局
     * @param rootElement 根元素
     * @return 验证信息，如果验证通过则error为VALIDATION_OK
     */
    ValidationInfo validate(XmlElement* rootElement);
    
    /**
     * @brief 验证XML文件
     * @param filepath XML文件路径
     * @return 验证信息
     */
    ValidationInfo validateFile(const std::string& filepath);
    
    /**
     * @brief 验证XML字符串
     * @param xmlContent XML内容
     * @return 验证信息
     */
    ValidationInfo validateString(const std::string& xmlContent);
    
    /**
     * @brief 获取所有验证错误
     * @return 错误列表
     */
    const std::vector<ValidationInfo>& getErrors() const;
    
    /**
     * @brief 清除错误信息
     */
    void clearErrors();
    
private:
    std::vector<ValidationInfo> m_errors;     ///< 错误列表
    std::vector<std::string> m_validTypes;    ///< 有效的控件类型列表
    
    /**
     * @brief 检查ID是否重复
     * @param rootElement 根元素
     * @return 重复的ID列表
     */
    std::vector<std::string> checkDuplicateIds(XmlElement* rootElement);
    
    /**
     * @brief 递归检查重复ID
     * @param element 元素
     * @param idMap ID映射表
     */
    void checkDuplicateIdsRecursive(XmlElement* element, 
                                     std::map<std::string, int>& idMap);
    
    /**
     * @brief 验证元素类型
     * @param element 元素
     * @return 验证结果
     */
    bool validateElementType(XmlElement* element);
    
    /**
     * @brief 验证元素属性
     * @param element 元素
     * @return 验证结果
     */
    bool validateElementAttributes(XmlElement* element);
    
    /**
     * @brief 添加错误信息
     * @param error 错误类型
     * @param message 错误消息
     * @param elementId 元素ID
     */
    void addError(ValidationError error, const std::string& message, 
                  const std::string& elementId = "");
    
    /**
     * @brief 初始化有效的控件类型列表
     */
    void initValidTypes();
};

} // namespace Xml
} // namespace LuaUI

#endif // LUAUI_LAYOUTVALIDATOR_H
