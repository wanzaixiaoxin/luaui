#pragma once

// 使用新架构的控件示例
#include "Control.h"
#include "Controls.h"
#include <iostream>

namespace demo {

using namespace luaui::controls;

/**
 * @brief 演示窗口
 */
class MainWindow {
public:
    MainWindow();
    
    void Initialize();
    void Run();
    
    void OnButtonClick(luaui::ControlBase* sender);
    void OnTextChanged(TextBox* sender, const std::wstring& text);

private:
    std::shared_ptr<StackPanel> m_root;
    std::shared_ptr<Button> m_button;
    std::shared_ptr<TextBlock> m_label;
    std::shared_ptr<TextBox> m_textBox;
    std::shared_ptr<Border> m_border;
};

} // namespace demo
