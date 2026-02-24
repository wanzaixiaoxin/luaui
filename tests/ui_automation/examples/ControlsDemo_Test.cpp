// ControlsDemo_Test.cpp - Automated tests for 01_controls_demo
// Tests the Controls Showcase application

#include "../UIAutomationFramework.h"
#include "../TestHooks.h"
#include "Button.h"
#include "TextBlock.h"
#include "TextBox.h"
#include "CheckBox.h"
#include "Slider.h"
#include "ProgressBar.h"
#include "ListBox.h"
#include "TabControl.h"
#include "Panel.h"
#include "Border.h"
#include "Window.h"
#include <memory>
#include <vector>

using namespace luaui;
using namespace luaui::controls;
using namespace luaui::test::automation;

// ============================================================================
// Test Helper: Replicate ControlsShowcaseWindow for testing
// ============================================================================
class TestableControlsShowcase : public TestWindow {
public:
    std::shared_ptr<TextBlock> m_statusText;
    std::shared_ptr<ProgressBar> m_progressBar;
    std::shared_ptr<Slider> m_slider;
    std::shared_ptr<TextBlock> m_sliderValue;
    std::shared_ptr<ListBox> m_listBox;
    std::shared_ptr<TabControl> m_mainTabs;
    
    void OnLoaded() override {
        // Create the same UI as ControlsShowcaseWindow
        auto root = std::make_shared<StackPanel>();
        root->SetOrientation(StackPanel::Orientation::Vertical);
        root->SetName("rootPanel");
        
        // Title
        auto title = std::make_shared<TextBlock>();
        title->SetName("windowTitle");
        title->SetText(L"LuaUI Controls Showcase");
        title->SetFontSize(28);
        root->AddChild(title);
        
        // Main TabControl
        m_mainTabs = std::make_shared<TabControl>();
        m_mainTabs->SetName("mainTabs");
        m_mainTabs->SetTabHeight(36);
        
        // Basic Tab
        auto tabBasic = std::make_shared<TabItem>();
        tabBasic->SetHeader(L"  Basic  ");
        tabBasic->SetName("tabBasic");
        tabBasic->SetContent(CreateBasicTab());
        m_mainTabs->AddTab(tabBasic);
        
        // Input Tab
        auto tabInput = std::make_shared<TabItem>();
        tabInput->SetHeader(L"  Input  ");
        tabInput->SetName("tabInput");
        tabInput->SetContent(CreateInputTab());
        m_mainTabs->AddTab(tabInput);
        
        // Selection Tab
        auto tabSelect = std::make_shared<TabItem>();
        tabSelect->SetHeader(L" Selection ");
        tabSelect->SetName("tabSelect");
        tabSelect->SetContent(CreateSelectionTab());
        m_mainTabs->AddTab(tabSelect);
        
        // Data Tab
        auto tabData = std::make_shared<TabItem>();
        tabData->SetHeader(L"  Data   ");
        tabData->SetName("tabData");
        tabData->SetContent(CreateDataTab());
        m_mainTabs->AddTab(tabData);
        
        // Graphics Tab
        auto tabGraphics = std::make_shared<TabItem>();
        tabGraphics->SetHeader(L" Graphics ");
        tabGraphics->SetName("tabGraphics");
        tabGraphics->SetContent(CreateGraphicsTab());
        m_mainTabs->AddTab(tabGraphics);
        
        if (auto* l = m_mainTabs->GetLayout()) {
            l->SetHeight(460);
        }
        root->AddChild(m_mainTabs);
        
        // Status Panel
        auto statusPanel = std::make_shared<StackPanel>();
        statusPanel->SetOrientation(StackPanel::Orientation::Vertical);
        statusPanel->SetName("statusPanel");
        
        auto statusLabel = std::make_shared<TextBlock>();
        statusLabel->SetText(L"Status");
        statusLabel->SetName("statusLabel");
        statusPanel->AddChild(statusLabel);
        
        m_statusText = std::make_shared<TextBlock>();
        m_statusText->SetName("statusText");
        m_statusText->SetText(L"Ready");
        statusPanel->AddChild(m_statusText);
        
        root->AddChild(statusPanel);
        
        SetRoot(root);
    }
    
private:
    std::shared_ptr<Panel> CreateBasicTab() {
        auto root = std::make_shared<StackPanel>();
        root->SetName("basicTabContent");
        root->SetOrientation(StackPanel::Orientation::Vertical);
        
        // Button row
        auto btnRow = std::make_shared<StackPanel>();
        btnRow->SetOrientation(StackPanel::Orientation::Horizontal);
        btnRow->SetName("buttonRow");
        
        auto btn1 = std::make_shared<Button>();
        btn1->SetName("defaultBtn");
        btn1->SetText(L"Default");
        btn1->Click.Add([this](Control*) { m_statusText->SetText(L"Default button clicked"); });
        btnRow->AddChild(btn1);
        
        auto btn2 = std::make_shared<Button>();
        btn2->SetName("primaryBtn");
        btn2->SetText(L"Primary");
        btn2->SetStateColors(
            Color::FromHex(0x2196F3),
            Color::FromHex(0x1976D2),
            Color::FromHex(0x0D47A1)
        );
        btn2->Click.Add([this](Control*) { m_statusText->SetText(L"Primary button clicked"); });
        btnRow->AddChild(btn2);
        
        auto btn3 = std::make_shared<Button>();
        btn3->SetName("successBtn");
        btn3->SetText(L"Success");
        btn3->SetStateColors(
            Color::FromHex(0x4CAF50),
            Color::FromHex(0x388E3C),
            Color::FromHex(0x1B5E20)
        );
        btn3->Click.Add([this](auto*) { m_statusText->SetText(L"Success button clicked"); });
        btnRow->AddChild(btn3);
        
        auto btn4 = std::make_shared<Button>();
        btn4->SetName("dangerBtn");
        btn4->SetText(L"Danger");
        btn4->SetStateColors(
            Color::FromHex(0xF44336),
            Color::FromHex(0xD32F2F),
            Color::FromHex(0xB71C1C)
        );
        btn4->Click.Add([this](auto*) { m_statusText->SetText(L"Danger button clicked"); });
        btnRow->AddChild(btn4);
        
        root->AddChild(btnRow);
        
        // Typography section
        auto normalText = std::make_shared<TextBlock>();
        normalText->SetName("normalText");
        normalText->SetText(L"Normal text (Regular)");
        root->AddChild(normalText);
        
        auto boldText = std::make_shared<TextBlock>();
        boldText->SetName("boldText");
        boldText->SetText(L"Bold text (SemiBold)");
        boldText->SetFontWeight(FontWeight::SemiBold);
        root->AddChild(boldText);
        
        auto italicText = std::make_shared<TextBlock>();
        italicText->SetName("italicText");
        italicText->SetText(L"Italic text (Oblique)");
        italicText->SetFontStyle(FontStyle::Italic);
        root->AddChild(italicText);
        
        return root;
    }
    
