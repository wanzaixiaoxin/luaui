#pragma once

#include <luaui.h>

// 前向声明
namespace luaui {
namespace controls {
class Button;
class Slider;
class ProgressBar;
class TextBlock;
}
}

class MainWindow : public luaui::Window {
public:
    MainWindow();
    ~MainWindow();

protected:
    void OnLoaded() override;

private:
    std::shared_ptr<luaui::Control> CreateContent();
    void BindEvents();
    
    // Control references for event binding
    std::shared_ptr<luaui::controls::Button> m_primaryBtn;
    std::shared_ptr<luaui::controls::Button> m_secondaryBtn;
    std::shared_ptr<luaui::controls::Slider> m_slider;
    std::shared_ptr<luaui::controls::ProgressBar> m_progressBar;
    std::shared_ptr<luaui::controls::TextBlock> m_sliderLabel;
};
