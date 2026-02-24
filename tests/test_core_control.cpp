// Core Module - Control Tests (simplified)
#include "TestFramework.h"
#include "Button.h"
#include "TextBlock.h"
#include "CheckBox.h"

using namespace luaui;
using namespace luaui::controls;

// ==================== Button Tests ====================
TEST(Button_DefaultConstruction) {
    auto btn = std::make_shared<Button>();
    
    ASSERT_TRUE(btn->GetIsVisible());
    ASSERT_TRUE(btn->GetIsEnabled());
    ASSERT_EQ(btn->GetTypeName(), "Button");
}

TEST(Button_Visibility) {
    auto btn = std::make_shared<Button>();
    
    btn->SetIsVisible(false);
    ASSERT_FALSE(btn->GetIsVisible());
    
    btn->SetIsVisible(true);
    ASSERT_TRUE(btn->GetIsVisible());
}

TEST(Button_Enabled) {
    auto btn = std::make_shared<Button>();
    
    btn->SetIsEnabled(false);
    ASSERT_FALSE(btn->GetIsEnabled());
    
    btn->SetIsEnabled(true);
    ASSERT_TRUE(btn->GetIsEnabled());
}

TEST(Button_Name) {
    auto btn = std::make_shared<Button>();
    
    btn->SetName("testButton");
    ASSERT_EQ(btn->GetName(), "testButton");
}

TEST(Button_TextProperty) {
    auto btn = std::make_shared<Button>();
    
    btn->SetText(L"Click Me");
    ASSERT_TRUE(btn->GetText() == L"Click Me");
    
    btn->SetText(L"");
    ASSERT_TRUE(btn->GetText() == L"");
}

// ==================== TextBlock Tests ====================
TEST(TextBlock_DefaultConstruction) {
    auto text = std::make_shared<TextBlock>();
    
    ASSERT_TRUE(text->GetIsVisible());
    ASSERT_TRUE(text->GetIsEnabled());
    ASSERT_EQ(text->GetTypeName(), "TextBlock");
}

TEST(TextBlock_Text) {
    auto text = std::make_shared<TextBlock>();
    
    text->SetText(L"Hello World");
    ASSERT_TRUE(text->GetText() == L"Hello World");
}

// ==================== CheckBox Tests ====================
TEST(CheckBox_DefaultConstruction) {
    auto check = std::make_shared<CheckBox>();
    
    ASSERT_TRUE(check->GetIsVisible());
    ASSERT_TRUE(check->GetIsEnabled());
    ASSERT_EQ(check->GetTypeName(), "CheckBox");
}

TEST(CheckBox_IsChecked) {
    auto check = std::make_shared<CheckBox>();
    
    ASSERT_FALSE(check->GetIsChecked());
    
    check->SetIsChecked(true);
    ASSERT_TRUE(check->GetIsChecked());
    
    check->SetIsChecked(false);
    ASSERT_FALSE(check->GetIsChecked());
}

TEST(CheckBox_Text) {
    auto check = std::make_shared<CheckBox>();
    
    check->SetText(L"Enable Feature");
    ASSERT_TRUE(check->GetText() == L"Enable Feature");
}

TEST(CheckBox_MultipleInstances) {
    auto check1 = std::make_shared<CheckBox>();
    auto check2 = std::make_shared<CheckBox>();
    
    check1->SetIsChecked(true);
    check2->SetIsChecked(false);
    
    ASSERT_TRUE(check1->GetIsChecked());
    ASSERT_FALSE(check2->GetIsChecked());
}

// ==================== Performance Tests ====================
TEST(Control_CreateManyButtons) {
    const int count = 1000;
    std::vector<std::shared_ptr<Button>> buttons;
    buttons.reserve(count);
    
    for (int i = 0; i < count; ++i) {
        buttons.push_back(std::make_shared<Button>());
    }
    
    ASSERT_EQ(buttons.size(), (size_t)count);
}

TEST(Control_CreateManyTextBlocks) {
    const int count = 1000;
    std::vector<std::shared_ptr<TextBlock>> texts;
    texts.reserve(count);
    
    for (int i = 0; i < count; ++i) {
        texts.push_back(std::make_shared<TextBlock>());
    }
    
    ASSERT_EQ(texts.size(), (size_t)count);
}

// ==================== Main ====================
int main() {
    return RUN_ALL_TESTS();
}