    std::shared_ptr<Panel> CreateInputTab() {
        auto root = std::make_shared<StackPanel>();
        root->SetName("inputTabContent");
        root->SetOrientation(StackPanel::Orientation::Vertical);
        
        // Username
        auto usernameLabel = std::make_shared<TextBlock>();
        usernameLabel->SetName("usernameLabel");
        usernameLabel->SetText(L"Username:");
        root->AddChild(usernameLabel);
        
        auto usernameInput = std::make_shared<TextBox>();
        usernameInput->SetName("usernameInput");
        if (auto* l = usernameInput->GetLayout()) {
            l->SetWidth(350);
            l->SetHeight(32);
        }
        root->AddChild(usernameInput);
        
        // Description
        auto descLabel = std::make_shared<TextBlock>();
        descLabel->SetName("descLabel");
        descLabel->SetText(L"Description:");
        root->AddChild(descLabel);
        
        auto descInput = std::make_shared<TextBox>();
        descInput->SetName("descInput");
        if (auto* l = descInput->GetLayout()) {
            l->SetWidth(350);
            l->SetHeight(60);
        }
        root->AddChild(descInput);
        
        // Slider
        m_sliderValue = std::make_shared<TextBlock>();
        m_sliderValue->SetName("sliderValue");
        m_sliderValue->SetText(L"50%");
        root->AddChild(m_sliderValue);
        
        m_slider = std::make_shared<Slider>();
        m_slider->SetName("volumeSlider");
        m_slider->SetValue(50);
        m_slider->SetMinimum(0);
        m_slider->SetMaximum(100);
        if (auto* l = m_slider->GetLayout()) {
            l->SetWidth(400);
        }
        m_slider->ValueChanged.Add([this](Control*, double v) {
            m_sliderValue->SetText(std::to_wstring((int)v) + L"%");
            if (m_progressBar) m_progressBar->SetValue(v);
        });
        root->AddChild(m_slider);
        
        // ProgressBar
        auto progressLabel = std::make_shared<TextBlock>();
        progressLabel->SetName("progressLabel");
        progressLabel->SetText(L"Download Progress:");
        root->AddChild(progressLabel);
        
        m_progressBar = std::make_shared<ProgressBar>();
        m_progressBar->SetName("downloadProgress");
        m_progressBar->SetValue(50);
        if (auto* l = m_progressBar->GetLayout()) {
            l->SetWidth(400);
            l->SetHeight(8);
        }
        root->AddChild(m_progressBar);
        
        return root;
    }
    
