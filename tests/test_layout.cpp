#include <iostream>
#include <cassert>
#include <cmath>
#include <memory>

#include "Panel.h"
#include "layouts/Grid.h"
#include "layouts/Canvas.h"
#include "layouts/DockPanel.h"
#include "layouts/WrapPanel.h"
#include "layouts/Layouts.h"
#include "../core/Control.h"
#include "../rendering/Types.h"

using namespace luaui;
using namespace luaui::controls;
using namespace luaui::rendering;

#define TEST(name) void test_##name()
#define RUN_TEST(name) \
    std::cout << "Running: " #name " ... "; \
    test_##name(); \
    std::cout << "PASSED" << std::endl;
#define ASSERT(cond) \
    if (!(cond)) { \
        std::cerr << "\nFAILED: " #cond " at line " << __LINE__ << std::endl; \
        std::exit(1); \
    }
#define ASSERT_EQ(a, b) \
    if ((a) != (b)) { \
        std::cerr << "\nFAILED: " #a " (" << (a) << ") == " #b " (" << (b) << ") at line " << __LINE__ << std::endl; \
        std::exit(1); \
    }
#define ASSERT_NEAR(a, b, eps) \
    if (std::abs((a) - (b)) > (eps)) { \
        std::cerr << "\nFAILED: " #a " (" << (a) << ") ~= " #b " (" << (b) << ") at line " << __LINE__ << std::endl; \
        std::exit(1); \
    }

// Test control that provides fixed desired size through LayoutComponent
class TestControl : public Control {
public:
    TestControl(float width, float height) : fixedSize(width, height) {
        SetName("TestControl");
    }
    
    std::string GetTypeName() const override { return "TestControl"; }
    
protected:
    void InitializeComponents() override {
        // Add layout component
        auto* layout = GetComponents().AddComponent<components::LayoutComponent>(this);
        // Set fixed desired size
        layout->SetWidth(fixedSize.width);
        layout->SetHeight(fixedSize.height);
    }
    
private:
    Size fixedSize;
};

TEST(stackpanel_vertical) {
    auto panel = std::make_shared<StackPanel>();
    panel->SetOrientation(StackPanel::Orientation::Vertical);
    panel->SetSpacing(10.0f);
    
    panel->AddChild(std::make_shared<TestControl>(100.0f, 50.0f));
    panel->AddChild(std::make_shared<TestControl>(150.0f, 60.0f));
    panel->AddChild(std::make_shared<TestControl>(80.0f, 40.0f));
    
    // Measure using LayoutComponent
    if (auto* layout = panel->GetLayout()) {
        interfaces::LayoutConstraint constraint;
        constraint.available = Size(500.0f, 500.0f);
        layout->Measure(constraint);
        
        Size desired = layout->GetDesiredSize();
        ASSERT_NEAR(desired.width, 150.0f, 0.001f);
        ASSERT_NEAR(desired.height, 170.0f, 0.001f); // 50+10+60+10+40
    }
}

TEST(stackpanel_horizontal) {
    auto panel = std::make_shared<StackPanel>();
    panel->SetOrientation(StackPanel::Orientation::Horizontal);
    panel->SetSpacing(5.0f);
    
    panel->AddChild(std::make_shared<TestControl>(100.0f, 50.0f));
    panel->AddChild(std::make_shared<TestControl>(80.0f, 60.0f));
    
    if (auto* layout = panel->GetLayout()) {
        interfaces::LayoutConstraint constraint;
        constraint.available = Size(500.0f, 500.0f);
        layout->Measure(constraint);
        
        Size desired = layout->GetDesiredSize();
        ASSERT_NEAR(desired.width, 185.0f, 0.001f); // 100+5+80
        ASSERT_NEAR(desired.height, 60.0f, 0.001f);
    }
}

