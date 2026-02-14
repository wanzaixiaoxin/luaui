#pragma once

#include "Panel.h"
#include "../core/Window.h"
#include "../rendering/Types.h"
#include <functional>
#include <memory>
#include <string>

namespace luaui {
namespace controls {

// 前向声明
class Dialog;
class DialogHost;

/**
 * @brief DialogResult 对话框结果
 */
enum class DialogResult {
    None,       // 无结果
    OK,         // 确定
    Cancel,     // 取消
    Yes,        // 是
    No,         // 否
    Abort,      // 中止
    Retry,      // 重试
    Ignore      // 忽略
};

/**
 * @brief DialogButton 对话框按钮类型
 */
enum class DialogButton {
    OK,                 // 确定
    OKCancel,           // 确定+取消
    YesNo,              // 是+否
    YesNoCancel,        // 是+否+取消
    RetryCancel,        // 重试+取消
    AbortRetryIgnore    // 中止+重试+忽略
};

/**
 * @brief DialogIcon 对话框图标
 */
enum class DialogIcon {
    None,       // 无图标
    Info,       // 信息
    Warning,    // 警告
    Error,      // 错误
    Question    // 问题
};

/**
 * @brief DialogWindow 模态对话框窗口（新架构）
 * 
 * 特点：
 * - 模态/非模态显示
 * - 自定义内容
 * - 标准按钮支持
 * - 居中显示
 * - 结果回调
 */
class DialogWindow : public Window {
public:
    DialogWindow();
    virtual ~DialogWindow();
    
    std::string GetTypeName() const { return "DialogWindow"; }
    
    // 标题
    std::wstring GetTitle() const { return m_title; }
    void SetTitle(const std::wstring& title) { m_title = title; }
    
    // 对话框结果
    DialogResult GetDialogResult() const { return m_dialogResult; }
    void SetDialogResult(DialogResult result) { m_dialogResult = result; }
    
    // 内容控件
    std::shared_ptr<Control> GetContent() const { return m_content; }
    void SetContent(const std::shared_ptr<Control>& content);
    
    // 显示对话框
    void ShowDialog(Window* owner = nullptr);  // 模态显示
    void Show(Window* owner = nullptr);        // 非模态显示
    void CloseDialog(DialogResult result = DialogResult::Cancel);
    
    // 是否模态
    bool GetIsModal() const { return m_isModal; }
    
    // 结果回调
    using DialogClosedHandler = std::function<void(DialogWindow*, DialogResult)>;
    void SetDialogClosedHandler(DialogClosedHandler handler) { m_closedHandler = handler; }
    
    // 事件
    luaui::Delegate<DialogWindow*> Opened;
    luaui::Delegate<DialogWindow*, DialogResult> Closed;

protected:
    void Initialize();
    void OnClosing();

private:
    void CenterToOwner(Window* owner);
    void EnableOwner(Window* owner, bool enable);
    
    std::wstring m_title = L"Dialog";
    std::shared_ptr<Control> m_content;
    DialogResult m_dialogResult = DialogResult::None;
    bool m_isModal = false;
    bool m_isClosed = false;
    
    Window* m_owner = nullptr;
    DialogClosedHandler m_closedHandler;
    int m_dialogWidth = 400;
    int m_dialogHeight = 250;
};

/**
 * @brief MessageDialog 消息对话框（简化版）
 * 
 * 便捷类，用于显示标准消息对话框
 */
class MessageDialog {
public:
    // 显示消息框（静态方法）
    static DialogResult Show(const std::wstring& message, 
                              const std::wstring& title = L"Info",
                              DialogButton buttons = DialogButton::OK,
                              DialogIcon icon = DialogIcon::None,
                              Window* owner = nullptr);
    
    // 异步显示（带回调）
    static void ShowAsync(const std::wstring& message,
                          const std::wstring& title,
                          DialogButton buttons,
                          DialogIcon icon,
                          std::function<void(DialogResult)> callback,
                          Window* owner = nullptr);
    