    std::shared_ptr<Panel> CreateSelectionTab() {
        auto root = std::make_shared<StackPanel>();
        root->SetName("selectionTabContent");
        root->SetOrientation(StackPanel::Orientation::Vertical);
        
        // CheckBoxes
        auto chk1 = std::make_shared<CheckBox>();
        chk1->SetName("notifyCheck");
        chk1->SetText(L"Enable notifications");
        chk1->SetIsChecked(true);
        chk1->CheckedChanged.Add([this](Control*, bool c) {
            m_statusText->SetText(c ? L"Notifications enabled" : L"Notifications disabled");
        });
        root->AddChild(chk1);
        
        auto chk2 = std::make_shared<CheckBox>();
        chk2->SetName("autosaveCheck");
        chk2->SetText(L"Auto-save changes");
        chk2->CheckedChanged.Add([this](auto*, bool c) {
            m_statusText->SetText(c ? L"Auto-save enabled" : L"Auto-save disabled");
        });
        root->AddChild(chk2);
        
        auto chk3 = std::make_shared<CheckBox>();
        chk3->SetName("darkModeCheck");
        chk3->SetText(L"Dark mode (experimental)");
        chk3->CheckedChanged.Add([this](auto*, bool c) {
            m_statusText->SetText(c ? L"Dark mode selected" : L"Light mode selected");
        });
        root->AddChild(chk3);
        
        // ListBox
        auto listLabel = std::make_shared<TextBlock>();
        listLabel->SetName("listLabel");
        listLabel->SetText(L"Operating Systems:");
        root->AddChild(listLabel);
        
        m_listBox = std::make_shared<ListBox>();
        m_listBox->SetName("osListBox");
        m_listBox->AddItem(L"Windows 11");
        m_listBox->AddItem(L"Windows 10");
        m_listBox->AddItem(L"Ubuntu 22.04 LTS");
        m_listBox->AddItem(L"macOS Sonoma");
        m_listBox->AddItem(L"Fedora 39");
        m_listBox->AddItem(L"Debian 12");
        if (auto* l = m_listBox->GetLayout()) {
            l->SetWidth(350);
            l->SetHeight(140);
        }
        root->AddChild(m_listBox);
        
        return root;
    }
    
