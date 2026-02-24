#pragma once

#include "Controls.h"
#include "Types.h"
#include "lua/LuaSandbox.h"
#include "tinyxml2.h"
#include <string>
#include <memory>
#include <functional>

namespace luaui {
namespace mvvm {

// 简化的 XML 布局加载器
class SimpleXmlLoader {
public:
    SimpleXmlLoader();
    ~SimpleXmlLoader();
    
    // 加载 XML 并创建 UI
    std::shared_ptr<luaui::Control> Load(const std::string& xmlPath, lua_State* L);
    
    // 获取最后错误
    std::string GetLastError() const { return m_lastError; }
    
private:
    lua_State* m_luaState = nullptr;
    std::string m_lastError;
    
    // 解析元素
    std::shared_ptr<luaui::Control> ParseElement(tinyxml2::XMLElement* element);
    
    // 创建控件
    std::shared_ptr<luaui::controls::StackPanel> CreateStackPanel(tinyxml2::XMLElement* elem);
    std::shared_ptr<luaui::controls::Button> CreateButton(tinyxml2::XMLElement* elem);
    std::shared_ptr<luaui::controls::TextBlock> CreateTextBlock(tinyxml2::XMLElement* elem);
    std::shared_ptr<luaui::controls::CheckBox> CreateCheckBox(tinyxml2::XMLElement* elem);
    std::shared_ptr<luaui::controls::Border> CreateBorder(tinyxml2::XMLElement* elem);
    
    // 应用通用属性
    void ApplyCommonProperties(std::shared_ptr<luaui::Control> control, tinyxml2::XMLElement* elem);
    
    // 解析绑定表达式
    std::string ParseBinding(const std::string& value);
    
    // 获取 ViewModel 属性值
    std::string GetViewModelProperty(const std::string& propertyName);
    
    // 颜色解析
    luaui::rendering::Color ParseColor(const std::string& colorStr);
};

} // namespace mvvm
} // namespace luaui
