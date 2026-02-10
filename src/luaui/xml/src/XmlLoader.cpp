#include "XmlLayout.h"
#include "tinyxml2.h"
#include "Logger.h"
#include "layout.h"
#include "Grid.h"
#include "RangeControls.h"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace luaui {
namespace xml {

using namespace controls;
using namespace rendering;

// 辅助函数：清理字符串中的换行符和多余空格
std::string Trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

// ============================================================================
// ElementFactory 实现
// ============================================================================
class ElementFactory : public IElementFactory {
public:
    ElementFactory() {
        RegisterDefaultElements();
    }
    
    void Register(const std::string& tagName, std::function<ControlPtr()> factory) override {
        m_factories[tagName] = factory;
    }
    
    ControlPtr Create(const std::string& tagName) override {
        auto it = m_factories.find(tagName);
        if (it != m_factories.end()) {
            return it->second();
        }
        return nullptr;
    }
    
    bool IsRegistered(const std::string& tagName) const override {
        return m_factories.find(tagName) != m_factories.end();
    }

private:
    void RegisterDefaultElements() {
        // 注册所有内置控件
        Register("StackPanel", []() { return std::make_shared<StackPanel>(); });
        Register("Grid", []() { return std::make_shared<Grid>(); });
        Register("Canvas", []() { return std::make_shared<Canvas>(); });
        Register("DockPanel", []() { return std::make_shared<DockPanel>(); });
        Register("WrapPanel", []() { return std::make_shared<WrapPanel>(); });
        
        Register("Border", []() { return std::make_shared<Border>(); });
        
        Register("Button", []() { return std::make_shared<Button>(); });
        Register("TextBlock", []() { return std::make_shared<TextBlock>(); });
        Register("TextBox", []() { return std::make_shared<TextBox>(); });
        
        // 范围控件
        Register("Slider", []() { return std::make_shared<Slider>(); });
        Register("ProgressBar", []() { return std::make_shared<ProgressBar>(); });
        
        // 形状控件（如果可用）
        // Register("Rectangle", []() { return std::make_shared<Rectangle>(); });
        // Register("Ellipse", []() { return std::make_shared<Ellipse>(); });
    }
    
