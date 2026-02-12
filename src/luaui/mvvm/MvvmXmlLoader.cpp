#include "MvvmXmlLoader.h"
#include "Logger.h"
#include "Converters.h"

// 首先包含 Core Control 基类定义
#include "../core/Control.h"

// Controls - 统一包含所有控件
#include "Controls.h"

#include <tinyxml2.h>
#include <sstream>
#include <algorithm>

namespace luaui {
namespace mvvm {

// ============================================================================
// MvvmXmlLoader 实现
// ============================================================================
MvvmXmlLoader::MvvmXmlLoader() 
    : m_baseLoader(xml::CreateXmlLoader()) 
{
}

std::shared_ptr<luaui::Control> MvvmXmlLoader::Load(const std::string& filePath) {
    utils::Logger::InfoF("[MVVM] Loading XML: %s", filePath.c_str());
    
    // 清空之前的绑定信息
    m_pendingBindingInfos.clear();
    m_pendingBindings.clear();
    
    // 先扫描 XML 提取绑定表达式
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(filePath.c_str()) != tinyxml2::XML_SUCCESS) {
        utils::Logger::ErrorF("[MVVM] Failed to load XML: %s", filePath.c_str());
        return nullptr;
    }
    
    // 提取绑定信息
    ExtractBindings(doc.RootElement());
    
    // 使用基础加载器加载控件树
    auto root = m_baseLoader->Load(filePath);
    m_rootControl = root;
    
    // 应用绑定
    if (root) {
        ApplyBindingsToControl(root);
    }
    ApplyBindings();
    
