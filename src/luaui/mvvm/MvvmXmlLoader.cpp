#include "MvvmXmlLoader.h"
#include "Logger.h"
#include "Converters.h"

// 首先包含 Core Control 基类定义
#include "../core/Control.h"

// Controls - 统一包含所有控件
#include "Controls.h"
#include "Button.h"
#include "ComboBox.h"

// Lua binding for collection support
#include "../lua/LuaObservableCollection.h"
#include "../lua/LuaAwareMvvmLoader.h"

#include <tinyxml2.h>
#include <sstream>
#include <algorithm>
#include <Windows.h>

namespace luaui {
namespace mvvm {
namespace {

std::wstring Utf8ToW(const std::string& s) {
    if (s.empty()) return std::wstring();
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    if (n <= 0) return std::wstring();
    std::wstring r(n - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &r[0], n);
    return r;
}

struct DataGridColumnSpec {
    std::string header;
    std::string path;
};

std::string TrimString(const std::string& value) {
    size_t first = value.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) {
        return "";
    }

    size_t last = value.find_last_not_of(" \t\n\r");
    return value.substr(first, last - first + 1);
}

std::vector<std::string> SplitString(const std::string& value, char delimiter) {
    std::vector<std::string> parts;
    std::stringstream ss(value);
    std::string part;
    while (std::getline(ss, part, delimiter)) {
        part = TrimString(part);
        if (!part.empty()) {
            parts.push_back(part);
        }
    }
    return parts;
}

std::wstring Utf8ToWString(const std::string& value) {
    if (value.empty()) {
        return L"";
    }

    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, nullptr, 0);
    if (sizeNeeded <= 0) {
        return L"";
    }

    std::wstring result(sizeNeeded - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, result.data(), sizeNeeded);
    return result;
}

bool PushLuaViewModel(lua_State* L) {
    if (!L) {
        return false;
    }

    lua_getglobal(L, "ViewModelInstance");
    if (lua_istable(L, -1)) {
        return true;
    }

    lua_pop(L, 1);
    lua_getglobal(L, "TestRunnerViewModel");
    if (lua_istable(L, -1)) {
        return true;
    }

    lua_pop(L, 1);
    return false;
}

bool PushLuaPathFromTable(lua_State* L, int tableIndex, const std::string& path) {
    if (!L || path.empty()) {
        return false;
    }

    int absTableIndex = lua_absindex(L, tableIndex);
    lua_pushvalue(L, absTableIndex);

    for (const auto& part : SplitString(path, '.')) {
        if (!lua_istable(L, -1)) {
            lua_pop(L, 1);
            return false;
        }

        lua_getfield(L, -1, part.c_str());
        lua_remove(L, -2);
    }

    return true;
}

bool PushLuaCollection(lua_State* L, const std::string& path) {
    if (!PushLuaViewModel(L)) {
        return false;
    }

    if (!PushLuaPathFromTable(L, -1, path)) {
        lua_pop(L, 1);
        return false;
    }

    lua_remove(L, -2);

    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return false;
    }

    lua_getfield(L, -1, "_items");
    if (lua_istable(L, -1)) {
        lua_remove(L, -2);
        return true;
    }

    lua_pop(L, 1);
    return true;
}

std::wstring LuaValueToDisplayText(lua_State* L, int valueIndex) {
    if (!L) {
        return L"";
    }

    int absValueIndex = lua_absindex(L, valueIndex);

    if (lua_isnil(L, absValueIndex)) {
        return L"";
    }

    if (lua_isboolean(L, absValueIndex)) {
        return lua_toboolean(L, absValueIndex) ? L"True" : L"False";
    }

    if (lua_isinteger(L, absValueIndex)) {
        return std::to_wstring(static_cast<long long>(lua_tointeger(L, absValueIndex)));
    }

    if (lua_isnumber(L, absValueIndex)) {
        std::ostringstream stream;
        stream << lua_tonumber(L, absValueIndex);
        return Utf8ToWString(stream.str());
    }

    if (lua_isstring(L, absValueIndex)) {
        return Utf8ToWString(lua_tostring(L, absValueIndex));
    }

    if (lua_istable(L, absValueIndex)) {
        lua_getfield(L, absValueIndex, "Name");
        if (lua_isstring(L, -1)) {
            std::wstring value = Utf8ToWString(lua_tostring(L, -1));
            lua_pop(L, 1);
            return value;
        }
        lua_pop(L, 1);
    }

    lua_getglobal(L, "tostring");
    lua_pushvalue(L, absValueIndex);
    if (lua_pcall(L, 1, 1, 0) == LUA_OK) {
        std::wstring value = lua_isstring(L, -1) ? Utf8ToWString(lua_tostring(L, -1)) : L"";
        lua_pop(L, 1);
        return value;
    }

    lua_pop(L, 1);
    return L"";
}

std::vector<DataGridColumnSpec> ParseDataGridColumnSpecs(const std::string& parameter) {
    std::vector<DataGridColumnSpec> specs;
    if (parameter.empty()) {
        return specs;
    }

    for (const auto& token : SplitString(parameter, '|')) {
        DataGridColumnSpec spec;
        size_t separator = token.find(':');
        if (separator == std::string::npos) {
            spec.header = token;
            spec.path = token;
        } else {
            spec.header = TrimString(token.substr(0, separator));
            spec.path = TrimString(token.substr(separator + 1));
        }

        if (!spec.path.empty()) {
            if (spec.header.empty()) {
                spec.header = spec.path;
            }
            specs.push_back(spec);
        }
    }

    return specs;
}

