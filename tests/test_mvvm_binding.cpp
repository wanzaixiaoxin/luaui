// MVVM Data Binding Tests
#include "TestFramework.h"
#include "mvvm/BindingEngine.h"
#include "mvvm/ViewModelBase.h"
#include "mvvm/Converters.h"
#include <memory>
#include <string>

using namespace luaui;
using namespace luaui::mvvm;

// Test ViewModel
class TestViewModel : public ViewModelBase {
public:
    std::string GetName() const { return m_name; }
    void SetName(const std::string& name) {
        if (m_name != name) {
            m_name = name;
            NotifyPropertyChanged("Name");
        }
    }
    
    int GetAge() const { return m_age; }
    void SetAge(int age) {
        if (m_age != age) {
            m_age = age;
            NotifyPropertyChanged("Age");
        }
    }
    
    bool GetIsActive() const { return m_isActive; }
    void SetIsActive(bool active) {
        if (m_isActive != active) {
            m_isActive = active;
            NotifyPropertyChanged("IsActive");
            NotifyPropertyChanged("StatusText");
        }
    }
    
    std::string GetStatusText() const {
        return m_isActive ? "Active" : "Inactive";
    }
    
    std::any GetPropertyValue(const std::string& name) const override {
        if (name == "Name") return m_name;
        if (name == "Age") return m_age;
        if (name == "IsActive") return m_isActive;
        if (name == "StatusText") return GetStatusText();
        return std::any();
    }
    
    void SetPropertyValue(const std::string& name, const std::any& value) override {
        if (name == "Name") {
            if (value.type() == typeid(std::string)) {
                SetName(std::any_cast<std::string>(value));
            }
        } else if (name == "Age") {
            if (value.type() == typeid(int)) {
                SetAge(std::any_cast<int>(value));
            }
        } else if (name == "IsActive") {
            if (value.type() == typeid(bool)) {
                SetIsActive(std::any_cast<bool>(value));
            }
        }
    }

private:
    std::string m_name = "Default";
    int m_age = 0;
    bool m_isActive = false;
};

// ==================== Binding Expression Tests ====================

TEST(BindingExpression_ParseSimple) {
    auto& engine = BindingEngine::Instance();
    auto expr = engine.ParseExpression("{Binding Name}");
    ASSERT_EQ(expr.path, "Name");
    ASSERT_EQ(static_cast<int>(expr.mode), static_cast<int>(BindingMode::OneWay));
}

TEST(BindingExpression_ParseTwoWay) {
    auto& engine = BindingEngine::Instance();
    auto expr = engine.ParseExpression("{Binding Name, Mode=TwoWay}");
    ASSERT_EQ(expr.path, "Name");
    ASSERT_EQ(static_cast<int>(expr.mode), static_cast<int>(BindingMode::TwoWay));
}

TEST(BindingExpression_ParseOneWay) {
    auto& engine = BindingEngine::Instance();
    auto expr = engine.ParseExpression("{Binding Age, Mode=OneWay}");
    ASSERT_EQ(expr.path, "Age");
    ASSERT_EQ(static_cast<int>(expr.mode), static_cast<int>(BindingMode::OneWay));
}

TEST(BindingExpression_ParseOneTime) {
    auto& engine = BindingEngine::Instance();
    auto expr = engine.ParseExpression("{Binding Age, Mode=OneTime}");
    ASSERT_EQ(expr.path, "Age");
    ASSERT_EQ(static_cast<int>(expr.mode), static_cast<int>(BindingMode::OneTime));
}

TEST(BindingExpression_ParseWithConverter) {
    auto& engine = BindingEngine::Instance();
    auto converter = std::make_shared<ToStringConverter>();
    engine.RegisterConverter("IntToString", converter);
    
    auto expr = engine.ParseExpression("{Binding Age, Converter=IntToString}");
    ASSERT_EQ(expr.path, "Age");
    ASSERT_TRUE(expr.converter != nullptr);
}

TEST(BindingExpression_ParseWithConverterAndParameter) {
    auto& engine = BindingEngine::Instance();
    auto converter = std::make_shared<FormatConverter>();
    engine.RegisterConverter("Format", converter);
    
    auto expr = engine.ParseExpression("{Binding Age, Converter=Format, ConverterParameter='Age: {0}'}");
    ASSERT_EQ(expr.path, "Age");
    ASSERT_TRUE(expr.converter != nullptr);
    // converterParameter 包含原始字符串（可能包含引号）
    ASSERT_TRUE(expr.converterParameter.find("Age: {0}") != std::string::npos);
}

// ==================== ViewModel Tests ====================

TEST(ViewModel_PropertyChanged) {
    TestViewModel vm;
    bool nameChanged = false;
    bool ageChanged = false;
    
    vm.SubscribePropertyChanged([&](const PropertyChangedEventArgs& args) {
        if (args.propertyName == "Name") nameChanged = true;
        if (args.propertyName == "Age") ageChanged = true;
    });
    
    vm.SetName("Test");
    vm.SetAge(25);
    
    ASSERT_TRUE(nameChanged);
    ASSERT_TRUE(ageChanged);
}

