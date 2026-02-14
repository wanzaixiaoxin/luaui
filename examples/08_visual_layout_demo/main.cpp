// Visual Layout Demo - 使用框架 Window 类
// 展示各种布局效果

#include "Controls.h"
#include "Panel.h"
#include "Window.h"
#include "Logger.h"
#include "Types.h"
#include "Shapes.h"
#include "IRenderContext.h"
#include <windows.h>
#include <string>

using namespace luaui;
using namespace luaui::controls;
using namespace luaui::rendering;
using namespace luaui::utils;

// 简化的彩色矩形控件
class ColorRect : public luaui::Control {
public:
    ColorRect(const Color& fillColor, const Color& strokeColor = Color::Transparent())
        : m_fillColor(fillColor), m_strokeColor(strokeColor) {
        SetName("ColorRect");
        InitializeComponents();
    }
    
    std::string GetTypeName() const override { return "ColorRect"; }
    
    void SetFillColor(const Color& c) { m_fillColor = c; }
    void SetStrokeColor(const Color& c) { m_strokeColor = c; }
    void SetStrokeThickness(float t) { m_strokeThickness = t; }

protected:
    void InitializeComponents() override {
        Logger::Debug("[ColorRect] InitializeComponents");
        
        Control::InitializeComponents();
        
        // 添加布局组件
        auto* layout = GetComponents().AddComponent<components::LayoutComponent>(this);
        layout->SetWidth(80);
        layout->SetHeight(60);
        
        // 添加渲染组件（关键！）
        auto* render = GetComponents().AddComponent<components::RenderComponent>(this);
        Logger::DebugF("[ColorRect] RenderComponent added: %p", (void*)render);
    }
    
    void OnRender(IRenderContext* context) override {
        if (!context) return;
        
        auto* render = GetRender();
        if (!render) return;
        
        float width = render->GetRenderRect().width;
        float height = render->GetRenderRect().height;
        
        static int count = 0;
        if (++count % 60 == 0 || width <= 0 || height <= 0) {
            Logger::TraceF("[ColorRect] OnRender: size=%.0fx%.0f", width, height);
        }
        
        // 跳过无效尺寸
        if (width <= 0 || height <= 0) {
            Logger::Trace("[ColorRect] Skipping render due to invalid size");
            return;
        }
        
        // 强制不透明背景
        auto fillBrush = context->CreateSolidColorBrush(m_fillColor);
        if (fillBrush) {
            context->FillRectangle(Rect(0, 0, width, height), fillBrush.get());
        }
        
        // 边框
        if (m_strokeColor.a > 0 && m_strokeThickness > 0) {
            auto strokeBrush = context->CreateSolidColorBrush(m_strokeColor);
            if (strokeBrush) {
                context->DrawRectangle(Rect(0, 0, width, height), strokeBrush.get(), m_strokeThickness);
            }
        }
    }

private:
    Color m_fillColor;
    Color m_strokeColor;
    float m_strokeThickness = 1.0f;
};

// 演示窗口 - 使用框架 Window 类
class VisualLayoutWindow : public Window {
public:
    VisualLayoutWindow() = default;

protected:
    void OnLoaded() override;
    void OnKeyDown(int keyCode) override;
    
private:
    void CreateDemo_VerticalStack();
    void CreateDemo_HorizontalStack();
    void CreateDemo_NestedStack();
    void CreateDemo_Shapes();
    void SwitchDemo(int demoIndex);
    void UpdateWindowTitle();
    
    std::shared_ptr<StackPanel> m_contentPanel;
    int m_currentDemoIndex = 0;
    const wchar_t* m_demoNames[4] = {
        L"Vertical StackPanel",
        L"Horizontal StackPanel", 
        L"Nested StackPanels",
        L"Shapes"
    };
};

void VisualLayoutWindow::OnLoaded() {
    Logger::Info("=== Visual Layout Demo Starting ===");
    
    // 创建根面板
    auto root = std::make_shared<StackPanel>();
    root->SetName("Root");
    root->SetOrientation(StackPanel::Orientation::Vertical);
    
    // 内容面板
    m_contentPanel = std::make_shared<StackPanel>();
    m_contentPanel->SetName("Content");
    root->AddChild(m_contentPanel);
    
    SetRoot(root);
    
    // 加载初始演示
    SwitchDemo(0);
}

