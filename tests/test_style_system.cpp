// test_style_system.cpp - Unit tests for Style System

#include "TestFramework.h"
#include "ResourceDictionary.h"
#include "Setter.h"
#include "Style.h"
#include "Trigger.h"
#include "Theme.h"
#include <memory>
#include <typeindex>

using namespace luaui::controls;

// ==================== ResourceDictionary Tests ====================

TEST(ResourceDictionary_BasicOperations) {
    ResourceDictionary dict;
    
    // Test Add and Get
    dict.Add("Key1", 100);
    dict.Add("Key2", std::string("Hello"));
    dict.Add("Key3", 3.14);
    
    ASSERT_EQ(dict.Get<int>("Key1"), 100);
    ASSERT_EQ(dict.Get<std::string>("Key2"), "Hello");
    ASSERT_NEAR(dict.Get<double>("Key3"), 3.14, 0.001);
}

TEST(ResourceDictionary_Contains) {
    ResourceDictionary dict;
    dict.Add("Existing", 42);
    
    ASSERT_TRUE(dict.Contains("Existing"));
    ASSERT_FALSE(dict.Contains("NonExisting"));
}

TEST(ResourceDictionary_Remove) {
    ResourceDictionary dict;
    dict.Add("Key", 100);
    ASSERT_TRUE(dict.Contains("Key"));
    
    dict.Remove("Key");
    ASSERT_FALSE(dict.Contains("Key"));
}

TEST(ResourceDictionary_ParentLookup) {
    ResourceDictionary parent;
    parent.Add("ParentKey", 100);
    
    ResourceDictionary child;
    child.SetParent(&parent);
    child.Add("ChildKey", 200);
    
    // Child should find its own key
    ASSERT_EQ(child.Get<int>("ChildKey"), 200);
    
    // Child should find parent's key
    ASSERT_EQ(child.Get<int>("ParentKey"), 100);
}

TEST(ResourceDictionary_GetOrDefault) {
    ResourceDictionary dict;
    dict.Add("Key", 42);
    
    ASSERT_EQ(dict.GetOrDefault<int>("Key", 0), 42);
    ASSERT_EQ(dict.GetOrDefault<int>("Missing", 99), 99);
}

TEST(ResourceDictionary_Merge) {
    ResourceDictionary dict1;
    dict1.Add("Key1", 100);
    dict1.Add("Key2", 200);
    
    ResourceDictionary dict2;
    dict2.Add("Key2", 999);  // This should not override
    dict2.Add("Key3", 300);
    
    dict1.Merge(dict2);
    
    ASSERT_EQ(dict1.Get<int>("Key1"), 100);  // Original preserved
    ASSERT_EQ(dict1.Get<int>("Key2"), 200);  // Not overridden
    ASSERT_EQ(dict1.Get<int>("Key3"), 300);  // New key added
}

// ==================== ResourceReference Tests ====================

TEST(ResourceReference_StaticLookup) {
    ResourceDictionary dict;
    dict.Add("IntKey", 42);
    
    ResourceReference ref("IntKey", ResourceLookupMode::Static);
    auto result = ref.Resolve(&dict, nullptr);
    
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(std::any_cast<int>(result), 42);
}

TEST(ResourceReference_Resolve_NotFound) {
    ResourceDictionary dict;
    ResourceReference ref("MissingKey");
    
    auto result = ref.Resolve(&dict, nullptr);
    ASSERT_FALSE(result.has_value());
}

// ==================== Setter Tests ====================

TEST(Setter_DirectValue) {
    // Create a test property
    auto propId = DependencyProperty::Register("TestProp", 
        PropertyMetadata{std::any(0), nullptr, false, false, true});
    
    Setter setter(propId, 42);
    
    ASSERT_EQ(setter.GetPropertyId(), propId);
    ASSERT_TRUE(setter.HasValue());
    
    // Cleanup
    DependencyProperty::s_properties.erase(propId);
    DependencyProperty::s_nameMap.erase("TestProp");
}

TEST(Setter_ResourceReference) {
    auto propId = DependencyProperty::Register("TestProp2", 
        PropertyMetadata{std::any(0), nullptr, false, false, true});
    
    ResourceDictionary dict;
    dict.Add("ResourceKey", 100);
    
    Setter setter(propId, ResourceReference("ResourceKey"));
    auto value = setter.GetResolvedValue(&dict);
    
    ASSERT_TRUE(value.has_value());
    ASSERT_EQ(std::any_cast<int>(value), 100);
    
    // Cleanup
    DependencyProperty::s_properties.erase(propId);
    DependencyProperty::s_nameMap.erase("TestProp2");
}

