// LayoutTest.cpp - Automated tests for layout system

#include "../UIAutomationFramework.h"
#include "../TestHooks.h"
#include "TextBlock.h"
#include "Panel.h"
#include "layouts/Grid.h"
#include "layouts/Canvas.h"
#include "layouts/DockPanel.h"
#include "Window.h"
#include <memory>

using namespace luaui;
using namespace luaui::controls;
using namespace luaui::test::automation;

// ============================================================================
// Test: StackPanel Layout
// ============================================================================
class StackPanelLayoutTest : public UITestScenario {
public:
    StackPanelLayoutTest() : UITestScenario("StackPanelLayoutTest") {}
    
protected:
    Window* CreateTestWindow() override {
        return new TestWindow();
    }
    
    void Run() override {
        SetUp();
        
        // Vertical StackPanel
        auto root = std::make_shared<StackPanel>();
        root->SetOrientation(StackPanel::Orientation::Vertical);
        root->SetSpacing(10);
        
        auto item1 = std::make_shared<TextBlock>();
        item1->SetName("item1");
        item1->SetText(L"Item 1");
        if (auto* l = item1->GetLayout()) {
            l->SetHeight(30);
        }
        root->AddChild(item1);
        
        auto item2 = std::make_shared<TextBlock>();
        item2->SetName("item2");
        item2->SetText(L"Item 2");
        if (auto* l = item2->GetLayout()) {
            l->SetHeight(30);
        }
        root->AddChild(item2);
        
        auto item3 = std::make_shared<TextBlock>();
        item3->SetName("item3");
        item3->SetText(L"Item 3");
        if (auto* l = item3->GetLayout()) {
            l->SetHeight(30);
        }
        root->AddChild(item3);
        
        if (auto* tw = dynamic_cast<TestWindow*>(m_window)) {
            tw->SetRoot(root);
        }
        
        WaitForLayout();
        
        // Test: Items should be stacked vertically
        auto* i1 = m_finder->FindById("item1");
        auto* i2 = m_finder->FindById("item2");
        auto* i3 = m_finder->FindById("item3");
        
        ASSERT_NOT_NULL(i1);
        ASSERT_NOT_NULL(i2);
        ASSERT_NOT_NULL(i3);
        
        auto rect1 = ControlTestHook::GetRenderRect(i1);
        auto rect2 = ControlTestHook::GetRenderRect(i2);
        auto rect3 = ControlTestHook::GetRenderRect(i3);
        
        // item2 should be below item1
        ASSERT_GT(rect2.y, rect1.y);
        
        // item3 should be below item2
        ASSERT_GT(rect3.y, rect2.y);
        
        // With 10px spacing and 30px height
        // Expected: item2.y = item1.y + 30 + 10 = item1.y + 40
        float expectedY2 = rect1.y + 30 + 10;
        ASSERT_NEAR(rect2.y, expectedY2, 1.0f);
        
        TearDown();
    }
};

UI_TEST(StackPanelLayoutTest);

// ============================================================================
// Test: Grid Layout
// ============================================================================
class GridLayoutTest : public UITestScenario {
public:
    GridLayoutTest() : UITestScenario("GridLayoutTest") {}
    
protected:
    Window* CreateTestWindow() override {
        return new TestWindow();
    }
    
