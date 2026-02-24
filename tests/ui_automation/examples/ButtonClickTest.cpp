// ButtonClickTest.cpp - Example UI automation test for Button control

#include "../UIAutomationFramework.h"
#include "../TestHooks.h"
#include "Button.h"
#include "TextBlock.h"
#include "Panel.h"
#include "Window.h"
#include <memory>

using namespace luaui;
using namespace luaui::controls;
using namespace luaui::test::automation;

// ============================================================================
// Test: Button Click Interaction
// ============================================================================
class ButtonClickTest : public UITestScenario {
public:
    ButtonClickTest() : UITestScenario("ButtonClickTest") {}
    
protected:
    Window* CreateTestWindow() override {
        return new TestWindow();
    }
    
    void Run() override {
        // Setup
        SetUp();
        
        // Create a simple UI with a button
        auto root = std::make_shared<StackPanel>();
        root->SetOrientation(StackPanel::Orientation::Vertical);
        root->SetSpacing(20);
        
        auto statusText = std::make_shared<TextBlock>();
        statusText->SetText(L"Not clicked");
        statusText->SetName("statusLabel");
        root->AddChild(statusText);
        
        auto button = std::make_shared<Button>();
        button->SetText(L"Click Me");
        button->SetName("testButton");
        
        // Track click count
        int clickCount = 0;
        button->Click.Add([&clickCount, &statusText](Control*) {
            clickCount++;
            statusText->SetText(L"Clicked!");
        });
        
        root->AddChild(button);
        
        // Set window content
        if (auto* tw = dynamic_cast<TestWindow*>(m_window)) {
            // Use public method to set root
            tw->SetRoot(root);
        }
        
        // Wait for layout
        WaitForLayout();
        
        // Test 1: Verify initial state
        {
            auto* foundButton = m_finder->FindById("testButton");
            ASSERT_NOT_NULL(foundButton);
            ASSERT_VISIBLE(foundButton);
            
            auto* foundLabel = m_finder->FindById("statusLabel");
            ASSERT_TEXT_EQUALS(dynamic_cast<TextBlock*>(foundLabel), L"Not clicked");
        }
        
        // Test 2: Click the button
        {
            auto* button = m_finder->FindById("testButton");
            m_input->ClickOn(button);
            
            WaitForAnimation();
            
            // Verify button was clicked
            auto* label = m_finder->FindById("statusLabel");
            ASSERT_TEXT_EQUALS(dynamic_cast<TextBlock*>(label), L"Clicked!");
            ASSERT_EQ(clickCount, 1);
        }
        
        // Test 3: Double click
        {
            clickCount = 0;
            auto* button = m_finder->FindById("testButton");
            m_input->ClickOn(button);
            m_input->ClickOn(button);
            
            WaitForAnimation();
            
            ASSERT_EQ(clickCount, 2);
        }
        
        // Test 4: Visual regression test
        {
            TakeScreenshot("button_click_final_state");
            // Compare with baseline if exists, or create baseline
            // ASSERT_MATCHES_BASELINE(m_window, "button_click_final_state");
        }
        
        // Cleanup
        TearDown();
    }
};

UI_TEST(ButtonClickTest);

// ============================================================================
// Test: Button State Changes (Enabled/Disabled)
// ============================================================================
class ButtonStateTest : public UITestScenario {
public:
    ButtonStateTest() : UITestScenario("ButtonStateTest") {}
    
protected:
    Window* CreateTestWindow() override {
        return new TestWindow();
    }
    
    void Run() override {
        SetUp();
        
        auto root = std::make_shared<StackPanel>();
        
        auto enabledButton = std::make_shared<Button>();
        enabledButton->SetText(L"Enabled");
        enabledButton->SetName("enabledBtn");
        root->AddChild(enabledButton);
        
        auto disabledButton = std::make_shared<Button>();
        disabledButton->SetText(L"Disabled");
        disabledButton->SetName("disabledBtn");
        disabledButton->SetIsEnabled(false);
        root->AddChild(disabledButton);
        
        if (auto* tw = dynamic_cast<TestWindow*>(m_window)) {
            tw->SetRoot(root);
        }
        
        WaitForLayout();
        
        // Verify enabled button can be clicked
        auto* enabled = m_finder->FindById("enabledBtn");
        ASSERT_NOT_NULL(enabled);
        UIAssert::IsEnabled(enabled);
        
        // Verify disabled button cannot be clicked
        auto* disabled = m_finder->FindById("disabledBtn");
        ASSERT_NOT_NULL(disabled);
        UIAssert::IsDisabled(disabled);
        
        // Try clicking disabled button (should not fire event)
        bool clicked = false;
        disabledButton->Click.Add([&clicked](Control*) { clicked = true; });
        m_input->ClickOn(disabled);
        WaitForAnimation();
        
        ASSERT_FALSE(clicked);
        
        TearDown();
    }
};

UI_TEST(ButtonStateTest);

// ============================================================================
// Test: Button Layout and Position
// ============================================================================
class ButtonLayoutTest : public UITestScenario {
public:
    ButtonLayoutTest() : UITestScenario("ButtonLayoutTest") {}
    
protected:
    Window* CreateTestWindow() override {
        return new TestWindow();
    }
    
    void Run() override {
        SetUp();
        
        auto root = std::make_shared<StackPanel>();
        root->SetOrientation(StackPanel::Orientation::Horizontal);
        root->SetSpacing(10);
        
        auto btn1 = std::make_shared<Button>();
        btn1->SetText(L"Button 1");
        btn1->SetName("btn1");
        if (auto* l = btn1->GetLayout()) {
            l->SetWidth(100);
            l->SetHeight(40);
        }
        root->AddChild(btn1);
        
        auto btn2 = std::make_shared<Button>();
        btn2->SetText(L"Button 2");
        btn2->SetName("btn2");
        if (auto* l = btn2->GetLayout()) {
            l->SetWidth(100);
            l->SetHeight(40);
        }
        root->AddChild(btn2);
        
        if (auto* tw = dynamic_cast<TestWindow*>(m_window)) {
            tw->SetRoot(root);
        }
        
        WaitForLayout();
        
        // Verify button sizes
        auto* button1 = m_finder->FindById("btn1");
        auto* button2 = m_finder->FindById("btn2");
        
        ASSERT_NOT_NULL(button1);
        ASSERT_NOT_NULL(button2);
        
        // Check positions (btn2 should be to the right of btn1)
        UIAssert::IsAligned(button2, button1, "right");
        
        // Check sizes
        UIAssert::HasSize(button1, 100, 40);
        UIAssert::HasSize(button2, 100, 40);
        
        TearDown();
    }
};

UI_TEST(ButtonLayoutTest);
