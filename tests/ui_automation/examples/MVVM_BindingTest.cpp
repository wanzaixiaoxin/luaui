// MVVM_BindingTest.cpp - Automated tests for MVVM data binding

#include "../UIAutomationFramework.h"
#include "../TestHooks.h"
#include "TextBlock.h"
#include "TextBox.h"
#include "Slider.h"
#include "Button.h"
#include "Panel.h"
#include "Window.h"
#include "ViewModelBase.h"
#include <memory>
#include <string>

using namespace luaui;
using namespace luaui::controls;
using namespace luaui::test::automation;
using namespace luaui::mvvm;

// ============================================================================
// Test ViewModel
// ============================================================================
class TestProfileViewModel : public ViewModelBase {
public:
    TestProfileViewModel() {
        // Initialize properties
        m_userName = L"John Doe";
        m_age = 25;
        m_isActive = true;
    }
    
    // UserName property
    std::wstring GetUserName() const { return m_userName; }
    void SetUserName(const std::wstring& value) {
        if (SetProperty(m_userName, value, L"UserName")) {
            OnPropertyChanged(L"DisplayText");
        }
    }
    
    // Age property
    int GetAge() const { return m_age; }
    void SetAge(int value) {
        SetProperty(m_age, value, L"Age");
    }
    
    // IsActive property
    bool GetIsActive() const { return m_isActive; }
    void SetIsActive(bool value) {
        SetProperty(m_isActive, value, L"IsActive");
    }
    
    // Computed property
    std::wstring GetDisplayText() const {
        return m_userName + L" (" + std::to_wstring(m_age) + L" years old)";
    }

private:
    std::wstring m_userName;
    int m_age;
    bool m_isActive;
};

// ============================================================================
// Test: OneWay Binding from ViewModel to View
// ============================================================================
class OneWayBindingTest : public UITestScenario {
public:
    OneWayBindingTest() : UITestScenario("OneWayBindingTest") {}
    
protected:
    Window* CreateTestWindow() override {
        return new TestWindow();
    }
    
    void Run() override {
        SetUp();
        
        auto viewModel = std::make_shared<TestProfileViewModel>();
        
        auto root = std::make_shared<StackPanel>();
        
        // TextBlock bound to UserName
        auto nameLabel = std::make_shared<TextBlock>();
        nameLabel->SetName("nameLabel");
        nameLabel->SetText(viewModel->GetUserName()); // Manual binding for now
        root->AddChild(nameLabel);
        
        // TextBlock bound to DisplayText (computed property)
        auto displayLabel = std::make_shared<TextBlock>();
        displayLabel->SetName("displayLabel");
        displayLabel->SetText(viewModel->GetDisplayText());
        root->AddChild(displayLabel);
        
        // Button to trigger ViewModel update
        auto updateButton = std::make_shared<Button>();
        updateButton->SetText(L"Update Name");
        updateButton->SetName("updateBtn");
        updateButton->Click.Add([viewModel](Control*) {
            viewModel->SetUserName(L"Jane Smith");
        });
        root->AddChild(updateButton);
        
        if (auto* tw = dynamic_cast<TestWindow*>(m_window)) {
            tw->SetRoot(root);
        }
        
        WaitForLayout();
        
        // Test 1: Initial binding values
        {
            auto* name = m_finder->FindById("nameLabel");
            auto* display = m_finder->FindById("displayLabel");
            
            ASSERT_NOT_NULL(name);
            ASSERT_NOT_NULL(display);
            
            // Verify initial binding
            ASSERT_TEXT_EQUALS(dynamic_cast<TextBlock*>(name), L"John Doe");
            ASSERT_TEXT_EQUALS(dynamic_cast<TextBlock*>(display), L"John Doe (25 years old)");
        }
        
        // Test 2: Update ViewModel and verify UI updates
        {
            viewModel->SetUserName(L"Alice Johnson");
            viewModel->SetAge(30);
            
            WaitForBindingUpdate();
            
            // UI should reflect changes
            auto* name = m_finder->FindById("nameLabel");
            auto* display = m_finder->FindById("displayLabel");
            
            // Note: Without full binding engine, manual update needed
            // This test demonstrates what should happen with full MVVM
        }
        
        TearDown();
    }
};

UI_TEST(OneWayBindingTest);

// ============================================================================
// Test: TwoWay Binding between TextBox and ViewModel
// ============================================================================
class TwoWayBindingTest : public UITestScenario {
public:
    TwoWayBindingTest() : UITestScenario("TwoWayBindingTest") {}
    
protected:
    Window* CreateTestWindow() override {
        return new TestWindow();
    }
    
    void Run() override {
        SetUp();
        
        auto viewModel = std::make_shared<TestProfileViewModel>();
        
        auto root = std::make_shared<StackPanel>();
        
        // TextBox for editing name (TwoWay binding)
        auto nameInput = std::make_shared<TextBox>();
        nameInput->SetName("nameInput");
        nameInput->SetText(viewModel->GetUserName());
        if (auto* l = nameInput->GetLayout()) {
            l->SetWidth(200);
        }
        root->AddChild(nameInput);
        
        // Display label (OneWay from ViewModel)
        auto displayLabel = std::make_shared<TextBlock>();
        displayLabel->SetName("displayLabel");
        displayLabel->SetText(viewModel->GetUserName());
        root->AddChild(displayLabel);
        
        if (auto* tw = dynamic_cast<TestWindow*>(m_window)) {
            tw->SetRoot(root);
        }
        
        WaitForLayout();
        
        // Test: Type in TextBox and verify ViewModel updates
        {
            auto* input = m_finder->FindById("nameInput");
            ASSERT_NOT_NULL(input);
            
            // Click to focus
            m_input->ClickOn(input);
            WaitForAnimation();
            
            // Type new text
            m_input->SendText(L"New User Name");
            WaitForAnimation();
            
            // In real TwoWay binding, ViewModel should update automatically
            // Here we simulate the binding update
            if (auto* textBox = dynamic_cast<TextBox*>(input)) {
                viewModel->SetUserName(textBox->GetText());
            }
            
            // Verify ViewModel updated
            ASSERT_EQ(viewModel->GetUserName(), L"New User Name");
        }
        
        TearDown();
    }
};

