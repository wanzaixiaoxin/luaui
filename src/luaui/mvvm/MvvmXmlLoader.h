#pragma once

#include "BindingEngine.h"
#include "XmlLayout.h"
#include "Control.h"
#include <functional>
#include <vector>

// Forward declare tinyxml2
namespace tinyxml2 {
    class XMLElement;
}

namespace luaui {
namespace mvvm {

// ============================================================================
// 待处理绑定信息
// ============================================================================
struct PendingBinding {
    std::weak_ptr<luaui::Control> control;
    std::string propertyName;
    BindingExpression expression;
};

// 从 XML 提取的原始绑定信息
struct PendingBindingInfo {
    std::string elementName;        // 控件名称（如果有）
    std::string propertyName;       // 属性名
    std::string expressionString;   // 原始绑定表达式字符串
};

// ============================================================================
// MVVM XML Loader - 支持数据绑定的XML加载器
// ============================================================================
class MvvmXmlLoader : public xml::IXmlLoader {
public:
    MvvmXmlLoader();
    
    // IXmlLoader 实现
    std::shared_ptr<luaui::Control> Load(const std::string& filePath) override;
    std::shared_ptr<luaui::Control> LoadFromString(const std::string& xmlString) override;
    void RegisterElement(const std::string& tagName, 
                         std::function<std::shared_ptr<luaui::Control>()> factory) override;
    
    void RegisterClickHandler(const std::string& methodName, 
                              std::function<void()> handler) override;
    void RegisterValueChangedHandler(const std::string& methodName, 
                                     std::function<void(double)> handler) override;
    void RegisterTextChangedHandler(const std::string& methodName, 
                                    std::function<void(const std::wstring&)> handler) override;
    
    // ========== MVVM 扩展 ==========
    
    // 设置 DataContext（绑定源）
    void SetDataContext(std::shared_ptr<INotifyPropertyChanged> context);
    std::shared_ptr<INotifyPropertyChanged> GetDataContext() const { return m_dataContext; }
    
    // 连接所有待处理的绑定到 DataContext
    void ConnectBindings();
    
    // 注册值转换器
    void RegisterConverter(const std::string& name, std::shared_ptr<IValueConverter> converter);

protected:
    void BindEventsFromInstance(void* instance) override;

private:
    std::shared_ptr<xml::IXmlLoader> m_baseLoader;
    std::shared_ptr<INotifyPropertyChanged> m_dataContext;
    std::vector<PendingBinding> m_pendingBindings;
    std::vector<PendingBindingInfo> m_pendingBindingInfos;
    std::weak_ptr<luaui::Control> m_rootControl;  // 根控件，用于查找命名控件
    
    void ApplyBindings();
    bool IsBindingExpression(const std::string& value);
    BindingExpression ParseBinding(const std::string& expression);
    
    // 从 XML 提取绑定表达式
    void ExtractBindings(const tinyxml2::XMLElement* element, const std::string& parentName = "");
    
    // 遍历控件树应用绑定
    void ApplyBindingsToControl(const std::shared_ptr<luaui::Control>& control);
    
    // 根据名称查找控件
    std::shared_ptr<luaui::Control> FindControlByName(
        const std::shared_ptr<luaui::Control>& root, 
        const std::string& name);
    
    // 创建实际绑定
    void CreateBinding(const std::shared_ptr<luaui::Control>& control,
                       const std::string& propertyName,
                       const BindingExpression& expression);
    
    // 控件特定的绑定创建
    void BindTextBlock(std::shared_ptr<luaui::controls::TextBlock> textBlock,
                       const std::string& propertyName,
                       const BindingExpression& expression);
    void BindTextBox(std::shared_ptr<luaui::controls::TextBox> textBox,
                     const std::string& propertyName,
                     const BindingExpression& expression);
    void BindProgressBar(std::shared_ptr<luaui::controls::ProgressBar> progressBar,
                         const std::string& propertyName,
                         const BindingExpression& expression);
    void BindSlider(std::shared_ptr<luaui::controls::Slider> slider,
                    const std::string& propertyName,
                    const BindingExpression& expression);
    void BindListBox(std::shared_ptr<luaui::controls::ListBox> listBox,
                     const std::string& propertyName,
                     const BindingExpression& expression);
};

// 创建 MVVM XML 加载器
std::shared_ptr<MvvmXmlLoader> CreateMvvmXmlLoader();

} // namespace mvvm
} // namespace luaui