    void Run() override {
        SetUp();
        
        auto grid = std::make_shared<Grid>();
        
        // Define 2x2 grid
        grid->AddColumn(GridLength(1, GridUnitType::Star));
        grid->AddColumn(GridLength(1, GridUnitType::Star));
        grid->AddRow(GridLength(1, GridUnitType::Star));
        grid->AddRow(GridLength(1, GridUnitType::Star));
        
        // Top-left cell
        auto cell1 = std::make_shared<TextBlock>();
        cell1->SetName("cell1");
        cell1->SetText(L"TL");
        Grid::SetColumn(cell1, 0);
        Grid::SetRow(cell1, 0);
        grid->AddChild(cell1);
        
        // Top-right cell
        auto cell2 = std::make_shared<TextBlock>();
        cell2->SetName("cell2");
        cell2->SetText(L"TR");
        Grid::SetColumn(cell2, 1);
        Grid::SetRow(cell2, 0);
        grid->AddChild(cell2);
        
        // Bottom-left cell
        auto cell3 = std::make_shared<TextBlock>();
        cell3->SetName("cell3");
        cell3->SetText(L"BL");
        Grid::SetColumn(cell3, 0);
        Grid::SetRow(cell3, 1);
        grid->AddChild(cell3);
        
        // Bottom-right cell
        auto cell4 = std::make_shared<TextBlock>();
        cell4->SetName("cell4");
        cell4->SetText(L"BR");
        Grid::SetColumn(cell4, 1);
        Grid::SetRow(cell4, 1);
        grid->AddChild(cell4);
        
        if (auto* tw = dynamic_cast<TestWindow*>(m_window)) {
            tw->SetRoot(grid);
        }
        
        WaitForLayout();
        
        // Test: Verify cell positions
        auto* tl = m_finder->FindById("cell1");
        auto* tr = m_finder->FindById("cell2");
        auto* bl = m_finder->FindById("cell3");
        auto* br = m_finder->FindById("cell4");
        
        ASSERT_NOT_NULL(tl);
        ASSERT_NOT_NULL(tr);
        ASSERT_NOT_NULL(bl);
        ASSERT_NOT_NULL(br);
        
        auto rectTL = ControlTestHook::GetRenderRect(tl);
        auto rectTR = ControlTestHook::GetRenderRect(tr);
        auto rectBL = ControlTestHook::GetRenderRect(bl);
        auto rectBR = ControlTestHook::GetRenderRect(br);
        
        // TR should be to the right of TL
        ASSERT_GT(rectTR.x, rectTL.x);
        
        // BL should be below TL
        ASSERT_GT(rectBL.y, rectTL.y);
        
        // BR should be to the right of BL
        ASSERT_GT(rectBR.x, rectBL.x);
        
        // BR should be below TR
        ASSERT_GT(rectBR.y, rectTR.y);
        
        // Verify column alignment
        ASSERT_NEAR(rectTL.x, rectBL.x, 1.0f);  // Same column
        ASSERT_NEAR(rectTR.x, rectBR.x, 1.0f);  // Same column
        
        // Verify row alignment
        ASSERT_NEAR(rectTL.y, rectTR.y, 1.0f);  // Same row
        ASSERT_NEAR(rectBL.y, rectBR.y, 1.0f);  // Same row
        
        TearDown();
    }
};

UI_TEST(GridLayoutTest);

// ============================================================================
// Test: Canvas Absolute Positioning
// ============================================================================
class CanvasLayoutTest : public UITestScenario {
public:
    CanvasLayoutTest() : UITestScenario("CanvasLayoutTest") {}
    
protected:
    Window* CreateTestWindow() override {
        return new TestWindow();
    }
    
    void Run() override {
        SetUp();
        
        auto canvas = std::make_shared<Canvas>();
        
        auto item1 = std::make_shared<TextBlock>();
        item1->SetName("item1");
        item1->SetText(L"At (10, 20)");
        Canvas::SetLeft(item1, 10);
        Canvas::SetTop(item1, 20);
        canvas->AddChild(item1);
        
        auto item2 = std::make_shared<TextBlock>();
        item2->SetName("item2");
        item2->SetText(L"At (100, 50)");
        Canvas::SetLeft(item2, 100);
        Canvas::SetTop(item2, 50);
        canvas->AddChild(item2);
        
        auto item3 = std::make_shared<TextBlock>();
        item3->SetName("item3");
        item3->SetText(L"At (50, 150)");
        Canvas::SetLeft(item3, 50);
        Canvas::SetTop(item3, 150);
        canvas->AddChild(item3);
        
        if (auto* tw = dynamic_cast<TestWindow*>(m_window)) {
            tw->SetRoot(canvas);
        }
        
        WaitForLayout();
        
        // Test: Verify absolute positions
        auto* i1 = m_finder->FindById("item1");
        auto* i2 = m_finder->FindById("item2");
        auto* i3 = m_finder->FindById("item3");
        
        ASSERT_NOT_NULL(i1);
        ASSERT_NOT_NULL(i2);
        ASSERT_NOT_NULL(i3);
        
        auto rect1 = ControlTestHook::GetRenderRect(i1);
        auto rect2 = ControlTestHook::GetRenderRect(i2);
        auto rect3 = ControlTestHook::GetRenderRect(i3);
        
        // Verify positions (with some tolerance)
        ASSERT_NEAR(rect1.x, 10.0f, 2.0f);
        ASSERT_NEAR(rect1.y, 20.0f, 2.0f);
        
        ASSERT_NEAR(rect2.x, 100.0f, 2.0f);
        ASSERT_NEAR(rect2.y, 50.0f, 2.0f);
        
        ASSERT_NEAR(rect3.x, 50.0f, 2.0f);
        ASSERT_NEAR(rect3.y, 150.0f, 2.0f);
        
        TearDown();
    }
};

