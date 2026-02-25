// test_style_system.cpp - Unit tests for Style System

#include "TestFramework.h"
#include "ResourceDictionary.h"
#include "Style.h"
#include "Trigger.h"
#include "Types.h"
#include <memory>

using namespace luaui::controls;
using namespace luaui::rendering;

// ==================== ResourceDictionary Tests ====================

TEST(ResourceDictionary_StyleOperations) {
    ResourceDictionary dict;
    
    // Create a style
    auto style = std::make_shared<Style>();
    style->AddSetter(Style::BackgroundSetter(Color::FromHex(0xFF0000)));
    
    // Test Add and Get
    dict.AddStyle("RedButton", style);
    
    auto retrieved = dict.GetStyle("RedButton");
    ASSERT_NOT_NULL(retrieved.get());
    ASSERT_EQ(retrieved->GetSetters().size(), 1);
}

TEST(ResourceDictionary_StyleNotFound) {
    ResourceDictionary dict;
    
    auto style = dict.GetStyle("NonExistent");
    ASSERT_NULL(style.get());
}

TEST(ResourceDictionary_ColorOperations) {
    ResourceDictionary dict;
    
    // Test AddColor and GetColor
    dict.AddColor("Primary", Color::FromHex(0xFF8040));
    dict.AddColor("Secondary", Color::FromRGBA(0, 128, 255, 255));
    
    Color primary = dict.GetColor("Primary");
    ASSERT_NEAR(primary.r, 1.0f, 0.01f);
    ASSERT_NEAR(primary.g, 0.5f, 0.01f);
    ASSERT_NEAR(primary.b, 0.25f, 0.01f);
}

TEST(ResourceDictionary_ColorNotFound) {
    ResourceDictionary dict;
    
    // Should return transparent for missing color
    Color c = dict.GetColor("Missing");
    ASSERT_NEAR(c.a, 0.0f, 0.01f);  // Transparent
}

// ==================== Setter Tests ====================

TEST(Setter_EmptyConstruction) {
    Setter setter;
    ASSERT_FALSE(setter.HasApplier());
}

TEST(Setter_BackgroundSetter) {
    // Test the static helper method
    Setter setter = Style::BackgroundSetter(Color::FromHex(0x00FF00));
    ASSERT_TRUE(setter.HasApplier());
}

TEST(Setter_WidthSetter) {
    Setter setter = Style::WidthSetter(100.0f);
    ASSERT_TRUE(setter.HasApplier());
}

TEST(Setter_HeightSetter) {
    Setter setter = Style::HeightSetter(200.0f);
    ASSERT_TRUE(setter.HasApplier());
}

// ==================== Style Tests ====================

TEST(Style_Construction) {
    Style style;
    
    // Initially no setters
    ASSERT_EQ(style.GetSetters().size(), 0);
}

TEST(Style_AddSetter) {
    Style style;
    
    style.AddSetter(Style::BackgroundSetter(Color::Blue()));
    style.AddSetter(Style::WidthSetter(50.0f));
    style.AddSetter(Style::HeightSetter(75.0f));
    
    auto setters = style.GetSetters();
    ASSERT_EQ(setters.size(), 3);
}

TEST(Style_ClearSetters) {
    Style style;
    
    style.AddSetter(Style::BackgroundSetter(Color::Red()));
    ASSERT_EQ(style.GetSetters().size(), 1);
    
    style.ClearSetters();
    ASSERT_EQ(style.GetSetters().size(), 0);
}

TEST(Style_SharedPtr) {
    // Test that Style works with shared_ptr
    auto style = std::make_shared<Style>();
    style->AddSetter(Style::BackgroundSetter(Color::Green()));
    
    ASSERT_EQ(style->GetSetters().size(), 1);
}

// ==================== Trigger Tests ====================

TEST(Trigger_Construction) {
    // Default construction
    Trigger trigger;
    // Just verify it compiles and doesn't crash
}

TEST(Trigger_AddSetter) {
    Trigger trigger;
    
    trigger.AddSetter(Style::WidthSetter(100.0f));
    trigger.AddSetter(Style::HeightSetter(200.0f));
    
    // Setters are stored internally, not directly accessible
    // Just verify it compiles
}

TEST(Trigger_SharedPtr) {
    auto trigger = std::make_shared<Trigger>();
    trigger->AddSetter(Style::BackgroundSetter(Color::White()));
    // Verify shared_ptr works with Trigger
}

// ==================== Theme Tests ====================

TEST(Theme_Construction) {
    Theme theme;
    
    // New theme has no default styles
    auto style = theme.GetDefaultStyle("Button");
    ASSERT_NULL(style.get());
}

