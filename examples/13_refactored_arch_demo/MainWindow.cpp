#include "MainWindow.h"

namespace demo {

MainWindow::MainWindow() {
    Initialize();
}

void MainWindow::Initialize() {
    std::cout << "=== Initializing New Architecture Demo ===" << std::endl;
    
    // 创建根容器
    m_root = std::make_shared<StackPanel>();
    m_root->SetName("rootPanel");
    
    // 创建标签
    m_label = std::make_shared<TextBlock>();
    m_label->SetName("label");
    m_label->SetText(L"New Architecture Demo");
    m_label->SetFontSize(18);
    
    // 创建按钮
    m_button = std::make_shared<Button>();
    m_button->SetName("clickButton");
    m_button->SetStateColors(
        luaui::rendering::Color::FromHex(0x2196F3),  // Normal
        luaui::rendering::Color::FromHex(0x1976D2),  // Hover
        luaui::rendering::Color::FromHex(0x0D47A1)   // Pressed
    );
    
    // 订阅按钮点击事件
    m_button->Click.Add(this, &MainWindow::OnButtonClick);
    
    // 创建文本框
    m_textBox = std::make_shared<TextBox>();
    m_textBox->SetName("textInput");
    m_textBox->SetPlaceholder(L"Enter text here...");
    
    // 订阅文本变化事件
    m_textBox->TextChanged.Add(this, &MainWindow::OnTextChanged);
    
    // 创建带边框的容器
    m_border = std::make_shared<Border>();
    m_border->SetName("contentBorder");
    m_border->SetBackground(luaui::rendering::Color::FromHex(0xF5F5F5));
    m_border->SetBorderColor(luaui::rendering::Color::FromHex(0xCCCCCC));
    m_border->SetBorderThickness(2);
    
    // 构建控件树
    m_root->AddChild(m_label);
    m_root->AddChild(m_button);
    m_root->AddChild(m_textBox);
    m_root->AddChild(m_border);
    
    std::cout << "Controls created:" << std::endl;
    std::cout << "  - Root Panel: " << m_root->GetTypeName() << " (ID: " << m_root->GetID() << ")" << std::endl;
    std::cout << "  - Label: " << m_label->GetTypeName() << " (ID: " << m_label->GetID() << ")" << std::endl;
    std::cout << "  - Button: " << m_button->GetTypeName() << " (ID: " << m_button->GetID() << ")" << std::endl;
    std::cout << "  - TextBox: " << m_textBox->GetTypeName() << " (ID: " << m_textBox->GetID() << ")" << std::endl;
    std::cout << "  - Border: " << m_border->GetTypeName() << " (ID: " << m_border->GetID() << ")" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Root panel child count: " << m_root->GetChildCount() << std::endl;
    std::cout << std::endl;
}

void MainWindow::Run() {
    std::cout << "=== Running Demo ===" << std::endl;
    
    // 模拟按钮点击
    std::cout << "Simulating button click..." << std::endl;
    if (auto* input = m_button->GetInput()) {
        input->RaiseClick();
    }
    
    // 模拟文本输入
    std::cout << "Setting text in TextBox..." << std::endl;
    m_textBox->SetText(L"Hello, SOLID Architecture!");
    
    std::cout << "TextBox content: ";
    std::wstring text = m_textBox->GetText();
    std::wcout << text << std::endl;
    std::cout << std::endl;
    
    std::cout << "=== Demo completed successfully ===" << std::endl;
}

void MainWindow::OnButtonClick(luaui::ControlBase* sender) {
    std::cout << "Button clicked! (sender: " << sender->GetName() << ")" << std::endl;
}

void MainWindow::OnTextChanged(TextBox* sender, const std::wstring& text) {
    std::wcout << L"Text changed: " << text << std::endl;
}

} // namespace demo
