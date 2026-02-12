// XML Layout Demo - MainWindow implementation
// 从 XML 文件加载布局

#include "MainWindow.h"
#include "Logger.h"
#include "XmlLayout.h"
#include <windows.h>
#include <filesystem>

using namespace luaui;
using namespace luaui::controls;
using namespace luaui::utils;

MainWindow::MainWindow() {
}

MainWindow::~MainWindow() {
}

void MainWindow::OnLoaded() {
    // 从 XML 文件加载布局
    auto root = LoadLayoutFromXml();
    if (!root) {
        Logger::Error("Failed to load XML layout, falling back to code");
        root = CreateFallbackContent();
    }
    
    SetRoot(root);
    
    // 查找命名控件
    FindNamedControls();
    
    // 绑定事件
    BindEvents();
}

std::shared_ptr<Control> MainWindow::LoadLayoutFromXml() {
    // 尝试多个可能的路径
    std::vector<std::string> possiblePaths = {
        "layouts/main_window.xml",
        "../layouts/main_window.xml",
        "../../examples/12_xml_layout_demo/layouts/main_window.xml",
        "examples/12_xml_layout_demo/layouts/main_window.xml"
    };
    
    std::string foundPath;
    for (const auto& path : possiblePaths) {
        if (std::filesystem::exists(path)) {
            foundPath = path;
            Logger::InfoF("Found XML layout at: %s", path.c_str());
            break;
        }
    }
    
    if (foundPath.empty()) {
        Logger::Error("Could not find main_window.xml in any search path");
        return nullptr;
    }
    
    try {
        auto loader = xml::CreateXmlLoader();
        auto root = loader->Load(foundPath);
        Logger::Info("XML layout loaded successfully");
        return root;
    } catch (const std::exception& e) {
        Logger::ErrorF("XML load error: %s", e.what());
        return nullptr;
    }
}

void MainWindow::FindNamedControls() {
    auto root = GetRoot();
    if (!root) return;
    
    // 递归查找命名控件
    std::function<void(const std::shared_ptr<interfaces::IControl>&)> findNamed = [&](const std::shared_ptr<interfaces::IControl>& control) {
        if (!control) return;
        
        std::string name = control->GetName();
        
        if (name == "newBtn") {
            m_newBtn = std::dynamic_pointer_cast<Button>(control);
        } else if (name == "openBtn") {
            m_openBtn = std::dynamic_pointer_cast<Button>(control);
        } else if (name == "saveBtn") {
            m_saveBtn = std::dynamic_pointer_cast<Button>(control);
        } else if (name == "searchBtn") {
            m_searchBtn = std::dynamic_pointer_cast<Button>(control);
        } else if (name == "submitBtn") {
            m_submitBtn = std::dynamic_pointer_cast<Button>(control);
        } else if (name == "cancelBtn") {
            m_cancelBtn = std::dynamic_pointer_cast<Button>(control);
        } else if (name == "resetBtn") {
            m_resetBtn = std::dynamic_pointer_cast<Button>(control);
        } else if (name == "navHomeBtn") {
            m_navHomeBtn = std::dynamic_pointer_cast<Button>(control);
        } else if (name == "navProfileBtn") {
            m_navProfileBtn = std::dynamic_pointer_cast<Button>(control);
        } else if (name == "settingsBtn") {
            m_settingsBtn = std::dynamic_pointer_cast<Button>(control);
        } else if (name == "volumeSlider") {
            m_volumeSlider = std::dynamic_pointer_cast<Slider>(control);
        } else if (name == "profileProgress") {
            m_profileProgress = std::dynamic_pointer_cast<ProgressBar>(control);
        } else if (name == "usernameBox") {
            m_usernameBox = std::dynamic_pointer_cast<TextBox>(control);
        } else if (name == "emailBox") {
            m_emailBox = std::dynamic_pointer_cast<TextBox>(control);
        } else if (name == "bioBox") {
            m_bioBox = std::dynamic_pointer_cast<TextBox>(control);
        } else if (name == "searchBox") {
            m_searchBox = std::dynamic_pointer_cast<TextBox>(control);
        } else if (name == "statusText") {
            m_statusText = std::dynamic_pointer_cast<TextBlock>(control);
        } else if (name == "progressPercentText") {
            m_progressPercentText = std::dynamic_pointer_cast<TextBlock>(control);
        }
        
        // 递归查找子控件
        if (auto* panel = dynamic_cast<Panel*>(control.get())) {
            for (auto& child : panel->GetChildren()) {
                findNamed(child);
            }
        }
    };
    
    findNamed(root);
    
    Logger::DebugF("Found named controls: newBtn=%p, saveBtn=%p, volumeSlider=%p", 
        (void*)m_newBtn.get(), (void*)m_saveBtn.get(), (void*)m_volumeSlider.get());
}

