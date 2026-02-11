#include "XmlLayout.h"
#include "tinyxml2.h"
#include "Logger.h"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace luaui {
namespace xml {

using namespace controls;
using namespace rendering;

// 辅助函数：清理字符串
std::string Trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

// ============================================================================
// XmlLoader 实现
// ============================================================================
class XmlLoader : public IXmlLoader {
public:
    XmlLoader() {
        RegisterDefaultElements();
    }
    
    std::shared_ptr<luaui::Control> Load(const std::string& filePath) override {
        tinyxml2::XMLDocument doc;
        if (doc.LoadFile(filePath.c_str()) != tinyxml2::XML_SUCCESS) {
            throw XmlLayoutException("Failed to load XML file: " + filePath);
        }
        return LoadElement(doc.RootElement());
    }
    
    std::shared_ptr<luaui::Control> LoadFromString(const std::string& xmlString) override {
        tinyxml2::XMLDocument doc;
        if (doc.Parse(xmlString.c_str()) != tinyxml2::XML_SUCCESS) {
            throw XmlLayoutException("Failed to parse XML string");
        }
        return LoadElement(doc.RootElement());
    }
    
    void RegisterElement(const std::string& tagName, 
                         std::function<std::shared_ptr<luaui::Control>()> factory) override {
        m_factories[tagName] = factory;
    }

private:
    std::unordered_map<std::string, std::function<std::shared_ptr<luaui::Control>()>> m_factories;
    
    void RegisterDefaultElements() {
        // 注册基本控件
        RegisterElement("StackPanel", []() { return std::make_shared<StackPanel>(); });
        RegisterElement("Panel", []() { return std::make_shared<Panel>(); });
        RegisterElement("Button", []() { return std::make_shared<Button>(); });
        RegisterElement("TextBlock", []() { return std::make_shared<TextBlock>(); });
        RegisterElement("TextBox", []() { return std::make_shared<TextBox>(); });
        RegisterElement("Border", []() { return std::make_shared<Border>(); });
        RegisterElement("CheckBox", []() { return std::make_shared<CheckBox>(); });
        RegisterElement("Slider", []() { return std::make_shared<Slider>(); });
        RegisterElement("ProgressBar", []() { return std::make_shared<ProgressBar>(); });
        RegisterElement("ListBox", []() { return std::make_shared<ListBox>(); });
        RegisterElement("Rectangle", []() { return std::make_shared<Rectangle>(); });
        RegisterElement("Ellipse", []() { return std::make_shared<Ellipse>(); });
    }
    
    std::shared_ptr<luaui::Control> LoadElement(const tinyxml2::XMLElement* element) {
        if (!element) return nullptr;
        
        std::string tagName = element->Name();
        auto it = m_factories.find(tagName);
        if (it == m_factories.end()) {
            throw XmlLayoutException("Unknown element: " + tagName);
        }
        
        auto control = it->second();
        if (!control) return nullptr;
        
        // 设置属性
        ApplyAttributes(control, element);
        
        // 处理子元素
        LoadChildren(control, element);
        
        return control;
    }
    
    void ApplyAttributes(const std::shared_ptr<luaui::Control>& control, 
                         const tinyxml2::XMLElement* element) {
        if (!control || !element) return;
        
        for (const tinyxml2::XMLAttribute* attr = element->FirstAttribute(); 
             attr; attr = attr->Next()) {
            std::string name = attr->Name();
            std::string value = attr->Value();
            
            // Name 属性
            if (name == "Name" || name == "x:Name") {
                control->SetName(value);
            }
            // 宽度
            else if (name == "Width") {
                float width;
                if (TypeConverter::ToFloat(value, width)) {
                    if (auto* layout = control->GetLayout()) {
                        layout->SetWidth(width);
                    }
                }
            }
            // 高度
            else if (name == "Height") {
                float height;
                if (TypeConverter::ToFloat(value, height)) {
                    if (auto* layout = control->GetLayout()) {
                        layout->SetHeight(height);
                    }
                }
            }
            // 背景色
            else if (name == "Background") {
                Color color;
                if (TypeConverter::ToColor(value, color)) {
                    if (auto* render = control->GetRender()) {
                        render->SetBackground(color);
                    }
                }
            }
        }
    }
    
    void LoadChildren(const std::shared_ptr<luaui::Control>& parent, 
                      const tinyxml2::XMLElement* element) {
        // 尝试作为 Panel 添加子元素
        auto panel = std::dynamic_pointer_cast<controls::Panel>(parent);
        if (!panel) return;
        
        for (const tinyxml2::XMLElement* childElem = element->FirstChildElement();
             childElem; childElem = childElem->NextSiblingElement()) {
            auto child = LoadElement(childElem);
            if (child) {
                panel->AddChild(child);
            }
        }
    }
};

// 创建 XML 加载器
IXmlLoaderPtr CreateXmlLoader() {
    return std::make_shared<XmlLoader>();
}

} // namespace xml
} // namespace luaui