TEST(ViewModel_GetSetPropertyValue) {
    TestViewModel vm;
    
    vm.SetPropertyValue("Name", std::string("John"));
    auto name = vm.GetPropertyValue("Name");
    ASSERT_TRUE(name.has_value());
    ASSERT_EQ(std::any_cast<std::string>(name), "John");
    
    vm.SetPropertyValue("Age", 30);
    auto age = vm.GetPropertyValue("Age");
    ASSERT_TRUE(age.has_value());
    ASSERT_EQ(std::any_cast<int>(age), 30);
    
    vm.SetPropertyValue("IsActive", true);
    auto active = vm.GetPropertyValue("IsActive");
    ASSERT_TRUE(active.has_value());
    ASSERT_EQ(std::any_cast<bool>(active), true);
}

TEST(ViewModel_ComputedProperty) {
    TestViewModel vm;
    
    ASSERT_EQ(vm.GetStatusText(), "Inactive");
    
    vm.SetIsActive(true);
    ASSERT_EQ(vm.GetStatusText(), "Active");
    
    auto status = vm.GetPropertyValue("StatusText");
    ASSERT_TRUE(status.has_value());
    ASSERT_EQ(std::any_cast<std::string>(status), "Active");
}

// ==================== Value Converter Tests ====================

TEST(BooleanToVisibilityConverter_Convert) {
    BooleanToVisibilityConverter converter;
    
    auto visible = converter.Convert(true, "");
    ASSERT_TRUE(visible.has_value());
    ASSERT_EQ(std::any_cast<bool>(visible), true);
    
    auto collapsed = converter.Convert(false, "");
    ASSERT_TRUE(collapsed.has_value());
    ASSERT_EQ(std::any_cast<bool>(collapsed), false);
}

TEST(BooleanInverterConverter_Convert) {
    BooleanInverterConverter converter;
    
    auto result1 = converter.Convert(true, "");
    ASSERT_TRUE(result1.has_value());
    ASSERT_EQ(std::any_cast<bool>(result1), false);
    
    auto result2 = converter.Convert(false, "");
    ASSERT_TRUE(result2.has_value());
    ASSERT_EQ(std::any_cast<bool>(result2), true);
}

TEST(ToStringConverter_Int) {
    ToStringConverter converter;
    
    auto result = converter.Convert(42, "");
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(std::any_cast<std::string>(result), "42");
}

TEST(ToStringConverter_Bool) {
    ToStringConverter converter;
    
    auto result1 = converter.Convert(true, "");
    ASSERT_TRUE(result1.has_value());
    ASSERT_EQ(std::any_cast<std::string>(result1), "True");
    
    auto result2 = converter.Convert(false, "");
    ASSERT_TRUE(result2.has_value());
    ASSERT_EQ(std::any_cast<std::string>(result2), "False");
}

TEST(ToStringConverter_FloatPrecision) {
    ToStringConverter converter;
    
    auto result = converter.Convert(3.14159f, "2");
    ASSERT_TRUE(result.has_value());
    std::string str = std::any_cast<std::string>(result);
    ASSERT_TRUE(str.find("3.14") != std::string::npos);
}

TEST(FormatConverter_Convert) {
    FormatConverter converter;
    
    auto result = converter.Convert(75, "Progress: {0}%");
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(std::any_cast<std::string>(result), "Progress: 75%");
}

TEST(NumberRangeConverter_Convert) {
    NumberRangeConverter converter(0, 100, 0, 1); // 0-100 -> 0-1
    
    auto result1 = converter.Convert(50, "");
    ASSERT_TRUE(result1.has_value());
    ASSERT_NEAR(std::any_cast<double>(result1), 0.5, 0.001);
    
    auto result2 = converter.Convert(0, "");
    ASSERT_TRUE(result2.has_value());
    ASSERT_NEAR(std::any_cast<double>(result2), 0.0, 0.001);
    
    auto result3 = converter.Convert(100, "");
    ASSERT_TRUE(result3.has_value());
    ASSERT_NEAR(std::any_cast<double>(result3), 1.0, 0.001);
}

TEST(NumberRangeConverter_ConvertBack) {
    NumberRangeConverter converter(0, 100, 0, 1); // 0-100 -> 0-1
    
    auto result = converter.ConvertBack(0.75, "");
    ASSERT_TRUE(result.has_value());
    ASSERT_NEAR(std::any_cast<double>(result), 75.0, 0.001);
}

// ==================== Binding Engine Tests ====================

TEST(BindingEngine_Instance) {
    auto& engine1 = BindingEngine::Instance();
    auto& engine2 = BindingEngine::Instance();
    
    ASSERT_EQ(&engine1, &engine2);
}

TEST(BindingEngine_RegisterConverter) {
    auto& engine = BindingEngine::Instance();
    
    auto converter = std::make_shared<ToStringConverter>();
    engine.RegisterConverter("TestConverter", converter);
    
    auto retrieved = engine.GetConverter("TestConverter");
    ASSERT_TRUE(retrieved.get() != nullptr);
}

TEST(BindingEngine_RegisterDefaultConverters) {
    auto& engine = BindingEngine::Instance();
    
    RegisterDefaultConverters(engine);
    
    ASSERT_TRUE(engine.GetConverter("BooleanToVisibility").get() != nullptr);
    ASSERT_TRUE(engine.GetConverter("BooleanInverter").get() != nullptr);
    ASSERT_TRUE(engine.GetConverter("ToString").get() != nullptr);
    ASSERT_TRUE(engine.GetConverter("Format").get() != nullptr);
}

// ==================== Main ====================
int main() {
    return RUN_ALL_TESTS();
}
