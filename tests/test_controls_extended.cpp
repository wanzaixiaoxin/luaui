// Extended Controls Tests - Slider, ProgressBar, TextBox, Image, etc.
#include "TestFramework.h"
#include "Slider.h"
#include "ProgressBar.h"
#include "TextBox.h"
#include "Image.h"
#include "Button.h"
#include "TextBlock.h"
#include "CheckBox.h"
#include "../core/Delegate.h"

using namespace luaui;
using namespace luaui::controls;

// ==================== Slider Tests ====================
TEST(Slider_DefaultConstruction) {
    auto slider = std::make_shared<Slider>();
    
    ASSERT_TRUE(slider->GetIsVisible());
    ASSERT_TRUE(slider->GetIsEnabled());
    ASSERT_EQ(slider->GetTypeName(), "Slider");
    
    // Default values
    ASSERT_NEAR(slider->GetValue(), 0.0, 0.001);
    ASSERT_NEAR(slider->GetMinimum(), 0.0, 0.001);
    ASSERT_NEAR(slider->GetMaximum(), 100.0, 0.001);
    ASSERT_FALSE(slider->GetIsVertical());
}

TEST(Slider_ValueProperty) {
    auto slider = std::make_shared<Slider>();
    
    slider->SetValue(50.0);
    ASSERT_NEAR(slider->GetValue(), 50.0, 0.001);
    
    slider->SetValue(75.5);
    ASSERT_NEAR(slider->GetValue(), 75.5, 0.001);
    
    // Value should be clamped to valid range
    slider->SetValue(-10.0);
    ASSERT_NEAR(slider->GetValue(), 0.0, 0.001);  // Clamped to minimum
    
    slider->SetValue(150.0);
    ASSERT_NEAR(slider->GetValue(), 100.0, 0.001);  // Clamped to maximum
}

TEST(Slider_RangeProperty) {
    auto slider = std::make_shared<Slider>();
    
    slider->SetMinimum(10.0);
    ASSERT_NEAR(slider->GetMinimum(), 10.0, 0.001);
    
    slider->SetMaximum(200.0);
    ASSERT_NEAR(slider->GetMaximum(), 200.0, 0.001);
    
    // Setting value in new range
    slider->SetValue(150.0);
    ASSERT_NEAR(slider->GetValue(), 150.0, 0.001);
}

TEST(Slider_Orientation) {
    auto slider = std::make_shared<Slider>();
    
    ASSERT_FALSE(slider->GetIsVertical());  // Default horizontal
    
    slider->SetIsVertical(true);
    ASSERT_TRUE(slider->GetIsVertical());
    
    slider->SetIsVertical(false);
    ASSERT_FALSE(slider->GetIsVertical());
}

TEST(Slider_ChangeProperties) {
    auto slider = std::make_shared<Slider>();
    
    slider->SetSmallChange(5.0);
    ASSERT_NEAR(slider->GetSmallChange(), 5.0, 0.001);
    
    slider->SetLargeChange(20.0);
    ASSERT_NEAR(slider->GetLargeChange(), 20.0, 0.001);
    
    slider->SetIsSnapToTick(true);
    ASSERT_TRUE(slider->GetIsSnapToTick());
}

// ==================== ProgressBar Tests ====================
TEST(ProgressBar_DefaultConstruction) {
    auto progress = std::make_shared<ProgressBar>();
    
    ASSERT_TRUE(progress->GetIsVisible());
    ASSERT_TRUE(progress->GetIsEnabled());
    ASSERT_EQ(progress->GetTypeName(), "ProgressBar");
    
    // Default values
    ASSERT_NEAR(progress->GetMinimum(), 0.0, 0.001);
    ASSERT_NEAR(progress->GetMaximum(), 100.0, 0.001);
    ASSERT_NEAR(progress->GetValue(), 0.0, 0.001);
    ASSERT_FALSE(progress->GetIsIndeterminate());
    ASSERT_TRUE(progress->GetOrientation() == ProgressBar::Orientation::Horizontal);
}

TEST(ProgressBar_ValueProperty) {
    auto progress = std::make_shared<ProgressBar>();
    
    progress->SetValue(50.0);
    ASSERT_NEAR(progress->GetValue(), 50.0, 0.001);
    
    progress->SetValue(100.0);
    ASSERT_NEAR(progress->GetValue(), 100.0, 0.001);
    
    // Value should be clamped
    progress->SetValue(-10.0);
    ASSERT_NEAR(progress->GetValue(), 0.0, 0.001);
    
    progress->SetValue(200.0);
    ASSERT_NEAR(progress->GetValue(), 100.0, 0.001);
}

TEST(ProgressBar_Percent) {
    auto progress = std::make_shared<ProgressBar>();
    
    progress->SetValue(0.0);
    ASSERT_NEAR(progress->GetPercent(), 0.0, 0.001);
    
    progress->SetValue(50.0);
    ASSERT_NEAR(progress->GetPercent(), 0.5, 0.001);
    
    progress->SetValue(100.0);
    ASSERT_NEAR(progress->GetPercent(), 1.0, 0.001);
}