    std::shared_ptr<Panel> CreateDataTab() {
        auto root = std::make_shared<StackPanel>();
        root->SetName("dataTabContent");
        root->SetOrientation(StackPanel::Orientation::Vertical);
        
        auto dataLabel = std::make_shared<TextBlock>();
        dataLabel->SetName("dataLabel");
        dataLabel->SetText(L"Data Tab - DataGrid Demo");
        root->AddChild(dataLabel);
        
        return root;
    }
    
    std::shared_ptr<Panel> CreateGraphicsTab() {
        auto root = std::make_shared<StackPanel>();
        root->SetName("graphicsTabContent");
        root->SetOrientation(StackPanel::Orientation::Vertical);
        
        auto graphicsLabel = std::make_shared<TextBlock>();
        graphicsLabel->SetName("graphicsLabel");
        graphicsLabel->SetText(L"Graphics Tab - Shapes Demo");
        root->AddChild(graphicsLabel);
        
        return root;
    }
};

// ============================================================================
// Test: Window Initialization
// ============================================================================
class ControlsDemo_InitTest : public UITestScenario {
public:
    ControlsDemo_InitTest() : UITestScenario("ControlsDemo_InitTest") {}
    
protected:
    Window* CreateTestWindow() override {
        return new TestableControlsShowcase();
    }
    
    void Run() override {
        SetUp();
        
        // Test 1: Verify window loaded correctly
        {
            auto* title = m_finder->FindById("windowTitle");
            ASSERT_NOT_NULL(title);
            ASSERT_VISIBLE(title);
            ASSERT_TEXT_EQUALS(dynamic_cast<TextBlock*>(title), L"LuaUI Controls Showcase");
        }
        
        // Test 2: Verify TabControl exists with all tabs
        {
            auto* tabs = m_finder->FindById("mainTabs");
            ASSERT_NOT_NULL(tabs);
            ASSERT_VISIBLE(tabs);
        }
        
        // Test 3: Verify status panel exists
        {
            auto* status = m_finder->FindById("statusText");
            ASSERT_NOT_NULL(status);
            ASSERT_TEXT_EQUALS(dynamic_cast<TextBlock*>(status), L"Ready");
        }
        
        // Test 4: Basic tab content visible by default
        {
            auto* basicContent = m_finder->FindById("basicTabContent");
            ASSERT_NOT_NULL(basicContent);
            ASSERT_VISIBLE(basicContent);
        }
        
        TakeScreenshot("controls_demo_initial_state");
        
        TearDown();
    }
};

UI_TEST(ControlsDemo_InitTest);

// ============================================================================
// Test: Button Interactions
// ============================================================================
class ControlsDemo_ButtonTest : public UITestScenario {
public:
    ControlsDemo_ButtonTest() : UITestScenario("ControlsDemo_ButtonTest") {}
    
protected:
    Window* CreateTestWindow() override {
        return new TestableControlsShowcase();
    }
    
    void Run() override {
        SetUp();
        WaitForLayout();
        
        // Test 1: Click Default button
        {
            auto* defaultBtn = m_finder->FindById("defaultBtn");
            ASSERT_NOT_NULL(defaultBtn);
            
            m_input->ClickOn(defaultBtn);
            WaitForAnimation();
            
            auto* status = dynamic_cast<TextBlock*>(m_finder->FindById("statusText"));
            ASSERT_TEXT_EQUALS(status, L"Default button clicked");
        }
        
        // Test 2: Click Primary button
        {
            auto* primaryBtn = m_finder->FindById("primaryBtn");
            ASSERT_NOT_NULL(primaryBtn);
            
            m_input->ClickOn(primaryBtn);
            WaitForAnimation();
            
            auto* status = dynamic_cast<TextBlock*>(m_finder->FindById("statusText"));
            ASSERT_TEXT_EQUALS(status, L"Primary button clicked");
        }
        
        // Test 3: Click Success button
        {
            auto* successBtn = m_finder->FindById("successBtn");
            ASSERT_NOT_NULL(successBtn);
            
            m_input->ClickOn(successBtn);
            WaitForAnimation();
            
            auto* status = dynamic_cast<TextBlock*>(m_finder->FindById("statusText"));
            ASSERT_TEXT_EQUALS(status, L"Success button clicked");
        }
        
        // Test 4: Click Danger button
        {
            auto* dangerBtn = m_finder->FindById("dangerBtn");
            ASSERT_NOT_NULL(dangerBtn);
            
            m_input->ClickOn(dangerBtn);
            WaitForAnimation();
            
            auto* status = dynamic_cast<TextBlock*>(m_finder->FindById("statusText"));
            ASSERT_TEXT_EQUALS(status, L"Danger button clicked");
        }
        
        TearDown();
    }
};

