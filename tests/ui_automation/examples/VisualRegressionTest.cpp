// VisualRegressionTest.cpp - Visual regression testing examples

#include "../UIAutomationFramework.h"
#include "../TestHooks.h"
#include "../PixelComparator.h"
#include "Button.h"
#include "TextBlock.h"
#include "TextBox.h"
#include "CheckBox.h"
#include "Slider.h"
#include "Panel.h"
#include "Border.h"
#include "Window.h"
#include <memory>

using namespace luaui;
using namespace luaui::controls;
using namespace luaui::test::automation;

// ============================================================================
// Test: Button Visual States
// ============================================================================
class ButtonVisualStateTest : public UITestScenario {
public:
    ButtonVisualStateTest() : UITestScenario("ButtonVisualStateTest") {}
    
protected:
    Window* CreateTestWindow() override {
        return new TestWindow();
    }
    
    void Run() override {
        SetUp();
        
        auto root = std::make_shared<StackPanel>();
        root->SetSpacing(20);
        
        auto button = std::make_shared<Button>();
        button->SetText(L"Test Button");
        button->SetName("testButton");
        root->AddChild(button);
        
        if (auto* tw = dynamic_cast<TestWindow*>(m_window)) {
            tw->SetRoot(root);
        }
        
        WaitForLayout();
        
        // Capture normal state
        TakeScreenshot("button_normal");
        
        // Hover over button
        auto* btn = m_finder->FindById("testButton");
        m_input->MouseMoveToControl(btn);
        WaitForAnimation();
        TakeScreenshot("button_hover");
        
        // Press button
        m_input->MouseDown(0);
        WaitForAnimation();
        TakeScreenshot("button_pressed");
        
        // Release
        m_input->MouseUp(0);
        WaitForAnimation();
        TakeScreenshot("button_released");
        
        // Compare with baselines if they exist
        // If baselines don't exist, they will be created for manual review
        
        // For demonstration, we'll just verify screenshots were captured
        auto config = UIAutomationRunner::Instance().GetConfig();
        auto normalPath = config.screenshotOutputPath / "button_normal.png";
        auto hoverPath = config.screenshotOutputPath / "button_hover.png";
        
        ASSERT_TRUE(std::filesystem::exists(normalPath));
        ASSERT_TRUE(std::filesystem::exists(hoverPath));
        
        TearDown();
    }
};

UI_TEST(ButtonVisualStateTest);

// ============================================================================
// Test: Control Theme Consistency
// ============================================================================
class ThemeConsistencyTest : public UITestScenario {
public:
    ThemeConsistencyTest() : UITestScenario("ThemeConsistencyTest") {}
    
protected:
    Window* CreateTestWindow() override {
        return new TestWindow();
    }
    
    void Run() override {
        SetUp();
        
        auto root = std::make_shared<StackPanel>();
        root->SetSpacing(10);
        
        // Primary button
        auto primaryBtn = std::make_shared<Button>();
        primaryBtn->SetText(L"Primary");
        primaryBtn->SetStateColors(
            Color::FromHex(0x2196F3),  // Normal
            Color::FromHex(0x1976D2),  // Hover
            Color::FromHex(0x0D47A1)   // Pressed
        );
        root->AddChild(primaryBtn);
        
        // Success button
        auto successBtn = std::make_shared<Button>();
        successBtn->SetText(L"Success");
        successBtn->SetStateColors(
            Color::FromHex(0x4CAF50),
            Color::FromHex(0x388E3C),
            Color::FromHex(0x1B5E20)
        );
        root->AddChild(successBtn);
        
        // Danger button
        auto dangerBtn = std::make_shared<Button>();
        dangerBtn->SetText(L"Danger");
        dangerBtn->SetStateColors(
            Color::FromHex(0xF44336),
            Color::FromHex(0xD32F2F),
            Color::FromHex(0xB71C1C)
        );
        root->AddChild(dangerBtn);
        
        if (auto* tw = dynamic_cast<TestWindow*>(m_window)) {
            tw->SetRoot(root);
        }
        
        WaitForLayout();
        
        // Capture all buttons
        TakeScreenshot("button_themes");
        
        // Verify each button has correct colors
        // This would require pixel-level inspection of the screenshot
        
        TearDown();
    }
};

UI_TEST(ThemeConsistencyTest);

// ============================================================================
// Test: Text Rendering Quality
// ============================================================================
class TextRenderingTest : public UITestScenario {
public:
    TextRenderingTest() : UITestScenario("TextRenderingTest") {}
    
protected:
    Window* CreateTestWindow() override {
        return new TestWindow();
    }
    
