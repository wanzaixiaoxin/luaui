// LuaUI Controls Demo - 使用框架提供的 Window 类
// 展示如何使用新架构 API 和完善的 Window 类

#include "Controls.h"
#include "Panel.h"  // for StackPanel
#include "Window.h"
#include "IRenderContext.h"
#include "Logger.h"
#include "Types.h"
#include "Shapes.h"
#include <windows.h>
#include <string>
#include <sstream>

using namespace luaui;
using namespace luaui::controls;
using namespace luaui::rendering;
using namespace luaui::utils;

// 使用框架提供的 Window 类
class ControlsDemoWindow : public Window {
public:
    ControlsDemoWindow() = default;

protected:
    void OnLoaded() override;

private:
    void SetupEventHandlers();
    
    // 控件引用
    std::shared_ptr<TextBlock> m_statusText;
    std::shared_ptr<ProgressBar> m_progressBar;
    std::shared_ptr<Slider> m_slider;
    std::shared_ptr<CheckBox> m_checkBox;
    std::shared_ptr<TextBox> m_textBox;
};

void ControlsDemoWindow::OnLoaded() {
    Logger::Info("=== LuaUI Controls Demo Starting ===");
    
    // 创建根面板
    auto rootPanel = std::make_shared<StackPanel>();
    rootPanel->SetName("root");
    rootPanel->SetOrientation(StackPanel::Orientation::Vertical);
    
    // 设置根面板背景色
    if (auto* render = rootPanel->GetRender()) {
        render->SetBackground(Color::FromRGBA(245, 245, 245, 255));
    }
    
    // ========== 标题区域 ==========
    auto title = std::make_shared<TextBlock>();
    title->SetText(L"LuaUI Controls Demo - New Architecture");
    title->SetFontSize(24);
    title->SetForeground(Color::FromHex(0x333333));
    rootPanel->AddChild(title);
    
    auto subtitle = std::make_shared<TextBlock>();
    subtitle->SetText(L"Using Framework Window Class");
    subtitle->SetFontSize(14);
    subtitle->SetForeground(Color::FromHex(0x666666));
    rootPanel->AddChild(subtitle);
    
    // ========== 按钮区域 ==========
    auto buttonLabel = std::make_shared<TextBlock>();
    buttonLabel->SetText(L"Buttons:");
    buttonLabel->SetFontSize(12);
    buttonLabel->SetForeground(Color::FromHex(0x999999));
    rootPanel->AddChild(buttonLabel);
    
    auto buttonPanel = std::make_shared<StackPanel>();
    buttonPanel->SetOrientation(StackPanel::Orientation::Horizontal);
    buttonPanel->SetSpacing(10);
    
    // 默认按钮
    auto btn1 = std::make_shared<Button>();
    btn1->SetStateColors(
        Color::FromHex(0x2196F3),  // Normal
        Color::FromHex(0x1976D2),  // Hover
        Color::FromHex(0x0D47A1)   // Pressed
    );
    btn1->Click.Add([this](Control*) {
        m_statusText->SetText(L"Blue button clicked!");
        double newValue = m_progressBar->GetValue() + 10;
        if (newValue > 100) newValue = 0;
        m_progressBar->SetValue(newValue);
        if (m_slider) m_slider->SetValue(newValue);
    });
    buttonPanel->AddChild(btn1);
    
    // 成功按钮
    auto btn2 = std::make_shared<Button>();
    btn2->SetStateColors(
        Color::FromHex(0x4CAF50),  // Normal
        Color::FromHex(0x388E3C),  // Hover
        Color::FromHex(0x1B5E20)   // Pressed
    );
    btn2->Click.Add([this](Control*) {
        m_statusText->SetText(L"Green button clicked!");
        double newValue = m_progressBar->GetValue() - 10;
        if (newValue < 0) newValue = 100;
        m_progressBar->SetValue(newValue);
        if (m_slider) m_slider->SetValue(newValue);
    });
    buttonPanel->AddChild(btn2);
    
    rootPanel->AddChild(buttonPanel);
    
    // ========== 输入区域 ==========
    auto inputLabel = std::make_shared<TextBlock>();
    inputLabel->SetText(L"Input Controls:");
    inputLabel->SetFontSize(12);
    inputLabel->SetForeground(Color::FromHex(0x999999));
    rootPanel->AddChild(inputLabel);
    
    // 文本框
    m_textBox = std::make_shared<TextBox>();
    rootPanel->AddChild(m_textBox);
    
    // 复选框
    m_checkBox = std::make_shared<CheckBox>();
    m_checkBox->SetText(L"Enable Feature");
    m_checkBox->CheckedChanged.Add([this](CheckBox*, bool isChecked) {
        std::wstring text = isChecked ? L"CheckBox checked!" : L"CheckBox unchecked!";
        m_statusText->SetText(text);
    });
    rootPanel->AddChild(m_checkBox);
    
    // ========== 滑块和进度条 ==========
    auto valueLabel = std::make_shared<TextBlock>();
    valueLabel->SetText(L"Value Controls:");
    valueLabel->SetFontSize(12);
    valueLabel->SetForeground(Color::FromHex(0x999999));
    rootPanel->AddChild(valueLabel);
    
    // 滑块
    m_slider = std::make_shared<Slider>();
    m_slider->SetValue(50);
    m_slider->ValueChanged.Add([this](Slider*, double value) {
        m_progressBar->SetValue(value);
        std::wstringstream ss;
        ss << L"Slider value: " << static_cast<int>(value);
        m_statusText->SetText(ss.str());
    });
    rootPanel->AddChild(m_slider);
    
    // 进度条
    m_progressBar = std::make_shared<ProgressBar>();
    m_progressBar->SetValue(50);
    rootPanel->AddChild(m_progressBar);
    
    // ========== Border 容器示例 ==========
    auto borderLabel = std::make_shared<TextBlock>();
    borderLabel->SetText(L"Border Container:");
    borderLabel->SetFontSize(12);
    borderLabel->SetForeground(Color::FromHex(0x999999));
    rootPanel->AddChild(borderLabel);
    
    auto border = std::make_shared<Border>();
    border->SetBackground(Color::White());
    border->SetBorderColor(Color::FromHex(0xCCCCCC));
    border->SetBorderThickness(2);
    if (auto* layout = border->GetLayout()) {
        layout->SetHeight(80);
    }
    
    auto borderContent = std::make_shared<TextBlock>();
    borderContent->SetText(L"Content inside Border");
    borderContent->SetFontSize(14);
    border->AddChild(borderContent);
    
    rootPanel->AddChild(border);
    
    // ========== 形状展示 ==========
    auto shapesLabel = std::make_shared<TextBlock>();
    shapesLabel->SetText(L"Shapes:");
    shapesLabel->SetFontSize(12);
    shapesLabel->SetForeground(Color::FromHex(0x999999));
    rootPanel->AddChild(shapesLabel);
    
    auto shapesPanel = std::make_shared<StackPanel>();
    shapesPanel->SetOrientation(StackPanel::Orientation::Horizontal);
    shapesPanel->SetSpacing(10);
    if (auto* layout = shapesPanel->GetLayout()) {
        layout->SetHeight(70);
    }
    
    auto rect = std::make_shared<luaui::controls::Rectangle>();
    if (auto* layout = rect->GetLayout()) {
        layout->SetWidth(60);
        layout->SetHeight(60);
    }
    rect->SetFill(Color::FromHex(0x4CAF50));
    shapesPanel->AddChild(rect);
    
    auto ellipse = std::make_shared<luaui::controls::Ellipse>();
    if (auto* layout = ellipse->GetLayout()) {
        layout->SetWidth(60);
        layout->SetHeight(60);
    }
    ellipse->SetFill(Color::FromHex(0x2196F3));
    shapesPanel->AddChild(ellipse);
    
    rootPanel->AddChild(shapesPanel);
    
    // ========== 状态栏 ==========
    m_statusText = std::make_shared<TextBlock>();
    m_statusText->SetText(L"Ready - Framework Window Demo");
    m_statusText->SetFontSize(12);
    m_statusText->SetForeground(Color::FromHex(0x666666));
    rootPanel->AddChild(m_statusText);
    
    // 设置根控件
    SetRoot(rootPanel);
    
    Logger::Info("Controls created and root set");
}

// ============================================================================
// 入口点
// ============================================================================

int main() {
    // 获取实例句柄
    HINSTANCE hInstance = GetModuleHandle(nullptr);
    
    // 初始化日志
    Logger::Initialize();
    Logger::SetConsoleLevel(LogLevel::Debug);
    
    try {
        // 创建并运行窗口
        ControlsDemoWindow window;
        
        if (!window.Create(hInstance, L"LuaUI Controls Demo - Framework Window", 900, 700)) {
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
