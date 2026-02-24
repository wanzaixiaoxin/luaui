// 简单的 MVVM 绑定测试

#include <iostream>
#include <luaui/mvvm/MVVM.h>
#include <luaui/mvvm/ViewModelBase.h>
#include <any>

using namespace luaui::mvvm;

// 简单的测试 ViewModel
class TestViewModel : public ViewModelBase {
public:
    TestViewModel() : m_name("Test"), m_value(42) {
        InitializeProperties();
    }
    
    std::string GetName() const { return m_name; }
    void SetName(const std::string& value) {
        if (SetProperty(m_name, value, "Name")) {
            std::cout << "Name changed to: " << value << std::endl;
        }
    }
    
    int GetValue() const { return m_value; }
    void SetValue(int value) {
        SetProperty(m_value, value, "Value");
    }
    
private:
    void InitializeProperties() {
        RegisterPropertyGetter<std::string>("Name", [this]() { return GetName(); });
        RegisterPropertyGetter<int>("Value", [this]() { return GetValue(); });
        RegisterPropertySetter<std::string>("Name", [this](const std::string& v) { SetName(v); });
        RegisterPropertySetter<int>("Value", [this](int v) { SetValue(v); });
    }
    
    std::string m_name;
    int m_value;
};

int main() {
    std::cout << "=== MVVM Binding Test ===" << std::endl;
    
    auto vm = std::make_shared<TestViewModel>();
    
    // 测试 GetPropertyValue
    std::any nameValue = vm->GetPropertyValue("Name");
    if (nameValue.has_value() && nameValue.type() == typeid(std::string)) {
        std::cout << "✓ GetPropertyValue('Name') = " << std::any_cast<std::string>(nameValue) << std::endl;
    } else {
        std::cout << "✗ GetPropertyValue('Name') failed" << std::endl;
        return 1;
    }
    
    std::any valueValue = vm->GetPropertyValue("Value");
    if (valueValue.has_value() && valueValue.type() == typeid(int)) {
        std::cout << "✓ GetPropertyValue('Value') = " << std::any_cast<int>(valueValue) << std::endl;
    } else {
        std::cout << "✗ GetPropertyValue('Value') failed" << std::endl;
        return 1;
    }
    
    // 测试 SetPropertyValue
    vm->SetPropertyValue("Name", std::string("Updated"));
    nameValue = vm->GetPropertyValue("Name");
    if (std::any_cast<std::string>(nameValue) == "Updated") {
        std::cout << "✓ SetPropertyValue('Name', 'Updated') works" << std::endl;
    } else {
        std::cout << "✗ SetPropertyValue failed" << std::endl;
        return 1;
    }
    
    // 测试属性变更通知
    bool notified = false;
    vm->SubscribePropertyChanged([&notified](const PropertyChangedEventArgs& args) {
        std::cout << "✓ Property changed: " << args.propertyName << std::endl;
        notified = true;
    });
    
    vm->SetValue(100);
    if (!notified) {
        std::cout << "✗ Property change notification failed" << std::endl;
        return 1;
    }
    
    std::cout << "\n=== All tests passed! ===" << std::endl;
    return 0;
}