std::vector<DataGridColumnSpec> BuildDataGridColumnSpecsFromLuaCollection(
    lua_State* L,
    int collectionIndex,
    const std::string& parameter) {
    std::vector<DataGridColumnSpec> specs = ParseDataGridColumnSpecs(parameter);
    if (!specs.empty()) {
        return specs;
    }

    int absCollectionIndex = lua_absindex(L, collectionIndex);
    if (lua_rawlen(L, absCollectionIndex) == 0) {
        return specs;
    }

    lua_rawgeti(L, absCollectionIndex, 1);
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        specs.push_back({"Value", ""});
        return specs;
    }

    std::vector<std::string> keys;
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        if (lua_type(L, -2) == LUA_TSTRING && lua_type(L, -1) != LUA_TFUNCTION) {
            std::string key = lua_tostring(L, -2);
            if (!key.empty() && key[0] != '_') {
                keys.push_back(key);
            }
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    std::sort(keys.begin(), keys.end());
    for (const auto& key : keys) {
        specs.push_back({key, key});
    }

    return specs;
}

std::vector<DataGridColumnSpec> BuildDataGridColumnSpecsFromControl(
    const std::shared_ptr<luaui::controls::DataGrid>& dataGrid) {
    std::vector<DataGridColumnSpec> specs;
    if (!dataGrid) {
        return specs;
    }

    for (size_t i = 0; i < dataGrid->GetColumnCount(); ++i) {
        auto column = dataGrid->GetColumn(i);
        if (!column) {
            continue;
        }

        std::string header(column->GetHeader().begin(), column->GetHeader().end());
        std::string path(column->GetBindingPath().begin(), column->GetBindingPath().end());
        if (path.empty()) {
            path = header;
        }

        if (!path.empty()) {
            specs.push_back({header.empty() ? path : header, path});
        }
    }

    return specs;
}

std::wstring GetLuaCellText(lua_State* L, int itemIndex, const DataGridColumnSpec& spec) {
    if (!L) {
        return L"";
    }

    int absItemIndex = lua_absindex(L, itemIndex);
    if (spec.path.empty() || !lua_istable(L, absItemIndex)) {
        return LuaValueToDisplayText(L, absItemIndex);
    }

    if (!PushLuaPathFromTable(L, absItemIndex, spec.path)) {
        return L"";
    }

    std::wstring value = LuaValueToDisplayText(L, -1);
    lua_pop(L, 1);
    return value;
}

} // namespace

// ============================================================================
// MvvmXmlLoader 实现
// ============================================================================
MvvmXmlLoader::MvvmXmlLoader() 
    : m_baseLoader(xml::CreateXmlLoader()) 
{
}

std::shared_ptr<luaui::Control> MvvmXmlLoader::Load(const std::string& filePath) {
    utils::Logger::InfoF("[MVVM] Loading XML: %s", filePath.c_str());
    
    // 清空之前的待处理绑定
    m_pendingBindings.clear();
    
    // 使用基础加载器加载控件树（会自动记录延迟绑定）
    auto root = m_baseLoader->Load(filePath);
    m_rootControl = root;
    
    if (!root) {
        utils::Logger::ErrorF("[MVVM] Failed to load XML: %s", filePath.c_str());
        return nullptr;
    }
    
    // 获取所有延迟绑定
    auto deferredBindings = m_baseLoader->GetDeferredBindings();
    utils::Logger::InfoF("[MVVM] Found %zu deferred bindings", deferredBindings.size());
    
    // 转换为待处理绑定
    for (auto& deferred : deferredBindings) {
        auto control = deferred.control.lock();
        if (!control) continue;
        
        // 解析绑定表达式
        auto expression = BindingEngine::Instance().ParseExpression(deferred.bindingExpression);
        if (expression.isValid()) {
            PendingBinding pending;
            pending.control = deferred.control;
            pending.propertyName = deferred.propertyName;
            pending.expression = expression;
            m_pendingBindings.push_back(pending);
            
            utils::Logger::DebugF("[MVVM] Queued binding: %s.%s -> %s",
                control->GetTypeName().c_str(),
                deferred.propertyName.c_str(),
                expression.path.c_str());
        }
    }
    
    // 如果已有 DataContext，立即应用绑定
    if (m_dataContext) {
        ApplyBindings();
    }
    
    return root;
}