    std::unordered_map<std::string, std::function<ControlPtr()>> m_factories;
};

// ============================================================================
// PropertySetter 实现
// ============================================================================
class PropertySetter : public IPropertySetter {
public:
    bool SetProperty(Control* control, const std::string& propertyName, 
                     const std::string& value) override {
        if (!control) return false;
        
        // 检查是否是标记扩展
        if (value.find('{') == 0 && value.find('}') == value.length() - 1) {
            return HandleMarkupExtension(control, propertyName, value);
        }
        
        // 根据控件类型和属性名设置值
        std::string tagName = GetTagName(control);
        
        // 通用属性
        if (propertyName == "Name" || propertyName == "x:Name") {
            // x:Name 特殊处理，存储在控件的 Tag 中供后续查找
            control->SetName(value);
            return true;
        }
        
        if (propertyName == "Width") {
            float width;
            if (TypeConverter::ToFloat(value, width)) {
                control->SetWidth(width);
                return true;
            }
        }
        else if (propertyName == "Height") {
            float height;
            if (TypeConverter::ToFloat(value, height)) {
                control->SetHeight(height);
                return true;
            }
        }
        else if (propertyName == "MinWidth") {
            float minWidth;
            if (TypeConverter::ToFloat(value, minWidth)) {
                control->SetMinWidth(minWidth);
                return true;
            }
        }
        else if (propertyName == "MinHeight") {
            float minHeight;
            if (TypeConverter::ToFloat(value, minHeight)) {
                control->SetMinHeight(minHeight);
                return true;
            }
        }
        else if (propertyName == "MaxWidth") {
            float maxWidth;
            if (TypeConverter::ToFloat(value, maxWidth)) {
                control->SetMaxWidth(maxWidth);
                return true;
            }
        }
        else if (propertyName == "MaxHeight") {
            float maxHeight;
            if (TypeConverter::ToFloat(value, maxHeight)) {
                control->SetMaxHeight(maxHeight);
                return true;
            }
        }
        else if (propertyName == "Margin") {
            Thickness margin;
            if (TypeConverter::ToThickness(value, margin)) {
                control->SetMargin(margin.left, margin.top, margin.right, margin.bottom);
                return true;
            }
        }
        else if (propertyName == "Padding") {
            Thickness padding;
            if (TypeConverter::ToThickness(value, padding)) {
                control->SetPadding(padding.left, padding.top, padding.right, padding.bottom);
                return true;
            }
        }
        else if (propertyName == "Background") {
            Color color;
            if (TypeConverter::ToColor(value, color)) {
                control->SetBackground(color);
                // 调试输出
                // if (dynamic_cast<Border*>(control)) {
                //     Logger::Debug("Border Background set to: {},{},{},{}", color.r, color.g, color.b, color.a);
                // }
                return true;
            }
        }
        else if (propertyName == "Opacity") {
            float opacity;
            if (TypeConverter::ToFloat(value, opacity)) {
                control->SetOpacity(opacity);
                return true;
            }
        }
        else if (propertyName == "IsVisible") {
            bool visible;
            if (TypeConverter::ToBool(value, visible)) {
                control->SetIsVisible(visible);
                return true;
            }
        }
        
        // StackPanel 特有属性
        if (auto stackPanel = dynamic_cast<StackPanel*>(control)) {
            if (propertyName == "Orientation") {
                // Logger::Debug("StackPanel Orientation: {}", value);
                if (value == "Horizontal" || value == "horizontal") {
                    stackPanel->SetOrientation(StackPanel::Orientation::Horizontal);
                    return true;
                } else if (value == "Vertical" || value == "vertical") {
                    stackPanel->SetOrientation(StackPanel::Orientation::Vertical);
                    return true;
                }
            }
            else if (propertyName == "Spacing") {
                float spacing;
                if (TypeConverter::ToFloat(value, spacing)) {
                    stackPanel->SetSpacing(spacing);
                    return true;
                }
            }
        }
        
        // Border 特有属性
        if (auto border = dynamic_cast<Border*>(control)) {
            if (propertyName == "BorderThickness") {
                float thickness;
                if (TypeConverter::ToFloat(value, thickness)) {
                    border->SetBorderThickness(thickness);
                    return true;
                }
            }
            else if (propertyName == "BorderBrush") {
                Color color;
                if (TypeConverter::ToColor(value, color)) {
                    border->SetBorderBrush(color);
                    return true;
                }
            }
            else if (propertyName == "CornerRadius") {
                float radius;
                if (TypeConverter::ToFloat(value, radius)) {
                    CornerRadius cr(radius, radius, radius, radius);
                    border->SetCornerRadius(cr);
                    return true;
                }
            }
        }
        
        // Button 特有属性
        if (auto button = dynamic_cast<Button*>(control)) {
            if (propertyName == "Content") {
                // 创建 TextBlock 作为内容
                auto textBlock = std::make_shared<TextBlock>();
                textBlock->SetText(std::wstring(value.begin(), value.end()));
                textBlock->SetForeground(Color::White());
                button->SetContent(textBlock);
                return true;
            }
            else if (propertyName == "SetStateColors") {
                // 解析 "normal,hover,pressed" 格式
                std::istringstream iss(value);
                std::string normalStr, hoverStr, pressedStr;
                if (std::getline(iss, normalStr, ',') && 
                    std::getline(iss, hoverStr, ',') && 
                    std::getline(iss, pressedStr, ',')) {
                    Color normal, hover, pressed;
                    if (TypeConverter::ToColor(normalStr, normal) &&
                        TypeConverter::ToColor(hoverStr, hover) &&
                        TypeConverter::ToColor(pressedStr, pressed)) {
                        button->SetStateColors(normal, hover, pressed);
                        return true;
                    }
                }
            }
        }
        
        // TextBlock 特有属性
        if (auto textBlock = dynamic_cast<TextBlock*>(control)) {
            if (propertyName == "Text") {
                textBlock->SetText(std::wstring(value.begin(), value.end()));
                return true;
            }
            else if (propertyName == "FontSize") {
                float fontSize;
                if (TypeConverter::ToFloat(value, fontSize)) {
                    textBlock->SetFontSize(fontSize);
                    return true;
                }
            }
            else if (propertyName == "Foreground") {
                Color color;
                if (TypeConverter::ToColor(value, color)) {
                    textBlock->SetForeground(color);
                    return true;
                }
            }
        }
        
        // TextBox 特有属性
        if (auto textBox = dynamic_cast<TextBox*>(control)) {
            if (propertyName == "Text") {
                textBox->SetText(std::wstring(value.begin(), value.end()));
                return true;
            }
            else if (propertyName == "Placeholder") {
                textBox->SetPlaceholder(std::wstring(value.begin(), value.end()));
                return true;
            }
        }
        
        // Slider 特有属性
        if (auto slider = dynamic_cast<Slider*>(control)) {
            if (propertyName == "Minimum") {
                double minimum;
                if (TypeConverter::ToDouble(value, minimum)) {
                    slider->SetMinimum(minimum);
                    return true;
                }
            }
            else if (propertyName == "Maximum") {
                double maximum;
                if (TypeConverter::ToDouble(value, maximum)) {
                    slider->SetMaximum(maximum);
                    return true;
                }
            }
            else if (propertyName == "Value") {
                double val;
                if (TypeConverter::ToDouble(value, val)) {
                    slider->SetValue(val);
                    return true;
                }
            }
            else if (propertyName == "Orientation") {
                if (value == "Horizontal" || value == "horizontal") {
                    slider->SetOrientation(Slider::Orientation::Horizontal);
                    return true;
                } else if (value == "Vertical" || value == "vertical") {
                    slider->SetOrientation(Slider::Orientation::Vertical);
                    return true;
                }
            }
        }
        
        // ProgressBar 特有属性
        if (auto progressBar = dynamic_cast<ProgressBar*>(control)) {
            if (propertyName == "Value") {
                double val;
                if (TypeConverter::ToDouble(value, val)) {
                    progressBar->SetValue(val);
                    return true;
                }
            }
            else if (propertyName == "ShowText") {
                bool show;
                if (TypeConverter::ToBool(value, show)) {
                    progressBar->SetShowText(show);
                    return true;
                }
            }
            else if (propertyName == "Orientation") {
                if (value == "Horizontal" || value == "horizontal") {
                    progressBar->SetOrientation(ProgressBar::Orientation::Horizontal);
                    return true;
                } else if (value == "Vertical" || value == "vertical") {
                    progressBar->SetOrientation(ProgressBar::Orientation::Vertical);
                    return true;
                }
            }
        }
        
        // 未知属性，记录警告
        // Logger::Warn("Unknown property: {} on {}", propertyName, tagName);
        return false;
    }
    
