#include "MainWindow.h"
#include "Logger.h"
#include "Event.h"
#include <functional>

using namespace luaui;
using namespace luaui::controls;
using namespace luaui::xml;
using luaui::utils::Logger;

namespace demo {

MainWindow::MainWindow() = default;
MainWindow::~MainWindow() = default;

bool MainWindow::LoadLayout(const std::string& xmlPath) {
    try {
        // Create XML loader
        m_xmlLoader = luaui::xml::CreateXmlLoader();
        if (!m_xmlLoader) {
            Logger::Error("Failed to create XML loader");
            return false;
        }
        
        // Load XML layout
        auto root = m_xmlLoader->Load(xmlPath);
        if (!root) {
            Logger::Error("Failed to load layout: " + xmlPath);
            return false;
        }
    
    // Bind named controls - need to traverse and find named controls
    std::function<void(const luaui::controls::ControlPtr&)> bindControls = [this, &bindControls](const luaui::controls::ControlPtr& ctrl) {
        if (!ctrl) return;
        
        const std::string& name = ctrl->GetName();
        if (!name.empty()) {
            // Bind named control
            luaui::controls::Control* c = ctrl.get();
            if (name == "newBtn") m_newBtn = dynamic_cast<Button*>(c);
            else if (name == "openBtn") m_openBtn = dynamic_cast<Button*>(c);
            else if (name == "saveBtn") m_saveBtn = dynamic_cast<Button*>(c);
            else if (name == "searchBtn") m_searchBtn = dynamic_cast<Button*>(c);
            else if (name == "submitBtn") m_submitBtn = dynamic_cast<Button*>(c);
            else if (name == "cancelBtn") m_cancelBtn = dynamic_cast<Button*>(c);
            else if (name == "resetBtn") m_resetBtn = dynamic_cast<Button*>(c);
            else if (name == "settingsBtn") m_settingsBtn = dynamic_cast<Button*>(c);
            else if (name == "navHomeBtn") m_navHomeBtn = dynamic_cast<Button*>(c);
            else if (name == "navProfileBtn") m_navProfileBtn = dynamic_cast<Button*>(c);
            else if (name == "navMessagesBtn") m_navMessagesBtn = dynamic_cast<Button*>(c);
            else if (name == "navSettingsBtn") m_navSettingsBtn = dynamic_cast<Button*>(c);
            else if (name == "usernameBox") m_usernameBox = dynamic_cast<TextBox*>(c);
            else if (name == "emailBox") m_emailBox = dynamic_cast<TextBox*>(c);
            else if (name == "bioBox") m_bioBox = dynamic_cast<TextBox*>(c);
            else if (name == "searchBox") m_searchBox = dynamic_cast<TextBox*>(c);
            else if (name == "volumeSlider") m_volumeSlider = dynamic_cast<Slider*>(c);
            else if (name == "profileProgress") m_profileProgress = dynamic_cast<ProgressBar*>(c);
            else if (name == "statusText") m_statusText = dynamic_cast<TextBlock*>(c);
            else if (name == "progressPercentText") m_progressPercentText = dynamic_cast<TextBlock*>(c);
        }
        
        // Recurse into children
        for (size_t i = 0; i < ctrl->GetChildCount(); ++i) {
            bindControls(ctrl->GetChild(i));
        }
        
        // Check ContentControl content (Border, etc.)
        if (auto contentCtrl = std::dynamic_pointer_cast<luaui::controls::ContentControl>(ctrl)) {
            bindControls(contentCtrl->GetContent());
        }
    };
    
    bindControls(root);
    
    // Set as root - OnLoaded() will be called automatically
    SetRoot(root);
    
    return true;
    } catch (const std::exception& e) {
        Logger::Error(std::string("LoadLayout exception: ") + e.what());
        return false;
    }
}

void MainWindow::OnLoaded() {
    // Bind button click events
    if (m_newBtn) {
        m_newBtn->AddClickHandler([this](Control* c) { OnNewClicked(c); });
    }
    if (m_openBtn) {
        m_openBtn->AddClickHandler([this](Control* c) { OnOpenClicked(c); });
    }
    if (m_saveBtn) {
        m_saveBtn->AddClickHandler([this](Control* c) { OnSaveClicked(c); });
    }
    if (m_searchBtn) {
        m_searchBtn->AddClickHandler([this](Control* c) { OnSearchClicked(c); });
    }
    if (m_submitBtn) {
        m_submitBtn->AddClickHandler([this](Control* c) { OnSubmitClicked(c); });
    }
    if (m_cancelBtn) {
        m_cancelBtn->AddClickHandler([this](Control* c) { OnCancelClicked(c); });
    }
    if (m_resetBtn) {
        m_resetBtn->AddClickHandler([this](Control* c) { OnResetClicked(c); });
    }
    if (m_settingsBtn) {
        m_settingsBtn->AddClickHandler([this](Control* c) { OnSettingsClicked(c); });
    }
    
    // Navigation buttons
    if (m_navHomeBtn) {
        m_navHomeBtn->AddClickHandler([this](Control* c) { OnNavHomeClicked(c); });
    }
    if (m_navProfileBtn) {
        m_navProfileBtn->AddClickHandler([this](Control* c) { OnNavProfileClicked(c); });
    }
    if (m_navMessagesBtn) {
        m_navMessagesBtn->AddClickHandler([this](Control* c) { OnNavMessagesClicked(c); });
    }
    if (m_navSettingsBtn) {
        m_navSettingsBtn->AddClickHandler([this](Control* c) { OnNavSettingsClicked(c); });
    }
    
    // Slider event
    if (m_volumeSlider) {
        m_volumeSlider->SetValueChangedHandler([this](Slider* s, float val) {
            OnVolumeChanged(s, val);
        });
    }
    
    // TextBox events
    if (m_usernameBox) {
        m_usernameBox->SetTextChangedHandler([this](TextBox* tb, const std::wstring& text) {
            OnTextChanged(tb, text);
        });
    }
    if (m_emailBox) {
        m_emailBox->SetTextChangedHandler([this](TextBox* tb, const std::wstring& text) {
            OnTextChanged(tb, text);
        });
    }
    if (m_bioBox) {
        m_bioBox->SetTextChangedHandler([this](TextBox* tb, const std::wstring& text) {
            OnTextChanged(tb, text);
        });
    }
    
    // Set initial status
    UpdateStatus(L"Application loaded. Ready.");
}

void MainWindow::OnClosing() {
}

// Event handler implementations
void MainWindow::OnNewClicked(luaui::controls::Control* sender) {
    Logger::Info("New button clicked");
    UpdateStatus(L"Creating new document...");
}

void MainWindow::OnOpenClicked(luaui::controls::Control* sender) {
    Logger::Info("Open button clicked");
    UpdateStatus(L"Opening file...");
}

void MainWindow::OnSaveClicked(luaui::controls::Control* sender) {
    Logger::Info("Save button clicked");
    UpdateStatus(L"Saving document...");
}

void MainWindow::OnSearchClicked(luaui::controls::Control* sender) {
    Logger::Info("Search button clicked");
    if (m_searchBox && !m_searchBox->GetText().empty()) {
        std::wstring msg = L"Searching for: " + m_searchBox->GetText();
        UpdateStatus(msg);
    } else {
        UpdateStatus(L"Please enter search term");
    }
}

void MainWindow::OnSubmitClicked(luaui::controls::Control* sender) {
    Logger::Info("Submit button clicked");
    
    std::wstring username = m_usernameBox ? m_usernameBox->GetText() : L"";
    std::wstring email = m_emailBox ? m_emailBox->GetText() : L"";
    
    if (username.empty() || email.empty()) {
        UpdateStatus(L"Please fill in required fields");
        return;
    }
    
    UpdateStatus(L"Form submitted successfully!");
    UpdateProgress();
}

void MainWindow::OnCancelClicked(luaui::controls::Control* sender) {
    Logger::Info("Cancel button clicked");
    UpdateStatus(L"Operation cancelled");
}

void MainWindow::OnResetClicked(luaui::controls::Control* sender) {
    Logger::Info("Reset button clicked");
    
    if (m_usernameBox) m_usernameBox->SetText(L"");
    if (m_emailBox) m_emailBox->SetText(L"");
    if (m_bioBox) m_bioBox->SetText(L"");
    if (m_searchBox) m_searchBox->SetText(L"");
    
    UpdateStatus(L"Form reset");
}

void MainWindow::OnSettingsClicked(luaui::controls::Control* sender) {
    Logger::Info("Settings button clicked");
    UpdateStatus(L"Opening settings...");
}

void MainWindow::OnNavHomeClicked(luaui::controls::Control* sender) {
    Logger::Info("Home navigation clicked");
    UpdateStatus(L"Navigating to Home");
}

void MainWindow::OnNavProfileClicked(luaui::controls::Control* sender) {
    Logger::Info("Profile navigation clicked");
    UpdateStatus(L"Navigating to Profile");
}

void MainWindow::OnNavMessagesClicked(luaui::controls::Control* sender) {
    Logger::Info("Messages navigation clicked");
    UpdateStatus(L"Navigating to Messages");
}

void MainWindow::OnNavSettingsClicked(luaui::controls::Control* sender) {
    Logger::Info("Settings navigation clicked");
    UpdateStatus(L"Navigating to Settings");
}

void MainWindow::OnVolumeChanged(Slider* sender, float value) {
    std::wstring msg = L"Volume changed to: " + std::to_wstring((int)(value * 100)) + L"%";
    UpdateStatus(msg);
}

void MainWindow::OnTextChanged(TextBox* sender, const std::wstring& text) {
    // Optional: Update UI based on text changes
    UpdateProgress();
}

void MainWindow::UpdateStatus(const std::wstring& message) {
    if (m_statusText) {
        m_statusText->SetText(message);
    }
    Logger::Info(std::string(message.begin(), message.end()));
}

void MainWindow::UpdateProgress() {
    if (!m_profileProgress) return;
    
    // Calculate profile completion
    float progress = 0.0f;
    int fields = 0;
    
    if (m_usernameBox && !m_usernameBox->GetText().empty()) { progress += 0.3f; fields++; }
    if (m_emailBox && !m_emailBox->GetText().empty()) { progress += 0.3f; fields++; }
    if (m_bioBox && !m_bioBox->GetText().empty()) { progress += 0.4f; fields++; }
    
    m_profileProgress->SetValue(progress);
    
    if (m_progressPercentText) {
        int percent = (int)(progress * 100);
        m_progressPercentText->SetText(std::to_wstring(percent) + L"%");
    }
}

} // namespace demo
