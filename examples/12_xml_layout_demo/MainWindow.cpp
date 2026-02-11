// XML Layout Demo - MainWindow implementation

#include "MainWindow.h"
#include <iostream>

using namespace luaui;
using namespace luaui::controls;

MainWindow::MainWindow() {
    // Initialize components
}

MainWindow::~MainWindow() {
}

void MainWindow::OnLoaded() {
    // Create the UI
    auto root = CreateContent();
    SetRoot(root);
    
    // Bind events
    BindEvents();
}

std::shared_ptr<Control> MainWindow::CreateContent() {
    // Create a simple StackPanel layout
    auto content = std::make_shared<StackPanel>();
    content->SetName("content");
    content->SetOrientation(StackPanel::Orientation::Vertical);
    content->SetSpacing(20);
    
    // Header
    auto header = std::make_shared<TextBlock>();
    header->SetText(L"XML Layout Demo - New Architecture");
    header->SetFontSize(24);
    content->AddChild(header);
    
    // Subtitle
    auto subtitle = std::make_shared<TextBlock>();
    subtitle->SetText(L"Using Architecture with Component Pattern");
    subtitle->SetFontSize(14);
    content->AddChild(subtitle);
    
    // Button section
    auto buttonRow = std::make_shared<StackPanel>();
    buttonRow->SetOrientation(StackPanel::Orientation::Horizontal);
    buttonRow->SetSpacing(10);
    
    auto primaryBtn = std::make_shared<Button>();
    primaryBtn->SetName("primaryBtn");
    buttonRow->AddChild(primaryBtn);
    
    auto secondaryBtn = std::make_shared<Button>();
    secondaryBtn->SetName("secondaryBtn");
    buttonRow->AddChild(secondaryBtn);
    
    content->AddChild(buttonRow);
    
    // Slider section
    auto sliderLabel = std::make_shared<TextBlock>();
    sliderLabel->SetText(L"Slider Value: 50");
    sliderLabel->SetName("sliderLabel");
    content->AddChild(sliderLabel);
    
    auto slider = std::make_shared<Slider>();
    slider->SetName("mainSlider");
    slider->SetValue(50);
    content->AddChild(slider);
    
    // Progress section
    auto progressRow = std::make_shared<StackPanel>();
    progressRow->SetOrientation(StackPanel::Orientation::Horizontal);
    progressRow->SetSpacing(10);
    
    auto progressLabel = std::make_shared<TextBlock>();
    progressLabel->SetText(L"Progress:");
    progressRow->AddChild(progressLabel);
    
    auto progressBar = std::make_shared<ProgressBar>();
    progressBar->SetName("progressBar");
    progressBar->SetValue(65);
    progressRow->AddChild(progressBar);
    
    content->AddChild(progressRow);
    
    // Store references for event binding
    m_primaryBtn = primaryBtn;
    m_secondaryBtn = secondaryBtn;
    m_slider = slider;
    m_progressBar = progressBar;
    m_sliderLabel = sliderLabel;
    
    return content;
}

void MainWindow::BindEvents() {
    // Bind events using stored references
    
    if (m_primaryBtn) {
        m_primaryBtn->Click.Add([this](Control*) {
            std::cout << "Primary button clicked!" << std::endl;
            
            // Update progress bar
            if (m_progressBar) {
                double current = m_progressBar->GetValue();
                m_progressBar->SetValue(std::min(100.0, current + 10.0));
            }
        });
    }
    
    if (m_secondaryBtn) {
        m_secondaryBtn->Click.Add([this](Control*) {
            std::cout << "Secondary button clicked!" << std::endl;
        });
    }
    
    if (m_slider) {
        m_slider->ValueChanged.Add([this](Slider*, double value) {
            std::cout << "Slider value: " << value << std::endl;
            
            // Update label
            if (m_sliderLabel) {
                m_sliderLabel->SetText(L"Value: " + std::to_wstring((int)value));
            }
        });
    }
}
