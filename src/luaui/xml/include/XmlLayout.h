#pragma once
#include "Control.h"
#include "layout.h"
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>
#include <any>

namespace luaui {
namespace xml {

// 前向声明
class IXmlLoader;
class IElementFactory;
class IPropertySetter;

using IXmlLoaderPtr = std::shared_ptr<IXmlLoader>;

// ============================================================================
// IXmlLoader - XML 布局加载器接口
// ============================================================================
class IXmlLoader {
public:
    virtual ~IXmlLoader() = default;
    
    // 从文件加载
    virtual controls::ControlPtr Load(const std::string& filePath) = 0;
    
    // 从字符串加载
    virtual controls::ControlPtr LoadFromString(const std::string& xmlString) = 0;
    
    // 注册自定义控件
    virtual void RegisterElement(const std::string& tagName, 
                                  std::function<controls::ControlPtr()> factory) = 0;
    
    // 获取元素工厂
    virtual IElementFactory* GetElementFactory() = 0;
};

// 创建 XML 加载器实例
IXmlLoaderPtr CreateXmlLoader();

// ============================================================================
// IElementFactory - 控件工厂接口
// ============================================================================
class IElementFactory {
public:
    virtual ~IElementFactory() = default;
    
    // 注册控件创建函数
    virtual void Register(const std::string& tagName, 
                          std::function<controls::ControlPtr()> factory) = 0;
    
    // 创建控件
    virtual controls::ControlPtr Create(const std::string& tagName) = 0;
    
    // 检查是否已注册
    virtual bool IsRegistered(const std::string& tagName) const = 0;
};

// ============================================================================
// IPropertySetter - 属性设置器接口
// ============================================================================
class IPropertySetter {
public:
    virtual ~IPropertySetter() = default;
    
    // 设置属性值（字符串值，需要转换）
    virtual bool SetProperty(controls::Control* control, 
                             const std::string& propertyName, 
                             const std::string& value) = 0;
    
    // 设置内容属性（子元素文本内容）
    virtual bool SetContent(controls::Control* control, 
                            const std::string& content) = 0;
};

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
// 内置标记扩展处理
// ============================================================================

// {x:Name} - 命名元素
class XNameExtension {
public:
    static std::string Parse(const std::string& value);
};

// {StaticResource} - 静态资源引用（简化版，暂不实现资源字典）
class StaticResourceExtension {
public:
    static bool IsResourceReference(const std::string& value);
    static std::string ParseKey(const std::string& value);
};

// ============================================================================
// 辅助函数
// ============================================================================

// 字符串转类型
namespace TypeConverter {
    bool ToBool(const std::string& str, bool& out);
    bool ToFloat(const std::string& str, float& out);
    bool ToDouble(const std::string& str, double& out);
    bool ToInt(const std::string& str, int& out);
    bool ToColor(const std::string& str, rendering::Color& out);
    bool ToThickness(const std::string& str, rendering::Thickness& out);
    rendering::Color HexToColor(const std::string& hex);
}

} // namespace xml
} // namespace luaui
