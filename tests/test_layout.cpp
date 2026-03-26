// Layout Module Tests - StackPanel, Grid, Canvas, DockPanel, WrapPanel
// Updated to use unified TestFramework.h
#include "TestFramework.h"
#include "Panel.h"
#include "layouts/Grid.h"
#include "layouts/Canvas.h"
#include "layouts/DockPanel.h"
#include "layouts/WrapPanel.h"
#include "layouts/Layouts.h"
#include "Button.h"
#include "../core/Control.h"
#include "../core/Components/LayoutComponent.h"
#include "../rendering/Types.h"

using namespace luaui;
using namespace luaui::controls;
using namespace luaui::rendering;
using namespace luaui::interfaces;

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

// ==================== StackPanel Tests ====================
TEST(StackPanel_DefaultConstruction) {
    auto panel = std::make_shared<StackPanel>();
    
    ASSERT_TRUE(panel->GetIsVisible());
    ASSERT_EQ(panel->GetTypeName(), "StackPanel");
    ASSERT_TRUE(panel->GetOrientation() == StackPanel::Orientation::Vertical);
    ASSERT_NEAR(panel->GetSpacing(), 0.0f, 0.001f);
}

TEST(StackPanel_Vertical) {
    auto panel = std::make_shared<StackPanel>();
    panel->SetOrientation(StackPanel::Orientation::Vertical);
    panel->SetSpacing(10.0f);
    
    panel->AddChild(std::make_shared<TestControl>(100.0f, 50.0f));
    panel->AddChild(std::make_shared<TestControl>(150.0f, 60.0f));
    panel->AddChild(std::make_shared<TestControl>(80.0f, 40.0f));
    
    // Measure using LayoutComponent
    if (auto* layout = panel->GetLayout()) {
        LayoutConstraint constraint;
        constraint.available = Size(500.0f, 500.0f);
        layout->Measure(constraint);
        
        Size desired = layout->GetDesiredSize();
        ASSERT_NEAR(desired.width, 150.0f, 0.001f);  // Max width
        ASSERT_NEAR(desired.height, 170.0f, 0.001f); // 50+10+60+10+40
    }
}

TEST(StackPanel_Horizontal) {
    auto panel = std::make_shared<StackPanel>();
    panel->SetOrientation(StackPanel::Orientation::Horizontal);
    panel->SetSpacing(5.0f);
    
    panel->AddChild(std::make_shared<TestControl>(100.0f, 50.0f));
    panel->AddChild(std::make_shared<TestControl>(80.0f, 60.0f));
    
    if (auto* layout = panel->GetLayout()) {
        LayoutConstraint constraint;
        constraint.available = Size(500.0f, 500.0f);
        layout->Measure(constraint);
        
        Size desired = layout->GetDesiredSize();
        ASSERT_NEAR(desired.width, 185.0f, 0.001f); // 100+5+80
        ASSERT_NEAR(desired.height, 60.0f, 0.001f); // Max height
    }
}

TEST(StackPanel_OrientationChange) {
    auto panel = std::make_shared<StackPanel>();
    
    panel->SetOrientation(StackPanel::Orientation::Horizontal);
    ASSERT_TRUE(panel->GetOrientation() == StackPanel::Orientation::Horizontal);
    
    panel->SetOrientation(StackPanel::Orientation::Vertical);
    ASSERT_TRUE(panel->GetOrientation() == StackPanel::Orientation::Vertical);
}

TEST(StackPanel_SpacingChange) {
    auto panel = std::make_shared<StackPanel>();
    
    panel->SetSpacing(20.0f);
    ASSERT_NEAR(panel->GetSpacing(), 20.0f, 0.001f);
    
    panel->SetSpacing(0.0f);
    ASSERT_NEAR(panel->GetSpacing(), 0.0f, 0.001f);
}