TEST(grid_basic) {
    auto grid = std::make_shared<Grid>();
    
    grid->AddColumn(GridLength::Star(1.0f));
    grid->AddColumn(GridLength::Star(1.0f));
    grid->AddRow(GridLength::Auto());
    grid->AddRow(GridLength::Auto());
    
    auto c1 = std::make_shared<TestControl>(100.0f, 50.0f);
    auto c2 = std::make_shared<TestControl>(80.0f, 40.0f);
    auto c3 = std::make_shared<TestControl>(120.0f, 60.0f);
    
    grid->SetRow(c1, 0); grid->SetColumn(c1, 0);
    grid->SetRow(c2, 0); grid->SetColumn(c2, 1);
    grid->SetRow(c3, 1); grid->SetColumn(c3, 0);
    grid->SetColumnSpan(c3, 2);
    
    grid->AddChild(c1);
    grid->AddChild(c2);
    grid->AddChild(c3);
    
    if (auto* layout = grid->GetLayout()) {
        interfaces::LayoutConstraint constraint;
        constraint.available = Size(400.0f, 400.0f);
        layout->Measure(constraint);
        layout->Arrange(Rect(0.0f, 0.0f, 400.0f, 400.0f));
        
        // Grid should have children arranged
        ASSERT_EQ(grid->GetChildCount(), 3);
    }
}

TEST(canvas_absolute) {
    auto canvas = std::make_shared<Canvas>();
    
    auto c1 = std::make_shared<TestControl>(100.0f, 50.0f);
    Canvas::SetLeft(c1, 10.0f);
    Canvas::SetTop(c1, 20.0f);
    
    auto c2 = std::make_shared<TestControl>(80.0f, 40.0f);
    Canvas::SetLeft(c2, 150.0f);
    Canvas::SetTop(c2, 100.0f);
    
    canvas->AddChild(c1);
    canvas->AddChild(c2);
    
    if (auto* layout = canvas->GetLayout()) {
        interfaces::LayoutConstraint constraint;
        constraint.available = Size(500.0f, 500.0f);
        layout->Measure(constraint);
        layout->Arrange(Rect(0.0f, 0.0f, 500.0f, 500.0f));
        
        // Verify attached properties were set
        ASSERT_NEAR(Canvas::GetLeft(c1), 10.0f, 0.001f);
        ASSERT_NEAR(Canvas::GetTop(c1), 20.0f, 0.001f);
    }
}

TEST(dockpanel_basic) {
    auto dock = std::make_shared<DockPanel>();
    dock->SetLastChildFill(true);
    
    auto top = std::make_shared<TestControl>(100.0f, 50.0f);
    DockPanel::SetDock(top, Dock::Top);
    
    auto left = std::make_shared<TestControl>(80.0f, 100.0f);
    DockPanel::SetDock(left, Dock::Left);
    
    auto fill = std::make_shared<TestControl>(100.0f, 100.0f);
    
    dock->AddChild(top);
    dock->AddChild(left);
    dock->AddChild(fill);
    
    if (auto* layout = dock->GetLayout()) {
        interfaces::LayoutConstraint constraint;
        constraint.available = Size(400.0f, 400.0f);
        layout->Measure(constraint);
        layout->Arrange(Rect(0.0f, 0.0f, 400.0f, 400.0f));
        
        ASSERT_EQ(dock->GetChildCount(), 3);
    }
}

TEST(wrappanel_horizontal) {
    auto wrap = std::make_shared<WrapPanel>();
    wrap->SetOrientation(WrapPanel::Orientation::Horizontal);
    
    for (int i = 0; i < 5; ++i) {
        wrap->AddChild(std::make_shared<TestControl>(100.0f, 50.0f));
    }
    
    if (auto* layout = wrap->GetLayout()) {
        interfaces::LayoutConstraint constraint;
        constraint.available = Size(250.0f, 500.0f);
        layout->Measure(constraint);
        layout->Arrange(Rect(0.0f, 0.0f, 250.0f, 500.0f));
        
        ASSERT_EQ(wrap->GetChildCount(), 5);
    }
}

int main() {
    std::cout << "=== LuaUI Controls Layout Tests ===" << std::endl;
    std::cout << std::endl;
    
    try {
        RUN_TEST(stackpanel_vertical);
        RUN_TEST(stackpanel_horizontal);
        RUN_TEST(grid_basic);
        RUN_TEST(canvas_absolute);
        RUN_TEST(dockpanel_basic);
        RUN_TEST(wrappanel_horizontal);
        
        std::cout << std::endl;
        std::cout << "All tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nTest failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