    bool SetContent(Control* control, const std::string& content) override {
        if (!control) return false;
        
        // 对于 Button，创建 TextBlock 作为内容
        if (auto button = dynamic_cast<Button*>(control)) {
            auto textBlock = std::make_shared<TextBlock>();
            textBlock->SetText(std::wstring(content.begin(), content.end()));
            textBlock->SetForeground(Color::White());
            button->SetContent(textBlock);
            return true;
        }
        
        // 对于 TextBlock，直接设置文本
        if (auto textBlock = dynamic_cast<TextBlock*>(control)) {
            textBlock->SetText(std::wstring(content.begin(), content.end()));
            return true;
        }
        
        // 对于 TextBox，设置文本
        if (auto textBox = dynamic_cast<TextBox*>(control)) {
            textBox->SetText(std::wstring(content.begin(), content.end()));
            return true;
        }
        
        return false;
    }

private:
    bool HandleMarkupExtension(Control* control, const std::string& propertyName, 
                               const std::string& value) {
        // 暂不实现复杂的标记扩展，仅支持 x:Name
        if (value.find("{x:Name ") == 0 || value.find("{x:Name=") == 0) {
            size_t start = value.find_first_of(" =");
            if (start != std::string::npos) {
                std::string name = value.substr(start + 1);
                // 去除末尾的 }
                if (!name.empty() && name.back() == '}') {
                    name.pop_back();
                }
                // 去除空格
                name.erase(0, name.find_first_not_of(" "));
                name.erase(name.find_last_not_of(" ") + 1);
                control->SetName(name);
                return true;
            }
        }
        return false;
    }
    