UI_TEST(ControlsDemo_ButtonTest);

// ============================================================================
// Test: Tab Navigation
// ============================================================================
class ControlsDemo_TabNavigationTest : public UITestScenario {
public:
    ControlsDemo_TabNavigationTest() : UITestScenario("ControlsDemo_TabNavigationTest") {}
    
protected:
    Window* CreateTestWindow() override {
        return new TestableControlsShowcase();
    }
    
    void Run() override {
        SetUp();
        WaitForLayout();
        
        // Initially on Basic tab
        auto* basicContent = m_finder->FindById("basicTabContent");
        ASSERT_VISIBLE(basicContent);
        
        // Test 1: Navigate to Input tab
        {
            // Find Input tab header and click
            auto* tabs = dynamic_cast<TabControl*>(m_finder->FindById("mainTabs"));
            ASSERT_NOT_NULL(tabs);
            
            // Switch to Input tab (index 1)
            tabs->SetSelectedIndex(1);
            WaitForAnimation();
            
            auto* inputContent = m_finder->FindById("inputTabContent");
            ASSERT_NOT_NULL(inputContent);
            ASSERT_VISIBLE(inputContent);
            
            // Verify Input tab controls
            auto* usernameInput = m_finder->FindById("usernameInput");
            ASSERT_NOT_NULL(usernameInput);
        }
        
        // Test 2: Navigate to Selection tab
        {
            auto* tabs = dynamic_cast<TabControl*>(m_finder->FindById("mainTabs"));
            tabs->SetSelectedIndex(2);
            WaitForAnimation();
            
            auto* selectionContent = m_finder->FindById("selectionTabContent");
            ASSERT_VISIBLE(selectionContent);
            
            // Verify CheckBoxes exist
            auto* notifyCheck = m_finder->FindById("notifyCheck");
            ASSERT_NOT_NULL(notifyCheck);
        }
        
        // Test 3: Navigate back to Basic tab
        {
            auto* tabs = dynamic_cast<TabControl*>(m_finder->FindById("mainTabs"));
            tabs->SetSelectedIndex(0);
            WaitForAnimation();
            
            auto* basicContent = m_finder->FindById("basicTabContent");
            ASSERT_VISIBLE(basicContent);
        }
        
        TearDown();
    }
};

UI_TEST(ControlsDemo_TabNavigationTest);

// ============================================================================
// Test: Slider and ProgressBar
// ============================================================================
class ControlsDemo_SliderTest : public UITestScenario {
public:
    ControlsDemo_SliderTest() : UITestScenario("ControlsDemo_SliderTest") {}
    
protected:
    Window* CreateTestWindow() override {
        return new TestableControlsShowcase();
    }
    
