// LuaUI Layout Demo - Controls Layout System
// Demonstrates the integrated Panel layout system in Controls

#include "layout.h"
#include <iostream>

using namespace luaui;
using namespace luaui::controls;

// Simple control for demo
class DemoControl : public Control {
public:
    DemoControl(const char* n, float w, float h) : fixedWidth(w), fixedHeight(h) {
        SetName(n);
    }
    
    std::string GetTypeName() const override { return "DemoControl"; }
    
protected:
    Size MeasureOverride(const Size&) override {
        return Size(fixedWidth, fixedHeight);
    }
    
    Size ArrangeOverride(const Size& finalSize) override {
        return finalSize;
    }
    
    void RenderOverride(IRenderContext*) override {
    }
    
private:
    float fixedWidth, fixedHeight;
};

void PrintControl(const ControlPtr& ctrl, int indent) {
    std::string prefix(indent * 2, ' ');
    Rect rect = ctrl->GetRenderRect();
    std::cout << prefix << ctrl->GetName() 
              << " [" << rect.x << "," << rect.y 
              << " " << rect.width << "x" << rect.height << "]"
              << std::endl;
    
    auto panel = std::dynamic_pointer_cast<Panel>(ctrl);
    if (panel) {
        for (auto& child : panel->GetChildren()) {
            PrintControl(child, indent + 1);
        }
    }
}

void TestStackPanel() {
    std::cout << "\n=== StackPanel Test ===" << std::endl;
    
    auto panel = std::make_shared<StackPanel>();
    panel->SetName("StackPanel");
    panel->SetOrientation(StackPanel::Orientation::Vertical);
    panel->SetSpacing(10.0f);
    
    panel->AddChild(std::make_shared<DemoControl>("Item1", 200.0f, 50.0f));
    panel->AddChild(std::make_shared<DemoControl>("Item2", 200.0f, 60.0f));
    panel->AddChild(std::make_shared<DemoControl>("Item3", 200.0f, 40.0f));
    
    panel->Measure(Size(500.0f, 500.0f));
    panel->Arrange(Rect(0.0f, 0.0f, 500.0f, 500.0f));
    
    std::cout << "Desired size: " << panel->GetDesiredSize().width << "x" << panel->GetDesiredSize().height << std::endl;
    PrintControl(panel, 0);
}

void TestGrid() {
    std::cout << "\n=== Grid Test ===" << std::endl;
    
    auto grid = std::make_shared<Grid>();
    grid->SetName("Grid");
    
    grid->AddColumn(GridLength::Star(1.0f));
    grid->AddColumn(GridLength::Star(1.0f));
    grid->AddRow(GridLength::Star(1.0f));
    grid->AddRow(GridLength::Star(1.0f));
    
    auto c1 = std::make_shared<DemoControl>("Cell1", 80.0f, 80.0f);
    auto c2 = std::make_shared<DemoControl>("Cell2", 80.0f, 80.0f);
    auto c3 = std::make_shared<DemoControl>("Cell3", 80.0f, 80.0f);
    auto c4 = std::make_shared<DemoControl>("Cell4", 80.0f, 80.0f);
    
    Grid::SetRow(c1.get(), 0); Grid::SetColumn(c1.get(), 0);
    Grid::SetRow(c2.get(), 0); Grid::SetColumn(c2.get(), 1);
    Grid::SetRow(c3.get(), 1); Grid::SetColumn(c3.get(), 0);
    Grid::SetRow(c4.get(), 1); Grid::SetColumn(c4.get(), 1);
    
    grid->AddChild(c1);
    grid->AddChild(c2);
    grid->AddChild(c3);
    grid->AddChild(c4);
    
    grid->Measure(Size(400.0f, 400.0f));
    grid->Arrange(Rect(0.0f, 0.0f, 400.0f, 400.0f));
    
    std::cout << "Desired size: " << grid->GetDesiredSize().width << "x" << grid->GetDesiredSize().height << std::endl;
    PrintControl(grid, 0);
}

void TestCanvas() {
    std::cout << "\n=== Canvas Test ===" << std::endl;
    
    auto canvas = std::make_shared<Canvas>();
    canvas->SetName("Canvas");
    
    auto r1 = std::make_shared<DemoControl>("Rect1", 100.0f, 100.0f);
    Canvas::SetLeft(r1.get(), 10.0f);
    Canvas::SetTop(r1.get(), 10.0f);
    
    auto r2 = std::make_shared<DemoControl>("Rect2", 80.0f, 80.0f);
    Canvas::SetLeft(r2.get(), 150.0f);
    Canvas::SetTop(r2.get(), 50.0f);
    
    canvas->AddChild(r1);
    canvas->AddChild(r2);
    
    canvas->Measure(Size(500.0f, 500.0f));
    canvas->Arrange(Rect(0.0f, 0.0f, 500.0f, 500.0f));
    
    std::cout << "Desired size: " << canvas->GetDesiredSize().width << "x" << canvas->GetDesiredSize().height << std::endl;
    PrintControl(canvas, 0);
}

void TestDockPanel() {
    std::cout << "\n=== DockPanel Test ===" << std::endl;
    
    auto dock = std::make_shared<DockPanel>();
    dock->SetName("DockPanel");
    dock->SetLastChildFill(true);
    
    auto top = std::make_shared<DemoControl>("Top", 100.0f, 50.0f);
    DockPanel::SetDock(top.get(), Dock::Top);
    
    auto left = std::make_shared<DemoControl>("Left", 80.0f, 100.0f);
    DockPanel::SetDock(left.get(), Dock::Left);
    
    auto fill = std::make_shared<DemoControl>("Fill", 100.0f, 100.0f);
    
    dock->AddChild(top);
    dock->AddChild(left);
    dock->AddChild(fill);
    
    dock->Measure(Size(400.0f, 400.0f));
    dock->Arrange(Rect(0.0f, 0.0f, 400.0f, 400.0f));
    
    std::cout << "Desired size: " << dock->GetDesiredSize().width << "x" << dock->GetDesiredSize().height << std::endl;
    PrintControl(dock, 0);
}

void TestWrapPanel() {
    std::cout << "\n=== WrapPanel Test ===" << std::endl;
    
    auto wrap = std::make_shared<WrapPanel>();
    wrap->SetName("WrapPanel");
    wrap->SetOrientation(WrapPanel::Orientation::Horizontal);
    
    for (int i = 0; i < 5; i++) {
        wrap->AddChild(std::make_shared<DemoControl>("Item", 80.0f, 50.0f));
    }
    
    wrap->Measure(Size(250.0f, 500.0f));
    wrap->Arrange(Rect(0.0f, 0.0f, 250.0f, 500.0f));
    
    std::cout << "Desired size: " << wrap->GetDesiredSize().width << "x" << wrap->GetDesiredSize().height << std::endl;
    PrintControl(wrap, 0);
}

int main() {
    std::cout << "LuaUI Controls Layout Demo" << std::endl;
    std::cout << "==========================" << std::endl;
    
    TestStackPanel();
    TestGrid();
    TestCanvas();
    TestDockPanel();
    TestWrapPanel();
    
    std::cout << "\nAll tests completed!" << std::endl;
    return 0;
}
