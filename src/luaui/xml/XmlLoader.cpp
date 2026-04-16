#include "XmlLayout.h"
#include "tinyxml2.h"
#include "Logger.h"
#include "../utils/StringUtils.h"
#include "layouts/Grid.h"
#include "TextBlock.h"
#include "TextBox.h"
#include "Slider.h"
#include "Button.h"
#include "Border.h"
#include "CheckBox.h"      // CheckBox and RadioButton
#include "ListBox.h"       // ListBox and ListBoxItem
#include "DataGrid.h"      // DataGrid
#include "layouts/ScrollViewer.h"
#include "layouts/Canvas.h"
#include "layouts/WrapPanel.h"
#include "layouts/Viewbox.h"
#include "Menu.h"
#include <fstream>
#include "SideBar.h"
#include "StatusBar.h"
#include "layouts/DockPanel.h"
#include <algorithm>
#include <cctype>

namespace luaui {
namespace xml {

using namespace controls;
using namespace rendering;

// ============================================================================
// XmlLoader 实现 - 支持 MVVM 数据绑定
// ============================================================================
class XmlLoader : public IXmlLoader {
public:
    XmlLoader() {
        RegisterDefaultElements();
    }
    
    std::shared_ptr<luaui::Control> Load(const std::string& filePath) override {
        // 清空之前的延迟绑定
        m_deferredBindings.clear();
        
        // 读取文件内容(使用二进制模式,避免编码转换)
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            throw XmlLayoutException("Failed to open XML file: " + filePath);
        }
        
        std::string content((std::istreambuf_iterator<char>(file)), 
                           std::istreambuf_iterator<char>());
        file.close();
        
        // 解析 XML
        tinyxml2::XMLDocument doc;
        if (doc.Parse(content.c_str()) != tinyxml2::XML_SUCCESS) {
            throw XmlLayoutException("Failed to parse XML file: " + filePath);
        }
        return LoadElement(doc.RootElement());
    }
    
