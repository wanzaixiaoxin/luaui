#pragma once

#include "Window.h"
#include "Control.h"
#include "layout.h"
#include "RangeControls.h"
#include "XmlLayout.h"
#include <unordered_map>
#include <functional>

// Forward declaration for xml namespace types
namespace luaui { namespace xml { class IXmlLoader; using IXmlLoaderPtr = std::shared_ptr<IXmlLoader>; } }

namespace demo {

/**
 * @brief MainWindow - Simplified window class using LuaUI framework
 * 
 * All message handling is done by the framework. Just override OnLoaded()
 * to setup your UI and bind events.
 */
class MainWindow : public luaui::Window {
public:
    MainWindow();
    ~MainWindow();

    // Initialize with XML layout
    bool LoadLayout(const std::string& xmlPath);

protected:
    // Called after window is created and root is set
    void OnLoaded() override;
    
    // Called when window is closing
    void OnClosing() override;

private:
    // Control binding helpers
    template<typename T>
    void BindControl(const std::string& name, T** ptr) {
        *ptr = FindControl<T>(name);
    }
    
    // Event handlers
    void OnNewClicked(luaui::controls::Control* sender);
    void OnOpenClicked(luaui::controls::Control* sender);
    void OnSaveClicked(luaui::controls::Control* sender);
    void OnSearchClicked(luaui::controls::Control* sender);
    void OnSubmitClicked(luaui::controls::Control* sender);
    void OnCancelClicked(luaui::controls::Control* sender);
    void OnResetClicked(luaui::controls::Control* sender);
    void OnSettingsClicked(luaui::controls::Control* sender);
    void OnNavHomeClicked(luaui::controls::Control* sender);
    void OnNavProfileClicked(luaui::controls::Control* sender);
    void OnNavMessagesClicked(luaui::controls::Control* sender);
    void OnNavSettingsClicked(luaui::controls::Control* sender);
    
    void OnVolumeChanged(luaui::controls::Slider* sender, float value);
    void OnTextChanged(luaui::controls::TextBox* sender, const std::wstring& text);
    
    // Update UI state
    void UpdateStatus(const std::wstring& message);
    void UpdateProgress();

private:
    // Named controls (auto-bound from XML)
    luaui::controls::Button* m_newBtn = nullptr;
    luaui::controls::Button* m_openBtn = nullptr;
    luaui::controls::Button* m_saveBtn = nullptr;
    luaui::controls::Button* m_searchBtn = nullptr;
    luaui::controls::Button* m_submitBtn = nullptr;
    luaui::controls::Button* m_cancelBtn = nullptr;
    luaui::controls::Button* m_resetBtn = nullptr;
    luaui::controls::Button* m_settingsBtn = nullptr;
    luaui::controls::Button* m_navHomeBtn = nullptr;
    luaui::controls::Button* m_navProfileBtn = nullptr;
    luaui::controls::Button* m_navMessagesBtn = nullptr;
    luaui::controls::Button* m_navSettingsBtn = nullptr;
    
    luaui::controls::TextBox* m_usernameBox = nullptr;
    luaui::controls::TextBox* m_emailBox = nullptr;
    luaui::controls::TextBox* m_bioBox = nullptr;
    luaui::controls::TextBox* m_searchBox = nullptr;
    
    luaui::controls::Slider* m_volumeSlider = nullptr;
    luaui::controls::ProgressBar* m_profileProgress = nullptr;
    luaui::controls::TextBlock* m_statusText = nullptr;
    luaui::controls::TextBlock* m_progressPercentText = nullptr;
    
    // XML loader
    luaui::xml::IXmlLoaderPtr m_xmlLoader;
};

} // namespace demo