    return root;
}

std::shared_ptr<luaui::Control> MvvmXmlLoader::LoadFromString(const std::string& xmlString) {
    // 清空之前的绑定信息
    m_pendingBindingInfos.clear();
    m_pendingBindings.clear();
    
    // 先扫描 XML 提取绑定表达式
    tinyxml2::XMLDocument doc;
    if (doc.Parse(xmlString.c_str()) != tinyxml2::XML_SUCCESS) {
        utils::Logger::Error("[MVVM] Failed to parse XML string");
        return nullptr;
    }
    
    // 提取绑定信息
    ExtractBindings(doc.RootElement());
    
    auto root = m_baseLoader->LoadFromString(xmlString);
    m_rootControl = root;
    
    if (root) {
        ApplyBindingsToControl(root);
    }
    ApplyBindings();
    return root;
}

// ============================================================================
// 从 XML 元素提取绑定表达式
// ============================================================================
void MvvmXmlLoader::ExtractBindings(const tinyxml2::XMLElement* element, const std::string& parentName) {
    if (!element) return;
    
    // 获取控件名称（如果有 x:Name 或 Name 属性）
    std::string controlName = parentName;
    if (const char* nameAttr = element->Attribute("Name")) {
        controlName = nameAttr;
    } else if (const char* xnameAttr = element->Attribute("x:Name")) {
        controlName = xnameAttr;
    }
    
    // 遍历所有属性，查找绑定表达式
    for (const tinyxml2::XMLAttribute* attr = element->FirstAttribute(); 
         attr; attr = attr->Next()) {
        std::string attrName = attr->Name();
        std::string attrValue = attr->Value();
        
        if (IsBindingExpression(attrValue)) {
            utils::Logger::DebugF("[MVVM] Found binding: %s.%s = %s", 
                controlName.c_str(), attrName.c_str(), attrValue.c_str());
            
            // 存储绑定信息，等待控件加载完成后解析
            PendingBindingInfo info;
            info.elementName = controlName;
            info.propertyName = attrName;
            info.expressionString = attrValue;
            m_pendingBindingInfos.push_back(info);
        }
    }
    
    // 递归处理子元素
    for (const tinyxml2::XMLElement* child = element->FirstChildElement(); 
         child; child = child->NextSiblingElement()) {
        ExtractBindings(child, controlName);
    }
}

void MvvmXmlLoader::RegisterElement(const std::string& tagName, 
                                    std::function<std::shared_ptr<luaui::Control>()> factory) {
    m_baseLoader->RegisterElement(tagName, factory);
}

void MvvmXmlLoader::RegisterClickHandler(const std::string& methodName, 
                                          std::function<void()> handler) {
    m_baseLoader->RegisterClickHandler(methodName, handler);
}

void MvvmXmlLoader::RegisterValueChangedHandler(const std::string& methodName, 
                                                 std::function<void(double)> handler) {
    m_baseLoader->RegisterValueChangedHandler(methodName, handler);
}

void MvvmXmlLoader::RegisterTextChangedHandler(const std::string& methodName, 
                                                std::function<void(const std::wstring&)> handler) {
    m_baseLoader->RegisterTextChangedHandler(methodName, handler);
}

void MvvmXmlLoader::SetDataContext(std::shared_ptr<INotifyPropertyChanged> context) {
    m_dataContext = context;
    ConnectBindings();
}

void MvvmXmlLoader::ConnectBindings() {
    if (!m_dataContext) {
        utils::Logger::Warning("[MVVM] Cannot connect bindings: DataContext is null");
        return;
    }
    
    utils::Logger::InfoF("[MVVM] Connecting %zu pending bindings", m_pendingBindings.size());
    
    for (auto& pending : m_pendingBindings) {
        auto control = pending.control.lock();
        if (!control) continue;
        
        CreateBinding(control, pending.propertyName, pending.expression);
    }
    
    m_pendingBindings.clear();
}

void MvvmXmlLoader::RegisterConverter(const std::string& name, 
                                       std::shared_ptr<IValueConverter> converter) {
    BindingEngine::Instance().RegisterConverter(name, converter);
}

void MvvmXmlLoader::BindEventsFromInstance(void* instance) {
    // TODO: 实现反射式事件绑定
    (void)instance;
}

// ============================================================================
// 绑定表达式处理
// ============================================================================
bool MvvmXmlLoader::IsBindingExpression(const std::string& value) {
    // 检查是否以 {Binding 开头，以 } 结尾
    if (value.length() < 10) return false;  // "{Binding }" 最小长度
    if (value.front() != '{' || value.back() != '}') return false;
    // 检查是否包含 "Binding"
    return value.find("Binding") != std::string::npos;
}

BindingExpression MvvmXmlLoader::ParseBinding(const std::string& expression) {
    return BindingEngine::Instance().ParseExpression(expression);
}

// ============================================================================
// 应用绑定 - 在加载后调用
// ============================================================================
void MvvmXmlLoader::ApplyBindings() {
    // 如果有 DataContext，立即连接绑定
    if (m_dataContext) {
        ConnectBindings();
    }
}

// ============================================================================
// 检查绑定表达式是否与控件严格匹配
// 控件当前的属性值应该等于绑定表达式字符串
// ============================================================================
static bool IsBindingValidForControl(const PendingBindingInfo& bindingInfo,
                                      const std::shared_ptr<luaui::Control>& control) {
    const std::string& propertyName = bindingInfo.propertyName;
    const std::string& expressionStr = bindingInfo.expressionString;
    
    // 获取控件当前的属性值（基础 XML 加载器设置的）
    std::string currentValue;
    if (propertyName == "Text") {
        if (auto tb = std::dynamic_pointer_cast<luaui::controls::TextBlock>(control)) {
            auto wtext = tb->GetText();
            currentValue = std::string(wtext.begin(), wtext.end());
        } else if (auto tx = std::dynamic_pointer_cast<luaui::controls::TextBox>(control)) {
            auto wtext = tx->GetText();
            currentValue = std::string(wtext.begin(), wtext.end());
        }
    } else if (propertyName == "Value") {
        // 对于数值属性，检查控件类型即可
        bool isSlider = std::dynamic_pointer_cast<luaui::controls::Slider>(control) != nullptr;
        bool isProgressBar = std::dynamic_pointer_cast<luaui::controls::ProgressBar>(control) != nullptr;
        
        // Slider 绑定通常包含 Mode=TwoWay
        bool isTwoWay = expressionStr.find("Mode=TwoWay") != std::string::npos;
        
        if (isTwoWay && isSlider) return true;
        if (!isTwoWay && isProgressBar) return true;
        return false;
    }
    
    // 对于 Text 属性，当前值应该等于绑定表达式
    return currentValue == expressionStr;
}

// ============================================================================
// 遍历控件树应用绑定（深度优先，子控件优先）
// ============================================================================
void MvvmXmlLoader::ApplyBindingsToControl(const std::shared_ptr<luaui::Control>& control) {
    if (!control) return;
    
    // 先递归处理子控件（深度优先）
    size_t childCount = control->GetChildCount();
    for (size_t i = 0; i < childCount; ++i) {
        auto child = control->GetChild(i);
        if (auto childControl = std::dynamic_pointer_cast<luaui::Control>(child)) {
            ApplyBindingsToControl(childControl);
        }
    }
    
    // 然后处理当前控件
    if (m_pendingBindingInfos.empty()) return;
    
    // 获取控件名称
    std::string controlName = control->GetName();
    std::string controlType = control->GetTypeName();
    
    // 查找匹配此控件的绑定信息
    for (auto it = m_pendingBindingInfos.begin(); it != m_pendingBindingInfos.end(); ) {
        // 检查绑定表达式是否与控件类型严格匹配
        if (!IsBindingValidForControl(*it, control)) {
            ++it;
            continue;
        }
        
        // 如果 binding info 有指定名称，按名称匹配；否则按类型匹配
        bool match = false;
        if (!it->elementName.empty()) {
            match = (it->elementName == controlName);
        } else {
            // 没有名称的控件，按类型匹配（每个控件只取第一个匹配的属性）
            match = true;
        }
        
        if (match) {
            // 解析绑定表达式
            auto expression = ParseBinding(it->expressionString);
            if (expression.isValid()) {
                utils::Logger::DebugF("[MVVM] Creating binding for %s.%s -> %s",
                    controlType.c_str(),
                    it->propertyName.c_str(),
                    expression.path.c_str());
                
                // 如果已有 DataContext，立即创建绑定
                if (m_dataContext) {
                    CreateBinding(control, it->propertyName, expression);
                } else {
                    // 否则存储为待处理绑定
                    PendingBinding pending;
                    pending.control = control;
                    pending.propertyName = it->propertyName;
                    pending.expression = expression;
                    m_pendingBindings.push_back(pending);
                }
            }
            
            // 移除已处理的绑定信息，继续处理下一个匹配项
            it = m_pendingBindingInfos.erase(it);
        } else {
            ++it;
        }
    }
}

// ============================================================================
// 根据名称查找控件
// ============================================================================
std::shared_ptr<luaui::Control> MvvmXmlLoader::FindControlByName(
    const std::shared_ptr<luaui::Control>& root, 
    const std::string& name) {
    if (!root) return nullptr;
    if (root->GetName() == name) return root;
    
    size_t childCount = root->GetChildCount();
    for (size_t i = 0; i < childCount; ++i) {
        auto child = root->GetChild(i);
        if (auto childControl = std::dynamic_pointer_cast<luaui::Control>(child)) {
            if (auto found = FindControlByName(childControl, name)) {
                return found;
            }
        }
    }
    return nullptr;
}

// ============================================================================
// 属性值获取辅助函数
// ============================================================================
static std::any GetPropertyValue(INotifyPropertyChanged* viewModel, const std::string& propertyName) {
    // 这里简化处理，实际应该通过反射获取
    // 返回空值，由订阅回调处理
    (void)viewModel;
    (void)propertyName;
    return std::any{};
}

// ============================================================================
// 创建实际绑定 - 核心实现
// ============================================================================
void MvvmXmlLoader::CreateBinding(const std::shared_ptr<luaui::Control>& control,
                                  const std::string& propertyName,
                                  const BindingExpression& expression) {
    if (!control || !m_dataContext) return;
    
    utils::Logger::DebugF("[MVVM] Creating binding: %s.%s -> %s (Mode=%d)",
        control->GetTypeName().c_str(),
        propertyName.c_str(),
        expression.path.c_str(),
        static_cast<int>(expression.mode));
    
    // 根据控件类型路由到具体绑定实现
    if (auto textBlock = std::dynamic_pointer_cast<luaui::controls::TextBlock>(control)) {
        if (propertyName == "Text") {
            BindTextBlock(textBlock, propertyName, expression);
        }
    }
    else if (auto textBox = std::dynamic_pointer_cast<luaui::controls::TextBox>(control)) {
        if (propertyName == "Text") {
            BindTextBox(textBox, propertyName, expression);
        }
    }
    else if (auto progressBar = std::dynamic_pointer_cast<luaui::controls::ProgressBar>(control)) {
        if (propertyName == "Value") {
            BindProgressBar(progressBar, propertyName, expression);
        }
    }
    else if (auto slider = std::dynamic_pointer_cast<luaui::controls::Slider>(control)) {
        if (propertyName == "Value") {
            BindSlider(slider, propertyName, expression);
        }
    }
    else if (auto listBox = std::dynamic_pointer_cast<luaui::controls::ListBox>(control)) {
        if (propertyName == "ItemsSource") {
            BindListBox(listBox, propertyName, expression);
        }
    }
}

// ============================================================================
// TextBlock 绑定 - OneWay（VM -> View）
// ============================================================================
void MvvmXmlLoader::BindTextBlock(std::shared_ptr<luaui::controls::TextBlock> textBlock,
                                  const std::string& /*propertyName*/,
                                  const BindingExpression& expression) {
    auto converter = expression.converter;
    auto dataContext = m_dataContext;
    auto boundPropertyName = expression.path;
    
    // 更新函数
    auto converterParameter = expression.converterParameter;
    auto updateView = [textBlock, dataContext, boundPropertyName, converter, converterParameter]() {
        // 从 ViewModel 获取属性值
        std::any value = dataContext->GetPropertyValue(boundPropertyName);
        
        if (!value.has_value()) {
            return;
        }
        
        // 应用转换器
        if (converter) {
            value = converter->Convert(value, converterParameter);
        }
        
        // 设置到控件
        try {
            if (value.type() == typeid(std::string)) {
                std::string str = std::any_cast<std::string>(value);
                textBlock->SetText(std::wstring(str.begin(), str.end()));
            } else if (value.type() == typeid(std::wstring)) {
                textBlock->SetText(std::any_cast<std::wstring>(value));
            } else if (value.type() == typeid(double)) {
                double val = std::any_cast<double>(value);
                textBlock->SetText(std::to_wstring(val));
            } else if (value.type() == typeid(int)) {
                int val = std::any_cast<int>(value);
                textBlock->SetText(std::to_wstring(val));
            } else if (value.type() == typeid(bool)) {
                bool val = std::any_cast<bool>(value);
                textBlock->SetText(val ? L"True" : L"False");
            }
        } catch (...) {
            // 忽略转换错误
        }
    };
    
    // 初始更新
    updateView();
    
    // 订阅属性变更
    if (expression.mode != BindingMode::OneTime) {
        dataContext->SubscribePropertyChanged(
            [boundPropertyName, updateView](const PropertyChangedEventArgs& args) {
            if (args.propertyName == boundPropertyName || args.propertyName.empty()) {
                updateView();
            }
        });
    }
}

// ============================================================================
// TextBox 绑定 - TwoWay（VM <-> View）
// ============================================================================
void MvvmXmlLoader::BindTextBox(std::shared_ptr<luaui::controls::TextBox> textBox,
                                const std::string& /*propertyName*/,
                                const BindingExpression& expression) {
    auto dataContext = m_dataContext;
    auto boundPropertyName = expression.path;
    
    // VM -> View 更新
    if (expression.mode != BindingMode::OneWayToSource && 
        expression.mode != BindingMode::OneTime) {
        
        // 初始值更新
        std::any initialValue = dataContext->GetPropertyValue(boundPropertyName);
        if (initialValue.type() == typeid(std::string)) {
            std::string str = std::any_cast<std::string>(initialValue);
            textBox->SetText(std::wstring(str.begin(), str.end()));
        } else if (initialValue.type() == typeid(std::wstring)) {
            textBox->SetText(std::any_cast<std::wstring>(initialValue));
        }
        
        // 订阅变更
        dataContext->SubscribePropertyChanged(
            [textBox, dataContext, boundPropertyName](const PropertyChangedEventArgs& args) {
            if (args.propertyName == boundPropertyName || args.propertyName.empty()) {
                std::any value = dataContext->GetPropertyValue(boundPropertyName);
                if (value.type() == typeid(std::string)) {
                    std::string str = std::any_cast<std::string>(value);
                    textBox->SetText(std::wstring(str.begin(), str.end()));
                } else if (value.type() == typeid(std::wstring)) {
                    textBox->SetText(std::any_cast<std::wstring>(value));
                }
            }
        });
    }
    
    // View -> VM (TwoWay)
    if (expression.mode == BindingMode::TwoWay || 
        expression.mode == BindingMode::OneWayToSource) {
        
        // 使用 TextBox 的 TextChanged 事件（如果已添加）
        textBox->TextChanged.Add([dataContext, boundPropertyName](luaui::controls::TextBox*, const std::wstring& text) {
            std::string str(text.begin(), text.end());
            dataContext->SetPropertyValue(boundPropertyName, str);
            utils::Logger::DebugF("[MVVM] TextBox changed: %s -> ViewModel.%s", 
                str.c_str(), boundPropertyName.c_str());
        });
    }
}

// ============================================================================
// ProgressBar 绑定 - OneWay
// ============================================================================
void MvvmXmlLoader::BindProgressBar(std::shared_ptr<luaui::controls::ProgressBar> progressBar,
                                    const std::string& /*propertyName*/,
                                    const BindingExpression& expression) {
    auto dataContext = m_dataContext;
    auto boundPropertyName = expression.path;
    
    // 辅助函数：应用值到控件
    auto applyValue = [progressBar](const std::any& value) {
        try {
            if (value.type() == typeid(double)) {
                progressBar->SetValue(std::any_cast<double>(value));
            } else if (value.type() == typeid(int)) {
                progressBar->SetValue(static_cast<double>(std::any_cast<int>(value)));
            } else if (value.type() == typeid(float)) {
                progressBar->SetValue(static_cast<double>(std::any_cast<float>(value)));
            }
        } catch (...) {
            // 忽略类型不匹配
        }
    };
    
    // VM -> View
    if (expression.mode != BindingMode::OneWayToSource) {
        // 初始值
        applyValue(dataContext->GetPropertyValue(boundPropertyName));
        
        // 订阅变更
        dataContext->SubscribePropertyChanged(
            [dataContext, boundPropertyName, applyValue](const PropertyChangedEventArgs& args) {
            if (args.propertyName == boundPropertyName || args.propertyName.empty()) {
                applyValue(dataContext->GetPropertyValue(boundPropertyName));
            }
        });
    }
}

// ============================================================================
// Slider 绑定 - TwoWay
// ============================================================================
void MvvmXmlLoader::BindSlider(std::shared_ptr<luaui::controls::Slider> slider,
                               const std::string& /*propertyName*/,
                               const BindingExpression& expression) {
    auto dataContext = m_dataContext;
    auto boundPropertyName = expression.path;
    
    // 辅助函数：应用值到控件
    auto applyValue = [slider](const std::any& value) {
        try {
            if (value.type() == typeid(double)) {
                slider->SetValue(std::any_cast<double>(value));
            } else if (value.type() == typeid(int)) {
                slider->SetValue(static_cast<double>(std::any_cast<int>(value)));
            } else if (value.type() == typeid(float)) {
                slider->SetValue(static_cast<double>(std::any_cast<float>(value)));
            }
        } catch (...) {}
    };
    
    // View -> VM (TwoWay)
    if (expression.mode == BindingMode::TwoWay || 
        expression.mode == BindingMode::OneWayToSource) {
        
        slider->ValueChanged.Add([dataContext, boundPropertyName](luaui::controls::Slider*, double value) {
            dataContext->SetPropertyValue(boundPropertyName, value);
            utils::Logger::DebugF("[MVVM] Slider value changed: %.1f -> ViewModel.%s", 
                value, boundPropertyName.c_str());
        });
    }
    
    // VM -> View
    if (expression.mode == BindingMode::TwoWay || 
        expression.mode == BindingMode::OneWay) {
        
        // 初始值
        applyValue(dataContext->GetPropertyValue(boundPropertyName));
        
        // 订阅变更
        dataContext->SubscribePropertyChanged(
            [dataContext, boundPropertyName, applyValue](const PropertyChangedEventArgs& args) {
            if (args.propertyName == boundPropertyName || args.propertyName.empty()) {
                applyValue(dataContext->GetPropertyValue(boundPropertyName));
            }
        });
    }
}

// ============================================================================
// ListBox 绑定 - ItemsSource（集合绑定）
// ============================================================================
void MvvmXmlLoader::BindListBox(std::shared_ptr<luaui::controls::ListBox> listBox,
                                const std::string& propertyName,
                                const BindingExpression& expression) {
    (void)propertyName;
    utils::Logger::InfoF("[MVVM] Binding ListBox.ItemsSource to %s", expression.path.c_str());
    
    // TODO: 实现集合绑定
    // 需要：
    // 1. 从 ViewModel 获取 ObservableCollection
    // 2. 监听 CollectionChanged 事件
    // 3. 同步到 ListBox 的 Items
    // 4. 支持 SelectedItem 双向绑定
    
    (void)listBox;
}

// ============================================================================
// 创建 MVVM XML 加载器
// ============================================================================
std::shared_ptr<MvvmXmlLoader> CreateMvvmXmlLoader() {
    return std::make_shared<MvvmXmlLoader>();
}

} // namespace mvvm
} // namespace luaui
