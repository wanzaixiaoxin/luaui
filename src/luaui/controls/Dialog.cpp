#include "Dialog.h"
#include "Button.h"
#include "TextBlock.h"
#include "TextBox.h"
#include "layouts/StackPanel.h"
#include "Components/LayoutComponent.h"
#include "IRenderContext.h"
#include <algorithm>

namespace luaui {
namespace controls {

// ============================================================================
// DialogWindow
// ============================================================================
DialogWindow::DialogWindow() {
    // 设置默认大小
    SetSize(400, 250);
    
    // 禁止调整大小
    // SetResizable(false);
    
    // 居中显示
    // SetStartPosition(WindowStartPosition::CenterParent);
}

DialogWindow::~DialogWindow() {
    CloseDialog(DialogResult::Cancel);
}

void DialogWindow::Initialize() {
    Window::Initialize();
    
    // 创建基本布局
    // 这里简化实现，实际应该创建内容面板、按钮面板等
}

void DialogWindow::SetContent(const std::shared_ptr<Control>& content) {
    m_content = content;
    // 设置到窗口的内容区域
    // SetContent(content);
}

void DialogWindow::ShowDialog(Window* owner) {
    if (m_isClosed) return;
    
    m_isModal = true;
    m_owner = owner;
    
    // 禁用父窗口
    EnableOwner(owner, false);
    
    // 居中显示
    CenterToOwner(owner);
    
    // 显示窗口
    Show(owner);
    
    Opened.Invoke(this);
    
    // 进入模态消息循环
    // 简化实现：实际应该阻塞直到对话框关闭
    // RunModal();
}

void DialogWindow::Show(Window* owner) {
    if (m_isClosed) return;
    
    m_isModal = false;
    m_owner = owner;
    
    // 显示窗口
    Window::Show();
    
    Opened.Invoke(this);
}

void DialogWindow::CloseDialog(DialogResult result) {
    if (m_isClosed) return;
    
    m_dialogResult = result;
    m_isClosed = true;
    
    // 启用父窗口
    if (m_isModal) {
        EnableOwner(m_owner, true);
    }
    
    // 触发事件
    Closed.Invoke(this, result);
    if (m_closedHandler) {
        m_closedHandler(this, result);
    }
    
    // 关闭窗口
    Close();
}

void DialogWindow::OnClosing() {
    // 如果用户直接关闭窗口，视为 Cancel
    if (!m_isClosed) {
        CloseDialog(DialogResult::Cancel);
    }
    Window::OnClosing();
}

void DialogWindow::CenterToOwner(Window* owner) {
    if (!owner) return;
    
    // 获取父窗口位置
    int ownerX, ownerY, ownerW, ownerH;
    owner->GetPosition(ownerX, ownerY);
    owner->GetSize(ownerW, ownerH);
    
    // 获取自身大小
    int width, height;
    GetSize(width, height);
    
    // 计算居中位置
    int x = ownerX + (ownerW - width) / 2;
    int y = ownerY + (ownerH - height) / 2;
    
    SetPosition(x, y);
}

void DialogWindow::EnableOwner(Window* owner, bool enable) {
    (void)owner;
    (void)enable;
    // 简化实现：实际应该调用 EnableWindow API
}

// ============================================================================
// MessageDialog
// ============================================================================
DialogResult MessageDialog::Show(const std::wstring& message,
                                  const std::wstring& title,
                                  DialogButton buttons,
                                  DialogIcon icon,
                                  Window* owner) {
    (void)icon;
    
    auto dialog = std::make_shared<DialogWindow>();
    dialog->SetTitle(title);
    
    // 创建内容面板
    auto panel = std::make_shared<StackPanel>();
    panel->SetOrientation(layouts::Orientation::Vertical);
    
    // 添加消息文本
    auto textBlock = std::make_shared<TextBlock>();
    textBlock->SetText(message);
    textBlock->SetTextWrapping(TextWrapping::Wrap);
    panel->AddChild(textBlock);
    
    // 添加按钮
    auto buttonPanel = std::make_shared<StackPanel>();
    buttonPanel->SetOrientation(layouts::Orientation::Horizontal);
    
    DialogResult result = DialogResult::None;
    
    auto addButton = [&](const std::wstring& text, DialogResult r) {
        auto button = std::make_shared<Button>();
        button->SetContent(text);
        button->Click.Add([&result, dialog, r](Control*) {
            result = r;
            dialog->CloseDialog(r);
        });
        buttonPanel->AddChild(button);
    };
    
    switch (buttons) {
        case DialogButton::OK:
            addButton(L"确定", DialogResult::OK);
            break;
        case DialogButton::OKCancel:
            addButton(L"确定", DialogResult::OK);
            addButton(L"取消", DialogResult::Cancel);
            break;
        case DialogButton::YesNo:
            addButton(L"是", DialogResult::Yes);
            addButton(L"否", DialogResult::No);
            break;
        case DialogButton::YesNoCancel:
            addButton(L"是", DialogResult::Yes);
            addButton(L"否", DialogResult::No);
            addButton(L"取消", DialogResult::Cancel);
            break;
        case DialogButton::RetryCancel:
            addButton(L"重试", DialogResult::Retry);
            addButton(L"取消", DialogResult::Cancel);
            break;
        case DialogButton::AbortRetryIgnore:
            addButton(L"中止", DialogResult::Abort);
            addButton(L"重试", DialogResult::Retry);
            addButton(L"忽略", DialogResult::Ignore);
            break;
    }
    
    panel->AddChild(buttonPanel);
    dialog->SetContent(panel);
    
    // 显示对话框
    dialog->ShowDialog(owner);
    
    return result;
}

void MessageDialog::ShowAsync(const std::wstring& message,
                               const std::wstring& title,
                               DialogButton buttons,
                               DialogIcon icon,
                               std::function<void(DialogResult)> callback,
                               Window* owner) {
    auto dialog = std::make_shared<DialogWindow>();
    dialog->SetTitle(title);
    
    dialog->SetDialogClosedHandler([callback](DialogWindow*, DialogResult result) {
        if (callback) {
            callback(result);
        }
    });
    
    // 创建内容...（简化）
    (void)message;
    (void)buttons;
    (void)icon;
    
    dialog->ShowDialog(owner);
}

void MessageDialog::ShowInfo(const std::wstring& message, 
                              const std::wstring& title,
                              Window* owner) {
    Show(message, title, DialogButton::OK, DialogIcon::Info, owner);
}

void MessageDialog::ShowWarning(const std::wstring& message,
                                 const std::wstring& title,
                                 Window* owner) {
    Show(message, title, DialogButton::OK, DialogIcon::Warning, owner);
}

void MessageDialog::ShowError(const std::wstring& message,
                               const std::wstring& title,
                               Window* owner) {
    Show(message, title, DialogButton::OK, DialogIcon::Error, owner);
}

DialogResult MessageDialog::ShowQuestion(const std::wstring& message,
                                          const std::wstring& title,
                                          DialogButton buttons,
                                          Window* owner) {
    return Show(message, title, buttons, DialogIcon::Question, owner);
}

DialogResult MessageDialog::ShowConfirm(const std::wstring& message,
                                         const std::wstring& title,
                                         Window* owner) {
    return Show(message, title, DialogButton::OKCancel, DialogIcon::Question, owner);
}

// ============================================================================
// InputDialog
// ============================================================================
DialogResult InputDialog::Show(const std::wstring& prompt,
                                std::wstring& outValue,
                                const std::wstring& title,
                                const std::wstring& defaultValue,
                                Window* owner) {
    auto dialog = std::make_shared<DialogWindow>();
    dialog->SetTitle(title);
    
    // 创建内容
    auto panel = std::make_shared<StackPanel>();
    panel->SetOrientation(layouts::Orientation::Vertical);
    
    // 提示文本
    auto promptBlock = std::make_shared<TextBlock>();
    promptBlock->SetText(prompt);
    panel->AddChild(promptBlock);
    
    // 输入框
    auto textBox = std::make_shared<TextBox>();
    textBox->SetText(defaultValue);
    panel->AddChild(textBox);
    
    // 按钮
    auto buttonPanel = std::make_shared<StackPanel>();
    buttonPanel->SetOrientation(layouts::Orientation::Horizontal);
    
    DialogResult result = DialogResult::Cancel;
    
    auto okButton = std::make_shared<Button>();
    okButton->SetContent(L"确定");
    okButton->Click.Add([&result, &outValue, dialog, textBox](Control*) {
        outValue = textBox->GetText();
        result = DialogResult::OK;
        dialog->CloseDialog(DialogResult::OK);
    });
    buttonPanel->AddChild(okButton);
    
    auto cancelButton = std::make_shared<Button>();
    cancelButton->SetContent(L"取消");
    cancelButton->Click.Add([&result, dialog](Control*) {
        result = DialogResult::Cancel;
        dialog->CloseDialog(DialogResult::Cancel);
    });
    buttonPanel->AddChild(cancelButton);
    
    panel->AddChild(buttonPanel);
    dialog->SetContent(panel);
    
    dialog->ShowDialog(owner);
    
    return result;
}

DialogResult InputDialog::ShowPassword(const std::wstring& prompt,
                                        std::wstring& outValue,
                                        const std::wstring& title,
                                        Window* owner) {
    (void)prompt;
    (void)outValue;
    (void)title;
    (void)owner;
    // 简化实现：实际应该创建密码输入框
    return DialogResult::Cancel;
}

DialogResult InputDialog::ShowMultiline(const std::wstring& prompt,
                                         std::wstring& outValue,
                                         const std::wstring& title,
                                         const std::wstring& defaultValue,
                                         Window* owner) {
    (void)prompt;
    (void)outValue;
    (void)title;
    (void)defaultValue;
    (void)owner;
    // 简化实现：实际应该创建多行文本输入框
    return DialogResult::Cancel;
}

// ============================================================================
// FileDialog
// ============================================================================
DialogResult FileDialog::ShowOpen(std::wstring& outFilePath,
                                   const std::vector<Filter>& filters,
                                   const std::wstring& title,
                                   Window* owner) {
    (void)outFilePath;
    (void)filters;
    (void)title;
    (void)owner;
    // 简化实现：实际应该调用 Windows 文件对话框 API
    return DialogResult::Cancel;
}

DialogResult FileDialog::ShowSave(std::wstring& outFilePath,
                                   const std::vector<Filter>& filters,
                                   const std::wstring& title,
                                   const std::wstring& defaultFileName,
                                   Window* owner) {
    (void)outFilePath;
    (void)filters;
    (void)title;
    (void)defaultFileName;
    (void)owner;
    return DialogResult::Cancel;
}

DialogResult FileDialog::ShowFolder(std::wstring& outFolderPath,
                                     const std::wstring& title,
                                     Window* owner) {
    (void)outFolderPath;
    (void)title;
    (void)owner;
    return DialogResult::Cancel;
}

DialogResult FileDialog::ShowOpenMultiple(std::vector<std::wstring>& outFilePaths,
                                           const std::vector<Filter>& filters,
                                           const std::wstring& title,
                                           Window* owner) {
    (void)outFilePaths;
    (void)filters;
    (void)title;
    (void)owner;
    return DialogResult::Cancel;
}

// ============================================================================
// DialogHost
// ============================================================================
DialogHost& DialogHost::GetInstance() {
    static DialogHost instance;
    return instance;
}

void DialogHost::ShowDialog(const std::shared_ptr<DialogWindow>& dialog, Window* owner) {
    if (!dialog) return;
    
    // 将对话框添加到栈顶
    m_dialogStack.push_back(dialog);
    
    // 显示对话框
    dialog->ShowDialog(owner);
}

void DialogHost::CloseCurrentDialog(DialogResult result) {
    if (!m_dialogStack.empty()) {
        auto dialog = m_dialogStack.back();
        m_dialogStack.pop_back();
        
        if (dialog) {
            dialog->CloseDialog(result);
        }
    }
}

std::shared_ptr<DialogWindow> DialogHost::GetCurrentDialog() const {
    if (!m_dialogStack.empty()) {
        return m_dialogStack.back();
    }
    return nullptr;
}

void DialogHost::CloseAllDialogs() {
    // 从栈顶开始关闭所有对话框
    while (!m_dialogStack.empty()) {
        CloseCurrentDialog(DialogResult::Cancel);
    }
}

} // namespace controls
} // namespace luaui
