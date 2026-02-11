#pragma once

#include "Controls.h"
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>
#include <any>

namespace luaui {
namespace xml {

// 前向声明
class IXmlLoader;
using IXmlLoaderPtr = std::shared_ptr<IXmlLoader>;

// ============================================================================
// IXmlLoader - XML 布局加载器接口（新架构简化版）
// ============================================================================
class IXmlLoader {
public:
    virtual ~IXmlLoader() = default;
    
    // 从文件加载
    virtual std::shared_ptr<Control> Load(const std::string& filePath) = 0;
    
    // 从字符串加载
    virtual std::shared_ptr<Control> LoadFromString(const std::string& xmlString) = 0;
    
    // 注册自定义控件工厂
    virtual void RegisterElement(const std::string& tagName, 
                                  std::function<std::shared_ptr<Control>()> factory) = 0;
};

// 创建 XML 加载器实例
IXmlLoaderPtr CreateXmlLoader();

// ============================================================================
// XmlLayoutException - XML 布局异常
// ============================================================================
class XmlLayoutException : public std::exception {
public:
    XmlLayoutException(const std::string& message, int line = 0, int column = 0)
        : m_message(message), m_line(line), m_column(column) {}
    
    const char* what() const noexcept override {
        return m_message.c_str();
    }
    
    int GetLine() const { return m_line; }
    int GetColumn() const { return m_column; }
    
private:
    std::string m_message;
    int m_line;
    int m_column;
};

// ============================================================================
// 辅助函数 - 类型转换
// ============================================================================
namespace TypeConverter {
    bool ToBool(const std::string& str, bool& out);
    bool ToFloat(const std::string& str, float& out);
    bool ToInt(const std::string& str, int& out);
    bool ToColor(const std::string& str, rendering::Color& out);
    rendering::Color HexToColor(const std::string& hex);
}

} // namespace xml
} // namespace luaui