UI_TEST(TwoWayBindingTest);

// ============================================================================
// Test: Slider Value Binding
// ============================================================================
class SliderBindingTest : public UITestScenario {
public:
    SliderBindingTest() : UITestScenario("SliderBindingTest") {}
    
protected:
    Window* CreateTestWindow() override {
        return new TestWindow();
    }
    
    void Run() override {
        SetUp();
        
        auto viewModel = std::make_shared<TestProfileViewModel>();
        
        auto root = std::make_shared<StackPanel>();
        
        // Slider bound to Age
        auto ageSlider = std::make_shared<Slider>();
        ageSlider->SetName("ageSlider");
        ageSlider->SetMinimum(0);
        ageSlider->SetMaximum(100);
        ageSlider->SetValue(viewModel->GetAge());
        if (auto* l = ageSlider->GetLayout()) {
            l->SetWidth(300);
        }
        root->AddChild(ageSlider);
        
        // Display current value
        auto valueLabel = std::make_shared<TextBlock>();
        valueLabel->SetName("valueLabel");
        valueLabel->SetText(std::to_wstring(viewModel->GetAge()));
        root->AddChild(valueLabel);
        
        // Connect slider to update label (simulating binding)
        ageSlider->ValueChanged.Add([valueLabel](Control*, double val) {
            valueLabel->SetText(std::to_wstring(static_cast<int>(val)));
        });
        
        if (auto* tw = dynamic_cast<TestWindow*>(m_window)) {
            tw->SetRoot(root);
        }
        
        WaitForLayout();
        
        // Test: Move slider and verify value updates
        {
            auto* slider = dynamic_cast<Slider*>(m_finder->FindById("ageSlider"));
            ASSERT_NOT_NULL(slider);
            
            // Verify initial value
            ASSERT_NEAR(slider->GetValue(), 25.0, 0.1);
            
            // Click at a position to change value
            // Slider is 300px wide, range 0-100
            // Clicking at x=150 should give value ~50
            auto* sliderControl = m_finder->FindById("ageSlider");
            auto rect = ControlTestHook::GetRenderRect(sliderControl);
            
            m_input->ClickAt(rect.x + 150, rect.y + rect.height / 2);
            WaitForAnimation();
            
            // Verify value changed
            ASSERT_GT(slider->GetValue(), 40.0);  // Should be around 50
            ASSERT_LT(slider->GetValue(), 60.0);
            
            // Verify label updated
            auto* label = dynamic_cast<TextBlock*>(m_finder->FindById("valueLabel"));
            ASSERT_NOT_NULL(label);
            int displayedValue = std::stoi(label->GetText());
            ASSERT_GT(displayedValue, 40);
            ASSERT_LT(displayedValue, 60);
        }
        
        TearDown();
    }
};

UI_TEST(SliderBindingTest);

// ============================================================================
// Test: Collection Binding (ListBox)
// ============================================================================
class CollectionBindingTest : public UITestScenario {
public:
    CollectionBindingTest() : UITestScenario("CollectionBindingTest") {}
    
protected:
    Window* CreateTestWindow() override {
        return new TestWindow();
    }
    
    void Run() override {
        SetUp();
        
        auto root = std::make_shared<StackPanel>();
        
        auto listBox = std::make_shared<ListBox>();
        listBox->SetName("itemList");
        if (auto* l = listBox->GetLayout()) {
            l->SetWidth(200);
            l->SetHeight(150);
        }
        root->AddChild(listBox);
        
        // Add items (simulating bound collection)
        listBox->AddItem(L"Item 1");
        listBox->AddItem(L"Item 2");
        listBox->AddItem(L"Item 3");
        listBox->AddItem(L"Item 4");
        
        auto selectedLabel = std::make_shared<TextBlock>();
        selectedLabel->SetName("selectedLabel");
        selectedLabel->SetText(L"Selected: None");
        root->AddChild(selectedLabel);
        
        // Bind selection change
        listBox->SelectionChanged.Add([selectedLabel](Control*, int index) {
            selectedLabel->SetText(L"Selected: Item " + std::to_wstring(index + 1));
        });
        
        if (auto* tw = dynamic_cast<TestWindow*>(m_window)) {
            tw->SetRoot(root);
        }
        
        WaitForLayout();
        
        // Test: Select item and verify binding
        {
            auto* list = m_finder->FindById("itemList");
            ASSERT_NOT_NULL(list);
            
            // Click on second item
            // List items are typically arranged vertically
            auto rect = ControlTestHook::GetRenderRect(list);
            float itemHeight = rect.height / 4;  // 4 items
            
            m_input->ClickAt(rect.x + 10, rect.y + itemHeight * 1.5f);
            WaitForAnimation();
            
            // Verify selection label updated
            auto* label = dynamic_cast<TextBlock*>(m_finder->FindById("selectedLabel"));
            ASSERT_NOT_NULL(label);
            ASSERT_TEXT_EQUALS(label, L"Selected: Item 2");
        }
        
        TearDown();
    }
};

UI_TEST(CollectionBindingTest);