    void Run() override {
        SetUp();
        
        auto root = std::make_shared<StackPanel>();
        root->SetSpacing(5);
        
        // Different font sizes
        for (int size : {10, 12, 14, 16, 18, 20, 24}) {
            auto text = std::make_shared<TextBlock>();
            text->SetText(L"The quick brown fox jumps over the lazy dog");
            text->SetFontSize(static_cast<float>(size));
            root->AddChild(text);
        }
        
        // Different font weights
        auto weights = {
            std::make_pair(FontWeight::Light, L"Light"),
            std::make_pair(FontWeight::Normal, L"Normal"),
            std::make_pair(FontWeight::SemiBold, L"SemiBold"),
            std::make_pair(FontWeight::Bold, L"Bold")
        };
        
        for (auto& [weight, name] : weights) {
            auto text = std::make_shared<TextBlock>();
            text->SetText(name);
            text->SetFontWeight(weight);
            text->SetFontSize(16);
            root->AddChild(text);
        }
        
        if (auto* tw = dynamic_cast<TestWindow*>(m_window)) {
            tw->SetRoot(root);
        }
        
        WaitForLayout();
        
        // Capture text rendering
        TakeScreenshot("text_rendering");
        
        // Visual inspection would verify:
        // - Text is crisp and readable
        // - Different sizes render correctly
        // - Different weights are distinguishable
        
        TearDown();
    }
};

UI_TEST(TextRenderingTest);

// ============================================================================
// Test: Form Layout Visual Consistency
// ============================================================================
class FormLayoutVisualTest : public UITestScenario {
public:
    FormLayoutVisualTest() : UITestScenario("FormLayoutVisualTest") {}
    
protected:
    Window* CreateTestWindow() override {
        return new TestWindow();
    }
    
    void Run() override {
        SetUp();
        
        auto root = std::make_shared<Grid>();
        root->AddColumn(GridLength(100, GridUnitType::Pixel));  // Labels
        root->AddColumn(GridLength(1, GridUnitType::Star));     // Inputs
        root->AddRow(GridLength(1, GridUnitType::Auto));
        root->AddRow(GridLength(1, GridUnitType::Auto));
        root->AddRow(GridLength(1, GridUnitType::Auto));
        root->AddRow(GridLength(1, GridUnitType::Auto));
        
        // Name field
        auto nameLabel = std::make_shared<TextBlock>();
        nameLabel->SetText(L"Name:");
        Grid::SetRow(nameLabel, 0);
        root->AddChild(nameLabel);
        
        auto nameInput = std::make_shared<TextBox>();
        nameInput->SetText(L"John Doe");
        Grid::SetRow(nameInput, 0);
        Grid::SetColumn(nameInput, 1);
        root->AddChild(nameInput);
        
        // Email field
        auto emailLabel = std::make_shared<TextBlock>();
        emailLabel->SetText(L"Email:");
        Grid::SetRow(emailLabel, 1);
        root->AddChild(emailLabel);
        
        auto emailInput = std::make_shared<TextBox>();
        emailInput->SetText(L"john@example.com");
        Grid::SetRow(emailInput, 1);
        Grid::SetColumn(emailInput, 1);
        root->AddChild(emailInput);
        
        // Subscribe checkbox
        auto subscribe = std::make_shared<CheckBox>();
        subscribe->SetText(L"Subscribe to newsletter");
        subscribe->SetIsChecked(true);
        Grid::SetRow(subscribe, 2);
        Grid::SetColumnSpan(subscribe, 2);
        root->AddChild(subscribe);
        
        // Submit button
        auto submit = std::make_shared<Button>();
        submit->SetText(L"Submit");
        Grid::SetRow(submit, 3);
        Grid::SetColumn(submit, 1);
        root->AddChild(submit);
        
        if (auto* tw = dynamic_cast<TestWindow*>(m_window)) {
            tw->SetRoot(root);
        }
        
        WaitForLayout();
        
        // Capture form layout
        TakeScreenshot("form_layout");
        
        // Compare with baseline
        // ASSERT_MATCHES_BASELINE(m_window, "form_layout_baseline");
        
        // Test: Form elements should be properly aligned
        auto* nameLabelCtrl = nameLabel.get();
        auto* emailLabelCtrl = emailLabel.get();
        
        auto nameRect = ControlTestHook::GetRenderRect(nameLabelCtrl);
        auto emailRect = ControlTestHook::GetRenderRect(emailLabelCtrl);
        
        // Labels should be left-aligned
        ASSERT_NEAR(nameRect.x, emailRect.x, 2.0f);
        
        TearDown();
    }
};

UI_TEST(FormLayoutVisualTest);

// ============================================================================
// Test: Animation Frame Consistency
// ============================================================================
class AnimationFrameTest : public UITestScenario {
public:
    AnimationFrameTest() : UITestScenario("AnimationFrameTest") {}
    
protected:
    Window* CreateTestWindow() override {
        return new TestWindow();
    }
    