void MainWindow::BindEvents() {
    // New button
    if (m_newBtn) {
        m_newBtn->Click.Add([this](Control*) {
            Logger::Info("New button clicked!");
            UpdateStatus(L"Creating new document...");
            if (m_usernameBox) m_usernameBox->SetText(L"");
            if (m_emailBox) m_emailBox->SetText(L"");
            if (m_bioBox) m_bioBox->SetText(L"");
        });
    }
    
    // Open button
    if (m_openBtn) {
        m_openBtn->Click.Add([this](Control*) {
            Logger::Info("Open button clicked!");
            UpdateStatus(L"Opening file...");
        });
    }
    
    // Save button
    if (m_saveBtn) {
        m_saveBtn->Click.Add([this](Control*) {
            Logger::Info("Save button clicked!");
            std::wstring username = m_usernameBox ? m_usernameBox->GetText() : L"";
            UpdateStatus(L"Saved: " + username);
            if (m_profileProgress) m_profileProgress->SetValue(100);
            UpdateProgressText();
        });
    }
    
    // Search button
    if (m_searchBtn) {
        m_searchBtn->Click.Add([this](Control*) {
            std::wstring query = m_searchBox ? m_searchBox->GetText() : L"";
            Logger::InfoF("Search: %S", query.c_str());
            UpdateStatus(L"Searching for: " + query);
        });
    }
    
    // Submit button
    if (m_submitBtn) {
        m_submitBtn->Click.Add([this](Control*) {
            Logger::Info("Submit button clicked!");
            UpdateStatus(L"Changes saved successfully!");
            if (m_profileProgress) m_profileProgress->SetValue(100);
            UpdateProgressText();
        });
    }
    
    // Cancel button
    if (m_cancelBtn) {
        m_cancelBtn->Click.Add([this](Control*) {
            Logger::Info("Cancel button clicked!");
            UpdateStatus(L"Operation cancelled");
        });
    }
    
    // Reset button
    if (m_resetBtn) {
        m_resetBtn->Click.Add([this](Control*) {
            Logger::Info("Reset button clicked!");
            if (m_usernameBox) m_usernameBox->SetText(L"");
            if (m_emailBox) m_emailBox->SetText(L"");
            if (m_bioBox) m_bioBox->SetText(L"");
            if (m_profileProgress) m_profileProgress->SetValue(0);
            UpdateProgressText();
            UpdateStatus(L"Form reset");
        });
    }
    
    // Navigation buttons
    if (m_navHomeBtn) {
        m_navHomeBtn->Click.Add([this](Control*) {
            Logger::Info("Home navigation clicked");
            UpdateStatus(L"Navigated to Home");
        });
    }
    
    if (m_navProfileBtn) {
        m_navProfileBtn->Click.Add([this](Control*) {
            Logger::Info("Profile navigation clicked");
            UpdateStatus(L"Navigated to Profile");
        });
    }
    
    if (m_settingsBtn) {
        m_settingsBtn->Click.Add([this](Control*) {
            Logger::Info("Settings button clicked");
            UpdateStatus(L"Opening settings...");
        });
    }
    
    // Volume slider
    if (m_volumeSlider) {
        m_volumeSlider->ValueChanged.Add([this](Slider*, double value) {
            Logger::InfoF("Volume changed: %.0f", value);
            UpdateStatus(L"Volume: " + std::to_wstring((int)value) + L"%");
        });
    }
}

void MainWindow::UpdateStatus(const std::wstring& message) {
    if (m_statusText) {
        m_statusText->SetText(message);
    }
    Logger::InfoF("Status: %S", message.c_str());
}

void MainWindow::UpdateProgressText() {
    if (m_progressPercentText && m_profileProgress) {
        int value = (int)m_profileProgress->GetValue();
        m_progressPercentText->SetText(std::to_wstring(value) + L"%");
    }
}

// Fallback content if XML loading fails
std::shared_ptr<Control> MainWindow::CreateFallbackContent() {
    auto content = std::make_shared<StackPanel>();
    content->SetName("content");
    content->SetOrientation(StackPanel::Orientation::Vertical);
    content->SetSpacing(20);
    
    auto header = std::make_shared<TextBlock>();
    header->SetText(L"XML Layout Demo - Fallback Mode");
    header->SetFontSize(24);
    content->AddChild(header);
    
    auto subtitle = std::make_shared<TextBlock>();
    subtitle->SetText(L"(XML file not found or failed to load)");
    subtitle->SetFontSize(14);
    subtitle->SetForeground(rendering::Color(0.5f, 0.5f, 0.5f, 1.0f));
    content->AddChild(subtitle);
    
    auto buttonRow = std::make_shared<StackPanel>();
    buttonRow->SetOrientation(StackPanel::Orientation::Horizontal);
    buttonRow->SetSpacing(10);
    
    m_newBtn = std::make_shared<Button>();
    m_newBtn->SetName("newBtn");
    buttonRow->AddChild(m_newBtn);
    
    m_saveBtn = std::make_shared<Button>();
    m_saveBtn->SetName("saveBtn");
    buttonRow->AddChild(m_saveBtn);
    
    content->AddChild(buttonRow);
    
    m_volumeSlider = std::make_shared<Slider>();
    m_volumeSlider->SetName("volumeSlider");
    m_volumeSlider->SetValue(50);
    content->AddChild(m_volumeSlider);
    
    m_profileProgress = std::make_shared<ProgressBar>();
    m_profileProgress->SetName("profileProgress");
    m_profileProgress->SetValue(65);
    content->AddChild(m_profileProgress);
    
    return content;
}