    // 便捷方法
    static void ShowInfo(const std::wstring& message, 
                         const std::wstring& title = L"Information",
                         Window* owner = nullptr);
    
    static void ShowWarning(const std::wstring& message,
                            const std::wstring& title = L"Warning",
                            Window* owner = nullptr);
    
    static void ShowError(const std::wstring& message,
                          const std::wstring& title = L"Error",
                          Window* owner = nullptr);
    
    static DialogResult ShowQuestion(const std::wstring& message,
                                      const std::wstring& title = L"Confirm",
                                      DialogButton buttons = DialogButton::YesNo,
                                      Window* owner = nullptr);
    
    static DialogResult ShowConfirm(const std::wstring& message,
                                     const std::wstring& title = L"Confirm",
                                     Window* owner = nullptr);
};

/**
 * @brief InputDialog 输入对话框
 * 
 * 便捷类，用于获取用户输入
 */
class InputDialog {
public:
    // 显示文本输入框
    static DialogResult Show(const std::wstring& prompt,
                              std::wstring& outValue,
                              const std::wstring& title = L"Input",
                              const std::wstring& defaultValue = L"",
                              Window* owner = nullptr);
    
    // 显示密码输入框
    static DialogResult ShowPassword(const std::wstring& prompt,
                                      std::wstring& outValue,
                                      const std::wstring& title = L"Enter Password",
                                      Window* owner = nullptr);
    
    // 显示多行文本输入框
    static DialogResult ShowMultiline(const std::wstring& prompt,
                                       std::wstring& outValue,
                                       const std::wstring& title = L"Input",
                                       const std::wstring& defaultValue = L"",
                                       Window* owner = nullptr);
};

/**
 * @brief FileDialog 文件对话框（简化版）
 * 
 * 文件选择对话框
 */
class FileDialog {
public:
    // 过滤器定义
    struct Filter {
        std::wstring description;
        std::wstring extension;  // 如 "*.txt;*.csv"
    };
    
    // 打开文件对话框
    static DialogResult ShowOpen(std::wstring& outFilePath,
                                  const std::vector<Filter>& filters = {},
                                  const std::wstring& title = L"Open File",
                                  Window* owner = nullptr);
    
    // 保存文件对话框
    static DialogResult ShowSave(std::wstring& outFilePath,
                                  const std::vector<Filter>& filters = {},
                                  const std::wstring& title = L"Save File",
                                  const std::wstring& defaultFileName = L"",
                                  Window* owner = nullptr);
    
    // 选择文件夹对话框
    static DialogResult ShowFolder(std::wstring& outFolderPath,
                                    const std::wstring& title = L"Select Folder",
                                    Window* owner = nullptr);
    
    // 多选文件对话框
    static DialogResult ShowOpenMultiple(std::vector<std::wstring>& outFilePaths,
                                          const std::vector<Filter>& filters = {},
                                          const std::wstring& title = L"Open File",
                                          Window* owner = nullptr);
};

/**
 * @brief DialogHost 对话框宿主
 * 
 * 管理对话框队列，支持对话框嵌套
 */
class DialogHost {
public:
    static DialogHost& GetInstance();
    
    // 显示对话框
    void ShowDialog(const std::shared_ptr<DialogWindow>& dialog, Window* owner = nullptr);
    
    // 关闭当前对话框
    void CloseCurrentDialog(DialogResult result = DialogResult::Cancel);
    
    // 获取当前对话框
    std::shared_ptr<DialogWindow> GetCurrentDialog() const;
    
    // 是否有打开的对话框
    bool HasOpenDialog() const { return !m_dialogStack.empty(); }
    
    // 关闭所有对话框
    void CloseAllDialogs();

private:
    DialogHost() = default;
    ~DialogHost() = default;
    DialogHost(const DialogHost&) = delete;
    DialogHost& operator=(const DialogHost&) = delete;
    
    std::vector<std::shared_ptr<DialogWindow>> m_dialogStack;
};

} // namespace controls
} // namespace luaui