    std::shared_ptr<luaui::Control> LoadFromString(const std::string& xmlString) override {
        // 清空之前的延迟绑定
        m_deferredBindings.clear();
        
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
    
    // ========== MVVM 数据绑定 ==========
    
    std::vector<DeferredBinding> GetDeferredBindings() const override {
        return m_deferredBindings;
    }
    
protected:
    void BindEventsFromInstance(void* instance) override {
        (void)instance;
        // 基类实现为空，具体绑定由外部完成
    }
    
private:
    std::unordered_map<std::string, std::function<std::shared_ptr<luaui::Control>()>> m_factories;
    std::unordered_map<std::string, ClickHandler> m_clickHandlers;
    std::unordered_map<std::string, ValueChangedHandler> m_valueChangedHandlers;
    std::unordered_map<std::string, TextChangedHandler> m_textChangedHandlers;
    
    // MVVM 延迟绑定列表
    std::vector<DeferredBinding> m_deferredBindings;
    
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
        RegisterElement("DataGrid", []() { return std::make_shared<DataGrid>(); });
        RegisterElement("ScrollViewer", []() { return std::make_shared<ScrollViewer>(); });
        RegisterElement("Image", []() { return std::make_shared<Image>(); });
        RegisterElement("Rectangle", []() { return std::make_shared<Rectangle>(); });
        RegisterElement("Ellipse", []() { return std::make_shared<Ellipse>(); });

        // Menu system
        RegisterElement("MenuBar", []() { return std::make_shared<MenuBar>(); });
        RegisterElement("Menu", []() { return std::make_shared<Menu>(); });
        RegisterElement("MenuItem", []() { return std::make_shared<MenuItem>(); });
        RegisterElement("ContextMenu", []() { return std::make_shared<ContextMenu>(); });
        RegisterElement("Separator", []() {
            auto item = std::make_shared<MenuItem>();
            item->SetItemType(MenuItem::ItemType::Separator);
            return item;
        });

        // SideBar
        RegisterElement("SideBar", []() { return std::make_shared<SideBar>(); });

        // StatusBar
        RegisterElement("StatusBar", []() { return std::make_shared<StatusBar>(); });
        RegisterElement("StatusBarItem", []() { return std::make_shared<StatusBarItem>(); });

        // DockPanel
        RegisterElement("DockPanel", []() { return std::make_shared<DockPanel>(); });
        
        // Canvas
        RegisterElement("Canvas", []() { return std::make_shared<Canvas>(); });
        
        // WrapPanel
        RegisterElement("WrapPanel", []() { return std::make_shared<WrapPanel>(); });
        
        // Viewbox
        RegisterElement("Viewbox", []() { return std::make_shared<Viewbox>(); });
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
                    if (auto sideBar = std::dynamic_pointer_cast<controls::SideBar>(control)) {
                        sideBar->SetSideBarWidth(width);
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
            // MinWidth
            else if (name == "MinWidth") {
                float minWidth;
                if (TypeConverter::ToFloat(value, minWidth)) {
                    if (auto* layout = control->GetLayout()) {
                        layout->SetMinWidth(minWidth);
                    }
                }
            }
            // MaxWidth
            else if (name == "MaxWidth") {
                float maxWidth;
                if (TypeConverter::ToFloat(value, maxWidth)) {
                    if (auto* layout = control->GetLayout()) {
                        layout->SetMaxWidth(maxWidth);
                    }
                }
            }
            // MinHeight
            else if (name == "MinHeight") {
                float minHeight;
                if (TypeConverter::ToFloat(value, minHeight)) {
                    if (auto* layout = control->GetLayout()) {
                        layout->SetMinHeight(minHeight);
                    }
                }
            }
            // MaxHeight
            else if (name == "MaxHeight") {
                float maxHeight;
                if (TypeConverter::ToFloat(value, maxHeight)) {
                    if (auto* layout = control->GetLayout()) {
                        layout->SetMaxHeight(maxHeight);
                    }
                }
            }
            // 外边距 Margin (支持多种格式: "24" 或 "32,24" 或 "32,24,24,24")
            else if (name == "Margin") {
                if (auto* layout = control->GetLayout()) {
                    std::wstring wvalue = Utf8ToW(value);
                    
                    // 分割逗号分隔的值
                    std::vector<std::wstring> parts;
                    std::wstringstream wss(wvalue);
                    std::wstring part;
                    while (std::getline(wss, part, L',')) {
                        // 去除空格
                        part.erase(0, part.find_first_not_of(L" \t"));
                        part.erase(part.find_last_not_of(L" \t") + 1);
                        parts.push_back(part);
                    }
                    
                    if (parts.size() == 1) {
                        // 单一数值: 四边相同
                        float margin;
                        if (TypeConverter::ToFloat(WToUtf8(parts[0]), margin)) {
                            layout->SetMargin(margin, margin, margin, margin);
                        }
                    } else if (parts.size() == 2) {
                        // 两个数值: 水平,垂直
                        float horizontal, vertical;
                        if (TypeConverter::ToFloat(WToUtf8(parts[0]), horizontal) &&
                            TypeConverter::ToFloat(WToUtf8(parts[1]), vertical)) {
                            layout->SetMargin(horizontal, vertical, horizontal, vertical);
                        }
                    } else if (parts.size() == 4) {
                        // 四个数值: 左,上,右,下
                        float left, top, right, bottom;
                        if (TypeConverter::ToFloat(WToUtf8(parts[0]), left) &&
                            TypeConverter::ToFloat(WToUtf8(parts[1]), top) &&
                            TypeConverter::ToFloat(WToUtf8(parts[2]), right) &&
                            TypeConverter::ToFloat(WToUtf8(parts[3]), bottom)) {
                            layout->SetMargin(left, top, right, bottom);
                        }
                    }
                }
            }
            // 内边距 Padding (支持多种格式: "24" 或 "32,24" 或 "32,24,24,24")
            else if (name == "Padding") {
                if (auto* layout = control->GetLayout()) {
                    std::wstring wvalue = Utf8ToW(value);

                    // 分割逗号分隔的值
                    std::vector<std::wstring> parts;
                    std::wstringstream wss(wvalue);
                    std::wstring part;
                    while (std::getline(wss, part, L',')) {
                        // 去除空格
                        part.erase(0, part.find_first_not_of(L" \t"));
                        part.erase(part.find_last_not_of(L" \t") + 1);
                        parts.push_back(part);
                    }

                    if (parts.size() == 1) {
                        // 单一数值: 四边相同
                        float padding;
                        if (TypeConverter::ToFloat(WToUtf8(parts[0]), padding)) {
                            layout->SetPadding(padding, padding, padding, padding);
                        }
                    } else if (parts.size() == 2) {
                        // 两个数值: 水平,垂直
                        float horizontal, vertical;
                        if (TypeConverter::ToFloat(WToUtf8(parts[0]), horizontal) &&
                            TypeConverter::ToFloat(WToUtf8(parts[1]), vertical)) {
                            layout->SetPadding(horizontal, vertical, horizontal, vertical);
                        }
                    } else if (parts.size() == 4) {
                        // 四个数值: 左,上,右,下
                        float left, top, right, bottom;
                        if (TypeConverter::ToFloat(WToUtf8(parts[0]), left) &&
                            TypeConverter::ToFloat(WToUtf8(parts[1]), top) &&
                            TypeConverter::ToFloat(WToUtf8(parts[2]), right) &&
                            TypeConverter::ToFloat(WToUtf8(parts[3]), bottom)) {
                            layout->SetPadding(left, top, right, bottom);
                        }
                    }
                }
            }
            // 前景色 Foreground (Button, TextBlock)
            else if (name == "Foreground") {
                Color color;
                if (TypeConverter::ToColor(value, color)) {
                    if (auto btn = std::dynamic_pointer_cast<controls::Button>(control)) {
                        btn->SetForeground(color);
                    } else if (auto tb = std::dynamic_pointer_cast<controls::TextBlock>(control)) {
                        tb->SetForeground(color);
                    }
                }
            }
            // 圆角 CornerRadius (Button)
            else if (name == "CornerRadius") {
                float radius;
                if (TypeConverter::ToFloat(value, radius)) {
                    if (auto btn = std::dynamic_pointer_cast<controls::Button>(control)) {
                        btn->SetCornerRadius(rendering::CornerRadius(radius));
                    }
                }
            }
            // 边框颜色 BorderBrush (Button)
            else if (name == "BorderBrush") {
                Color color;
                if (TypeConverter::ToColor(value, color)) {
                    if (auto btn = std::dynamic_pointer_cast<controls::Button>(control)) {
                        btn->SetBorderBrush(color);
                    }
                }
            }
            // 边框粗细 BorderThickness (Button)
            else if (name == "BorderThickness") {
                float thickness;
                if (TypeConverter::ToFloat(value, thickness)) {
                    if (auto btn = std::dynamic_pointer_cast<controls::Button>(control)) {
                        btn->SetBorderThickness(thickness);
                    }
                }
            }
            // 背景色
            else if (name == "Background") {
                Color color;
                if (TypeConverter::ToColor(value, color)) {
                    // For Button, use SetCustomBackground to preserve custom color
                    if (auto btn = std::dynamic_pointer_cast<controls::Button>(control)) {
                        btn->SetCustomBackground(color);
                    } else if (auto* render = control->GetRender()) {
                        render->SetBackground(color);
                    }
                }
            }
            // SourcePath (Image)
            else if (name == "SourcePath") {
                std::wstring wpath = Utf8ToW(value);
                if (auto img = std::dynamic_pointer_cast<controls::Image>(control)) {
                    img->SetSourcePath(wpath);
                }
            }
            // Stretch (Image, Viewbox)
            else if (name == "Stretch") {
                if (auto img = std::dynamic_pointer_cast<controls::Image>(control)) {
                    if (value == "None") {
                        img->SetStretch(controls::Stretch::None);
                    } else if (value == "Fill") {
                        img->SetStretch(controls::Stretch::Fill);
                    } else if (value == "UniformToFill") {
                        img->SetStretch(controls::Stretch::UniformToFill);
                    } else {
                        // Default Uniform
                        img->SetStretch(controls::Stretch::Uniform);
                    }
                } else if (auto viewbox = std::dynamic_pointer_cast<controls::Viewbox>(control)) {
                    if (value == "None") {
                        viewbox->SetStretch(controls::Stretch::None);
                    } else if (value == "Fill") {
                        viewbox->SetStretch(controls::Stretch::Fill);
                    } else if (value == "UniformToFill") {
                        viewbox->SetStretch(controls::Stretch::UniformToFill);
                    } else {
                        // Default Uniform
                        viewbox->SetStretch(controls::Stretch::Uniform);
                    }
                }
            }
            // Spacing (StackPanel, WrapPanel)
            else if (name == "Spacing") {
                if (IsBindingExpression(value)) {
                    RecordDeferredBinding(control, "Spacing", value);
                } else {
                    float spacing;
                    if (TypeConverter::ToFloat(value, spacing)) {
                        if (auto stack = std::dynamic_pointer_cast<controls::StackPanel>(control)) {
                            stack->SetSpacing(spacing);
                        } else if (auto wrapPanel = std::dynamic_pointer_cast<controls::WrapPanel>(control)) {
                            wrapPanel->SetSpacing(spacing);
                        }
                    }
                }
            }
            // Orientation (StackPanel, WrapPanel)
            else if (name == "Orientation") {
                if (IsBindingExpression(value)) {
                    RecordDeferredBinding(control, "Orientation", value);
                } else {
                    if (auto stack = std::dynamic_pointer_cast<controls::StackPanel>(control)) {
                        if (value == "Horizontal") {
                            stack->SetOrientation(controls::StackPanel::Orientation::Horizontal);
                        } else if (value == "Vertical") {
                            stack->SetOrientation(controls::StackPanel::Orientation::Vertical);
                        }
                    } else if (auto wrapPanel = std::dynamic_pointer_cast<controls::WrapPanel>(control)) {
                        if (value == "Horizontal") {
                            wrapPanel->SetOrientation(controls::WrapPanel::Orientation::Horizontal);
                        } else if (value == "Vertical") {
                            wrapPanel->SetOrientation(controls::WrapPanel::Orientation::Vertical);
                        }
                    }
                }
            }
            // ItemWidth/ItemHeight (WrapPanel)
            else if (name == "ItemWidth") {
                float width;
                if (TypeConverter::ToFloat(value, width)) {
                    if (auto wrapPanel = std::dynamic_pointer_cast<controls::WrapPanel>(control)) {
                        wrapPanel->SetItemWidth(width);
                    }
                }
            }
            else if (name == "ItemHeight") {
                float height;
                if (TypeConverter::ToFloat(value, height)) {
                    if (auto wrapPanel = std::dynamic_pointer_cast<controls::WrapPanel>(control)) {
                        wrapPanel->SetItemHeight(height);
                    }
                }
            }
            // Text (TextBlock, TextBox, CheckBox, RadioButton, Button)
            else if (name == "Text") {
                // 检查是否为绑定表达式
                if (IsBindingExpression(value)) {
                    // 记录延迟绑定，清空默认文本
                    RecordDeferredBinding(control, "Text", value);
                    std::wstring wtext = L"";
                    if (auto tb = std::dynamic_pointer_cast<controls::TextBlock>(control)) {
                        tb->SetText(wtext);
                    } else if (auto tx = std::dynamic_pointer_cast<controls::TextBox>(control)) {
                        tx->SetText(wtext);
                    } else if (auto cb = std::dynamic_pointer_cast<controls::CheckBox>(control)) {
                        cb->SetText(wtext);
                    } else if (auto rb = std::dynamic_pointer_cast<controls::RadioButton>(control)) {
                        rb->SetText(wtext);
                    } else if (auto btn = std::dynamic_pointer_cast<controls::Button>(control)) {
                        btn->SetText(wtext);
                    } else if (auto si = std::dynamic_pointer_cast<controls::StatusBarItem>(control)) {
                        si->SetText(wtext);
                    }
                } else {
                    std::wstring wtext = Utf8ToW(value);
                    if (auto tb = std::dynamic_pointer_cast<controls::TextBlock>(control)) {
                        tb->SetText(wtext);
                    } else if (auto tx = std::dynamic_pointer_cast<controls::TextBox>(control)) {
                        tx->SetText(wtext);
                    } else if (auto cb = std::dynamic_pointer_cast<controls::CheckBox>(control)) {
                        cb->SetText(wtext);
                    } else if (auto rb = std::dynamic_pointer_cast<controls::RadioButton>(control)) {
                        rb->SetText(wtext);
                    } else if (auto btn = std::dynamic_pointer_cast<controls::Button>(control)) {
                        btn->SetText(wtext);
                    } else if (auto si = std::dynamic_pointer_cast<controls::StatusBarItem>(control)) {
                        si->SetText(wtext);
                    }
                }
            }
            // Content (Button)
            else if (name == "Content") {
                // 检查是否为绑定表达式
                if (IsBindingExpression(value)) {
                    // 记录延迟绑定，清空默认文本
                    RecordDeferredBinding(control, "Content", value);
                    if (auto btn = std::dynamic_pointer_cast<controls::Button>(control)) {
                        btn->SetText(L"");
                    }
                } else {
                    std::wstring wtext = Utf8ToW(value);
                    if (auto btn = std::dynamic_pointer_cast<controls::Button>(control)) {
                        btn->SetText(wtext);
                    }
                }
            }
            // FontSize
            else if (name == "FontSize") {
                float size;
                if (TypeConverter::ToFloat(value, size)) {
                    if (auto tb = std::dynamic_pointer_cast<controls::TextBlock>(control)) {
                        tb->SetFontSize(size);
                    } else if (auto btn = std::dynamic_pointer_cast<controls::Button>(control)) {
                        btn->SetFontSize(size);
                    }
                }
            }
            // FontWeight (TextBlock)
            else if (name == "FontWeight") {
                if (auto tb = std::dynamic_pointer_cast<controls::TextBlock>(control)) {
                    if (value == "Bold") {
                        tb->SetFontWeight(rendering::FontWeight::Bold);
                    } else if (value == "SemiBold") {
                        tb->SetFontWeight(rendering::FontWeight::SemiBold);
                    } else if (value == "Light") {
                        tb->SetFontWeight(rendering::FontWeight::Light);
                    } else {
                        tb->SetFontWeight(rendering::FontWeight::Regular);
                    }
                }
            }
            // FontStyle (TextBlock)
            else if (name == "FontStyle") {
                if (auto tb = std::dynamic_pointer_cast<controls::TextBlock>(control)) {
                    if (value == "Italic") {
                        tb->SetFontStyle(rendering::FontStyle::Italic);
                    } else {
                        tb->SetFontStyle(rendering::FontStyle::Normal);
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
                // 检查是否为绑定表达式
                if (IsBindingExpression(value)) {
                    // 记录延迟绑定
                    RecordDeferredBinding(control, "IsChecked", value);
                } else if (value == "True" || value == "true" || value == "1") {
                    if (auto cb = std::dynamic_pointer_cast<controls::CheckBox>(control)) {
                        cb->SetIsChecked(true);
                    } else if (auto rb = std::dynamic_pointer_cast<controls::RadioButton>(control)) {
                        rb->SetIsChecked(true);
                    }
                }
            }
            // IsEnabled (all controls)
            else if (name == "IsEnabled") {
                if (IsBindingExpression(value)) {
                    RecordDeferredBinding(control, "IsEnabled", value);
                } else {
                    control->SetIsEnabled(value == "True" || value == "true" || value == "1");
                }
            }
            // Placeholder (TextBox)
            else if (name == "Placeholder") {
                if (auto textBox = std::dynamic_pointer_cast<controls::TextBox>(control)) {
                    textBox->SetPlaceholder(Utf8ToW(value));
                }
            }
            // IsReadOnly (TextBox)
            else if (name == "IsReadOnly") {
                if (auto textBox = std::dynamic_pointer_cast<controls::TextBox>(control)) {
                    textBox->SetIsReadOnly(value == "True" || value == "true" || value == "1");
                }
            }
            // IsPassword (TextBox)
            else if (name == "IsPassword") {
                if (auto textBox = std::dynamic_pointer_cast<controls::TextBox>(control)) {
                    textBox->SetIsPassword(value == "True" || value == "true" || value == "1");
                }
            }
            // MaxLength (TextBox)
            else if (name == "MaxLength") {
                int maxLen = std::stoi(value);
                if (auto textBox = std::dynamic_pointer_cast<controls::TextBox>(control)) {
                    textBox->SetMaxLength(maxLen);
                }
            }
            // SetStateColors (Button) - 格式: "normal,hover,pressed"
            else if (name == "SetStateColors") {
                std::stringstream ss(value);
                std::string colorStr;
                std::vector<Color> colors;
                while (std::getline(ss, colorStr, ',')) {
                    Color c;
                    if (TypeConverter::ToColor(luaui::utils::StringUtils::Trim(colorStr), c)) {
                        colors.push_back(c);
                    }
                }
                if (colors.size() >= 3) {
                    if (auto btn = std::dynamic_pointer_cast<controls::Button>(control)) {
                        btn->SetStateColors(colors[0], colors[1], colors[2]);
                    }
                }
            }
            // ItemsSource (ListBox, ComboBox) - 绑定表达式
            else if (name == "ItemsSource") {
                if (IsBindingExpression(value)) {
                    RecordDeferredBinding(control, "ItemsSource", value);
                }
            }
            // SelectedIndex (ListBox, ComboBox) - 绑定表达式
            else if (name == "SelectedIndex") {
                if (IsBindingExpression(value)) {
                    RecordDeferredBinding(control, "SelectedIndex", value);
                } else {
                    // 直接设置整数值
                    if (auto listBox = std::dynamic_pointer_cast<controls::ListBox>(control)) {
                        int index = std::stoi(value);
                        listBox->SetSelectedIndex(index);
                    }
                }
            }
            // Header (MenuItem, Menu)
            else if (name == "Header") {
                std::wstring wval = Utf8ToW(value);
                if (auto menuItem = std::dynamic_pointer_cast<controls::MenuItem>(control)) {
                    menuItem->SetHeader(wval);
                }
            }
            // InputGestureText (MenuItem)
            else if (name == "InputGestureText") {
                std::wstring wval = Utf8ToW(value);
                if (auto menuItem = std::dynamic_pointer_cast<controls::MenuItem>(control)) {
                    menuItem->SetInputGestureText(wval);
                }
            }
            // IsCheckable (MenuItem)
            else if (name == "IsCheckable") {
                if (auto menuItem = std::dynamic_pointer_cast<controls::MenuItem>(control)) {
                    menuItem->SetIsCheckable(value == "True" || value == "true" || value == "1");
                }
            }
            // IsChecked (MenuItem) - extends existing IsChecked for CheckBox/RadioButton
            // MenuHeight (MenuBar)
            else if (name == "MenuHeight") {
                float h;
                if (TypeConverter::ToFloat(value, h)) {
                    if (auto menuBar = std::dynamic_pointer_cast<controls::MenuBar>(control)) {
                        menuBar->SetMenuHeight(h);
                    }
                }
            }
            // Title (SideBar)
            else if (name == "Title") {
                std::wstring wval = Utf8ToW(value);
                if (auto sideBar = std::dynamic_pointer_cast<controls::SideBar>(control)) {
                    sideBar->SetTitle(wval);
                }
            }
            // Collapsed (SideBar)
            else if (name == "Collapsed") {
                if (auto sideBar = std::dynamic_pointer_cast<controls::SideBar>(control)) {
                    sideBar->SetIsCollapsed(value == "True" || value == "true" || value == "1");
                }
            }
            // Pinned (SideBar)
            else if (name == "Pinned") {
                if (auto sideBar = std::dynamic_pointer_cast<controls::SideBar>(control)) {
                    sideBar->SetIsPinned(value == "True" || value == "true" || value == "1");
                }
            }
            // ShowSizingGrip (StatusBar)
            else if (name == "ShowSizingGrip") {
                if (auto statusBar = std::dynamic_pointer_cast<controls::StatusBar>(control)) {
                    statusBar->SetShowSizingGrip(value == "True" || value == "true" || value == "1");
                }
            }
            // ItemType (StatusBarItem)
            else if (name == "ItemType") {
                if (auto item = std::dynamic_pointer_cast<controls::StatusBarItem>(control)) {
                    if (value == "Text") item->SetItemType(StatusBarItem::ItemType::Text);
                    else if (value == "Progress") item->SetItemType(StatusBarItem::ItemType::Progress);
                    else if (value == "Panel") item->SetItemType(StatusBarItem::ItemType::Panel);
                    else if (value == "Spring") item->SetItemType(StatusBarItem::ItemType::Spring);
                }
            }
            // ShowBorder (StatusBarItem)
            else if (name == "ShowBorder") {
                if (auto item = std::dynamic_pointer_cast<controls::StatusBarItem>(control)) {
                    item->SetShowBorder(value == "True" || value == "true" || value == "1");
                }
            }
            // DockPanel.Dock attached property
            else if (name == "DockPanel.Dock" || name == "Dock") {
                Dock dock = Dock::Left;
                if (value == "Top") dock = Dock::Top;
                else if (value == "Right") dock = Dock::Right;
                else if (value == "Bottom") dock = Dock::Bottom;
                DockPanel::SetDock(control, dock);
            }
            // LastChildFill (DockPanel)
            else if (name == "LastChildFill") {
                if (auto dockPanel = std::dynamic_pointer_cast<controls::DockPanel>(control)) {
                    dockPanel->SetLastChildFill(value == "True" || value == "true" || value == "1");
                }
            }
            // Visibility attribute
            else if (name == "Visibility") {
                if (IsBindingExpression(value)) {
                    // Defer to MvvmXmlLoader for binding
                    utils::Logger::InfoF("[XML] Visibility binding found: %s on %s (ptr=%p)", 
                        value.c_str(), control->GetTypeName().c_str(), control.get());
                    RecordDeferredBinding(control, "Visibility", value);
                } else {
                    // Direct value: "Visible", "Collapsed", "Hidden"
                    bool visible = (value == "Visible" || value == "true" || value == "True");
                    control->SetIsVisible(visible);
                }
            }
            // ScrollViewer ScrollBarVisibility
            else if (name == "HorizontalScrollBarVisibility") {
                if (auto scrollViewer = std::dynamic_pointer_cast<controls::ScrollViewer>(control)) {
                    if (value == "Visible") {
                        scrollViewer->SetHorizontalScrollBarVisibility(controls::ScrollBarVisibility::Visible);
                    } else if (value == "Hidden") {
                        scrollViewer->SetHorizontalScrollBarVisibility(controls::ScrollBarVisibility::Hidden);
                    } else if (value == "Disabled") {
                        scrollViewer->SetHorizontalScrollBarVisibility(controls::ScrollBarVisibility::Disabled);
                    } else {
                        scrollViewer->SetHorizontalScrollBarVisibility(controls::ScrollBarVisibility::Auto);
                    }
                }
            }
            else if (name == "VerticalScrollBarVisibility") {
                if (auto scrollViewer = std::dynamic_pointer_cast<controls::ScrollViewer>(control)) {
                    if (value == "Visible") {
                        scrollViewer->SetVerticalScrollBarVisibility(controls::ScrollBarVisibility::Visible);
                    } else if (value == "Hidden") {
                        scrollViewer->SetVerticalScrollBarVisibility(controls::ScrollBarVisibility::Hidden);
                    } else if (value == "Disabled") {
                        scrollViewer->SetVerticalScrollBarVisibility(controls::ScrollBarVisibility::Disabled);
                    } else {
                        scrollViewer->SetVerticalScrollBarVisibility(controls::ScrollBarVisibility::Auto);
                    }
                }
            }
            // Canvas attached properties
            else if (name == "Canvas.Left") {
                float left;
                if (TypeConverter::ToFloat(value, left)) {
                    controls::Canvas::SetLeft(control, left);
                }
            }
            else if (name == "Canvas.Top") {
                float top;
                if (TypeConverter::ToFloat(value, top)) {
                    controls::Canvas::SetTop(control, top);
                }
            }
            else if (name == "Canvas.Right") {
                float right;
                if (TypeConverter::ToFloat(value, right)) {
                    controls::Canvas::SetRight(control, right);
                }
            }
            else if (name == "Canvas.Bottom") {
                float bottom;
                if (TypeConverter::ToFloat(value, bottom)) {
                    controls::Canvas::SetBottom(control, bottom);
                }
            }
            else if (name == "Canvas.ZIndex") {
                try {
                    int zIndex = std::stoi(value);
                    controls::Canvas::SetZIndex(control, zIndex);
                } catch (...) {}
            }
            // DockPanel attached property
            else if (name == "DockPanel.Dock") {
                if (value == "Left") {
                    controls::DockPanel::SetDock(control, controls::Dock::Left);
                } else if (value == "Top") {
                    controls::DockPanel::SetDock(control, controls::Dock::Top);
                } else if (value == "Right") {
                    controls::DockPanel::SetDock(control, controls::Dock::Right);
                } else if (value == "Bottom") {
                    controls::DockPanel::SetDock(control, controls::Dock::Bottom);
                }
            }
            // Click event: Click="SomeCommand" or Click="{Binding SomeCommand}"
            else if (name == "Click") {
                auto it = m_clickHandlers.find(value);
                if (it != m_clickHandlers.end()) {
                    if (auto btn = std::dynamic_pointer_cast<controls::Button>(control)) {
                        //luaui::utils::Logger::InfoF("[XML] Binding Click event for button '%s' to handler '%s'", 
                        //    control->GetName().c_str(), value.c_str());
                        std::string handlerName = value;
                        btn->Click.Add([handler = it->second, handlerName](luaui::Control*) { 
                            //luaui::utils::Logger::DebugF("[XML] Button clicked, invoking handler '%s'", handlerName.c_str());
                            handler(); 
                        });
                    } else {
                        luaui::utils::Logger::WarningF("[XML] Click attribute on non-button control: '%s'", control->GetTypeName().c_str());
                    }
                } else if (std::dynamic_pointer_cast<controls::Button>(control)) {
                    // no registered C++ handler -> defer to MvvmXmlLoader (Lua command binding)
                    RecordDeferredBinding(control, "Command", value);
                } else if (std::dynamic_pointer_cast<controls::MenuItem>(control)) {
                    // MenuItem Click -> defer to MvvmXmlLoader (Lua command binding)
                    RecordDeferredBinding(control, "Command", value);
                } else {
                    luaui::utils::Logger::WarningF("[XML] Click handler '%s' not found (registered handlers: %zu)", 
                        value.c_str(), m_clickHandlers.size());
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

        // MenuBar: 子元素为 Menu，通过 AddMenu 添加
        if (auto menuBar = std::dynamic_pointer_cast<controls::MenuBar>(parent)) {
            for (const auto* childElem = element->FirstChildElement(); childElem;
                 childElem = childElem->NextSiblingElement()) {
                std::string tag = childElem->Name();
                if (tag == "Menu") {
                    auto menu = std::make_shared<Menu>();
                    ApplyAttributes(menu, childElem);
                    LoadMenuItems(menu, childElem);
                    std::wstring header;
                    if (const auto* attr = childElem->FindAttribute("Header")) {
                        header = Utf8ToW(attr->Value());
                    }
                    menuBar->AddMenu(header, menu);
                }
            }
            return;
        }

        // Menu / ContextMenu: 子元素为 MenuItem / Separator
        if (auto menu = std::dynamic_pointer_cast<controls::Menu>(parent)) {
            LoadMenuItems(menu, element);
            return;
        }
        if (auto ctxMenu = std::dynamic_pointer_cast<controls::ContextMenu>(parent)) {
            LoadMenuItems(ctxMenu, element);
            return;
        }

        // SideBar: 单内容子元素，通过 SetContent
        if (auto sideBar = std::dynamic_pointer_cast<controls::SideBar>(parent)) {
            if (const auto* childElem = element->FirstChildElement()) {
                auto child = LoadElement(childElem);
                if (child) {
                    sideBar->SetContent(child);
                }
            }
            return;
        }

        // StatusBar: 子元素为 StatusBarItem
        if (auto statusBar = std::dynamic_pointer_cast<controls::StatusBar>(parent)) {
            for (const auto* childElem = element->FirstChildElement(); childElem;
                 childElem = childElem->NextSiblingElement()) {
                std::string tag = childElem->Name();
                if (tag == "StatusBarItem") {
                    auto item = std::make_shared<StatusBarItem>();
                    ApplyAttributes(item, childElem);
                    statusBar->AddItem(item);
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

    // 辅助方法：加载菜单项（Menu 和 ContextMenu 共用）
    void LoadMenuItems(const std::shared_ptr<Menu>& menu,
                       const tinyxml2::XMLElement* element) {
        for (const auto* childElem = element->FirstChildElement(); childElem;
             childElem = childElem->NextSiblingElement()) {
            std::string tag = childElem->Name();

            if (tag == "Separator") {
                auto item = std::make_shared<MenuItem>();
                item->SetItemType(MenuItem::ItemType::Separator);
                menu->AddItem(item);
            }
            else if (tag == "MenuItem") {
                auto item = std::make_shared<MenuItem>();
                ApplyAttributes(item, childElem);

                // 检查是否有子 Menu（子菜单）
                if (const auto* subMenuElem = childElem->FirstChildElement("Menu")) {
                    auto subMenu = std::make_shared<Menu>();
                    ApplyAttributes(subMenu, subMenuElem);
                    LoadMenuItems(subMenu, subMenuElem);
                    item->SetSubmenu(subMenu);
                }

                menu->AddItem(item);
            }
        }
    }
    
    // 检查是否为绑定表达式
    bool IsBindingExpression(const std::string& value) {
        return value.find("{Binding") == 0;
    }
    
    // 记录延迟绑定
    void RecordDeferredBinding(const std::shared_ptr<luaui::Control>& control,
                               const std::string& propertyName,
                               const std::string& expression) {
        DeferredBinding binding;
        binding.control = control;
        binding.propertyName = propertyName;
        binding.bindingExpression = expression;
        m_deferredBindings.push_back(binding);
        /*
        utils::Logger::DebugF("[XML] Recorded deferred binding: %s.%s = %s",
            control->GetTypeName().c_str(), propertyName.c_str(), expression.c_str());
        */
    }
    
    // 应用所有延迟绑定（由外部调用，如 MvvmXmlLoader）
    void ApplyDeferredBindings() {
        // 这个方法保留以便未来扩展，当前实现为空
        // 延迟绑定会通过 GetDeferredBindings() 返回给调用者处理
    }
};

// 创建 XML 加载器
IXmlLoaderPtr CreateXmlLoader() {
    return std::make_shared<XmlLoader>();
}

} // namespace xml
} // namespace luaui
