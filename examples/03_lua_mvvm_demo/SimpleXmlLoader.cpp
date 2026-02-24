// SimpleXmlLoader.cpp - 简化版 XML 布局加载器
#include "SimpleXmlLoader.h"
#include "Panel.h"
#include "Button.h"
#include "TextBlock.h"
#include "CheckBox.h"
#include "Border.h"
#include "Logger.h"
#include <fstream>
#include <sstream>

using namespace luaui;
using namespace luaui::controls;
using namespace luaui::rendering;

namespace luaui {
namespace mvvm {

SimpleXmlLoader::SimpleXmlLoader() {}
SimpleXmlLoader::~SimpleXmlLoader() {}

std::shared_ptr<luaui::Control> SimpleXmlLoader::Load(const std::string& xmlPath, lua_State* L) {
    m_luaState = L;
    m_lastError.clear();
    
    // 读取 XML 文件
    std::ifstream file(xmlPath);
    if (!file.is_open()) {
        m_lastError = "Cannot open file: " + xmlPath;
        return nullptr;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    
    if (content.empty()) {
        m_lastError = "XML file is empty";
        return nullptr;
    }
    
    // 解析 XML
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError xmlError = doc.Parse(content.c_str());
    if (xmlError != tinyxml2::XML_SUCCESS) {
        m_lastError = "XML parse error (" + std::to_string(xmlError) + "): " + 
                      (doc.ErrorStr() ? doc.ErrorStr() : "unknown error");
        return nullptr;
    }
    
    // 获取根元素
    tinyxml2::XMLElement* rootElem = doc.RootElement();
    if (!rootElem) {
        m_lastError = "No root element in XML";
        return nullptr;
    }
    
    // 解析根元素
    auto result = ParseElement(rootElem);
    if (!result) {
        if (m_lastError.empty()) {
            m_lastError = "Failed to parse root element: " + std::string(rootElem->Name());
        }
    }
    
    return result;
}

std::shared_ptr<luaui::Control> SimpleXmlLoader::ParseElement(tinyxml2::XMLElement* element) {
    if (!element) {
        m_lastError = "ParseElement: null element";
        return nullptr;
    }
    
    const char* name = element->Name();
    if (!name) {
        m_lastError = "ParseElement: element has no name";
        return nullptr;
    }
    
    if (strcmp(name, "StackPanel") == 0) {
        return CreateStackPanel(element);
    } else if (strcmp(name, "Button") == 0) {
        return CreateButton(element);
    } else if (strcmp(name, "TextBlock") == 0) {
        return CreateTextBlock(element);
    } else if (strcmp(name, "CheckBox") == 0) {
        return CreateCheckBox(element);
    } else if (strcmp(name, "Border") == 0) {
        return CreateBorder(element);
    } else if (strcmp(name, "Window") == 0) {
        // Window 是容器，解析其子元素作为内容
        tinyxml2::XMLElement* child = element->FirstChildElement();
        if (child) {
            return ParseElement(child);
        } else {
            m_lastError = "Window element has no children";
            return nullptr;
        }
    }
    
    // 未知元素，记录错误
    m_lastError = std::string("Unknown element type: ") + name;
    return nullptr;
}

std::shared_ptr<luaui::controls::StackPanel> SimpleXmlLoader::CreateStackPanel(tinyxml2::XMLElement* elem) {
    auto panel = std::make_shared<StackPanel>();
    
    // 解析 Orientation
    const char* orientation = elem->Attribute("Orientation");
    if (orientation) {
        if (strcmp(orientation, "Horizontal") == 0) {
            panel->SetOrientation(StackPanel::Orientation::Horizontal);
        } else {
            panel->SetOrientation(StackPanel::Orientation::Vertical);
        }
    }
    
    // 解析 Spacing
    float spacing = 0;
    if (elem->QueryFloatAttribute("Spacing", &spacing) == tinyxml2::XML_SUCCESS) {
        panel->SetSpacing(spacing);
    }
    
    // 解析 Margin
    float margin = 0;
    if (elem->QueryFloatAttribute("Margin", &margin) == tinyxml2::XML_SUCCESS) {
        // panel->SetMargin(margin); // 如果支持
    }
    
    // 解析子元素
    for (tinyxml2::XMLElement* child = elem->FirstChildElement(); child; child = child->NextSiblingElement()) {
        auto control = ParseElement(child);
        if (control) {
            panel->AddChild(control);
        }
    }
    
    return panel;
}

std::shared_ptr<luaui::controls::Button> SimpleXmlLoader::CreateButton(tinyxml2::XMLElement* elem) {
    auto button = std::make_shared<Button>();
    
    // 解析 Content (文本)
    const char* content = elem->Attribute("Content");
    if (content) {
        std::string text = content;
        // 检查是否是绑定表达式
        if (text.find("{Binding") != std::string::npos) {
            std::string propName = ParseBinding(text);
            text = GetViewModelProperty(propName);
        }
        button->SetText(std::wstring(text.begin(), text.end()));
    }
    
    // 解析 Style (简化处理，直接应用颜色)
    const char* style = elem->Attribute("Style");
    if (style) {
        std::string styleStr = style;
        // 这里可以根据样式名应用不同的颜色
        if (styleStr.find("Primary") != std::string::npos) {
            button->SetStateColors(
                Color::FromHex(0x2196F3),  // Normal
                Color::FromHex(0x1976D2),  // Hover
                Color::FromHex(0x0D47A1)   // Pressed
            );
        } else if (styleStr.find("Accent") != std::string::npos) {
            button->SetStateColors(
                Color::FromHex(0x4CAF50),
                Color::FromHex(0x388E3C),
                Color::FromHex(0x1B5E20)
            );
        } else if (styleStr.find("Danger") != std::string::npos) {
            button->SetStateColors(
                Color::FromHex(0xF44336),
                Color::FromHex(0xD32F2F),
                Color::FromHex(0xB71C1C)
            );
        }
    }
    
    // 解析 Command 绑定
    const char* command = elem->Attribute("Command");
    if (command && m_luaState) {
        std::string cmdName = ParseBinding(command);
        lua_State* L = m_luaState; // 局部拷贝避免捕获 this
        // 绑定点击事件到 Lua Command
        button->Click.Add([L, cmdName](luaui::Control*) {
            if (!L) return;
            lua_getglobal(L, "ViewModelInstance");
            if (lua_istable(L, -1)) {
                lua_getfield(L, -1, cmdName.c_str());
                if (lua_isfunction(L, -1)) {
                    lua_pushvalue(L, -2); // self
                    lua_pcall(L, 1, 0, 0);
                } else if (lua_istable(L, -1)) {
                    // Command 对象
                    lua_getfield(L, -1, "Execute");
                    if (lua_isfunction(L, -1)) {
                        lua_pushvalue(L, -2);
                        lua_pcall(L, 1, 0, 0);
                    }
                }
                lua_pop(L, 1);
            }
            lua_pop(L, 1);
        });
    }
    
    return button;
}

std::shared_ptr<luaui::controls::TextBlock> SimpleXmlLoader::CreateTextBlock(tinyxml2::XMLElement* elem) {
    auto textBlock = std::make_shared<TextBlock>();
    
    // 解析 Text
    const char* text = elem->Attribute("Text");
    if (text) {
        std::string textStr = text;
        // 检查是否是绑定表达式
        if (textStr.find("{Binding") != std::string::npos) {
            std::string propName = ParseBinding(textStr);
            textStr = GetViewModelProperty(propName);
        }
        textBlock->SetText(std::wstring(textStr.begin(), textStr.end()));
    }
    
    // 解析 Style
    const char* style = elem->Attribute("Style");
    if (style) {
        std::string styleStr = style;
        if (styleStr.find("Title") != std::string::npos) {
            textBlock->SetFontSize(24);
            textBlock->SetFontWeight(FontWeight::Bold);
            textBlock->SetForeground(Color::FromHex(0x1976D2));
        } else if (styleStr.find("Description") != std::string::npos) {
            textBlock->SetFontSize(13);
            textBlock->SetForeground(Color::FromHex(0x757575));
        }
    }
    
    // 解析 FontSize
    float fontSize = 0;
    if (elem->QueryFloatAttribute("FontSize", &fontSize) == tinyxml2::XML_SUCCESS) {
        textBlock->SetFontSize(fontSize);
    }
    
    // 解析 Foreground (简化处理)
    const char* foreground = elem->Attribute("Foreground");
    if (foreground) {
        textBlock->SetForeground(ParseColor(foreground));
    }
    
    return textBlock;
}

std::shared_ptr<luaui::controls::CheckBox> SimpleXmlLoader::CreateCheckBox(tinyxml2::XMLElement* elem) {
    auto checkBox = std::make_shared<CheckBox>();
    
    const char* content = elem->Attribute("Content");
    if (content) {
        checkBox->SetText(std::wstring(content, content + strlen(content)));
    }
    
    // 解析 IsChecked 绑定
    const char* isChecked = elem->Attribute("IsChecked");
    if (isChecked) {
        std::string bindingStr = isChecked;
        if (bindingStr.find("{Binding") != std::string::npos) {
            std::string propName = ParseBinding(bindingStr);
            // 获取初始值
            lua_getglobal(m_luaState, "ViewModelInstance");
            if (lua_istable(m_luaState, -1)) {
                lua_getfield(m_luaState, -1, propName.c_str());
                if (lua_isboolean(m_luaState, -1)) {
                    checkBox->SetIsChecked(lua_toboolean(m_luaState, -1));
                }
                lua_pop(m_luaState, 1);
            }
            lua_pop(m_luaState, 1);
            
            // 绑定变更事件 (简化版)
            lua_State* L2 = m_luaState;
            checkBox->CheckedChanged.Add([L2, propName](luaui::controls::CheckBox*, bool checked) {
                if (!L2) return;
                lua_getglobal(L2, "ViewModelInstance");
                if (lua_istable(L2, -1)) {
                    lua_pushboolean(L2, checked);
                    lua_setfield(L2, -2, propName.c_str());
                    // 触发 PropertyChanged
                    lua_getfield(L2, -1, "PropertyChanged");
                    if (lua_isfunction(L2, -1)) {
                        lua_pushvalue(L2, -2);
                        lua_pushstring(L2, propName.c_str());
                        lua_pcall(L2, 2, 0, 0);
                    } else {
                        lua_pop(L2, 1);
                    }
                }
                lua_pop(L2, 1);
            });
        }
    }
    
    return checkBox;
}

std::shared_ptr<luaui::controls::Border> SimpleXmlLoader::CreateBorder(tinyxml2::XMLElement* elem) {
    auto border = std::make_shared<Border>();
    
    // 解析 Background
    const char* background = elem->Attribute("Background");
    if (background) {
        border->SetBackground(ParseColor(background));
    }
    
    // 解析 CornerRadius
    float cornerRadius = 0;
    if (elem->QueryFloatAttribute("CornerRadius", &cornerRadius) == tinyxml2::XML_SUCCESS) {
        // border->SetCornerRadius(cornerRadius); // 如果支持
    }
    
    // 解析子元素 (Border 通常包含一个子元素)
    tinyxml2::XMLElement* child = elem->FirstChildElement();
    if (child) {
        auto childControl = ParseElement(child);
        if (childControl) {
            border->SetChild(childControl);
        }
    }
    
    return border;
}

std::string SimpleXmlLoader::ParseBinding(const std::string& value) {
    // 简单解析 {Binding PropertyName, ...}
    size_t start = value.find("{Binding ");
    if (start == std::string::npos) return "";
    
    start += 9; // 跳过 "{Binding "
    size_t end = value.find_first_of(" ,}", start);
    if (end == std::string::npos) end = value.length() - 1;
    
    return value.substr(start, end - start);
}

std::string SimpleXmlLoader::GetViewModelProperty(const std::string& propertyName) {
    if (!m_luaState) return "";
    
    lua_getglobal(m_luaState, "ViewModelInstance");
    if (!lua_istable(m_luaState, -1)) {
        lua_pop(m_luaState, 1);
        return "";
    }
    
    lua_getfield(m_luaState, -1, propertyName.c_str());
    std::string result;
    if (lua_isstring(m_luaState, -1)) {
        result = lua_tostring(m_luaState, -1);
    } else if (lua_isnumber(m_luaState, -1)) {
        result = std::to_string(static_cast<int>(lua_tonumber(m_luaState, -1)));
    } else if (lua_isboolean(m_luaState, -1)) {
        result = lua_toboolean(m_luaState, -1) ? "true" : "false";
    }
    lua_pop(m_luaState, 2);
    
    return result;
}

Color SimpleXmlLoader::ParseColor(const std::string& colorStr) {
    if (colorStr.empty()) return Color::Black();
    
    // 检查是否是资源引用
    if (colorStr.find("{StaticResource ") != std::string::npos) {
        // 简化处理，返回默认颜色
        return Color::FromHex(0x757575);
    }
    
    // 解析十六进制颜色 #RRGGBB
    if (colorStr[0] == '#' && colorStr.length() >= 7) {
        unsigned int hex = std::stoul(colorStr.substr(1), nullptr, 16);
        return Color::FromHex(hex);
    }
    
    // 命名颜色
    if (colorStr == "White") return Color::White();
    if (colorStr == "Black") return Color::Black();
    if (colorStr == "Red") return Color::Red();
    if (colorStr == "Green") return Color::Green();
    if (colorStr == "Blue") return Color::Blue();
    
    return Color::Black();
}

} // namespace mvvm
} // namespace luaui