// ==================== Grid Layout Tests ====================
TEST(Grid_LayoutBasic) {
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
        LayoutConstraint constraint;
        constraint.available = Size(400.0f, 400.0f);
        layout->Measure(constraint);
        layout->Arrange(Rect(0.0f, 0.0f, 400.0f, 400.0f));
        
        // Grid should have children arranged
        ASSERT_EQ(grid->GetChildCount(), 3u);
    }
}

// ==================== Canvas Tests ====================
TEST(Canvas_DefaultConstruction) {
    auto canvas = std::make_shared<Canvas>();
    
    ASSERT_TRUE(canvas->GetIsVisible());
    ASSERT_EQ(canvas->GetTypeName(), "Canvas");
}

TEST(Canvas_AbsolutePositioning) {
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
        LayoutConstraint constraint;
        constraint.available = Size(500.0f, 500.0f);
        layout->Measure(constraint);
        layout->Arrange(Rect(0.0f, 0.0f, 500.0f, 500.0f));
        
        // Verify attached properties were set
        ASSERT_NEAR(Canvas::GetLeft(c1), 10.0f, 0.001f);
        ASSERT_NEAR(Canvas::GetTop(c1), 20.0f, 0.001f);
        ASSERT_NEAR(Canvas::GetLeft(c2), 150.0f, 0.001f);
        ASSERT_NEAR(Canvas::GetTop(c2), 100.0f, 0.001f);
    }
}

TEST(Canvas_ZeroPosition) {
    auto canvas = std::make_shared<Canvas>();
    
    auto ctrl = std::make_shared<TestControl>(50.0f, 30.0f);
    Canvas::SetLeft(ctrl, 0.0f);
    Canvas::SetTop(ctrl, 0.0f);
    
    canvas->AddChild(ctrl);
    
    ASSERT_NEAR(Canvas::GetLeft(ctrl), 0.0f, 0.001f);
    ASSERT_NEAR(Canvas::GetTop(ctrl), 0.0f, 0.001f);
}

// ==================== DockPanel Tests ====================
TEST(DockPanel_DefaultConstruction) {
    auto dock = std::make_shared<DockPanel>();
    
    ASSERT_TRUE(dock->GetIsVisible());
    ASSERT_EQ(dock->GetTypeName(), "DockPanel");
    ASSERT_TRUE(dock->GetLastChildFill());
}

TEST(DockPanel_BasicLayout) {
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
        LayoutConstraint constraint;
        constraint.available = Size(400.0f, 400.0f);
        layout->Measure(constraint);
        layout->Arrange(Rect(0.0f, 0.0f, 400.0f, 400.0f));
        
        ASSERT_EQ(dock->GetChildCount(), 3u);
    }
}

TEST(DockPanel_DockPositions) {
    auto ctrl = std::make_shared<TestControl>(50.0f, 50.0f);
    
    DockPanel::SetDock(ctrl, Dock::Top);
    ASSERT_TRUE(DockPanel::GetDock(ctrl) == Dock::Top);
    
    DockPanel::SetDock(ctrl, Dock::Bottom);
    ASSERT_TRUE(DockPanel::GetDock(ctrl) == Dock::Bottom);
    
    DockPanel::SetDock(ctrl, Dock::Left);
    ASSERT_TRUE(DockPanel::GetDock(ctrl) == Dock::Left);
    
    DockPanel::SetDock(ctrl, Dock::Right);
    ASSERT_TRUE(DockPanel::GetDock(ctrl) == Dock::Right);
}

TEST(DockPanel_LastChildFill) {
    auto dock = std::make_shared<DockPanel>();
    
    dock->SetLastChildFill(false);
    ASSERT_FALSE(dock->GetLastChildFill());
    
    dock->SetLastChildFill(true);
    ASSERT_TRUE(dock->GetLastChildFill());
}