UI_TEST(CanvasLayoutTest);

// ============================================================================
// Test: Layout Performance
// ============================================================================
class LayoutPerformanceTest : public UITestScenario {
public:
    LayoutPerformanceTest() : UITestScenario("LayoutPerformanceTest") {}
    
protected:
    Window* CreateTestWindow() override {
        return new TestWindow();
    }
    
    void Run() override {
        SetUp();
        
        // Create a complex layout with many nested panels
        auto root = std::make_shared<StackPanel>();
        
        for (int i = 0; i < 10; i++) {
            auto row = std::make_shared<StackPanel>();
            row->SetOrientation(StackPanel::Orientation::Horizontal);
            
            for (int j = 0; j < 10; j++) {
                auto item = std::make_shared<TextBlock>();
                item->SetText(L"Item");
                row->AddChild(item);
            }
            
            root->AddChild(row);
        }
        
        if (auto* tw = dynamic_cast<TestWindow*>(m_window)) {
            tw->SetRoot(root);
        }
        
        // Measure layout performance
        auto start = std::chrono::high_resolution_clock::now();
        
        WaitForLayout();
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        double ms = duration.count() / 1000.0;
        
        // Assert layout completes within reasonable time
        // 100 controls should layout in < 100ms
        std::cout << "Layout time for 100 controls: " << ms << "ms" << std::endl;
        ASSERT_LT(ms, 100.0);
        
        // Use LayoutTester for more detailed metrics
        auto perf = LayoutTester::MeasurePerformance(m_window);
        
        std::cout << "Measure time: " << perf.measureTimeMs << "ms" << std::endl;
        std::cout << "Arrange time: " << perf.arrangeTimeMs << "ms" << std::endl;
        std::cout << "Control count: " << perf.controlCount << std::endl;
        
        // Performance assertions
        ASSERT_LT(perf.measureTimeMs + perf.arrangeTimeMs, 100.0);
        ASSERT_EQ(perf.controlCount, 100);  // 10 rows x 10 items
        
        TearDown();
    }
};

UI_TEST(LayoutPerformanceTest);

// ============================================================================
// Test: Responsive Layout (Window Resize)
// ============================================================================
class ResponsiveLayoutTest : public UITestScenario {
public:
    ResponsiveLayoutTest() : UITestScenario("ResponsiveLayoutTest") {}
    
protected:
    Window* CreateTestWindow() override {
        return new TestWindow();
    }
    
    void Run() override {
        SetUp();
        
        auto grid = std::make_shared<Grid>();
        grid->AddColumn(GridLength(1, GridUnitType::Star));
        grid->AddColumn(GridLength(2, GridUnitType::Star));  // Twice as wide
        grid->AddRow(GridLength(1, GridUnitType::Star));
        
        auto left = std::make_shared<TextBlock>();
        left->SetName("left");
        left->SetText(L"Left");
        Grid::SetColumn(left, 0);
        grid->AddChild(left);
        
        auto right = std::make_shared<TextBlock>();
        right->SetName("right");
        right->SetText(L"Right");
        Grid::SetColumn(right, 1);
        grid->AddChild(right);
        
        if (auto* tw = dynamic_cast<TestWindow*>(m_window)) {
            tw->SetRoot(grid);
        }
        
        WaitForLayout();
        
        // Initial size check
        auto* leftCtrl = m_finder->FindById("left");
        auto* rightCtrl = m_finder->FindById("right");
        
        auto leftRect = ControlTestHook::GetRenderRect(leftCtrl);
        auto rightRect = ControlTestHook::GetRenderRect(rightCtrl);
        
        // Right column should be twice as wide
        float ratio = rightRect.width / leftRect.width;
        ASSERT_NEAR(ratio, 2.0f, 0.1f);
        
        // Resize window
        if (auto* tw = dynamic_cast<TestWindow*>(m_window)) {
            tw->SetSizeForTest(1200, 600);
        }
        
        WaitForLayout();
        
        // Verify ratio is maintained after resize
        leftRect = ControlTestHook::GetRenderRect(leftCtrl);
        rightRect = ControlTestHook::GetRenderRect(rightCtrl);
        
        ratio = rightRect.width / leftRect.width;
        ASSERT_NEAR(ratio, 2.0f, 0.1f);
        
        TearDown();
    }
};

UI_TEST(ResponsiveLayoutTest);