TEST(Theme_SetDefaultStyle) {
    Theme theme;
    
    auto buttonStyle = std::make_shared<Style>();
    buttonStyle->AddSetter(Style::BackgroundSetter(Color::FromHex(0x0078D4)));
    
    theme.SetDefaultStyle("Button", buttonStyle);
    
    auto retrieved = theme.GetDefaultStyle("Button");
    ASSERT_NOT_NULL(retrieved.get());
    ASSERT_EQ(retrieved->GetSetters().size(), 1);
}

TEST(Theme_MultipleStyles) {
    Theme theme;
    
    auto buttonStyle = std::make_shared<Style>();
    buttonStyle->AddSetter(Style::BackgroundSetter(Color::Blue()));
    
    auto textBoxStyle = std::make_shared<Style>();
    textBoxStyle->AddSetter(Style::BackgroundSetter(Color::White()));
    
    theme.SetDefaultStyle("Button", buttonStyle);
    theme.SetDefaultStyle("TextBox", textBoxStyle);
    
    ASSERT_EQ(theme.GetDefaultStyle("Button")->GetSetters().size(), 1);
    ASSERT_EQ(theme.GetDefaultStyle("TextBox")->GetSetters().size(), 1);
    ASSERT_NULL(theme.GetDefaultStyle("Slider").get());
}

TEST(Theme_GetCurrent) {
    // Test singleton-like access
    Theme& theme1 = Theme::GetCurrent();
    Theme& theme2 = Theme::GetCurrent();
    
    // Both should be the same instance
    ASSERT_EQ(&theme1, &theme2);
}

// ==================== Integration Tests ====================

TEST(Integration_StyleWithSetters) {
    // Create a complete style
    auto style = std::make_shared<Style>();
    
    style->AddSetter(Style::BackgroundSetter(Color::FromHex(0xFF8040)));
    style->AddSetter(Style::WidthSetter(120.0f));
    style->AddSetter(Style::HeightSetter(40.0f));
    
    ASSERT_EQ(style->GetSetters().size(), 3);
}

TEST(Integration_ResourceDictionaryWithStyles) {
    ResourceDictionary dict;
    
    // Create and add multiple styles
    auto primaryStyle = std::make_shared<Style>();
    primaryStyle->AddSetter(Style::BackgroundSetter(Color::FromHex(0x0078D4)));
    
    auto dangerStyle = std::make_shared<Style>();
    dangerStyle->AddSetter(Style::BackgroundSetter(Color::FromHex(0xD13438)));
    
    dict.AddStyle("PrimaryButton", primaryStyle);
    dict.AddStyle("DangerButton", dangerStyle);
    
    // Verify both can be retrieved
    ASSERT_EQ(dict.GetStyle("PrimaryButton")->GetSetters().size(), 1);
    ASSERT_EQ(dict.GetStyle("DangerButton")->GetSetters().size(), 1);
    ASSERT_NULL(dict.GetStyle("SuccessButton").get());
}

TEST(Integration_ThemeWithResourceDictionary) {
    // Create a resource dictionary with colors
    ResourceDictionary resources;
    resources.AddColor("Background", Color::FromHex(0xFFFFFF));
    resources.AddColor("Foreground", Color::FromHex(0x000000));
    resources.AddColor("Accent", Color::FromHex(0x0078D4));
    
    // Create a theme with styles using those colors
    Theme theme;
    
    auto buttonStyle = std::make_shared<Style>();
    buttonStyle->AddSetter(Style::BackgroundSetter(resources.GetColor("Accent")));
    
    theme.SetDefaultStyle("Button", buttonStyle);
    
    ASSERT_NOT_NULL(theme.GetDefaultStyle("Button").get());
}

TEST(Integration_StyleInheritanceViaComposition) {
    // Simulate style "inheritance" by composing setters
    auto baseStyle = std::make_shared<Style>();
    baseStyle->AddSetter(Style::WidthSetter(100.0f));
    baseStyle->AddSetter(Style::HeightSetter(50.0f));
    
    auto derivedStyle = std::make_shared<Style>();
    derivedStyle->AddSetter(Style::BackgroundSetter(Color::Red()));
    
    // Apply base style setters to derived
    for (const auto& setter : baseStyle->GetSetters()) {
        derivedStyle->AddSetter(setter);
    }
    
    ASSERT_EQ(derivedStyle->GetSetters().size(), 3);
}

// ==================== ResourceReference Tests ====================

TEST(ResourceReference_Construction) {
    ResourceReference ref;
    ASSERT_TRUE(ref.key.empty());
}

TEST(ResourceReference_WithKey) {
    ResourceReference ref("PrimaryColor");
    ASSERT_EQ(ref.key, "PrimaryColor");
}

TEST(ResourceReference_CopyConstruction) {
    ResourceReference ref1("TestKey");
    ResourceReference ref2 = ref1;
    ASSERT_EQ(ref2.key, "TestKey");
}

// Main entry point
int main() {
    return luaui::test::TestRunner::RunAll().failedTests == 0 ? 0 : 1;
}