    void Run() override {
        SetUp();
        WaitForLayout();
        
        // Navigate to Input tab
        auto* tabs = dynamic_cast<TabControl*>(m_finder->FindById("mainTabs"));
        tabs->SetSelectedIndex(1);
        WaitForAnimation();
        
        // Test 1: Verify initial values
        {
            auto* slider = dynamic_cast<Slider*>(m_finder->FindById("volumeSlider"));
            auto* progress = dynamic_cast<ProgressBar*>(m_finder->FindById("downloadProgress"));
            auto* valueLabel = dynamic_cast<TextBlock*>(m_finder->FindById("sliderValue"));
            
            ASSERT_NOT_NULL(slider);
            ASSERT_NOT_NULL(progress);
            ASSERT_NOT_NULL(valueLabel);
            
            ASSERT_NEAR(slider->GetValue(), 50.0, 0.1);
            ASSERT_NEAR(progress->GetValue(), 50.0, 0.1);
            ASSERT_TEXT_EQUALS(valueLabel, L"50%");
        }
        
        // Test 2: Drag slider to change value
        {
            auto* slider = dynamic_cast<Slider*>(m_finder->FindById("volumeSlider"));
            auto* sliderControl = m_finder->FindById("volumeSlider");
            auto rect = ControlTestHook::GetRenderRect(sliderControl);
            
            // Click at 75% position of slider
            float targetX = rect.x + rect.width * 0.75f;
            float targetY = rect.y + rect.height / 2;
            
            m_input->ClickAt(targetX, targetY);
            WaitForAnimation();
            
            // Verify slider value increased
            ASSERT_GT(slider->GetValue(), 60.0);
            
            // Verify label updated
            auto* valueLabel = dynamic_cast<TextBlock*>(m_finder->FindById("sliderValue"));
            int displayedValue = std::stoi(valueLabel->GetText());
            ASSERT_GT(displayedValue, 60);
            
            // Verify ProgressBar synced
            auto* progress = dynamic_cast<ProgressBar*>(m_finder->FindById("downloadProgress"));
            ASSERT_NEAR(progress->GetValue(), slider->GetValue(), 1.0);
        }
        
        // Test 3: Drag to minimum
        {
            auto* slider = dynamic_cast<Slider*>(m_finder->FindById("volumeSlider"));
            auto* sliderControl = m_finder->FindById("volumeSlider");
            auto rect = ControlTestHook::GetRenderRect(sliderControl);
            
            m_input->ClickAt(rect.x + 5, rect.y + rect.height / 2);
            WaitForAnimation();
            
            ASSERT_NEAR(slider->GetValue(), 0.0, 5.0);
        }
        
        // Test 4: Drag to maximum
        {
            auto* slider = dynamic_cast<Slider*>(m_finder->FindById("volumeSlider"));
            auto* sliderControl = m_finder->FindById("volumeSlider");
            auto rect = ControlTestHook::GetRenderRect(sliderControl);
            
            m_input->ClickAt(rect.x + rect.width - 5, rect.y + rect.height / 2);
            WaitForAnimation();
            
            ASSERT_NEAR(slider->GetValue(), 100.0, 5.0);
        }
        
        TearDown();
    }
};

UI_TEST(ControlsDemo_SliderTest);

// ============================================================================
// Test: CheckBox Interactions
// ============================================================================
class ControlsDemo_CheckBoxTest : public UITestScenario {
public:
    ControlsDemo_CheckBoxTest() : UITestScenario("ControlsDemo_CheckBoxTest") {}
    
protected:
    Window* CreateTestWindow() override {
        return new TestableControlsShowcase();
    }
    
