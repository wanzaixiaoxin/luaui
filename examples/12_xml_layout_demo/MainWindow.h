#pragma once

#include <luaui.h>

// 前向声明
namespace luaui {
namespace controls {
class Button;
class Slider;
class ProgressBar;
class TextBlock;
class TextBox;
}
}

class MainWindow : public luaui::Window {
public:
    MainWindow();
    ~MainWindow();

protected:
    void OnLoaded() override;

private:
    // 从 XML 加载布局
    std::shared_ptr<luaui::Control> LoadLayoutFromXml();
    
    // 回退内容（XML 加载失败时使用）
    std::shared_ptr<luaui::Control> CreateFallbackContent();
    
    // 查找命名控件
    void FindNamedControls();
    
    // 绑定事件
    void BindEvents();
    
    // 更新状态栏
    void UpdateStatus(const std::wstring& message);
    
    // 更新进度文本
    void UpdateProgressText();
    
    // Toolbar buttons
    std::shared_ptr<luaui::controls::Button> m_newBtn;
    std::shared_ptr<luaui::controls::Button> m_openBtn;
    std::shared_ptr<luaui::controls::Button> m_saveBtn;
    std::shared_ptr<luaui::controls::Button> m_searchBtn;
    
    // Action buttons
    std::shared_ptr<luaui::controls::Button> m_submitBtn;
    std::shared_ptr<luaui::controls::Button> m_cancelBtn;
    std::shared_ptr<luaui::controls::Button> m_resetBtn;
    
    // Navigation buttons
    std::shared_ptr<luaui::controls::Button> m_navHomeBtn;
    std::shared_ptr<luaui::controls::Button> m_navProfileBtn;
    std::shared_ptr<luaui::controls::Button> m_navMessagesBtn;
    std::shared_ptr<luaui::controls::Button> m_navSettingsBtn;
    std::shared_ptr<luaui::controls::Button> m_settingsBtn;
    
    // Form controls
    std::shared_ptr<luaui::controls::TextBox> m_usernameBox;
    std::shared_ptr<luaui::controls::TextBox> m_emailBox;
    std::shared_ptr<luaui::controls::TextBox> m_bioBox;
    std::shared_ptr<luaui::controls::TextBox> m_searchBox;
    
    // Other controls
    std::shared_ptr<luaui::controls::Slider> m_volumeSlider;
    std::shared_ptr<luaui::controls::ProgressBar> m_profileProgress;
    std::shared_ptr<luaui::controls::TextBlock> m_statusText;
    std::shared_ptr<luaui::controls::TextBlock> m_progressPercentText;
    std::shared_ptr<luaui::controls::TextBlock> m_titleText;
};