TEST(ProgressBar_Range) {
    auto progress = std::make_shared<ProgressBar>();
    
    progress->SetMinimum(25.0);
    progress->SetMaximum(75.0);
    
    ASSERT_NEAR(progress->GetMinimum(), 25.0, 0.001);
    ASSERT_NEAR(progress->GetMaximum(), 75.0, 0.001);
    
    progress->SetValue(50.0);
    ASSERT_NEAR(progress->GetValue(), 50.0, 0.001);
}

TEST(ProgressBar_IndeterminateMode) {
    auto progress = std::make_shared<ProgressBar>();
    
    ASSERT_FALSE(progress->GetIsIndeterminate());
    
    progress->SetIsIndeterminate(true);
    ASSERT_TRUE(progress->GetIsIndeterminate());
    
    progress->SetIsIndeterminate(false);
    ASSERT_FALSE(progress->GetIsIndeterminate());
}

TEST(ProgressBar_Orientation) {
    auto progress = std::make_shared<ProgressBar>();
    
    ASSERT_TRUE(progress->GetOrientation() == ProgressBar::Orientation::Horizontal);
    
    progress->SetOrientation(ProgressBar::Orientation::Vertical);
    ASSERT_TRUE(progress->GetOrientation() == ProgressBar::Orientation::Vertical);
    
    progress->SetOrientation(ProgressBar::Orientation::Horizontal);
    ASSERT_TRUE(progress->GetOrientation() == ProgressBar::Orientation::Horizontal);
}

TEST(ProgressBar_TextFormat) {
    auto progress = std::make_shared<ProgressBar>();
    
    progress->SetShowText(true);
    ASSERT_TRUE(progress->GetShowText());
    
    progress->SetTextFormat(L"Loading: {0}%");
    ASSERT_TRUE(progress->GetTextFormat() == L"Loading: {0}%");
}

// ==================== TextBox Tests ====================
TEST(TextBox_DefaultConstruction) {
    auto textbox = std::make_shared<TextBox>();
    
    ASSERT_TRUE(textbox->GetIsVisible());
    ASSERT_TRUE(textbox->GetIsEnabled());
    ASSERT_EQ(textbox->GetTypeName(), "TextBox");
    
    ASSERT_TRUE(textbox->GetText().empty());
    ASSERT_FALSE(textbox->GetIsReadOnly());
    ASSERT_FALSE(textbox->GetIsPassword());
    ASSERT_EQ(textbox->GetMaxLength(), 0);
}

TEST(TextBox_TextProperty) {
    auto textbox = std::make_shared<TextBox>();
    
    textbox->SetText(L"Hello World");
    ASSERT_TRUE(textbox->GetText() == L"Hello World");
    
    textbox->SetText(L"");
    ASSERT_TRUE(textbox->GetText().empty());
    
    textbox->SetText(L"Chinese text: 中文测试");
    ASSERT_TRUE(textbox->GetText() == L"Chinese text: 中文测试");
}

TEST(TextBox_Placeholder) {
    auto textbox = std::make_shared<TextBox>();
    
    textbox->SetPlaceholder(L"Enter your name...");
    ASSERT_TRUE(textbox->GetPlaceholder() == L"Enter your name...");
}

TEST(TextBox_ReadOnlyMode) {
    auto textbox = std::make_shared<TextBox>();
    
    ASSERT_FALSE(textbox->GetIsReadOnly());
    
    textbox->SetIsReadOnly(true);
    ASSERT_TRUE(textbox->GetIsReadOnly());
}

TEST(TextBox_PasswordMode) {
    auto textbox = std::make_shared<TextBox>();
    
    ASSERT_FALSE(textbox->GetIsPassword());
    
    textbox->SetIsPassword(true);
    ASSERT_TRUE(textbox->GetIsPassword());
}

TEST(TextBox_MaxLength) {
    auto textbox = std::make_shared<TextBox>();
    
    textbox->SetMaxLength(100);
    ASSERT_EQ(textbox->GetMaxLength(), 100);
    
    textbox->SetMaxLength(0);  // No limit
    ASSERT_EQ(textbox->GetMaxLength(), 0);
}

TEST(TextBox_CaretPosition) {
    auto textbox = std::make_shared<TextBox>();
    
    textbox->SetText(L"Hello");
    textbox->SetCaretPosition(3);
    ASSERT_EQ(textbox->GetCaretPosition(), 3);
}

TEST(TextBox_Selection) {
    auto textbox = std::make_shared<TextBox>();
    
    textbox->SetText(L"Hello World");
    
    ASSERT_FALSE(textbox->HasSelection());
    
    textbox->SelectAll();
    ASSERT_TRUE(textbox->HasSelection());
    
    textbox->ClearSelection();
    ASSERT_FALSE(textbox->HasSelection());
}

