#include "XmlLayout.h"
#include "tinyxml2.h"
#include "Logger.h"
#include "layouts/Grid.h"
#include "TextBlock.h"
#include "TextBox.h"
#include "Slider.h"
#include "Button.h"
#include "Border.h"
#include "CheckBox.h"      // CheckBox and RadioButton
#include "ListBox.h"       // ListBox and ListBoxItem
#include "Image.h"
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
    
    // ========== 事件处理器注册 ==========
    
    void RegisterClickHandler(const std::string& methodName, ClickHandler handler) override {
        m_clickHandlers[methodName] = handler;
    }
    
    void RegisterValueChangedHandler(const std::string& methodName, ValueChangedHandler handler) override {
        m_valueChangedHandlers[methodName] = handler;
    }
    
    void RegisterTextChangedHandler(const std::string& methodName, TextChangedHandler handler) override {
        m_textChangedHandlers[methodName] = handler;
    }
    
protected:
    void BindEventsFromInstance(void* instance) override {
        // 基类实现为空，具体绑定由外部完成
    }
    
private:
    std::unordered_map<std::string, std::function<std::shared_ptr<luaui::Control>()>> m_factories;
    std::unordered_map<std::string, ClickHandler> m_clickHandlers;
    std::unordered_map<std::string, ValueChangedHandler> m_valueChangedHandlers;
    std::unordered_map<std::string, TextChangedHandler> m_textChangedHandlers;
    
    void RegisterDefaultElements() {
        RegisterElement("StackPanel", []() { return std::make_shared<StackPanel>(); });
        RegisterElement("Panel", []() { return std::make_shared<Panel>(); });
        RegisterElement("Grid", []() { return std::make_shared<Grid>(); });
        RegisterElement("Button", []() { return std::make_shared<Button>(); });
        RegisterElement("TextBlock", []() { return std::make_shared<TextBlock>(); });
        RegisterElement("TextBox", []() { return std::make_shared<TextBox>(); });
        RegisterElement("Border", []() { return std::make_shared<Border>(); });
        RegisterElement("CheckBox", []() { return std::make_shared<CheckBox>(); });
        RegisterElement("RadioButton", []() { return std::make_shared<RadioButton>(); });
        RegisterElement("Slider", []() { return std::make_shared<Slider>(); });
        RegisterElement("ProgressBar", []() { return std::make_shared<ProgressBar>(); });
        RegisterElement("ListBox", []() { return std::make_shared<ListBox>(); });
        RegisterElement("ListBoxItem", []() { return std::make_shared<ListBoxItem>(); });
        RegisterElement("Image", []() { return std::make_shared<Image>(); });
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
        
        // 设置属性（包括事件绑定）
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
            // 外边距 Margin (简化为统一值)
            else if (name == "Margin") {
                float margin;
                if (TypeConverter::ToFloat(value, margin)) {
                    if (auto* layout = control->GetLayout()) {
                        layout->SetMargin(margin, margin, margin, margin);
                    }
                }
            }
            // 内边距 Padding (简化为统一值)
            else if (name == "Padding") {
                float padding;
                if (TypeConverter::ToFloat(value, padding)) {
                    if (auto* layout = control->GetLayout()) {
                        layout->SetPadding(padding, padding, padding, padding);
                    }
                }
            }
            // 背景色
            else if (name == "Background") {
                Color color;
                if (TypeConverter::ToColor(value, color)) {
                    // For Button, use SetStateColors to set all states
                    if (auto btn = std::dynamic_pointer_cast<controls::Button>(control)) {
                        // Auto-generate hover (lighter) and pressed (darker) colors
                        auto hover = Color(color.r * 1.15f, color.g * 1.15f, color.b * 1.15f, color.a);
                        auto pressed = Color(color.r * 0.85f, color.g * 0.85f, color.b * 0.85f, color.a);
                        btn->SetStateColors(color, hover, pressed);
                    } else if (auto* render = control->GetRender()) {
                        render->SetBackground(color);
                    }
                }
            }
            // SourcePath (Image)
            else if (name == "SourcePath") {
                std::wstring wpath(value.begin(), value.end());
                if (auto img = std::dynamic_pointer_cast<controls::Image>(control)) {
                    img->SetSourcePath(wpath);
                }
            }
            // Stretch (Image)
            else if (name == "Stretch") {
                if (auto img = std::dynamic_pointer_cast<controls::Image>(control)) {
                    if (value == "None") {
                        img->SetStretch(controls::Stretch::None);
                    } else if (value == "Fill") {
                        img->SetStretch(controls::Stretch::Fill);
                    } else if (value == "UniformToFill") {
                        img->SetStretch(controls::Stretch::UniformToFill);
                    } else {
                        // 默认 Uniform
                        img->SetStretch(controls::Stretch::Uniform);
                    }
                }
            }
            // Spacing (StackPanel)
            else if (name == "Spacing") {
                float spacing;
                if (TypeConverter::ToFloat(value, spacing)) {
                    if (auto stack = std::dynamic_pointer_cast<controls::StackPanel>(control)) {
                        stack->SetSpacing(spacing);
                    }
                }
            }
            // Orientation (StackPanel)
            else if (name == "Orientation") {
                if (auto stack = std::dynamic_pointer_cast<controls::StackPanel>(control)) {
                    if (value == "Horizontal") {
                        stack->SetOrientation(controls::StackPanel::Orientation::Horizontal);
                    } else if (value == "Vertical") {
                        stack->SetOrientation(controls::StackPanel::Orientation::Vertical);
                    }
                }
            }
            // Text (TextBlock, TextBox, CheckBox, RadioButton)
            else if (name == "Text") {
                std::wstring wtext(value.begin(), value.end());
                if (auto tb = std::dynamic_pointer_cast<controls::TextBlock>(control)) {
                    tb->SetText(wtext);
                } else if (auto tx = std::dynamic_pointer_cast<controls::TextBox>(control)) {
                    tx->SetText(wtext);
                } else if (auto cb = std::dynamic_pointer_cast<controls::CheckBox>(control)) {
                    cb->SetText(wtext);
                } else if (auto rb = std::dynamic_pointer_cast<controls::RadioButton>(control)) {
                    rb->SetText(wtext);
                }
            }
            // FontSize
            else if (name == "FontSize") {
                float size;
                if (TypeConverter::ToFloat(value, size)) {
                    if (auto tb = std::dynamic_pointer_cast<controls::TextBlock>(control)) {
                        tb->SetFontSize(size);
                    }
                }
            }
            // Value (Slider, ProgressBar)
            else if (name == "Value") {
                float val;
                if (TypeConverter::ToFloat(value, val)) {
                    if (auto s = std::dynamic_pointer_cast<controls::Slider>(control)) {
                        s->SetValue(val);
                    } else if (auto p = std::dynamic_pointer_cast<controls::ProgressBar>(control)) {
                        p->SetValue(val);
                    }
                }
            }
            // Minimum (Slider)
            else if (name == "Minimum") {
                float min;
                if (TypeConverter::ToFloat(value, min)) {
                    if (auto s = std::dynamic_pointer_cast<controls::Slider>(control)) {
                        s->SetMinimum(min);
                    }
                }
            }
            // Maximum (Slider)
            else if (name == "Maximum") {
                float max;
                if (TypeConverter::ToFloat(value, max)) {
                    if (auto s = std::dynamic_pointer_cast<controls::Slider>(control)) {
                        s->SetMaximum(max);
                    }
                }
            }
            // GroupName (RadioButton)
            else if (name == "GroupName") {
                if (auto rb = std::dynamic_pointer_cast<controls::RadioButton>(control)) {
                    rb->SetGroupName(value);
                }
            }
            // IsChecked (CheckBox, RadioButton)
            else if (name == "IsChecked") {
                if (value == "True" || value == "true" || value == "1") {
                    if (auto cb = std::dynamic_pointer_cast<controls::CheckBox>(control)) {
                        cb->SetIsChecked(true);
                    } else if (auto rb = std::dynamic_pointer_cast<controls::RadioButton>(control)) {
                        rb->SetIsChecked(true);
                    }
                }
            }
            // SetStateColors (Button) - 格式: "normal,hover,pressed"
            else if (name == "SetStateColors") {
                std::stringstream ss(value);
                std::string colorStr;
                std::vector<Color> colors;
                while (std::getline(ss, colorStr, ',')) {
                    Color c;
                    if (TypeConverter::ToColor(Trim(colorStr), c)) {
                        colors.push_back(c);
                    }
                }
                if (colors.size() >= 3) {
                    if (auto btn = std::dynamic_pointer_cast<controls::Button>(control)) {
                        btn->SetStateColors(colors[0], colors[1], colors[2]);
                    }
                }
            }
            // ========== 声明式事件绑定 ==========
            // Click 事件 - XML 中写: Click="OnSaveClick"
            else if (name == "Click") {
                auto it = m_clickHandlers.find(value);
                if (it != m_clickHandlers.end()) {
                    if (auto btn = std::dynamic_pointer_cast<controls::Button>(control)) {
                        btn->Click.Add([handler = it->second](luaui::Control*) { handler(); });
                    }
                } else {
                    luaui::utils::Logger::WarningF("[XML] Click handler '%s' not found", value.c_str());
                }
            }
            // ValueChanged 事件 - XML 中写: ValueChanged="OnVolumeChanged"
            else if (name == "ValueChanged") {
                auto it = m_valueChangedHandlers.find(value);
                if (it != m_valueChangedHandlers.end()) {
                    if (auto slider = std::dynamic_pointer_cast<controls::Slider>(control)) {
                        slider->ValueChanged.Add([handler = it->second](controls::Slider*, double val) { 
                            handler(val); 
                        });
                    }
                } else {
                    luaui::utils::Logger::WarningF("[XML] ValueChanged handler '%s' not found", value.c_str());
                }
            }
        }
    }
    
    void LoadChildren(const std::shared_ptr<luaui::Control>& parent, 
                      const tinyxml2::XMLElement* element) {
        // 特殊处理 Border - 使用 SetChild 而不是 AddChild
        if (auto border = std::dynamic_pointer_cast<controls::Border>(parent)) {
            if (const tinyxml2::XMLElement* childElem = element->FirstChildElement()) {
                std::string tagName = childElem->Name();
                auto it = m_factories.find(tagName);
                if (it != m_factories.end()) {
                    auto child = it->second();
                    if (child) {
                        ApplyAttributes(child, childElem);
                        border->SetChild(child);
                        LoadChildren(child, childElem);
                    }
                }
            }
            return;
        }
        
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