// ==================== Style Tests ====================

TEST(Style_TargetType) {
    // Use a dummy type for testing
    Style style(typeid(int));
    
    ASSERT_TRUE(style.GetTargetType() == typeid(int));
    ASSERT_TRUE(style.CanApplyTo(typeid(int)));
    ASSERT_FALSE(style.CanApplyTo(typeid(double)));
}

TEST(Style_BasedOn) {
    auto baseStyle = std::make_shared<Style>(typeid(int));
    
    Style derivedStyle(typeid(int));
    derivedStyle.SetBasedOn(baseStyle);
    
    ASSERT_EQ(derivedStyle.GetBasedOn(), baseStyle);
}

TEST(Style_Setters) {
    Style style(typeid(int));
    
    auto propId = DependencyProperty::Register("StyleTestProp", 
        PropertyMetadata{std::any(0), nullptr, false, false, true});
    
    style.AddSetter(propId, 42);
    style.AddSetter(propId, 100);  // Add another with same property
    
    auto setters = style.GetSetters();
    ASSERT_EQ(setters.size(), 2);
    
    // Cleanup
    DependencyProperty::s_properties.erase(propId);
    DependencyProperty::s_nameMap.erase("StyleTestProp");
}

TEST(Style_Triggers) {
    Style style(typeid(int));
    
    auto propId = DependencyProperty::Register("TriggerTestProp", 
        PropertyMetadata{std::any(false), nullptr, false, false, true});
    
    auto trigger = std::make_shared<PropertyTrigger>(propId, true);
    style.AddTrigger(trigger);
    
    auto triggers = style.GetTriggers();
    ASSERT_EQ(triggers.size(), 1);
    
    // Cleanup
    DependencyProperty::s_properties.erase(propId);
    DependencyProperty::s_nameMap.erase("TriggerTestProp");
}

// ==================== Trigger Tests ====================

TEST(PropertyTrigger_Construction) {
    auto propId = DependencyProperty::Register("PropTriggerTest", 
        PropertyMetadata{std::any(0), nullptr, false, false, true});
    
    PropertyTrigger trigger(propId, 42);
    
    ASSERT_FALSE(trigger.IsActive());
    ASSERT_EQ(trigger.GetName(), "PropertyTrigger");
    
    // Cleanup
    DependencyProperty::s_properties.erase(propId);
    DependencyProperty::s_nameMap.erase("PropTriggerTest");
}

TEST(PropertyTrigger_Setters) {
    auto propId = DependencyProperty::Register("PropTriggerTest2", 
        PropertyMetadata{std::any(0), nullptr, false, false, true});
    
    PropertyTrigger trigger(propId, true);
    trigger.AddSetter(propId, 999);
    
    auto& setters = trigger.GetSetters();
    ASSERT_EQ(setters.size(), 1);
    
    // Cleanup
    DependencyProperty::s_properties.erase(propId);
    DependencyProperty::s_nameMap.erase("PropTriggerTest2");
}

TEST(EventTrigger_Construction) {
    EventTrigger trigger("Click");
    
    ASSERT_EQ(trigger.GetName(), "EventTrigger:Click");
}

TEST(MultiTrigger_Conditions) {
    MultiTrigger trigger;
    
    auto propId1 = DependencyProperty::Register("MultiProp1", 
        PropertyMetadata{std::any(0), nullptr, false, false, true});
    auto propId2 = DependencyProperty::Register("MultiProp2", 
        PropertyMetadata{std::any(false), nullptr, false, false, true});
    
    trigger.AddCondition(propId1, 42);
    trigger.AddCondition(propId2, true);
    
    // Initially not active
    ASSERT_FALSE(trigger.IsActive());
    
    // Cleanup
    DependencyProperty::s_properties.erase(propId1);
    DependencyProperty::s_properties.erase(propId2);
    DependencyProperty::s_nameMap.erase("MultiProp1");
    DependencyProperty::s_nameMap.erase("MultiProp2");
}

// ==================== Theme Tests ====================

TEST(Theme_BasicInfo) {
    Theme theme;
    theme.SetName("DarkTheme");
    theme.SetBaseTheme("LightTheme");
    theme.SetVersion("1.0.0");
    theme.SetAuthor("Test Author");
    theme.SetDescription("Test theme description");
    
    ASSERT_EQ(theme.GetName(), "DarkTheme");
    ASSERT_EQ(theme.GetBaseTheme(), "LightTheme");
    ASSERT_EQ(theme.GetVersion(), "1.0.0");
    ASSERT_EQ(theme.GetAuthor(), "Test Author");
    ASSERT_EQ(theme.GetDescription(), "Test theme description");
}

