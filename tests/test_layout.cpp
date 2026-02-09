#include <iostream>
#include <cassert>
#include <cmath>
#include <memory>

#include "luaui/controls/layout.h"

using namespace luaui;
using namespace luaui::controls;

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
#define ASSERT_NEAR(a, b, eps) \
    if (std::abs((a) - (b)) > (eps)) { \
        std::cerr << "\nFAILED: " #a " (" << (a) << ") ~= " #b " (" << (b) << ") at line " << __LINE__ << std::endl; \
        std::exit(1); \
    }

class TestControl : public Control {
public:
    TestControl(float width, float height) : fixedSize(width, height) {
        SetName("TestControl");
    }
    
    std::string GetTypeName() const override { return "TestControl"; }
    
protected:
    Size MeasureOverride(const Size&) override {
        return fixedSize;
    }
    
    Size ArrangeOverride(const Size& finalSize) override {
        return finalSize;
    }
    
    void RenderOverride(IRenderContext*) override {
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
    
    panel->Measure(Size(500.0f, 500.0f));
    
    ASSERT_NEAR(panel->GetDesiredSize().width, 150.0f, 0.001f);
    ASSERT_NEAR(panel->GetDesiredSize().height, 170.0f, 0.001f);
    
    panel->Arrange(Rect(0.0f, 0.0f, 500.0f, 500.0f));
}

TEST(stackpanel_horizontal) {
    auto panel = std::make_shared<StackPanel>();
    panel->SetOrientation(StackPanel::Orientation::Horizontal);
    panel->SetSpacing(5.0f);
    
    panel->AddChild(std::make_shared<TestControl>(100.0f, 50.0f));
    panel->AddChild(std::make_shared<TestControl>(80.0f, 60.0f));
    
    panel->Measure(Size(500.0f, 500.0f));
    
    ASSERT_NEAR(panel->GetDesiredSize().width, 185.0f, 0.001f);
    ASSERT_NEAR(panel->GetDesiredSize().height, 60.0f, 0.001f);
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
    
    Grid::SetRow(c1.get(), 0); Grid::SetColumn(c1.get(), 0);
    Grid::SetRow(c2.get(), 0); Grid::SetColumn(c2.get(), 1);
    Grid::SetRow(c3.get(), 1); Grid::SetColumn(c3.get(), 0);
    Grid::SetColumnSpan(c3.get(), 2);
    
    grid->AddChild(c1);
    grid->AddChild(c2);
    grid->AddChild(c3);
    
    grid->Measure(Size(400.0f, 400.0f));
    grid->Arrange(Rect(0.0f, 0.0f, 400.0f, 400.0f));
    
    ASSERT_NEAR(grid->GetChild(0)->GetRenderRect().x, 0.0f, 0.001f);
}

TEST(canvas_absolute) {
    auto canvas = std::make_shared<Canvas>();
    
    auto c1 = std::make_shared<TestControl>(100.0f, 50.0f);
    Canvas::SetLeft(c1.get(), 10.0f);
    Canvas::SetTop(c1.get(), 20.0f);
    
    auto c2 = std::make_shared<TestControl>(80.0f, 40.0f);
    Canvas::SetLeft(c2.get(), 150.0f);
    Canvas::SetTop(c2.get(), 100.0f);
    
    canvas->AddChild(c1);
    canvas->AddChild(c2);
    
    canvas->Measure(Size(500.0f, 500.0f));
    canvas->Arrange(Rect(0.0f, 0.0f, 500.0f, 500.0f));
    
    ASSERT_NEAR(c1->GetRenderRect().x, 10.0f, 0.001f);
    ASSERT_NEAR(c1->GetRenderRect().y, 20.0f, 0.001f);
}

TEST(dockpanel_basic) {
    auto dock = std::make_shared<DockPanel>();
    dock->SetLastChildFill(true);
    
    auto top = std::make_shared<TestControl>(100.0f, 50.0f);
    DockPanel::SetDock(top.get(), Dock::Top);
    
    auto left = std::make_shared<TestControl>(80.0f, 100.0f);
    DockPanel::SetDock(left.get(), Dock::Left);
    
    auto fill = std::make_shared<TestControl>(100.0f, 100.0f);
    
    dock->AddChild(top);
    dock->AddChild(left);
    dock->AddChild(fill);
    
    dock->Measure(Size(400.0f, 400.0f));
    dock->Arrange(Rect(0.0f, 0.0f, 400.0f, 400.0f));
    
    ASSERT_NEAR(top->GetRenderRect().width, 400.0f, 0.001f);
    ASSERT_NEAR(top->GetRenderRect().height, 50.0f, 0.001f);
}

TEST(wrappanel_horizontal) {
    auto wrap = std::make_shared<WrapPanel>();
    wrap->SetOrientation(WrapPanel::Orientation::Horizontal);
    
    for (int i = 0; i < 5; ++i) {
        wrap->AddChild(std::make_shared<TestControl>(100.0f, 50.0f));
    }
    
    wrap->Measure(Size(250.0f, 500.0f));
    wrap->Arrange(Rect(0.0f, 0.0f, 250.0f, 500.0f));
    
    ASSERT_NEAR(wrap->GetChild(0)->GetRenderRect().y, 0.0f, 0.001f);
    ASSERT_NEAR(wrap->GetChild(2)->GetRenderRect().y, 50.0f, 0.001f);
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
