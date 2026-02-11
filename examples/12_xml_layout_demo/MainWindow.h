#pragma once

#include <luaui.h>

// 前向声明
namespace luaui {
namespace controls {
class Slider;
}
}

class MainWindow : public luaui::Window {
public:
    MainWindow();
    ~MainWindow();

protected:
    void OnLoaded() override;

private:
    void LoadLayout();
    std::shared_ptr<luaui::controls::Panel> CreateSidebar();
    std::shared_ptr<luaui::controls::Panel> CreateMainContent();
    
    void BindEvents();
    
    // 事件处理 - 使用新Delegate API
    void OnPrimaryClicked(luaui::Control* sender);
    void OnSecondaryClicked(luaui::Control* sender);
    void OnSliderValueChanged(luaui::controls::Slider* sender, double value);
};