TEST(Theme_Resources) {
    Theme theme;
    theme.GetResources().Add("TestInt", 42);
    
    ASSERT_TRUE(theme.GetResources().Contains("TestInt"));
    ASSERT_EQ(theme.GetResources().Get<int>("TestInt"), 42);
}

TEST(Theme_Styles) {
    Theme theme;
    auto style = std::make_shared<Style>(typeid(int));
    
    theme.AddStyle("TestStyle", style);
    ASSERT_EQ(theme.GetStyle("TestStyle"), style);
    
    theme.RemoveStyle("TestStyle");
    ASSERT_EQ(theme.GetStyle("TestStyle"), nullptr);
}

TEST(Theme_ImplicitStyles) {
    Theme theme;
    auto style = std::make_shared<Style>(typeid(int));
    
    theme.SetImplicitStyle(typeid(int), style);
    ASSERT_EQ(theme.GetImplicitStyle(typeid(int)), style);
    ASSERT_EQ(theme.GetImplicitStyle(typeid(double)), nullptr);
}

TEST(Theme_Clear) {
    Theme theme;
    theme.SetName("Test");
    theme.GetResources().Add("Key", 42);
    theme.AddStyle("Style", std::make_shared<Style>(typeid(int)));
    
    theme.Clear();
    
    ASSERT_TRUE(theme.GetName().empty());
    ASSERT_EQ(theme.GetResources().GetCount(), 0);
    ASSERT_TRUE(theme.GetStyleKeys().empty());
}

// ==================== ThemeManager Tests ====================

TEST(ThemeManager_Singleton) {
    auto& instance1 = ThemeManager::GetInstance();
    auto& instance2 = ThemeManager::GetInstance();
    
    ASSERT_EQ(&instance1, &instance2);
}

TEST(ThemeManager_CurrentTheme) {
    auto& manager = ThemeManager::GetInstance();
    
    // Initially no theme
    ASSERT_TRUE(manager.GetCurrentThemeName().empty());
    ASSERT_EQ(manager.GetCurrentTheme(), nullptr);
}

TEST(ThemeManager_GetResource_NoTheme) {
    auto& manager = ThemeManager::GetInstance();
    
    auto result = manager.GetResource("AnyKey");
    ASSERT_FALSE(result.has_value());
}

// ==================== Integration Tests ====================

TEST(Integration_StyleWithResources) {
    // Create a resource dictionary
    auto resources = std::make_shared<ResourceDictionary>();
    resources->Add("PrimaryValue", 100);
    resources->Add("SecondaryValue", 200);
    
    // Create a style that uses resources
    auto style = std::make_shared<Style>(typeid(int));
    
    auto propId = DependencyProperty::Register("IntegrationProp", 
        PropertyMetadata{std::any(0), nullptr, false, false, true});
    
    style->AddSetter(propId, ResourceReference("PrimaryValue"));
    
    // Verify we can get the setter
    auto setters = style->GetAllSetters();
    ASSERT_EQ(setters.size(), 1);
    
    // Cleanup
    DependencyProperty::s_properties.erase(propId);
    DependencyProperty::s_nameMap.erase("IntegrationProp");
}

TEST(Integration_StyleInheritance) {
    auto baseStyle = std::make_shared<Style>(typeid(int));
    auto propId1 = DependencyProperty::Register("InheritProp1", 
        PropertyMetadata{std::any(0), nullptr, false, false, true});
    auto propId2 = DependencyProperty::Register("InheritProp2", 
        PropertyMetadata{std::any(0), nullptr, false, false, true});
    
    baseStyle->AddSetter(propId1, 100);
    
    auto derivedStyle = std::make_shared<Style>(typeid(int));
    derivedStyle->SetBasedOn(baseStyle);
    derivedStyle->AddSetter(propId2, 200);
    
    // Derived should have both setters
    auto allSetters = derivedStyle->GetAllSetters();
    ASSERT_EQ(allSetters.size(), 2);
    
    // Cleanup
    DependencyProperty::s_properties.erase(propId1);
    DependencyProperty::s_properties.erase(propId2);
    DependencyProperty::s_nameMap.erase("InheritProp1");
    DependencyProperty::s_nameMap.erase("InheritProp2");
}

// Main entry point
int main() {
    return luaui::test::TestRunner::RunAll().failedTests == 0 ? 0 : 1;
}