std::shared_ptr<luaui::Control> MvvmXmlLoader::LoadFromString(const std::string& xmlString) {
    // 清空之前的待处理绑定
    m_pendingBindings.clear();
    
    // 使用基础加载器加载控件树
    auto root = m_baseLoader->LoadFromString(xmlString);
    m_rootControl = root;
    
    if (!root) {
        utils::Logger::Error("[MVVM] Failed to parse XML string");
        return nullptr;
    }
    
    // 获取所有延迟绑定
    auto deferredBindings = m_baseLoader->GetDeferredBindings();
    
    // 转换为待处理绑定
    for (auto& deferred : deferredBindings) {
        auto control = deferred.control.lock();
        if (!control) continue;
        
        auto expression = BindingEngine::Instance().ParseExpression(deferred.bindingExpression);
        if (expression.isValid()) {
            PendingBinding pending;
            pending.control = deferred.control;
            pending.propertyName = deferred.propertyName;
            pending.expression = expression;
            m_pendingBindings.push_back(pending);
        }
    }
    
    // 如果已有 DataContext，立即应用绑定
    if (m_dataContext) {
        ApplyBindings();
    }
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
        const char* attrName = attr->Name();
        const char* attrValue = attr->Value();
        
        if (attrValue && IsBindingExpression(attrValue)) {
            utils::Logger::DebugF("[MVVM] Found binding: %s.%s = %s", 
                controlName.c_str(), attrName, attrValue);
            
            // 存储绑定信息，等待控件加载完成后解析
            PendingBindingInfo info;
            info.elementName = controlName;
            info.propertyName = attrName ? attrName : "";
            info.expressionString = attrValue;
            info.controlTag = element->Name();  // 记录控件标签类型
            info.index = static_cast<int>(m_pendingBindingInfos.size());  // 记录索引
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
// 检查绑定表达式是否与控件类型匹配
// ============================================================================
static bool IsBindingValidForControl(const PendingBindingInfo& bindingInfo,
                                      const std::shared_ptr<luaui::Control>& control) {
    const std::string& propertyName = bindingInfo.propertyName;
    const std::string& expressionStr = bindingInfo.expressionString;
    
    // 根据属性名和控件类型进行匹配验证
    if (propertyName == "Text") {
        // Text 属性可以绑定到 TextBlock 或 TextBox
        if (std::dynamic_pointer_cast<luaui::controls::TextBlock>(control) != nullptr) return true;
        if (std::dynamic_pointer_cast<luaui::controls::TextBox>(control) != nullptr) return true;
        return false;
    } else if (propertyName == "Value") {
        // 对于数值属性，检查控件类型
        bool isSlider = std::dynamic_pointer_cast<luaui::controls::Slider>(control) != nullptr;
        bool isProgressBar = std::dynamic_pointer_cast<luaui::controls::ProgressBar>(control) != nullptr;
        
        // Slider 绑定通常包含 Mode=TwoWay
        bool isTwoWay = expressionStr.find("Mode=TwoWay") != std::string::npos;
        
        if (isTwoWay && isSlider) return true;
        if (!isTwoWay && isProgressBar) return true;
        return false;
    } else if (propertyName == "IsChecked") {
        // 对于 IsChecked 属性，检查控件类型
        bool isCheckBox = std::dynamic_pointer_cast<luaui::controls::CheckBox>(control) != nullptr;
        bool isRadioButton = std::dynamic_pointer_cast<luaui::controls::RadioButton>(control) != nullptr;
        
        if (isCheckBox || isRadioButton) return true;
        return false;
    } else if (propertyName == "ItemsSource") {
        bool isListBox = std::dynamic_pointer_cast<luaui::controls::ListBox>(control) != nullptr;
        bool isComboBox = std::dynamic_pointer_cast<luaui::controls::ComboBox>(control) != nullptr;
        bool isDataGrid = std::dynamic_pointer_cast<luaui::controls::DataGrid>(control) != nullptr;
        return isListBox || isComboBox || isDataGrid;
    }
    
    // 未知属性，允许绑定尝试
    return true;
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
    
    // 获取控件名称和类型
    std::string controlName = control->GetName();
    std::string controlType = control->GetTypeName();
    
    // 查找匹配此控件的绑定信息
    // 策略：
    // 1. 如果控件有名称，按名称匹配
    // 2. 如果控件没有名称，按类型匹配并使用索引确保顺序
    
    for (auto it = m_pendingBindingInfos.begin(); it != m_pendingBindingInfos.end(); ) {
        // 检查绑定表达式是否与控件类型严格匹配
        if (!IsBindingValidForControl(*it, control)) {
            ++it;
            continue;
        }
        
        bool match = false;
        
        if (!controlName.empty() && !it->elementName.empty()) {
            // 两个都有名称，按名称匹配
            match = (it->elementName == controlName);
        } else if (controlName.empty() && it->elementName.empty()) {
            // 都没有名称，按类型匹配
            // 检查控件类型是否与绑定的控件类型一致
            match = (it->controlTag == controlType);
        }
        // 其他情况（一个有名一个没名）不匹配
        
        if (match) {
            // 解析绑定表达式
            auto expression = ParseBinding(it->expressionString);
            if (expression.isValid()) {
                utils::Logger::DebugF("[MVVM] Creating binding for %s.%s -> %s (index=%d)",
                    controlType.c_str(),
                    it->propertyName.c_str(),
                    expression.path.c_str(),
                    it->index);
                
                // 清空原始的绑定表达式文本，避免显示 {Binding XXX}
                ClearBindingExpressionText(control, it->propertyName);
                
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
            
            // 移除已处理的绑定信息
            it = m_pendingBindingInfos.erase(it);
            return;  // 一个控件只处理一个绑定
        } else {
            ++it;
        }
    }
}

// 清空绑定表达式文本，避免显示 {Binding XXX}
void MvvmXmlLoader::ClearBindingExpressionText(const std::shared_ptr<luaui::Control>& control,
                                                const std::string& propertyName) {
    // 对于 TextBlock，如果当前文本是绑定表达式，清空它
    if (auto textBlock = std::dynamic_pointer_cast<luaui::controls::TextBlock>(control)) {
        if (propertyName == "Text") {
            std::wstring currentText = textBlock->GetText();
            std::wstring bindingPrefix = L"{Binding ";
            if (currentText.find(bindingPrefix) != std::wstring::npos || 
                currentText.find(L"{") != std::wstring::npos) {
                textBlock->SetText(L"");
            }
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
        } else if (propertyName == "SelectedItem" || propertyName == "SelectedIndex") {
            BindListBoxSelectedItem(listBox, expression);
        }
    }
    else if (auto dataGrid = std::dynamic_pointer_cast<luaui::controls::DataGrid>(control)) {
        if (propertyName == "ItemsSource") {
            BindDataGrid(dataGrid, propertyName, expression);
        }
    }
    else if (auto comboBox = std::dynamic_pointer_cast<luaui::controls::ComboBox>(control)) {
        if (propertyName == "ItemsSource" || propertyName == "SelectedItem" || propertyName == "SelectedIndex") {
            BindComboBox(comboBox, propertyName, expression);
        }
    }
    else if (auto checkBox = std::dynamic_pointer_cast<luaui::controls::CheckBox>(control)) {
        if (propertyName == "IsChecked") {
            BindCheckBox(checkBox, propertyName, expression);
        }
    }
    else if (auto radioButton = std::dynamic_pointer_cast<luaui::controls::RadioButton>(control)) {
        if (propertyName == "IsChecked") {
            BindRadioButton(radioButton, propertyName, expression);
        }
    }
    else if (auto button = std::dynamic_pointer_cast<luaui::controls::Button>(control)) {
        if (propertyName == "Command") {
            BindButtonCommand(button, expression);
        } else if (propertyName == "Text" || propertyName == "Content") {
            BindButtonText(button, expression);
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
            utils::Logger::DebugF("[MVVM] GetPropertyValue('%s') returned empty", boundPropertyName.c_str());
            return;
        }
        
        utils::Logger::DebugF("[MVVM] GetPropertyValue('%s') success, type=%s", 
            boundPropertyName.c_str(), value.type().name());
        
        // 应用转换器
        if (converter) {
            value = converter->Convert(value, converterParameter);
        }
        
        // 设置到控件
        try {
            if (value.type() == typeid(std::string)) {
                std::string str = std::any_cast<std::string>(value);
                utils::Logger::DebugF("[MVVM] Setting TextBlock text to: %s", str.c_str());
                textBlock->SetText(Utf8ToW(str));
            } else if (value.type() == typeid(std::wstring)) {
                std::wstring str = std::any_cast<std::wstring>(value);
                utils::Logger::DebugF("[MVVM] Setting TextBlock text to (wstring)");
                textBlock->SetText(str);
            } else if (value.type() == typeid(double)) {
                double val = std::any_cast<double>(value);
                utils::Logger::DebugF("[MVVM] Setting TextBlock text to (double): %f", val);
                textBlock->SetText(std::to_wstring(val));
            } else if (value.type() == typeid(int)) {
                int val = std::any_cast<int>(value);
                utils::Logger::DebugF("[MVVM] Setting TextBlock text to (int): %d", val);
                textBlock->SetText(std::to_wstring(val));
            } else if (value.type() == typeid(bool)) {
                bool val = std::any_cast<bool>(value);
                utils::Logger::DebugF("[MVVM] Setting TextBlock text to (bool): %s", val ? "true" : "false");
                textBlock->SetText(val ? L"True" : L"False");
            } else {
                utils::Logger::DebugF("[MVVM] Unknown value type: %s", value.type().name());
            }
        } catch (...) {
            // 忽略转换错误
            utils::Logger::Debug("[MVVM] Exception in SetText");
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
            textBox->SetText(Utf8ToW(str));
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
                    textBox->SetText(Utf8ToW(str));
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
// ListBox 绑定 - ItemsSource（集合绑定，支持增量更新）
// ============================================================================
void MvvmXmlLoader::BindListBox(std::shared_ptr<luaui::controls::ListBox> listBox,
                                const std::string& propertyName,
                                const BindingExpression& expression) {
    (void)propertyName;
    utils::Logger::InfoF("[MVVM] Binding ListBox.ItemsSource to %s (with incremental updates)",
        expression.path.c_str());

    auto dataContext = m_dataContext;
    if (!dataContext) {
        utils::Logger::Error("[MVVM] No DataContext available for ListBox binding");
        return;
    }

    auto luaDataContext = std::dynamic_pointer_cast<lua::LuaPropertyNotifier>(dataContext);
    if (!luaDataContext) {
        std::any value = dataContext->GetPropertyValue(expression.path);
        if (value.type() == typeid(std::vector<std::wstring>)) {
            auto items = std::any_cast<std::vector<std::wstring>>(value);
            for (const auto& item : items) {
                listBox->AddItem(item);
            }
        }
        return;
    }

    lua_State* L = luaDataContext->GetLuaState();
    if (!L) {
        utils::Logger::Error("[MVVM] Lua state not available");
        return;
    }

    lua_getglobal(L, "ViewModelInstance");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_getglobal(L, "TestRunnerViewModel");
    }

    if (!lua_istable(L, -1)) {
        utils::Logger::Error("[MVVM] ViewModel not found in Lua");
        lua_pop(L, 1);
        return;
    }

    lua_getfield(L, -1, expression.path.c_str());
    if (!lua_istable(L, -1)) {
        utils::Logger::Warning("[MVVM] Property '" + expression.path + "' is not a table");
        lua_pop(L, 2);
        return;
    }

    lua_getfield(L, -1, "Subscribe");
    bool isObservableCollection = lua_isfunction(L, -1);
    lua_pop(L, 1);

    auto collection = std::make_shared<lua::LuaObservableCollection>(L, -1);

    if (!expression.converterParameter.empty()) {
        collection->SetDisplayMemberPath(expression.converterParameter);
    }

    if (isObservableCollection) {
        utils::Logger::Info("[MVVM] Collection supports incremental updates");
        collection->EnableIncrementalUpdates(true);

        auto binding = std::make_shared<lua::ObservableCollectionBinding>(listBox, collection);
        m_collectionBindings.push_back(binding);

        utils::Logger::InfoF("[MVVM] ObservableCollectionBinding saved, total bindings: %zu",
            m_collectionBindings.size());
    } else {
        auto texts = collection->GetAllDisplayTexts();
        for (const auto& text : texts) {
            listBox->AddItem(text);
        }
        utils::Logger::Warning("[MVVM] Collection does not support incremental updates, "
            "consider using ObservableCollection.new()");
    }

    lua_pop(L, 2);

    if (expression.mode == BindingMode::OneWay || expression.mode == BindingMode::TwoWay) {
        dataContext->SubscribePropertyChanged(
            [listBox, expression, luaDataContext, isObservableCollection](const PropertyChangedEventArgs& args) {
            if (args.propertyName == expression.path) {
                utils::Logger::InfoF("[MVVM] Property changed: %s, isObservable=%d", 
                    args.propertyName.c_str(), isObservableCollection);
                if (isObservableCollection) {
                    // ObservableCollection 的更新由 ObservableCollectionBinding 处理
                    return;
                }
                // 非 ObservableCollection：属性变更时重新加载整个列表
                utils::Logger::InfoF("[MVVM] Clearing ListBox (had %zu items)", listBox->GetItemCount());
                listBox->ClearItems();
                utils::Logger::Info("[MVVM] ListBox cleared");

                lua_State* L = luaDataContext->GetLuaState();
                if (!L) {
                    utils::Logger::Error("[MVVM] Lua state is null");
                    return;
                }

                lua_getglobal(L, "ViewModelInstance");
                if (!lua_istable(L, -1)) {
                    lua_pop(L, 1);
                    lua_getglobal(L, "TestRunnerViewModel");
                }

                if (lua_istable(L, -1)) {
                    lua_getfield(L, -1, expression.path.c_str());
                    if (lua_istable(L, -1)) {
                        size_t count = lua_rawlen(L, -1);
                        utils::Logger::InfoF("[MVVM] Reloading %zu items", count);
                        auto newCollection = std::make_shared<lua::LuaObservableCollection>(L, -1);
                        auto texts = newCollection->GetAllDisplayTexts();
                        utils::Logger::InfoF("[MVVM] Got %zu display texts", texts.size());
                        for (const auto& text : texts) {
                            utils::Logger::InfoF("[MVVM] Adding item: %s", 
                                std::string(text.begin(), text.end()).c_str());
                            listBox->AddItem(text);
                        }
                        utils::Logger::InfoF("[MVVM] ListBox now has %zu items", listBox->GetItemCount());
                    } else {
                        utils::Logger::Warning("[MVVM] Property is not a table");
                    }
                    lua_pop(L, 1);
                } else {
                    utils::Logger::Warning("[MVVM] ViewModel not found");
                }
                lua_pop(L, 1);
            }
        });
    }
}

// ============================================================================
// ListBox SelectedItem 绑定 - 支持 TwoWay
// ============================================================================
// ============================================================================
// DataGrid 缁戝畾 - ItemsSource
// ============================================================================
void MvvmXmlLoader::BindDataGrid(std::shared_ptr<luaui::controls::DataGrid> dataGrid,
                                 const std::string& propertyName,
                                 const BindingExpression& expression) {
    (void)propertyName;

    auto dataContext = m_dataContext;
    if (!dataContext) {
        utils::Logger::Error("[MVVM] No DataContext available for DataGrid binding");
        return;
    }

    auto luaDataContext = std::dynamic_pointer_cast<lua::LuaPropertyNotifier>(dataContext);
    if (!luaDataContext) {
        utils::Logger::Warning("[MVVM] DataGrid.ItemsSource currently requires a Lua ViewModel");
        return;
    }

    lua_State* L = luaDataContext->GetLuaState();
    if (!L) {
        utils::Logger::Error("[MVVM] Lua state not available");
        return;
    }

    lua_getglobal(L, "ViewModelInstance");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_getglobal(L, "TestRunnerViewModel");
    }

    if (!lua_istable(L, -1)) {
        utils::Logger::Error("[MVVM] ViewModel not found in Lua");
        lua_pop(L, 1);
        return;
    }

    lua_getfield(L, -1, expression.path.c_str());
    if (!lua_istable(L, -1)) {
        utils::Logger::Warning("[MVVM] Property '" + expression.path + "' is not a table");
        lua_pop(L, 2);
        return;
    }

    lua_getfield(L, -1, "Subscribe");
    bool isObservableCollection = lua_isfunction(L, -1);
    lua_pop(L, 1);

    auto collection = std::make_shared<lua::LuaObservableCollection>(L, -1);

    if (!expression.converterParameter.empty()) {
        collection->SetDisplayMemberPath(expression.converterParameter);
    }

    auto columnSpecs = BuildDataGridColumnSpecsFromLuaCollection(L, -1, expression.converterParameter);
    dataGrid->ClearColumns();
    for (const auto& spec : columnSpecs) {
        auto column = std::make_shared<luaui::controls::DataGridColumn>(Utf8ToWString(spec.header));
        column->SetBindingPath(Utf8ToWString(spec.path));
        dataGrid->AddColumn(column);
    }

    int collectionIndex = lua_absindex(L, -1);
    size_t count = lua_rawlen(L, collectionIndex);
    for (size_t i = 0; i < count; ++i) {
        lua_rawgeti(L, collectionIndex, static_cast<int>(i) + 1);

        auto row = std::make_shared<luaui::controls::DataGridRow>();
        for (const auto& spec : columnSpecs) {
            auto cell = std::make_shared<luaui::controls::DataGridCell>();
            cell->SetText(GetLuaCellText(L, -1, spec));
            row->AddCell(cell);
        }
        dataGrid->AddRow(row);

        lua_pop(L, 1);
    }

    if (isObservableCollection) {
        utils::Logger::Info("[MVVM] DataGrid collection supports incremental updates (initial sync only for now)");
    } else {
        utils::Logger::Warning("[MVVM] DataGrid collection does not support incremental updates");
    }

    lua_pop(L, 2);

    if (expression.mode != BindingMode::OneTime) {
        dataContext->SubscribePropertyChanged(
            [expression, isObservableCollection](const PropertyChangedEventArgs& args) {
            if (args.propertyName == expression.path) {
                if (isObservableCollection) {
                    return;
                }
                utils::Logger::Warning("[MVVM] DataGrid non-observable collection changed, requires manual refresh");
            }
        });
    }
}

void MvvmXmlLoader::BindListBoxSelectedItem(
    std::shared_ptr<luaui::controls::ListBox> listBox,
    const BindingExpression& expression) {
    
    auto dataContext = m_dataContext;
    auto propertyName = expression.path;
    
    utils::Logger::InfoF("[MVVM] Binding ListBox.SelectedItem to %s", propertyName.c_str());
    
    if (!dataContext) {
        utils::Logger::Error("[MVVM] No DataContext available for ListBox SelectedItem binding");
        return;
    }
    
    auto luaDataContext = std::dynamic_pointer_cast<lua::LuaPropertyNotifier>(dataContext);
    if (!luaDataContext) {
        utils::Logger::Warning("[MVVM] SelectedItem binding only supported with Lua ViewModel");
        return;
    }
    
    // ViewModel -> View: 当 SelectedItem 变更时，更新 ListBox 选择
    auto updateView = [listBox, dataContext, propertyName]() {
        std::any value = dataContext->GetPropertyValue(propertyName);
        if (!value.has_value()) return;
        
        // 获取索引值（假设 ViewModel 存储的是索引）
        try {
            if (value.type() == typeid(int)) {
                int index = std::any_cast<int>(value);
                listBox->SetSelectedIndex(index);
            } else if (value.type() == typeid(double)) {
                int index = static_cast<int>(std::any_cast<double>(value));
                listBox->SetSelectedIndex(index);
            }
        } catch (...) {
            // 忽略转换错误
        }
    };
    
    // 应用初始值
    updateView();
    
    // 订阅属性变更
    if (expression.mode == BindingMode::OneWay || expression.mode == BindingMode::TwoWay) {
        dataContext->SubscribePropertyChanged(
            [propertyName, updateView](const PropertyChangedEventArgs& args) {
            if (args.propertyName == propertyName) {
                updateView();
            }
        });
    }
    
    // TwoWay: View -> ViewModel
    if (expression.mode == BindingMode::TwoWay) {
        listBox->SelectionChanged.Add([dataContext, propertyName](luaui::controls::ListBox*, int selectedIndex) {
            utils::Logger::DebugF("[MVVM] ListBox.SelectionChanged: %s -> %d", propertyName.c_str(), selectedIndex);
            dataContext->SetPropertyValue(propertyName, selectedIndex);
        });
    }
}

// ============================================================================
// ComboBox 绑定 - ItemsSource 和 SelectedItem
// ============================================================================
void MvvmXmlLoader::BindComboBox(std::shared_ptr<luaui::controls::ComboBox> comboBox,
                                 const std::string& propertyName,
                                 const BindingExpression& expression) {
    auto dataContext = m_dataContext;
    if (!dataContext) {
        utils::Logger::Error("[MVVM] No DataContext available for ComboBox binding");
        return;
    }
    
    auto luaDataContext = std::dynamic_pointer_cast<lua::LuaPropertyNotifier>(dataContext);
    
    if (propertyName == "ItemsSource") {
        utils::Logger::InfoF("[MVVM] Binding ComboBox.ItemsSource to %s", expression.path.c_str());
        
        if (!luaDataContext) {
            // 非 Lua 上下文，尝试通用方式
            std::any value = dataContext->GetPropertyValue(expression.path);
            if (value.type() == typeid(std::vector<std::wstring>)) {
                auto items = std::any_cast<std::vector<std::wstring>>(value);
                for (const auto& item : items) {
                    comboBox->AddItem(item);
                }
            }
            return;
        }
        
        // Lua 上下文
        lua_State* L = luaDataContext->GetLuaState();
        if (!L) return;
        
        lua_getglobal(L, "ViewModelInstance");
        if (!lua_istable(L, -1)) {
            lua_pop(L, 1);
            lua_getglobal(L, "TestRunnerViewModel");
        }
        
        if (lua_istable(L, -1)) {
            lua_getfield(L, -1, expression.path.c_str());
            if (lua_istable(L, -1)) {
                auto collection = std::make_shared<lua::LuaObservableCollection>(L, -1);
                auto texts = collection->GetAllDisplayTexts();
                for (const auto& text : texts) {
                    comboBox->AddItem(text);
                }
            }
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
        
        // 订阅变更
        if (expression.mode == BindingMode::OneWay || expression.mode == BindingMode::TwoWay) {
            dataContext->SubscribePropertyChanged(
                [comboBox, dataContext, expression, luaDataContext](const PropertyChangedEventArgs& args) {
                if (args.propertyName == expression.path) {
                    comboBox->ClearItems();
                    
                    lua_State* L = luaDataContext->GetLuaState();
                    if (!L) return;
                    
                    lua_getglobal(L, "ViewModelInstance");
                    if (!lua_istable(L, -1)) {
                        lua_pop(L, 1);
                        lua_getglobal(L, "TestRunnerViewModel");
                    }
                    
                    if (lua_istable(L, -1)) {
                        lua_getfield(L, -1, expression.path.c_str());
                        if (lua_istable(L, -1)) {
                            auto newCollection = std::make_shared<lua::LuaObservableCollection>(L, -1);
                            auto texts = newCollection->GetAllDisplayTexts();
                            for (const auto& text : texts) {
                                comboBox->AddItem(text);
                            }
                        }
                        lua_pop(L, 1);
                    }
                    lua_pop(L, 1);
                }
            });
        }
    }
    else if (propertyName == "SelectedItem" || propertyName == "SelectedIndex") {
        utils::Logger::InfoF("[MVVM] Binding ComboBox.SelectedItem to %s", expression.path.c_str());
        
        // ViewModel -> View
        auto updateView = [comboBox, dataContext, expression]() {
            std::any value = dataContext->GetPropertyValue(expression.path);
            if (!value.has_value()) return;
            
            try {
                int index = -1;
                if (value.type() == typeid(int)) {
                    index = std::any_cast<int>(value);
                } else if (value.type() == typeid(double)) {
                    index = static_cast<int>(std::any_cast<double>(value));
                }
                if (index >= 0) {
                    comboBox->SetSelectedIndex(index);
                }
            } catch (...) {}
        };
        
        updateView();
        
        // 订阅变更
        if (expression.mode == BindingMode::OneWay || expression.mode == BindingMode::TwoWay) {
            dataContext->SubscribePropertyChanged(
                [expression, updateView](const PropertyChangedEventArgs& args) {
                if (args.propertyName == expression.path) {
                    updateView();
                }
            });
        }
        
        // TwoWay: View -> ViewModel
        if (expression.mode == BindingMode::TwoWay && luaDataContext) {
            comboBox->SelectionChanged.Add([dataContext, expression](luaui::controls::ComboBox*, int selectedIndex) {
                utils::Logger::DebugF("[MVVM] ComboBox.SelectionChanged: %s -> %d", expression.path.c_str(), selectedIndex);
                dataContext->SetPropertyValue(expression.path, selectedIndex);
            });
        }
    }
}

// ============================================================================
// CheckBox 绑定 - IsChecked（支持 TwoWay）
// ============================================================================
void MvvmXmlLoader::BindCheckBox(std::shared_ptr<luaui::controls::CheckBox> checkBox,
                                 const std::string& /*propertyName*/,
                                 const BindingExpression& expression) {
    auto converter = expression.converter;
    auto dataContext = m_dataContext;
    auto boundPropertyName = expression.path;
    auto converterParameter = expression.converterParameter;
    
    utils::Logger::InfoF("[MVVM] Binding CheckBox.IsChecked to %s, mode=%d", 
        boundPropertyName.c_str(), static_cast<int>(expression.mode));
    
    // 更新函数：ViewModel -> View
    auto updateView = [checkBox, dataContext, boundPropertyName, converter, converterParameter]() {
        std::any value = dataContext->GetPropertyValue(boundPropertyName);
        if (!value.has_value()) return;
        
        bool boolValue = false;
        if (value.type() == typeid(bool)) {
            boolValue = std::any_cast<bool>(value);
        } else if (converter) {
            // 使用转换器
            auto result = converter->ConvertBack(value, converterParameter);
            if (result.has_value() && result.type() == typeid(bool)) {
                boolValue = std::any_cast<bool>(result);
            }
        }
        
        checkBox->SetIsChecked(boolValue);
    };
    
    // 应用初始值
    updateView();
    
    // 订阅属性变化通知
    if (expression.mode == BindingMode::OneWay || expression.mode == BindingMode::TwoWay) {
        dataContext->SubscribePropertyChanged(
            [dataContext, boundPropertyName, updateView](const PropertyChangedEventArgs& args) {
            if (args.propertyName == boundPropertyName || args.propertyName.empty()) {
                updateView();
            }
        });
    }
    
    // TwoWay：监听控件状态变化并更新 ViewModel
    if (expression.mode == BindingMode::TwoWay) {
        utils::Logger::Debug("[MVVM] Setting up TwoWay binding for CheckBox");
        checkBox->CheckedChanged.Add([dataContext, boundPropertyName, converter, converterParameter](luaui::controls::CheckBox*, bool isChecked) {
            utils::Logger::DebugF("[MVVM] CheckBox.CheckedChanged: %s -> %s", boundPropertyName.c_str(), isChecked ? "true" : "false");
            std::any value = isChecked;
            if (converter) {
                value = converter->Convert(value, converterParameter);
            }
            dataContext->SetPropertyValue(boundPropertyName, value);
        });
    }
}

// ============================================================================
// RadioButton 绑定 - IsChecked（支持 TwoWay）
// ============================================================================
void MvvmXmlLoader::BindRadioButton(std::shared_ptr<luaui::controls::RadioButton> radioButton,
                                    const std::string& /*propertyName*/,
                                    const BindingExpression& expression) {
    auto converter = expression.converter;
    auto dataContext = m_dataContext;
    auto boundPropertyName = expression.path;
    auto converterParameter = expression.converterParameter;
    
    utils::Logger::InfoF("[MVVM] Binding RadioButton.IsChecked to %s, mode=%d", 
        boundPropertyName.c_str(), static_cast<int>(expression.mode));
    
    // 更新函数：ViewModel -> View
    auto updateView = [radioButton, dataContext, boundPropertyName, converter, converterParameter]() {
        std::any value = dataContext->GetPropertyValue(boundPropertyName);
        if (!value.has_value()) return;
        
        bool boolValue = false;
        if (value.type() == typeid(bool)) {
            boolValue = std::any_cast<bool>(value);
        } else if (converter) {
            auto result = converter->ConvertBack(value, converterParameter);
            if (result.has_value() && result.type() == typeid(bool)) {
                boolValue = std::any_cast<bool>(result);
            }
        }
        
        radioButton->SetIsChecked(boolValue);
    };
    
    // 应用初始值
    updateView();
    
    // 订阅属性变化通知
    if (expression.mode == BindingMode::OneWay || expression.mode == BindingMode::TwoWay) {
        dataContext->SubscribePropertyChanged(
            [dataContext, boundPropertyName, updateView](const PropertyChangedEventArgs& args) {
            if (args.propertyName == boundPropertyName || args.propertyName.empty()) {
                updateView();
            }
        });
    }
    
    // TwoWay：监听控件状态变化并更新 ViewModel
    if (expression.mode == BindingMode::TwoWay) {
        radioButton->CheckedChanged.Add([dataContext, boundPropertyName, converter, converterParameter](luaui::controls::RadioButton*, bool isChecked) {
            std::any value = isChecked;
            if (converter) {
                value = converter->Convert(value, converterParameter);
            }
            dataContext->SetPropertyValue(boundPropertyName, value);
        });
    }
}

// ============================================================================
// Button 绑定 - Command（点击命令）
// ============================================================================
void MvvmXmlLoader::BindButtonCommand(std::shared_ptr<luaui::controls::Button> button,
                                      const BindingExpression& expression) {
    auto dataContext = m_dataContext;
    auto commandName = expression.path;
    
    utils::Logger::InfoF("[MVVM] Binding Button.Command to %s", commandName.c_str());
    
    if (!dataContext) {
        utils::Logger::Error("[MVVM] No DataContext available for Button Command binding");
        return;
    }
    
    // 尝试转换为 LuaPropertyNotifier
    auto luaNotifier = std::dynamic_pointer_cast<lua::LuaPropertyNotifier>(dataContext);
    if (luaNotifier) {
        // Lua ViewModel：检查函数是否存在
        if (!luaNotifier->HasFunction(commandName)) {
            utils::Logger::Warning("[MVVM] Command '" + commandName + "' not found in Lua ViewModel");
            return;
        }
        
        // 绑定点击事件到 Lua 函数
        button->Click.Add([luaNotifier, commandName](luaui::Control*) {
            utils::Logger::InfoF("[MVVM] Executing Lua command: %s", commandName.c_str());
            bool result = luaNotifier->CallFunction(commandName);
            if (!result) {
                utils::Logger::Warning("[MVVM] Lua command '" + commandName + "' execution failed");
            }
        });
    } else {
        // C++ ViewModel：暂不支持，记录警告
        utils::Logger::Warning("[MVVM] Button Command binding only supported with Lua ViewModel");
        return;
    }
    
    utils::Logger::InfoF("[MVVM] Button command '%s' bound successfully", commandName.c_str());
}

// ============================================================================
// Button Text 绑定 - OneWay（VM -> View）
// ============================================================================
void MvvmXmlLoader::BindButtonText(std::shared_ptr<luaui::controls::Button> button,
                                   const BindingExpression& expression) {
    auto dataContext = m_dataContext;
    auto boundPropertyName = expression.path;
    auto converter = expression.converter;
    auto converterParameter = expression.converterParameter;

    auto updateView = [button, dataContext, boundPropertyName, converter, converterParameter]() {
        std::any value = dataContext->GetPropertyValue(boundPropertyName);
        if (!value.has_value()) return;

        if (converter) {
            value = converter->Convert(value, converterParameter);
        }

        try {
            if (value.type() == typeid(std::string)) {
                std::string str = std::any_cast<std::string>(value);
                button->SetText(Utf8ToW(str));
            } else if (value.type() == typeid(std::wstring)) {
                button->SetText(std::any_cast<std::wstring>(value));
            } else if (value.type() == typeid(double)) {
                double d = std::any_cast<double>(value);
                std::wstring ws = std::to_wstring((int)d);
                button->SetText(ws);
            } else if (value.type() == typeid(int)) {
                int i = std::any_cast<int>(value);
                button->SetText(std::to_wstring(i));
            }
        } catch (const std::bad_any_cast&) {}
    };

    updateView();

    dataContext->SubscribePropertyChanged(
        [updateView, boundPropertyName](const PropertyChangedEventArgs& args) {
        if (args.propertyName == boundPropertyName || args.propertyName.empty()) {
            updateView();
        }
    });
}

// ============================================================================
// 创建 MVVM XML 加载器
// ============================================================================
std::shared_ptr<MvvmXmlLoader> CreateMvvmXmlLoader() {
    return std::make_shared<MvvmXmlLoader>();
}

} // namespace mvvm
} // namespace luaui