    void Run() override {
        SetUp();
        WaitForLayout();
        
        // Navigate to Selection tab
        auto* tabs = dynamic_cast<TabControl*>(m_finder->FindById("mainTabs"));
        tabs->SetSelectedIndex(2);
        WaitForAnimation();
        
        // Test 1: Verify initial state (notifications enabled by default)
        {
            auto* notifyCheck = dynamic_cast<CheckBox*>(m_finder->FindById("notifyCheck"));
            ASSERT_NOT_NULL(notifyCheck);
            ASSERT_TRUE(notifyCheck->GetIsChecked());
        }
        
        // Test 2: Uncheck notifications
        {
            auto* notifyCheck = m_finder->FindById("notifyCheck");
            m_input->ClickOn(notifyCheck);
            WaitForAnimation();
            
            auto* status = dynamic_cast<TextBlock*>(m_finder->FindById("statusText"));
            ASSERT_TEXT_EQUALS(status, L"Notifications disabled");
        }
        
        // Test 3: Check notifications again
        {
            auto* notifyCheck = m_finder->FindById("notifyCheck");
            m_input->ClickOn(notifyCheck);
            WaitForAnimation();
            
            auto* status = dynamic_cast<TextBlock*>(m_finder->FindById("statusText"));
            ASSERT_TEXT_EQUALS(status, L"Notifications enabled");
        }
        
        // Test 4: Toggle auto-save
        {
            auto* autosaveCheck = m_finder->FindById("autosaveCheck");
            ASSERT_NOT_NULL(autosaveCheck);
            
            auto* checkBox = dynamic_cast<CheckBox*>(autosaveCheck);
            ASSERT_FALSE(checkBox->GetIsChecked());  // Initially unchecked
            
            m_input->ClickOn(autosaveCheck);
            WaitForAnimation();
            
            auto* status = dynamic_cast<TextBlock*>(m_finder->FindById("statusText"));
            ASSERT_TEXT_EQUALS(status, L"Auto-save enabled");
        }
        
        // Test 5: Toggle dark mode
        {
            auto* darkModeCheck = m_finder->FindById("darkModeCheck");
            ASSERT_NOT_NULL(darkModeCheck);
            
            m_input->ClickOn(darkModeCheck);
            WaitForAnimation();
            
            auto* status = dynamic_cast<TextBlock*>(m_finder->FindById("statusText"));
            ASSERT_TEXT_EQUALS(status, L"Dark mode selected");
        }
        
        TearDown();
    }
};

UI_TEST(ControlsDemo_CheckBoxTest);

// ============================================================================
// Test: TextBox Input
// ============================================================================
class ControlsDemo_TextBoxTest : public UITestScenario {
public:
    ControlsDemo_TextBoxTest() : UITestScenario("ControlsDemo_TextBoxTest") {}
    
protected:
    Window* CreateTestWindow() override {
        return new TestableControlsShowcase();
    }
    
    void Run() override {
        SetUp();
        WaitForLayout();
        
        // Navigate to Input tab
        auto* tabs = dynamic_cast<TabControl*>(m_finder->FindById("mainTabs"));
        tabs->SetSelectedIndex(1);
        WaitForAnimation();
        
        // Test 1: Type in username field
        {
            auto* usernameInput = m_finder->FindById("usernameInput");
            ASSERT_NOT_NULL(usernameInput);
            
            m_input->ClickOn(usernameInput);
            WaitForAnimation();
            
            m_input->SendText(L"TestUser123");
            WaitForAnimation();
            
            auto* textBox = dynamic_cast<TextBox*>(usernameInput);
            ASSERT_TEXT_EQUALS(textBox, L"TestUser123");
        }
        
        // Test 2: Type in description field
        {
            auto* descInput = m_finder->FindById("descInput");
            ASSERT_NOT_NULL(descInput);
            
            m_input->ClickOn(descInput);
            WaitForAnimation();
            
            m_input->SendText(L"This is a test description");
            WaitForAnimation();
            
            auto* textBox = dynamic_cast<TextBox*>(descInput);
            ASSERT_TEXT_EQUALS(textBox, L"This is a test description");
        }
        
        TearDown();
    }
};

UI_TEST(ControlsDemo_TextBoxTest);

// ============================================================================
// Test: Visual Regression - All Tabs
// ============================================================================
class ControlsDemo_VisualRegressionTest : public UITestScenario {
public:
    ControlsDemo_VisualRegressionTest() : UITestScenario("ControlsDemo_VisualRegressionTest") {}
    
protected:
    Window* CreateTestWindow() override {
        return new TestableControlsShowcase();
    }
    
