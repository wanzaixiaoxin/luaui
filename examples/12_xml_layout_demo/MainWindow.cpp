#include "MainWindow.h"
#include <iostream>

using namespace luaui;
using namespace luaui::controls;

MainWindow::MainWindow() {
}

MainWindow::~MainWindow() = default;

void MainWindow::OnLoaded() {
    // 加载布局
    LoadLayout();
    
    // 绑定事件 - 使用新的Delegate API
    BindEvents();
}

void MainWindow::LoadLayout() {
    // 创建主布局
    auto root = std::make_shared<StackPanel>();
    root->SetOrientation(StackPanel::Orientation::Vertical);
    
    // 1. 标题栏
    auto titleBar = std::make_shared<StackPanel>();
    titleBar->SetOrientation(StackPanel::Orientation::Horizontal);
    titleBar->SetPadding(20, 10, 20, 10);
    
    auto titleLabel = std::make_shared<TextBlock>();
    titleLabel->SetText(L"LuaUI Demo");
    titleBar->AddChild(titleLabel);
    
    root->AddChild(titleBar);
    
    // 2. 主内容区
    auto contentArea = std::make_shared<StackPanel>();
    contentArea->SetOrientation(StackPanel::Orientation::Horizontal);
    
    // 2.1 侧边栏
    auto sidebar = CreateSidebar();
    sidebar->SetName("sidebar");
    contentArea->AddChild(sidebar);
    
    // 2.2 内容区
    auto mainContent = CreateMainContent();
    mainContent->SetName("mainContent");
    contentArea->AddChild(mainContent);
    
    root->AddChild(contentArea);
    
    SetRoot(root);
}

std::shared_ptr<Panel> MainWindow::CreateSidebar() {
    auto sidebar = std::make_shared<StackPanel>();
    sidebar->SetOrientation(StackPanel::Orientation::Vertical);
    sidebar->SetWidth(200);
    sidebar->SetPadding(10, 10, 10, 10);
    
    // 导航按钮
    const char* navItems[] = {"Controls", "Layout", "Styling", "Animation"};
    for (const auto& item : navItems) {
        auto btn = std::make_shared<Button>();
        btn->SetName(item);
        btn->SetHeight(40);
        btn->SetMargin(0, 0, 0, 5);
        sidebar->AddChild(btn);
    }
    
    return sidebar;
}

std::shared_ptr<Panel> MainWindow::CreateMainContent() {
    auto content = std::make_shared<StackPanel>();
    content->SetOrientation(StackPanel::Orientation::Vertical);
    content->SetPadding(20, 20, 20, 20);
    
    // 控件展示区
    auto section = std::make_shared<StackPanel>();
    section->SetOrientation(StackPanel::Orientation::Vertical);
    section->SetPadding(20, 20, 20, 20);
    section->SetMargin(0, 0, 0, 20);
    
    // 标题
    auto sectionTitle = std::make_shared<TextBlock>();
    sectionTitle->SetText(L"Interactive Controls");
    sectionTitle->SetMargin(0, 0, 0, 15);
    section->AddChild(sectionTitle);
    
    // 按钮示例
    auto btnRow = std::make_shared<StackPanel>();
    btnRow->SetOrientation(StackPanel::Orientation::Horizontal);
    
    auto primaryBtn = std::make_shared<Button>();
    primaryBtn->SetName("primaryBtn");
    primaryBtn->SetWidth(120);
    primaryBtn->SetHeight(36);
    btnRow->AddChild(primaryBtn);
    
    auto secondaryBtn = std::make_shared<Button>();
    secondaryBtn->SetName("secondaryBtn");
    secondaryBtn->SetWidth(100);
    secondaryBtn->SetHeight(36);
    secondaryBtn->SetMargin(10, 0, 0, 0);
    btnRow->AddChild(secondaryBtn);
    
    section->AddChild(btnRow);
    
    // Slider 示例
    auto sliderRow = std::make_shared<StackPanel>();
    sliderRow->SetOrientation(StackPanel::Orientation::Horizontal);
    sliderRow->SetMargin(0, 15, 0, 0);
    
    auto sliderLabel = std::make_shared<TextBlock>();
    sliderLabel->SetText(L"Value: 50");
    sliderLabel->SetWidth(80);
    sliderLabel->SetName("sliderLabel");
    sliderRow->AddChild(sliderLabel);
    
    auto slider = std::make_shared<Slider>();
    slider->SetWidth(200);
    slider->SetHeight(20);
    slider->SetValue(50);
    slider->SetName("mainSlider");
    sliderRow->AddChild(slider);
    
    section->AddChild(sliderRow);
    
    // 进度条示例
    auto progressRow = std::make_shared<StackPanel>();
    progressRow->SetOrientation(StackPanel::Orientation::Horizontal);
    progressRow->SetMargin(0, 15, 0, 0);
    
    auto progressLabel = std::make_shared<TextBlock>();
    progressLabel->SetText(L"Progress:");
    progressLabel->SetWidth(80);
    progressRow->AddChild(progressLabel);
    
    auto progressBar = std::make_shared<ProgressBar>();
    progressBar->SetWidth(200);
    progressBar->SetHeight(8);
    progressBar->SetValue(65);
    progressBar->SetName("progressBar");
    progressRow->AddChild(progressBar);
    
    section->AddChild(progressRow);
    
    content->AddChild(section);
    
    return content;
}

void MainWindow::BindEvents() {
    // 使用新的Delegate API绑定事件
    
    auto primaryBtn = FindControl<Button>("primaryBtn");
    if (primaryBtn) {
        primaryBtn->Click.Add(this, &MainWindow::OnPrimaryClicked);
    }
    
    auto secondaryBtn = FindControl<Button>("secondaryBtn");
    if (secondaryBtn) {
        secondaryBtn->Click.Add(this, &MainWindow::OnSecondaryClicked);
    }
    
    // Lambda示例 - Delegate支持lambda
    auto navBtn = FindControl<Button>("Controls");
    if (navBtn) {
        navBtn->Click.Add([this](Control*) {
            std::cout << "Navigation clicked (lambda)" << std::endl;
        });
    }
    
    // 查找Slider并绑定事件
    auto slider = FindControl<Slider>("mainSlider");
    if (slider) {
        slider->ValueChanged.Add(this, &MainWindow::OnSliderValueChanged);
    }
}

// 事件处理函数
void MainWindow::OnPrimaryClicked(Control* sender) {
    std::cout << "Primary button clicked!" << std::endl;
    
    // 更新进度条作为反馈
    auto progressBar = FindControl<ProgressBar>("progressBar");
    if (progressBar) {
        double current = progressBar->GetValue();
        progressBar->SetValue(std::min(100.0, current + 10.0));
    }
}

void MainWindow::OnSecondaryClicked(Control* sender) {
    std::cout << "Secondary button clicked!" << std::endl;
}

void MainWindow::OnSliderValueChanged(Slider* sender, double value) {
    std::cout << "Slider value changed: " << value << std::endl;
    
    // 更新标签显示
    auto label = FindControl<TextBlock>("sliderLabel");
    if (label) {
        label->SetText(L"Value: " + std::to_wstring((int)value));
    }
}
