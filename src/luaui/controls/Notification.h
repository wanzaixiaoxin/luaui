#pragma once

#include "Control.h"
#include "../rendering/Types.h"
#include <chrono>
#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <vector>

namespace luaui {
namespace controls {

// 前向声明
class ToastNotification;
class NotificationManager;

/**
 * @brief NotificationType 通知类型
 */
enum class NotificationType {
    Info,       // 信息
    Success,    // 成功
    Warning,    // 警告
    Error       // 错误
};

/**
 * @brief NotificationPosition 通知显示位置
 */
enum class NotificationPosition {
    TopLeft,      // 左上
    TopRight,     // 右上（默认）
    BottomLeft,   // 左下
    BottomRight,  // 右下
    TopCenter,    // 顶部居中
    BottomCenter  // 底部居中
};

/**
 * @brief ToastNotification 通知项（新架构）
 * 
 * 单个通知气泡
 */
class ToastNotification : public luaui::Control,
                          public std::enable_shared_from_this<ToastNotification> {
public:
    ToastNotification();
    
    std::string GetTypeName() const override { return "ToastNotification"; }
    
    // 内容
    std::wstring GetTitle() const { return m_title; }
    void SetTitle(const std::wstring& title);
    
    std::wstring GetMessage() const { return m_message; }
    void SetMessage(const std::wstring& message);
    
    // 类型
    NotificationType GetType() const { return m_type; }
    void SetType(NotificationType type);
    
    // 持续时间（毫秒，0 表示不自动关闭）
    int GetDuration() const { return m_durationMs; }
    void SetDuration(int ms) { m_durationMs = ms; }
    
    // 是否显示关闭按钮
    bool GetShowCloseButton() const { return m_showCloseButton; }
    void SetShowCloseButton(bool show) { m_showCloseButton = show; }
    
    // 操作按钮文本和回调
    void SetAction(const std::wstring& text, std::function<void()> callback);
    
    // 显示/关闭
    void Show();
    void Close();
    bool GetIsOpen() const { return m_isOpen; }
    
    // 动画进度（0.0 - 1.0）
    float GetAnimationProgress() const { return m_animationProgress; }
    void SetAnimationProgress(float progress) { m_animationProgress = progress; }
    
    // 事件
    luaui::Delegate<ToastNotification*> Opened;
    luaui::Delegate<ToastNotification*> Closed;

protected:
    void InitializeComponents() override;
    void OnRender(rendering::IRenderContext* context) override;
    rendering::Size OnMeasure(const rendering::Size& availableSize) override;
    
    void OnMouseEnter() override;
    void OnMouseLeave() override;
    void OnClick() override;

private:
    void UpdateVisualState();
    void DrawIcon(rendering::IRenderContext* context, const rendering::Rect& rect);
    void DrawCloseButton(rendering::IRenderContext* context, const rendering::Rect& rect);
    bool HitTestCloseButton(float x, float y);
    
    std::wstring m_title;
    std::wstring m_message;
    NotificationType m_type = NotificationType::Info;
    int m_durationMs = 3000;  // 默认3秒
    bool m_showCloseButton = true;
    
    std::wstring m_actionText;
    std::function<void()> m_actionCallback;
    
    bool m_isOpen = false;
    bool m_isHovered = false;
    float m_animationProgress = 0.0f;
    
    // 外观
    float m_padding = 12.0f;
    float m_iconSize = 24.0f;
    float m_closeButtonSize = 16.0f;
    float m_cornerRadius = 4.0f;
    float m_fontSize = 14.0f;
    float m_titleFontSize = 14.0f;
    float m_maxWidth = 360.0f;
    float m_shadowOffset = 4.0f;
    
    // 颜色（根据类型变化）
    rendering::Color m_bgColor = rendering::Color::White();
    rendering::Color m_borderColor = rendering::Color::FromHex(0xCCCCCC);
    rendering::Color m_titleColor = rendering::Color::Black();
    rendering::Color m_messageColor = rendering::Color::FromHex(0x666666);
    rendering::Color m_closeButtonColor = rendering::Color::FromHex(0x999999);
    rendering::Color m_closeButtonHoverColor = rendering::Color::FromHex(0x333333);
    rendering::Color m_iconColor = rendering::Color::FromHex(0x0078D4);
    
    // 类型颜色
    rendering::Color m_infoColor = rendering::Color::FromHex(0x0078D4);
    rendering::Color m_successColor = rendering::Color::FromHex(0x107C10);
    rendering::Color m_warningColor = rendering::Color::FromHex(0xFFB900);
    rendering::Color m_errorColor = rendering::Color::FromHex(0xD13438);
};

/**
 * @brief NotificationManager 通知管理器（单例）
 * 
 * 管理所有通知的显示和排队
 */
class NotificationManager {
public:
    static NotificationManager& GetInstance();
    