    std::string GetTagName(Control* control) {
        if (dynamic_cast<StackPanel*>(control)) return "StackPanel";
        if (dynamic_cast<Grid*>(control)) return "Grid";
        if (dynamic_cast<Canvas*>(control)) return "Canvas";
        if (dynamic_cast<Border*>(control)) return "Border";
        if (dynamic_cast<Button*>(control)) return "Button";
        if (dynamic_cast<TextBlock*>(control)) return "TextBlock";
        if (dynamic_cast<TextBox*>(control)) return "TextBox";
        return "Unknown";
    }
};

// ============================================================================
// XmlLoader 实现
// ============================================================================
class XmlLoader : public IXmlLoader {
public:
    XmlLoader() 
        : m_factory(std::make_unique<ElementFactory>()),
          m_setter(std::make_unique<PropertySetter>()) {}
    
    ControlPtr Load(const std::string& filePath) override {
        tinyxml2::XMLDocument doc;
        tinyxml2::XMLError result = doc.LoadFile(filePath.c_str());
        
        if (result != tinyxml2::XML_SUCCESS) {
            throw XmlLayoutException("Failed to load XML file: " + filePath + 
                                     " Error: " + std::to_string(result));
        }
        
        return ParseElement(doc.RootElement());
    }
    
    ControlPtr LoadFromString(const std::string& xmlString) override {
        tinyxml2::XMLDocument doc;
        tinyxml2::XMLError result = doc.Parse(xmlString.c_str());
        
        if (result != tinyxml2::XML_SUCCESS) {
            throw XmlLayoutException("Failed to parse XML string. Error: " + 
                                     std::to_string(result));
        }
        
        return ParseElement(doc.RootElement());
    }
    
    void RegisterElement(const std::string& tagName, 
                          std::function<ControlPtr()> factory) override {
        m_factory->Register(tagName, factory);
    }
    
    IElementFactory* GetElementFactory() override {
        return m_factory.get();
    }

private:
    // 解析 Grid 的行列定义
    void ParseGridDefinitions(Grid* grid, tinyxml2::XMLElement* element) {
        std::string tagName = element->Name();
        
        // 调试输出
        // Logger::Debug("ParseGridDefinitions: {}", tagName);
        
        if (tagName == "Grid.RowDefinitions" || tagName == "RowDefinitions") {
            for (tinyxml2::XMLElement* rowDef = element->FirstChildElement(); 
                 rowDef; rowDef = rowDef->NextSiblingElement()) {
                if (std::string(rowDef->Name()) == "RowDefinition") {
                    const char* height = rowDef->Attribute("Height");
                    if (height) {
                        std::string heightStr(height);
                        if (heightStr == "Auto") {
                            grid->AddRow(GridLength::Auto());
                        } else if (heightStr == "*") {
                            grid->AddRow(GridLength::Star(1));
                        } else {
                            // 尝试解析为像素值
                            try {
                                float pixels = std::stof(heightStr);
                                grid->AddRow(GridLength::Pixel(pixels));
                            } catch (...) {
                                grid->AddRow(GridLength::Star(1));
                            }
                        }
                    } else {
                        grid->AddRow(GridLength::Star(1));
                    }
                }
            }
        }
        else if (tagName == "Grid.ColumnDefinitions" || tagName == "ColumnDefinitions") {
            for (tinyxml2::XMLElement* colDef = element->FirstChildElement(); 
                 colDef; colDef = colDef->NextSiblingElement()) {
                if (std::string(colDef->Name()) == "ColumnDefinition") {
                    const char* width = colDef->Attribute("Width");
                    if (width) {
                        std::string widthStr(width);
                        if (widthStr == "Auto") {
                            grid->AddColumn(GridLength::Auto());
                        } else if (widthStr == "*") {
                            grid->AddColumn(GridLength::Star(1));
                        } else {
                            // 尝试解析为像素值
                            try {
                                float pixels = std::stof(widthStr);
                                grid->AddColumn(GridLength::Pixel(pixels));
                            } catch (...) {
                                grid->AddColumn(GridLength::Star(1));
                            }
                        }
                    } else {
                        grid->AddColumn(GridLength::Star(1));
                    }
                }
            }
        }
    }