    void Run() override {
        SetUp();
        
        auto root = std::make_shared<StackPanel>();
        
        auto slider = std::make_shared<Slider>();
        slider->SetName("animSlider");
        slider->SetMinimum(0);
        slider->SetMaximum(100);
        slider->SetValue(0);
        if (auto* l = slider->GetLayout()) {
            l->SetWidth(400);
        }
        root->AddChild(slider);
        
        auto valueLabel = std::make_shared<TextBlock>();
        valueLabel->SetName("valueLabel");
        valueLabel->SetText(L"0");
        root->AddChild(valueLabel);
        
        slider->ValueChanged.Add([valueLabel](Control*, double val) {
            valueLabel->SetText(std::to_wstring(static_cast<int>(val)));
        });
        
        if (auto* tw = dynamic_cast<TestWindow*>(m_window)) {
            tw->SetRoot(root);
        }
        
        WaitForLayout();
        
        // Capture frames during slider animation
        auto* s = dynamic_cast<Slider*>(m_finder->FindById("animSlider"));
        
        // Start position
        TakeScreenshot("slider_frame_0");
        
        // Drag to different positions
        float positions[] = {25.0f, 50.0f, 75.0f, 100.0f};
        for (int i = 0; i < 4; i++) {
            // Click at position on slider
            auto* sliderCtrl = m_finder->FindById("animSlider");
            auto rect = ControlTestHook::GetRenderRect(sliderCtrl);
            float x = rect.x + (rect.width * positions[i] / 100.0f);
            float y = rect.y + rect.height / 2;
            
            m_input->ClickAt(x, y);
            WaitForAnimation();
            
            std::string screenshotName = "slider_frame_" + std::to_string(i + 1);
            TakeScreenshot(screenshotName);
            
            // Verify slider value
            ASSERT_NEAR(s->GetValue(), positions[i], 5.0);  // Allow some tolerance
        }
        
        // Verify each frame is different (animation occurred)
        // This would require comparing screenshots
        
        TearDown();
    }
};

UI_TEST(AnimationFrameTest);

// ============================================================================
// Test: Cross-Platform Pixel Consistency
// ============================================================================
class PixelConsistencyTest : public UITestScenario {
public:
    PixelConsistencyTest() : UITestScenario("PixelConsistencyTest") {}
    
protected:
    Window* CreateTestWindow() override {
        return new TestWindow();
    }
    
    void Run() override {
        SetUp();
        
        // Create a reference UI with known colors and positions
        auto root = std::make_shared<Panel>();
        
        auto redBox = std::make_shared<Border>();
        if (auto* l = redBox->GetLayout()) {
            l->SetWidth(50);
            l->SetHeight(50);
            l->SetMargin(10, 10, 0, 0);
        }
        if (auto* r = redBox->GetRender()) {
            r->SetBackground(Color::FromHex(0xFF0000));
        }
        root->AddChild(redBox);
        
        auto greenBox = std::make_shared<Border>();
        if (auto* l = greenBox->GetLayout()) {
            l->SetWidth(50);
            l->SetHeight(50);
            l->SetMargin(70, 10, 0, 0);
        }
        if (auto* r = greenBox->GetRender()) {
            r->SetBackground(Color::FromHex(0x00FF00));
        }
        root->AddChild(greenBox);
        
        auto blueBox = std::make_shared<Border>();
        if (auto* l = blueBox->GetLayout()) {
            l->SetWidth(50);
            l->SetHeight(50);
            l->SetMargin(130, 10, 0, 0);
        }
        if (auto* r = blueBox->GetRender()) {
            r->SetBackground(Color::FromHex(0x0000FF));
        }
        root->AddChild(blueBox);
        
        if (auto* tw = dynamic_cast<TestWindow*>(m_window)) {
            tw->SetRoot(root);
        }
        
        WaitForLayout();
        
        TakeScreenshot("color_reference");
        
        // Load the screenshot and verify pixel colors
        auto config = UIAutomationRunner::Instance().GetConfig();
        auto screenshotPath = config.screenshotOutputPath / "color_reference.png";
        
        if (std::filesystem::exists(screenshotPath)) {
            auto image = PixelComparator::LoadImage(screenshotPath);
            
            if (image.IsValid()) {
                // Check red box (at approximately 35, 35)
                auto* redPixel = image.GetPixel(35, 35);
                if (redPixel && image.channels >= 3) {
                    // Red component should be high, green and blue low
                    ASSERT_GT(redPixel[0], 200);  // R
                    ASSERT_LT(redPixel[1], 50);   // G
                    ASSERT_LT(redPixel[2], 50);   // B
                }
                
                // Check green box (at approximately 95, 35)
                auto* greenPixel = image.GetPixel(95, 35);
                if (greenPixel && image.channels >= 3) {
                    ASSERT_LT(greenPixel[0], 50);   // R
                    ASSERT_GT(greenPixel[1], 200);  // G
                    ASSERT_LT(greenPixel[2], 50);   // B
                }
                
                // Check blue box (at approximately 155, 35)
                auto* bluePixel = image.GetPixel(155, 35);
                if (bluePixel && image.channels >= 3) {
                    ASSERT_LT(bluePixel[0], 50);   // R
                    ASSERT_LT(bluePixel[1], 50);   // G
                    ASSERT_GT(bluePixel[2], 200);  // B
                }
            }
        }
        
        TearDown();
    }
};

UI_TEST(PixelConsistencyTest);