    // 配置
    void SetPosition(NotificationPosition position) { m_position = position; }
    NotificationPosition GetPosition() const { return m_position; }
    
    void SetSpacing(float spacing) { m_spacing = spacing; }
    float GetSpacing() const { return m_spacing; }
    
    void SetMaxNotifications(int max) { m_maxNotifications = max; }
    int GetMaxNotifications() const { return m_maxNotifications; }
    
    // 显示通知（便捷方法）
    std::shared_ptr<ToastNotification> Show(
        const std::wstring& message,
        const std::wstring& title = L"",
        NotificationType type = NotificationType::Info,
        int durationMs = 3000);
    
    std::shared_ptr<ToastNotification> ShowInfo(const std::wstring& message,
                                                 const std::wstring& title = L"");
    
    std::shared_ptr<ToastNotification> ShowSuccess(const std::wstring& message,
                                                    const std::wstring& title = L"");
    
    std::shared_ptr<ToastNotification> ShowWarning(const std::wstring& message,
                                                    const std::wstring& title = L"");
    
    std::shared_ptr<ToastNotification> ShowError(const std::wstring& message,
                                                  const std::wstring& title = L"");
    
    // 显示自定义通知
    void ShowNotification(const std::shared_ptr<ToastNotification>& notification);
    
    // 关闭所有通知
    void CloseAll();
    
    // 获取活动通知数量
    int GetActiveCount() const { return static_cast<int>(m_notifications.size()); }

private:
    NotificationManager() = default;
    ~NotificationManager() = default;
    NotificationManager(const NotificationManager&) = delete;
    NotificationManager& operator=(const NotificationManager&) = delete;
    
    void AddNotification(const std::shared_ptr<ToastNotification>& notification);
    void RemoveNotification(const std::shared_ptr<ToastNotification>& notification);
    void UpdatePositions();
    void StartAutoClose(const std::shared_ptr<ToastNotification>& notification);
    
    std::vector<std::shared_ptr<ToastNotification>> m_notifications;
    std::queue<std::shared_ptr<ToastNotification>> m_pendingQueue;
    
    NotificationPosition m_position = NotificationPosition::TopRight;
    float m_spacing = 8.0f;
    int m_maxNotifications = 5;
    float m_offsetX = 16.0f;  // 距离边缘距离
    float m_offsetY = 16.0f;
};

/**
 * @brief Snackbar 底部提示条
 * 
 * Material Design 风格的底部提示
 */
class Snackbar : public luaui::Control {
public:
    Snackbar();
    
    std::string GetTypeName() const override { return "Snackbar"; }
    
    // 消息
    std::wstring GetMessage() const { return m_message; }
    void SetMessage(const std::wstring& message);
    
    // 操作按钮
    void SetAction(const std::wstring& text, std::function<void()> callback);
    
    // 持续时间
    int GetDuration() const { return m_durationMs; }
    void SetDuration(int ms) { m_durationMs = ms; }
    
    // 显示/隐藏
    void Show();
    void Hide();
    bool GetIsVisible() const { return m_isVisible; }

protected:
    void InitializeComponents() override;
    void OnRender(rendering::IRenderContext* context) override;
    rendering::Size OnMeasure(const rendering::Size& availableSize) override;

private:
    std::wstring m_message;
    std::wstring m_actionText;
    std::function<void()> m_actionCallback;
    int m_durationMs = 3000;
    bool m_isVisible = false;
    float m_animationProgress = 0.0f;
    
    // 外观
    float m_height = 48.0f;
    float m_padding = 16.0f;
    float m_fontSize = 14.0f;
    rendering::Color m_bgColor = rendering::Color::FromHex(0x323232);
    rendering::Color m_textColor = rendering::Color::White();
    rendering::Color m_actionColor = rendering::Color::FromHex(0xBB86FC);
};

} // namespace controls
} // namespace luaui