void VisualLayoutWindow::UpdateWindowTitle() {
    std::wstring title = std::wstring(L"Visual Layout Demo - ") + m_demoNames[m_currentDemoIndex] + L" [Press 1-4 to switch]";
    SetWindowTextW(GetHandle(), title.c_str());
}

void VisualLayoutWindow::SwitchDemo(int demoIndex) {
    Logger::DebugF("[SwitchDemo] index=%d, current children=%zu", demoIndex, m_contentPanel->GetChildCount());
    
    m_currentDemoIndex = demoIndex % 4;
    
    // 清空内容面板
    m_contentPanel->ClearChildren();
    Logger::Debug("[SwitchDemo] Cleared children, calling InvalidateLayout...");
    
    // 关键：子控件改变后必须调用 InvalidateLayout
    InvalidateLayout();
    Logger::Debug("[SwitchDemo] InvalidateLayout called");
    
    switch (m_currentDemoIndex) {
        case 0: CreateDemo_VerticalStack(); break;
        case 1: CreateDemo_HorizontalStack(); break;
        case 2: CreateDemo_NestedStack(); break;
        case 3: CreateDemo_Shapes(); break;
    }
    
    UpdateWindowTitle();
    InvalidateLayout();
}

void VisualLayoutWindow::OnKeyDown(int keyCode) {
    Logger::DebugF("[OnKeyDown] keyCode=%d", keyCode);
    if (keyCode >= '1' && keyCode <= '4') {
        SwitchDemo(keyCode - '1');
    }
    Window::OnKeyDown(keyCode);
}

void VisualLayoutWindow::CreateDemo_VerticalStack() {
    auto panel = std::make_shared<StackPanel>();
    panel->SetName("VerticalStackDemo");
    panel->SetOrientation(StackPanel::Orientation::Vertical);
    panel->SetSpacing(10);
    
    Color colors[] = {
        Color::FromHex(0xFF6B6B),
        Color::FromHex(0x4ECDC4),
        Color::FromHex(0x45B7D1),
        Color::FromHex(0x96CEB4),
        Color::FromHex(0xFECA57),
    };
    
    for (int i = 0; i < 5; i++) {
        auto rect = std::make_shared<ColorRect>(colors[i]);
        if (auto* layout = rect->GetLayout()) {
            layout->SetWidth(200);
            layout->SetHeight(60);
        }
        panel->AddChild(rect);
    }
    
    Logger::DebugF("[CreateDemo_VerticalStack] Panel has %zu children", panel->GetChildCount());
    
    m_contentPanel->AddChild(panel);
    
    Logger::DebugF("[CreateDemo_VerticalStack] ContentPanel has %zu children", m_contentPanel->GetChildCount());
}

void VisualLayoutWindow::CreateDemo_HorizontalStack() {
    auto panel = std::make_shared<StackPanel>();
    panel->SetName("HorizontalStackDemo");
    panel->SetOrientation(StackPanel::Orientation::Horizontal);
    panel->SetSpacing(15);
    
    Color colors[] = {
        Color::FromHex(0xFF6B6B),
        Color::FromHex(0x4ECDC4),
        Color::FromHex(0x45B7D1),
        Color::FromHex(0x96CEB4),
        Color::FromHex(0xFECA57),
    };
    
    for (int i = 0; i < 5; i++) {
        auto rect = std::make_shared<ColorRect>(colors[i]);
        if (auto* layout = rect->GetLayout()) {
            layout->SetWidth(100);
            layout->SetHeight(150);
        }
        panel->AddChild(rect);
    }
    
    m_contentPanel->AddChild(panel);
}