// ==================== WrapPanel Tests ====================
TEST(WrapPanel_DefaultConstruction) {
    auto wrap = std::make_shared<WrapPanel>();
    
    ASSERT_TRUE(wrap->GetIsVisible());
    ASSERT_EQ(wrap->GetTypeName(), "WrapPanel");
    ASSERT_TRUE(wrap->GetOrientation() == WrapPanel::Orientation::Horizontal);
}

TEST(WrapPanel_HorizontalWrap) {
    auto wrap = std::make_shared<WrapPanel>();
    wrap->SetOrientation(WrapPanel::Orientation::Horizontal);
    
    for (int i = 0; i < 5; ++i) {
        wrap->AddChild(std::make_shared<TestControl>(100.0f, 50.0f));
    }
    
    if (auto* layout = wrap->GetLayout()) {
        LayoutConstraint constraint;
        constraint.available = Size(250.0f, 500.0f);
        layout->Measure(constraint);
        layout->Arrange(Rect(0.0f, 0.0f, 250.0f, 500.0f));
        
        ASSERT_EQ(wrap->GetChildCount(), 5u);
    }
}

TEST(WrapPanel_OrientationChange) {
    auto wrap = std::make_shared<WrapPanel>();
    
    wrap->SetOrientation(WrapPanel::Orientation::Vertical);
    ASSERT_TRUE(wrap->GetOrientation() == WrapPanel::Orientation::Vertical);
    
    wrap->SetOrientation(WrapPanel::Orientation::Horizontal);
    ASSERT_TRUE(wrap->GetOrientation() == WrapPanel::Orientation::Horizontal);
}

// ==================== Panel Base Tests ====================
TEST(Panel_ChildManagement) {
    auto panel = std::make_shared<Panel>();
    
    auto c1 = std::make_shared<Button>();
    auto c2 = std::make_shared<Button>();
    auto c3 = std::make_shared<Button>();
    
    panel->AddChild(c1);
    ASSERT_EQ(panel->GetChildCount(), 1u);
    
    panel->AddChild(c2);
    ASSERT_EQ(panel->GetChildCount(), 2u);
    
    panel->RemoveChild(c1);
    ASSERT_EQ(panel->GetChildCount(), 1u);
    
    panel->ClearChildren();
    ASSERT_EQ(panel->GetChildCount(), 0u);
}

TEST(Panel_InsertChild) {
    auto panel = std::make_shared<Panel>();
    
    auto c1 = std::make_shared<Button>();
    auto c2 = std::make_shared<Button>();
    auto c3 = std::make_shared<Button>();
    
    panel->AddChild(c1);
    panel->AddChild(c3);
    panel->InsertChild(1, c2);
    
    ASSERT_EQ(panel->GetChildCount(), 3u);
}

// ==================== GridLength Tests ====================
TEST(GridLength_Types) {
    GridLength autoLen = GridLength::Auto();
    ASSERT_TRUE(autoLen.IsAuto());
    ASSERT_EQ(autoLen.value, 0.0f);
    
    GridLength pixelLen = GridLength::Pixel(100.0f);
    ASSERT_TRUE(pixelLen.IsPixel());
    ASSERT_EQ(pixelLen.value, 100.0f);
    
    GridLength starLen = GridLength::Star(2.0f);
    ASSERT_TRUE(starLen.IsStar());
    ASSERT_EQ(starLen.value, 2.0f);
    
    GridLength defaultStar = GridLength::Star();
    ASSERT_TRUE(defaultStar.IsStar());
    ASSERT_EQ(defaultStar.value, 1.0f);
}

// ==================== Performance Tests ====================
TEST(Layout_ManyChildren) {
    auto panel = std::make_shared<StackPanel>();
    
    for (int i = 0; i < 1000; ++i) {
        panel->AddChild(std::make_shared<TestControl>(50.0f, 30.0f));
    }
    
    ASSERT_EQ(panel->GetChildCount(), 1000u);
}

// ==================== Main ====================
int main() {
    return RUN_ALL_TESTS();
}
