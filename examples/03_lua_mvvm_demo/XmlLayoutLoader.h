#pragma once

#include "Controls.h"
#include "lua/LuaSandbox.h"
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>

namespace luaui {
namespace mvvm {

// 绑定信息
struct BindingInfo {
    std::string targetProperty;    // UI 属性 (Text, IsChecked, etc.)
    std::string sourcePath;        // ViewModel 路径 (Counter, Title, etc.)
    std::string mode;              // OneWay, TwoWay, OneTime
};

// 命令绑定信息
struct CommandBindingInfo {
    std::string eventName;         // Click, CheckedChanged, etc.
    std::string commandName;       // ViewModel 命令名
};

// 样式定义
struct StyleDefinition {
    std::string targetType;
    std::unordered_map<std::string, std::string> setters;
};

// XML 布局加载器 - 支持 MVVM 数据绑定
class XmlLayoutLoader {
public:
    XmlLayoutLoader();
    ~XmlLayoutLoader();
    
    // 加载 XML 布局并创建 UI
    bool Load(const std::string& xmlPath);
    
    // 设置 DataContext (ViewModel Lua 表名)
    void SetDataContext(lua_State* L, const std::string& viewModelName);
    
    // 获取根控件
    std::shared_ptr<luaui::Control> GetRoot() const { return m_root; }
    
    // 应用所有绑定 (在 ViewModel 准备好后调用)
    bool ApplyBindings();
    
    // 更新绑定 (当 ViewModel 属性变更时调用)
    void UpdateBinding(const std::string& propertyName);
    
private:
    // 解析 XML
    bool ParseXml(const std::string& xmlContent);
    
    // 创建控件树
    std::shared_ptr<luaui::Control> CreateControl(tinyxml2::XMLElement* element);
    
    // 应用样式
    void ApplyStyle(std::shared_ptr<luaui::Control> control, const std::string& styleKey);
    void ApplyInlineProperties(std::shared_ptr<luaui::Control> control, tinyxml2::XMLElement* element);
    
    // 解析绑定表达式 {Binding PropertyName, Mode=TwoWay}
    BindingInfo ParseBinding(const std::string& bindingExpr);
    
    // 解析命令绑定
    CommandBindingInfo ParseCommand(const std::string& commandExpr);
    
    // 获取资源 (颜色、样式等)
    std::string GetResource(const std::string& key);
    
    // 创建特定控件类型
    std::shared_ptr<luaui::controls::StackPanel> CreateStackPanel(tinyxml2::XMLElement* element);
    std::shared_ptr<luaui::controls::Button> CreateButton(tinyxml2::XMLElement* element);
    std::shared_ptr<luaui::controls::TextBlock> CreateTextBlock(tinyxml2::XMLElement* element);
    std::shared_ptr<luaui::controls::CheckBox> CreateCheckBox(tinyxml2::XMLElement* element);
    std::shared_ptr<luaui::controls::Border> CreateBorder(tinyxml2::XMLElement* element);
    std::shared_ptr<luaui::controls::DataGrid> CreateDataGrid(tinyxml2::XMLElement* element);
    
    // 颜色转换
    Color ParseColor(const std::string& colorStr);
    
    // 成员变量
    lua_State* m_luaState = nullptr;
    std::string m_viewModelName;
    std::shared_ptr<luaui::Control> m_root;
    
    // 样式和资源
    std::unordered_map<std::string, StyleDefinition> m_styles;
    std::unordered_map<std::string, std::string> m_resources;
    
    // 绑定列表 (控件 -> 绑定信息)
    struct ControlBinding {
        std::weak_ptr<luaui::Control> control;
        BindingInfo binding;
        CommandBindingInfo command;
    };
    std::vector<ControlBinding> m_bindings;
    
    std::string m_lastError;
};

} // namespace mvvm
} // namespace luaui
