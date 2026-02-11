// LuaUI Layout Demo - 使用新SOLID架构
// 演示集成的Panel布局系统

#include "Controls.h"
#include "Interfaces/ILayoutable.h"
#include <iostream>

using namespace luaui;
using namespace luaui::controls;

// 简单的演示控件
class DemoControl : public ControlBase {
public:
    DemoControl(const char* n, float w, float h) : fixedWidth(w), fixedHeight(h) {
        SetName(n);
        InitializeComponents();
        
        if (auto* layout = GetLayout()) {
            layout->SetWidth(w);
            layout->SetHeight(h);
        }
    }
    
    std::string GetTypeName() const override { return "DemoControl"; }

private:
    float fixedWidth, fixedHeight;
};

void PrintControl(const std::shared_ptr<IControl>& ctrl, int indent) {
    std::string prefix(indent * 2, ' ');
    
    rendering::Rect rect;
    if (auto* layoutable = ctrl->AsLayoutable()) {
        rect = layoutable->GetRenderRect();
    }
    
    std::cout << prefix << ctrl->GetName() 
              << " [" << rect.x << "," << rect.y 
              << " " << rect.width << "x" << rect.height << "]"
              << std::endl;
    
    // 尝试作为Panel打印子控件
    if (auto panel = std::dynamic_pointer_cast<Panel>(ctrl)) {
        for (auto& child : panel->GetChildren()) {
            PrintControl(child, indent + 1);
        }
    }
}

void TestStackPanel() {
    std::cout << "\n=== StackPanel Test (New Architecture) ===" << std::endl;
    
    auto panel = std::make_shared<StackPanel>();
    panel->SetName("StackPanel");
    panel->SetOrientation(StackPanel::Orientation::Vertical);
    panel->SetSpacing(10.0f);
    
    panel->AddChild(std::make_shared<DemoControl>("Item1", 200.0f, 50.0f));
    panel->AddChild(std::make_shared<DemoControl>("Item2", 200.0f, 60.0f));
    panel->AddChild(std::make_shared<DemoControl>("Item3", 200.0f, 40.0f));
    
    interfaces::LayoutConstraint constraint;
    constraint.available = rendering::Size(500.0f, 500.0f);
    
    if (auto* layoutable = panel->AsLayoutable()) {
        layoutable->Measure(constraint);
        layoutable->Arrange(rendering::Rect(0.0f, 0.0f, 500.0f, 500.0f));
        
        auto desired = layoutable->GetDesiredSize();
        std::cout << "Desired size: " << desired.width << "x" << desired.height << std::endl;
    }
    
    PrintControl(panel, 0);
}

void TestHorizontalStackPanel() {
    std::cout << "\n=== Horizontal StackPanel Test ===" << std::endl;
    
    auto panel = std::make_shared<StackPanel>();
    panel->SetName("HStackPanel");
    panel->SetOrientation(StackPanel::Orientation::Horizontal);
    panel->SetSpacing(15.0f);
    
    panel->AddChild(std::make_shared<DemoControl>("H1", 80.0f, 50.0f));
    panel->AddChild(std::make_shared<DemoControl>("H2", 100.0f, 50.0f));
    panel->AddChild(std::make_shared<DemoControl>("H3", 60.0f, 50.0f));
    
    interfaces::LayoutConstraint constraint;
    constraint.available = rendering::Size(500.0f, 100.0f);
    
    if (auto* layoutable = panel->AsLayoutable()) {
        layoutable->Measure(constraint);
        layoutable->Arrange(rendering::Rect(0.0f, 0.0f, 500.0f, 100.0f));
        
        auto desired = layoutable->GetDesiredSize();
        std::cout << "Desired size: " << desired.width << "x" << desired.height << std::endl;
    }
    
    PrintControl(panel, 0);
}

void TestNestedPanels() {
    std::cout << "\n=== Nested Panels Test ===" << std::endl;
    
    auto root = std::make_shared<StackPanel>();
    root->SetName("Root");
    root->SetOrientation(StackPanel::Orientation::Vertical);
    root->SetSpacing(10.0f);
    
    // 添加一些子控件
    root->AddChild(std::make_shared<DemoControl>("Header", 400.0f, 30.0f));
    
    // 嵌套的水平面板
    auto contentPanel = std::make_shared<StackPanel>();
    contentPanel->SetName("Content");
    contentPanel->SetOrientation(StackPanel::Orientation::Horizontal);
    contentPanel->SetSpacing(5.0f);
    contentPanel->AddChild(std::make_shared<DemoControl>("Left", 100.0f, 200.0f));
    contentPanel->AddChild(std::make_shared<DemoControl>("Center", 200.0f, 200.0f));
    contentPanel->AddChild(std::make_shared<DemoControl>("Right", 100.0f, 200.0f));
    
    root->AddChild(contentPanel);
    root->AddChild(std::make_shared<DemoControl>("Footer", 400.0f, 30.0f));
    
    interfaces::LayoutConstraint constraint;
    constraint.available = rendering::Size(500.0f, 500.0f);
    
    if (auto* layoutable = root->AsLayoutable()) {
        layoutable->Measure(constraint);
        layoutable->Arrange(rendering::Rect(0.0f, 0.0f, 500.0f, 500.0f));
        
        auto desired = layoutable->GetDesiredSize();
        std::cout << "Root desired size: " << desired.width << "x" << desired.height << std::endl;
    }
    
    PrintControl(root, 0);
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  LuaUI Layout Demo - New Architecture" << std::endl;
    std::cout << "========================================" << std::endl;
    
    try {
        TestStackPanel();
        TestHorizontalStackPanel();
        TestNestedPanels();
        
        std::cout << "\n=== All tests completed successfully ===" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