    void Run() override {
        SetUp();
        WaitForLayout();
        
        auto* tabs = dynamic_cast<TabControl*>(m_finder->FindById("mainTabs"));
        
        // Capture each tab
        const char* tabNames[] = {"Basic", "Input", "Selection", "Data", "Graphics"};
        
        for (int i = 0; i < 5; i++) {
            tabs->SetSelectedIndex(i);
            WaitForAnimation();
            Wait(200);  // Extra wait for rendering
            
            std::string screenshotName = std::string("controls_demo_tab_") + tabNames[i];
            TakeScreenshot(screenshotName);
            
            // Verify content visible
            auto* content = m_finder->FindById(std::string(tabNames[i]) + "TabContent");
            ASSERT_VISIBLE(content);
        }
        
        TearDown();
    }
};

UI_TEST(ControlsDemo_VisualRegressionTest);

// ============================================================================
// Test: Layout Validation
// ============================================================================
class ControlsDemo_LayoutValidationTest : public UITestScenario {
public:
    ControlsDemo_LayoutValidationTest() : UITestScenario("ControlsDemo_LayoutValidationTest") {}
    
protected:
    Window* CreateTestWindow() override {
        return new TestableControlsShowcase();
    }
    
    void Run() override {
        SetUp();
        WaitForLayout();
        
        // Test 1: All buttons in Basic tab should be horizontally aligned
        {
            auto* btn1 = m_finder->FindById("defaultBtn");
            auto* btn2 = m_finder->FindById("primaryBtn");
            auto* btn3 = m_finder->FindById("successBtn");
            auto* btn4 = m_finder->FindById("dangerBtn");
            
            ASSERT_NOT_NULL(btn1);
            ASSERT_NOT_NULL(btn2);
            ASSERT_NOT_NULL(btn3);
            ASSERT_NOT_NULL(btn4);
            
            // Check horizontal arrangement
            auto rect1 = ControlTestHook::GetRenderRect(btn1);
            auto rect2 = ControlTestHook::GetRenderRect(btn2);
            auto rect3 = ControlTestHook::GetRenderRect(btn3);
            auto rect4 = ControlTestHook::GetRenderRect(btn4);
            
            // All buttons should be roughly same Y position (horizontal row)
            ASSERT_NEAR(rect1.y, rect2.y, 5.0f);
            ASSERT_NEAR(rect2.y, rect3.y, 5.0f);
            ASSERT_NEAR(rect3.y, rect4.y, 5.0f);
            
            // Each button should be to the right of previous
            ASSERT_GT(rect2.x, rect1.x);
            ASSERT_GT(rect3.x, rect2.x);
            ASSERT_GT(rect4.x, rect3.x);
        }
        
        // Test 2: Validate no layout violations
        {
            auto* root = m_finder->FindById("rootPanel");
            auto violations = LayoutTester::ValidateLayout(root);
            
            if (!violations.empty()) {
                for (const auto& v : violations) {
                    std::cout << "Layout violation: " << v.message << std::endl;
                }
                ASSERT_TRUE(false);
            }
        }
        
        // Test 3: Measure layout performance
        {
            auto perf = LayoutTester::MeasurePerformance(m_window);
            
            std::cout << "Layout performance:" << std::endl;
            std::cout << "  Measure time: " << perf.measureTimeMs << "ms" << std::endl;
            std::cout << "  Arrange time: " << perf.arrangeTimeMs << "ms" << std::endl;
            std::cout << "  Control count: " << perf.controlCount << std::endl;
            
            // Layout should complete within 16ms (60fps budget)
            ASSERT_LT(perf.measureTimeMs + perf.arrangeTimeMs, 16.0);
        }
        
        TearDown();
    }
};

UI_TEST(ControlsDemo_LayoutValidationTest);