// ==================== Image Tests ====================
TEST(Image_DefaultConstruction) {
    auto image = std::make_shared<Image>();
    
    ASSERT_TRUE(image->GetIsVisible());
    ASSERT_TRUE(image->GetIsEnabled());
    ASSERT_EQ(image->GetTypeName(), "Image");
    
    ASSERT_TRUE(image->GetStretch() == Stretch::Uniform);
    ASSERT_NEAR(image->GetOpacity(), 1.0f, 0.001f);
    ASSERT_FALSE(image->IsLoaded());
}

TEST(Image_StretchProperty) {
    auto image = std::make_shared<Image>();
    
    image->SetStretch(Stretch::Fill);
    ASSERT_TRUE(image->GetStretch() == Stretch::Fill);
    
    image->SetStretch(Stretch::Uniform);
    ASSERT_TRUE(image->GetStretch() == Stretch::Uniform);
    
    image->SetStretch(Stretch::UniformToFill);
    ASSERT_TRUE(image->GetStretch() == Stretch::UniformToFill);
    
    image->SetStretch(Stretch::None);
    ASSERT_TRUE(image->GetStretch() == Stretch::None);
}

TEST(Image_OpacityProperty) {
    auto image = std::make_shared<Image>();
    
    image->SetOpacity(0.5f);
    ASSERT_NEAR(image->GetOpacity(), 0.5f, 0.001f);
    
    image->SetOpacity(0.0f);
    ASSERT_NEAR(image->GetOpacity(), 0.0f, 0.001f);
    
    image->SetOpacity(1.0f);
    ASSERT_NEAR(image->GetOpacity(), 1.0f, 0.001f);
}

TEST(Image_SourcePath) {
    auto image = std::make_shared<Image>();
    
    image->SetSourcePath(L"C:/test/image.png");
    ASSERT_TRUE(image->GetSourcePath() == L"C:/test/image.png");
    
    image->Unload();
    ASSERT_FALSE(image->IsLoaded());
}

// ==================== Multiple Instances Tests ====================
TEST(Slider_MultipleInstances) {
    auto slider1 = std::make_shared<Slider>();
    auto slider2 = std::make_shared<Slider>();
    
    slider1->SetValue(25.0);
    slider2->SetValue(75.0);
    
    ASSERT_NEAR(slider1->GetValue(), 25.0, 0.001);
    ASSERT_NEAR(slider2->GetValue(), 75.0, 0.001);
    
    slider1->SetIsVertical(true);
    slider2->SetIsVertical(false);
    
    ASSERT_TRUE(slider1->GetIsVertical());
    ASSERT_FALSE(slider2->GetIsVertical());
}

TEST(ProgressBar_MultipleInstances) {
    auto p1 = std::make_shared<ProgressBar>();
    auto p2 = std::make_shared<ProgressBar>();
    
    p1->SetValue(30.0);
    p2->SetValue(70.0);
    
    ASSERT_NEAR(p1->GetPercent(), 0.3, 0.001);
    ASSERT_NEAR(p2->GetPercent(), 0.7, 0.001);
    
    p1->SetIsIndeterminate(true);
    p2->SetIsIndeterminate(false);
    
    ASSERT_TRUE(p1->GetIsIndeterminate());
    ASSERT_FALSE(p2->GetIsIndeterminate());
}

TEST(TextBox_MultipleInstances) {
    auto t1 = std::make_shared<TextBox>();
    auto t2 = std::make_shared<TextBox>();
    
    t1->SetText(L"First");
    t2->SetText(L"Second");
    
    ASSERT_TRUE(t1->GetText() == L"First");
    ASSERT_TRUE(t2->GetText() == L"Second");
    
    t1->SetIsPassword(true);
    t2->SetIsReadOnly(true);
    
    ASSERT_TRUE(t1->GetIsPassword());
    ASSERT_FALSE(t1->GetIsReadOnly());
    ASSERT_TRUE(t2->GetIsReadOnly());
    ASSERT_FALSE(t2->GetIsPassword());
}

// ==================== Performance Tests ====================
TEST(Controls_CreateManySliders) {
    const int count = 1000;
    std::vector<std::shared_ptr<Slider>> sliders;
    sliders.reserve(count);
    
    for (int i = 0; i < count; ++i) {
        sliders.push_back(std::make_shared<Slider>());
    }
    
    ASSERT_EQ(sliders.size(), (size_t)count);
}

TEST(Controls_CreateManyProgressBars) {
    const int count = 1000;
    std::vector<std::shared_ptr<ProgressBar>> bars;
    bars.reserve(count);
    
    for (int i = 0; i < count; ++i) {
        bars.push_back(std::make_shared<ProgressBar>());
    }
    
    ASSERT_EQ(bars.size(), (size_t)count);
}

TEST(Controls_CreateManyTextBoxes) {
    const int count = 1000;
    std::vector<std::shared_ptr<TextBox>> boxes;
    boxes.reserve(count);
    
    for (int i = 0; i < count; ++i) {
        boxes.push_back(std::make_shared<TextBox>());
    }
    
    ASSERT_EQ(boxes.size(), (size_t)count);
}

// ==================== Main ====================
int main() {
    return RUN_ALL_TESTS();
}
