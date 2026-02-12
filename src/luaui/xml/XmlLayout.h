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

// 事件处理器类型定义
using ClickHandler = std::function<void()>;
using ValueChangedHandler = std::function<void(double)>;
using TextChangedHandler = std::function<void(const std::wstring&)>;

// ============================================================================
// IXmlLoader - XML 布局加载器接口（支持声明式事件绑定）
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
    
    // ========== 声明式事件绑定 ==========
    
    // 注册 Click 事件处理器（通过方法名）
    virtual void RegisterClickHandler(const std::string& methodName, ClickHandler handler) = 0;
    
    // 注册 ValueChanged 事件处理器
    virtual void RegisterValueChangedHandler(const std::string& methodName, ValueChangedHandler handler) = 0;
    
    // 注册 TextChanged 事件处理器
    virtual void RegisterTextChangedHandler(const std::string& methodName, TextChangedHandler handler) = 0;
    
    // 批量注册事件处理器（从对象实例自动提取）
    template<typename T>
    void BindEventHandlers(T* instance) {
        // 这个模板方法在头文件中实现，允许自动绑定
        BindEventsFromInstance(instance);
    }
    
protected:
    // 子类需要实现的内部方法
    virtual void BindEventsFromInstance(void* instance) = 0;
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