    ControlPtr ParseElement(tinyxml2::XMLElement* element) {
        if (!element) return nullptr;
        
        std::string tagName = element->Name();
        
        // 跳过定义元素（它们已在父元素中处理）
        if (tagName == "RowDefinition" || tagName == "ColumnDefinition") {
            return nullptr;
        }
        
        // 创建控件
        ControlPtr control = m_factory->Create(tagName);
        if (!control) {
            // 尝试作为内容处理（可能是文本内容）
            // Logger::Warn("Unknown element type: {}", tagName);
            return nullptr;
        }
        
        // 调试输出
        // Logger::Debug("Created element: {}", tagName);
        
        // 设置属性
        int gridRow = 0, gridColumn = 0;
        bool hasGridRow = false, hasGridColumn = false;
        
        for (const tinyxml2::XMLAttribute* attr = element->FirstAttribute(); 
             attr; attr = attr->Next()) {
            std::string attrName = Trim(attr->Name());
            std::string attrValue = Trim(attr->Value());
            
            // 调试输出所有属性
            // if (tagName == "Border" || tagName == "Button") {
            //     Logger::Debug("Element: {}, Attr: {} = {}", tagName, attrName, attrValue);
            // }
            
            // 处理 Grid 附加属性
            if (attrName == "Grid.Row") {
                try {
                    gridRow = std::stoi(attrValue);
                    hasGridRow = true;
                } catch (...) {
                    gridRow = 0;
                }
                continue;
            }
            if (attrName == "Grid.Column") {
                try {
                    gridColumn = std::stoi(attrValue);
                    hasGridColumn = true;
                } catch (...) {
                    gridColumn = 0;
                }
                continue;
            }
            
            m_setter->SetProperty(control.get(), attrName, attrValue);
        }
        
        // 设置 Grid 位置（存储在控件中，供后续使用）
        if (hasGridRow) Grid::SetRow(control.get(), gridRow);
        if (hasGridColumn) Grid::SetColumn(control.get(), gridColumn);
        
        // 处理子元素
        for (tinyxml2::XMLElement* child = element->FirstChildElement(); 
             child; child = child->NextSiblingElement()) {
            
            std::string childTag = child->Name();
            
            // 特殊处理：Grid 行列定义
            if (auto grid = dynamic_cast<Grid*>(control.get())) {
                if (childTag.find("RowDefinitions") != std::string::npos ||
                    childTag.find("ColumnDefinitions") != std::string::npos) {
                    // 调试输出
                    // Logger::Debug("Parsing Grid definitions: {}", childTag);
                    ParseGridDefinitions(grid, child);
                    continue;
                }
            }
            
            // 特殊处理：属性元素语法（如 <Button.Content>）
            if (childTag.find(tagName + ".") == 0) {
                std::string propertyName = childTag.substr(tagName.length() + 1);
                // 获取子元素的文本内容或子控件
                // 简化处理：假设是单一子控件
                ControlPtr propertyControl = ParseElement(child->FirstChildElement());
                if (propertyControl && dynamic_cast<ContentControl*>(control.get())) {
                    dynamic_cast<ContentControl*>(control.get())->SetContent(propertyControl);
                }
                continue;
            }
            
            // 普通子元素
            ControlPtr childControl = ParseElement(child);
            if (childControl) {
                // 注意：检查顺序很重要！Border 继承自 Panel，所以要先检查 Border
                if (auto border = dynamic_cast<Border*>(control.get())) {
                    // Border 使用 SetContent 设置单一内容
                    // Logger::Debug("Setting Border content to: {}", childControl->GetTypeName());
                    border->SetContent(childControl);
                }
                else if (auto contentControl = dynamic_cast<ContentControl*>(control.get())) {
                    // ContentControl 使用 SetContent 设置单一内容
                    contentControl->SetContent(childControl);
                }
                else if (auto panel = dynamic_cast<Panel*>(control.get())) {
                    // Panel 使用 AddChild 添加多个子元素
                    panel->AddChild(childControl);
                }
            }
        }
        
        // 处理文本内容
        if (element->GetText()) {
            std::string text = element->GetText();
            // 去除空白
            text.erase(0, text.find_first_not_of(" \t\n\r"));
            text.erase(text.find_last_not_of(" \t\n\r") + 1);
            if (!text.empty()) {
                m_setter->SetContent(control.get(), text);
            }
        }
        
        return control;
    }
    
    std::unique_ptr<ElementFactory> m_factory;
    std::unique_ptr<PropertySetter> m_setter;
};

// ============================================================================
// 工厂函数
// ============================================================================
IXmlLoaderPtr CreateXmlLoader() {
    return std::make_shared<XmlLoader>();
}

} // namespace xml
} // namespace luaui