void VisualLayoutWindow::CreateDemo_NestedStack() {
    auto outerPanel = std::make_shared<StackPanel>();
    outerPanel->SetName("NestedDemo");
    outerPanel->SetOrientation(StackPanel::Orientation::Vertical);
    outerPanel->SetSpacing(10);
    
    // Header
    auto header = std::make_shared<ColorRect>(Color::FromHex(0x333333));
    if (auto* layout = header->GetLayout()) {
        layout->SetWidth(600);
        layout->SetHeight(40);
    }
    outerPanel->AddChild(header);
    
    // Content - Horizontal panel
    auto contentPanel = std::make_shared<StackPanel>();
    contentPanel->SetName("Content");
    contentPanel->SetOrientation(StackPanel::Orientation::Horizontal);
    contentPanel->SetSpacing(5);
    
    auto left = std::make_shared<ColorRect>(Color::FromHex(0xFF6B6B));
    if (auto* layout = left->GetLayout()) {
        layout->SetWidth(150);
        layout->SetHeight(300);
    }
    contentPanel->AddChild(left);
    
    auto center = std::make_shared<ColorRect>(Color::FromHex(0x4ECDC4));
    if (auto* layout = center->GetLayout()) {
        layout->SetWidth(300);
        layout->SetHeight(300);
    }
    contentPanel->AddChild(center);
    
    auto right = std::make_shared<ColorRect>(Color::FromHex(0x45B7D1));
    if (auto* layout = right->GetLayout()) {
        layout->SetWidth(150);
        layout->SetHeight(300);
    }
    contentPanel->AddChild(right);
    
    outerPanel->AddChild(contentPanel);
    
    // Footer
    auto footer = std::make_shared<ColorRect>(Color::FromHex(0x666666));
    if (auto* layout = footer->GetLayout()) {
        layout->SetWidth(600);
        layout->SetHeight(40);
    }
    outerPanel->AddChild(footer);
    
    m_contentPanel->AddChild(outerPanel);
}

void VisualLayoutWindow::CreateDemo_Shapes() {
    auto panel = std::make_shared<StackPanel>();
    panel->SetName("ShapesDemo");
    panel->SetOrientation(StackPanel::Orientation::Horizontal);
    panel->SetSpacing(20);
    
    // Rectangle
    auto rect = std::make_shared<luaui::controls::Rectangle>();
    if (auto* layout = rect->GetLayout()) {
        layout->SetWidth(100);
        layout->SetHeight(80);
    }
    rect->SetFill(Color::FromHex(0x4CAF50));
    rect->SetStroke(Color::FromHex(0x2E7D32));
    rect->SetStrokeThickness(2);
    panel->AddChild(rect);
    
    // Rounded Rectangle
    auto roundRect = std::make_shared<luaui::controls::Rectangle>();
    if (auto* layout = roundRect->GetLayout()) {
        layout->SetWidth(100);
        layout->SetHeight(80);
    }
    roundRect->SetFill(Color::FromHex(0x2196F3));
    roundRect->SetStroke(Color::FromHex(0x1565C0));
    roundRect->SetStrokeThickness(2);
    roundRect->SetRadiusX(10);
    roundRect->SetRadiusY(10);
    panel->AddChild(roundRect);
    
    // Ellipse
    auto ellipse = std::make_shared<luaui::controls::Ellipse>();
    if (auto* layout = ellipse->GetLayout()) {
        layout->SetWidth(100);
        layout->SetHeight(80);
    }
    ellipse->SetFill(Color::FromHex(0xFF5722));
    ellipse->SetStroke(Color::FromHex(0xD84315));
    ellipse->SetStrokeThickness(2);
    panel->AddChild(ellipse);
    
    m_contentPanel->AddChild(panel);
}

// ============================================================================
// 入口点
// ============================================================================

int main() {
    HINSTANCE hInstance = GetModuleHandle(nullptr);
    
    Logger::Initialize();
    Logger::SetConsoleLevel(LogLevel::Debug);
    
    try {
        VisualLayoutWindow window;
        
        if (!window.Create(hInstance, L"Visual Layout Demo", 1024, 768)) {
            Logger::Error("Failed to create window");
            return 1;
        }
        
        window.Show(SW_SHOW);
        return window.Run();
        
    } catch (const std::exception& e) {
        Logger::ErrorF("Exception: %s", e.what());
        return 1;
    }
}
