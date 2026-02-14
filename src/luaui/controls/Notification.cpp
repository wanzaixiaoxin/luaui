#include "Notification.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include "IRenderContext.h"
#include "Window.h"
#include <algorithm>

namespace luaui {
namespace controls {

// ============================================================================
// ToastNotification
// ============================================================================
ToastNotification::ToastNotification() {}

void ToastNotification::InitializeComponents() {
    GetComponents().AddComponent<components::LayoutComponent>(this);
    GetComponents().AddComponent<components::RenderComponent>(this);
    
    // 默认隐藏
    SetIsVisible(false);
}

void ToastNotification::SetTitle(const std::wstring& title) {
    if (m_title != title) {
        m_title = title;
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void ToastNotification::SetMessage(const std::wstring& message) {
    if (m_message != message) {
        m_message = message;
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void ToastNotification::SetType(NotificationType type) {
    if (m_type != type) {
        m_type = type;
        
        // 更新颜色
        switch (type) {
            case NotificationType::Info:
                m_iconColor = m_infoColor;
                break;
            case NotificationType::Success:
                m_iconColor = m_successColor;
                break;
            case NotificationType::Warning:
                m_iconColor = m_warningColor;
                break;
            case NotificationType::Error:
                m_iconColor = m_errorColor;
                break;
        }
        
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void ToastNotification::SetAction(const std::wstring& text, std::function<void()> callback) {
    m_actionText = text;
    m_actionCallback = callback;
}

void ToastNotification::Show() {
    m_isOpen = true;
    SetIsVisible(true);
    Opened.Invoke(this);
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void ToastNotification::Close() {
    if (m_isOpen) {
        m_isOpen = false;
        SetIsVisible(false);
        Closed.Invoke(this);
    }
}

void ToastNotification::OnMouseEnter() {
    m_isHovered = true;
    UpdateVisualState();
}

void ToastNotification::OnMouseLeave() {
    m_isHovered = false;
    UpdateVisualState();
}

void ToastNotification::OnClick() {
    if (!m_actionText.empty() && m_actionCallback) {
        m_actionCallback();
    }
}

void ToastNotification::UpdateVisualState() {
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

bool ToastNotification::HitTestCloseButton(float x, float y) {
    rendering::Rect rect;
    if (auto* renderable = AsRenderable()) {
        rect = renderable->GetRenderRect();
    }
    
    float closeX = rect.x + rect.width - m_padding - m_closeButtonSize;
    float closeY = rect.y + m_padding;
    
    return x >= closeX && x <= closeX + m_closeButtonSize &&
           y >= closeY && y <= closeY + m_closeButtonSize;
}

void ToastNotification::DrawIcon(rendering::IRenderContext* context, const rendering::Rect& rect) {
    auto brush = context->CreateSolidColorBrush(m_iconColor);
    if (!brush) return;
    
    float centerX = rect.x + rect.width / 2;
    float centerY = rect.y + rect.height / 2;
    
    switch (m_type) {
        case NotificationType::Info: {
            // 信息图标：i
            auto textBrush = context->CreateSolidColorBrush(m_iconColor);
            auto textFormat = context->CreateTextFormat(L"Microsoft YaHei", m_iconSize);
            if (textBrush && textFormat) {
                context->DrawTextString(L"i", textFormat.get(),
                                        rendering::Point(centerX - m_iconSize/4, 
                                                         centerY - m_iconSize/2),
                                        textBrush.get());
            }
            break;
        }
        case NotificationType::Success: {
            // 成功图标：对勾
            context->DrawLine(rendering::Point(centerX - 6, centerY),
                              rendering::Point(centerX - 2, centerY + 6),
                              brush.get(), 2.0f);
            context->DrawLine(rendering::Point(centerX - 2, centerY + 6),
                              rendering::Point(centerX + 6, centerY - 4),
                              brush.get(), 2.0f);
            break;
        }
        case NotificationType::Warning: {
            // 警告图标：!
            auto textBrush = context->CreateSolidColorBrush(m_iconColor);
            auto textFormat = context->CreateTextFormat(L"Microsoft YaHei", m_iconSize);
            if (textBrush && textFormat) {
                context->DrawTextString(L"!", textFormat.get(),
                                        rendering::Point(centerX - m_iconSize/4, 
                                                         centerY - m_iconSize/2),
                                        textBrush.get());
            }
            break;
        }
        case NotificationType::Error: {
            // 错误图标：X
            context->DrawLine(rendering::Point(centerX - 5, centerY - 5),
                              rendering::Point(centerX + 5, centerY + 5),
                              brush.get(), 2.0f);
            context->DrawLine(rendering::Point(centerX + 5, centerY - 5),
                              rendering::Point(centerX - 5, centerY + 5),
                              brush.get(), 2.0f);
            break;
        }
    }
}

void ToastNotification::DrawCloseButton(rendering::IRenderContext* context, 
                                         const rendering::Rect& rect) {
    auto brush = context->CreateSolidColorBrush(
        m_isHovered ? m_closeButtonHoverColor : m_closeButtonColor);
    if (!brush) return;
    
    float centerX = rect.x + rect.width / 2;
    float centerY = rect.y + rect.height / 2;
    
    // 绘制 X
    context->DrawLine(rendering::Point(centerX - 4, centerY - 4),
                      rendering::Point(centerX + 4, centerY + 4),
                      brush.get(), 1.5f);
    context->DrawLine(rendering::Point(centerX + 4, centerY - 4),
                      rendering::Point(centerX - 4, centerY + 4),
                      brush.get(), 1.5f);
}

rendering::Size ToastNotification::OnMeasure(const rendering::Size& availableSize) {
    (void)availableSize;
    
    // 估算尺寸
    float width = m_maxWidth;
    
    // 计算内容高度
    float contentHeight = 0;
    if (!m_title.empty()) {
        contentHeight += m_titleFontSize + 4; // 标题 + 间距
    }
    if (!m_message.empty()) {
        // 估算消息行数（简化）
        int estimatedLines = static_cast<int>(m_message.length() / 30) + 1;
        contentHeight += estimatedLines * (m_fontSize + 4);
    }
    
    float height = std::max(contentHeight + m_padding * 2, m_iconSize + m_padding * 2);
    
    return rendering::Size(width, height);
}

void ToastNotification::OnRender(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    auto rect = render->GetRenderRect();
    
    // 应用动画（淡出效果）
    float alpha = m_animationProgress;
    
    // 绘制阴影
    auto shadowBrush = context->CreateSolidColorBrush(
        rendering::Color(0, 0, 0, 0.2f * alpha));
    if (shadowBrush) {
        context->FillRectangle(
            rendering::Rect(rect.x + m_shadowOffset, rect.y + m_shadowOffset,
                           rect.width, rect.height),
            shadowBrush.get());
    }
    
    // 绘制背景
    auto bgBrush = context->CreateSolidColorBrush(
        rendering::Color(m_bgColor.r, m_bgColor.g, m_bgColor.b, m_bgColor.a * alpha));
    if (bgBrush) {
        context->FillRectangle(rect, bgBrush.get());
    }
    
    // 绘制左边框（类型颜色）
    auto accentBrush = context->CreateSolidColorBrush(
        rendering::Color(m_iconColor.r, m_iconColor.g, m_iconColor.b, alpha));
    if (accentBrush) {
        context->FillRectangle(
            rendering::Rect(rect.x, rect.y, 4, rect.height),
            accentBrush.get());
    }
    
    // 绘制边框
    auto borderBrush = context->CreateSolidColorBrush(
        rendering::Color(m_borderColor.r, m_borderColor.g, m_borderColor.b, alpha));
    if (borderBrush) {
        context->DrawRectangle(rect, borderBrush.get(), 1.0f);
    }
    
    // 绘制图标
    rendering::Rect iconRect(rect.x + m_padding, 
                              rect.y + m_padding,
                              m_iconSize, m_iconSize);
    DrawIcon(context, iconRect);
    
    // 绘制关闭按钮
    if (m_showCloseButton) {
        rendering::Rect closeRect(rect.x + rect.width - m_padding - m_closeButtonSize,
                                   rect.y + m_padding,
                                   m_closeButtonSize, m_closeButtonSize);
        DrawCloseButton(context, closeRect);
    }
    
    // 绘制标题
    float contentX = rect.x + m_padding + m_iconSize + m_padding;
    float contentY = rect.y + m_padding;
    float contentWidth = rect.width - contentX - m_padding - 
                         (m_showCloseButton ? m_closeButtonSize + m_padding : 0);
    
    if (!m_title.empty()) {
        auto titleBrush = context->CreateSolidColorBrush(
            rendering::Color(m_titleColor.r, m_titleColor.g, m_titleColor.b, alpha));
        auto titleFormat = context->CreateTextFormat(L"Microsoft YaHei", m_titleFontSize);
        
        if (titleBrush && titleFormat) {
            rendering::Point titlePos(contentX, contentY);
            context->DrawTextString(m_title, titleFormat.get(), titlePos, titleBrush.get());
        }
        contentY += m_titleFontSize + 4;
    }
    
    // 绘制消息
    if (!m_message.empty()) {
        auto msgBrush = context->CreateSolidColorBrush(
            rendering::Color(m_messageColor.r, m_messageColor.g, m_messageColor.b, alpha));
        auto msgFormat = context->CreateTextFormat(L"Microsoft YaHei", m_fontSize);
        
        if (msgBrush && msgFormat) {
            rendering::Point msgPos(contentX, contentY);
            context->DrawTextString(m_message, msgFormat.get(), msgPos, msgBrush.get());
        }
    }
}

// ============================================================================
// NotificationManager
// ============================================================================
NotificationManager& NotificationManager::GetInstance() {
    static NotificationManager instance;
    return instance;
}

std::shared_ptr<ToastNotification> NotificationManager::Show(
    const std::wstring& message,
    const std::wstring& title,
    NotificationType type,
    int durationMs) {
    
    auto notification = std::make_shared<ToastNotification>();
    notification->SetMessage(message);
    notification->SetTitle(title);
    notification->SetType(type);
    notification->SetDuration(durationMs);
    
    ShowNotification(notification);
    
    return notification;
}

std::shared_ptr<ToastNotification> NotificationManager::ShowInfo(
    const std::wstring& message,
    const std::wstring& title) {
    return Show(message, title, NotificationType::Info, 3000);
}

std::shared_ptr<ToastNotification> NotificationManager::ShowSuccess(
    const std::wstring& message,
    const std::wstring& title) {
    return Show(message, title, NotificationType::Success, 3000);
}

std::shared_ptr<ToastNotification> NotificationManager::ShowWarning(
    const std::wstring& message,
    const std::wstring& title) {
    return Show(message, title, NotificationType::Warning, 5000);
}

std::shared_ptr<ToastNotification> NotificationManager::ShowError(
    const std::wstring& message,
    const std::wstring& title) {
    return Show(message, title, NotificationType::Error, 0); // 错误消息不自动关闭
}

void NotificationManager::ShowNotification(
    const std::shared_ptr<ToastNotification>& notification) {
    
    if (!notification) return;
    
    // 如果超过最大数量，加入队列
    if (m_maxNotifications > 0 && 
        static_cast<int>(m_notifications.size()) >= m_maxNotifications) {
        m_pendingQueue.push(notification);
        return;
    }
    
    AddNotification(notification);
}

void NotificationManager::AddNotification(
    const std::shared_ptr<ToastNotification>& notification) {
    
    m_notifications.push_back(notification);
    
    // 监听关闭事件
    notification->Closed.Add([this, notification]() {
        RemoveNotification(notification);
    });
    
    // 显示
    notification->Show();
    
    // 更新位置
    UpdatePositions();
    
    // 自动关闭
    StartAutoClose(notification);
}

void NotificationManager::RemoveNotification(
    const std::shared_ptr<ToastNotification>& notification) {
    
    auto it = std::find(m_notifications.begin(), m_notifications.end(), notification);
    if (it != m_notifications.end()) {
        m_notifications.erase(it);
        UpdatePositions();
    }
    
    // 检查是否有等待中的通知
    if (!m_pendingQueue.empty() && 
        static_cast<int>(m_notifications.size()) < m_maxNotifications) {
        auto next = m_pendingQueue.front();
        m_pendingQueue.pop();
        AddNotification(next);
    }
}

void NotificationManager::UpdatePositions() {
    // 简化实现：实际应该根据位置枚举计算每个通知的位置
    // 这里仅作为占位
    (void)m_position;
    (void)m_spacing;
    (void)m_offsetX;
    (void)m_offsetY;
}

void NotificationManager::StartAutoClose(
    const std::shared_ptr<ToastNotification>& notification) {
    
    int duration = notification->GetDuration();
    if (duration <= 0) return; // 0 表示不自动关闭
    
    // 简化实现：实际应该使用计时器
    // 这里只是一个占位
    (void)notification;
}

void NotificationManager::CloseAll() {
    // 关闭所有活动通知
    auto notifications = m_notifications;
    for (auto& notification : notifications) {
        if (notification) {
            notification->Close();
        }
    }
    m_notifications.clear();
    
    // 清空队列
    while (!m_pendingQueue.empty()) {
        m_pendingQueue.pop();
    }
}

// ============================================================================
// Snackbar
// ============================================================================
Snackbar::Snackbar() {}

void Snackbar::InitializeComponents() {
    GetComponents().AddComponent<components::LayoutComponent>(this);
    GetComponents().AddComponent<components::RenderComponent>(this);
    
    SetIsVisible(false);
}

void Snackbar::SetMessage(const std::wstring& message) {
    m_message = message;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void Snackbar::SetAction(const std::wstring& text, std::function<void()> callback) {
    m_actionText = text;
    m_actionCallback = callback;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void Snackbar::Show() {
    m_isVisible = true;
    SetIsVisible(true);
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void Snackbar::Hide() {
    m_isVisible = false;
    SetIsVisible(false);
}

rendering::Size Snackbar::OnMeasure(const rendering::Size& availableSize) {
    (void)availableSize;
    return rendering::Size(availableSize.width > 0 ? availableSize.width : 400, m_height);
}

void Snackbar::OnRender(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    auto rect = render->GetRenderRect();
    
    // 绘制背景
    auto bgBrush = context->CreateSolidColorBrush(m_bgColor);
    if (bgBrush) {
        context->FillRectangle(rect, bgBrush.get());
    }
    
    // 绘制消息
    auto msgBrush = context->CreateSolidColorBrush(m_textColor);
    auto msgFormat = context->CreateTextFormat(L"Microsoft YaHei", m_fontSize);
    
    if (msgBrush && msgFormat) {
        rendering::Point msgPos(rect.x + m_padding, 
                                rect.y + (rect.height - m_fontSize) / 2);
        context->DrawTextString(m_message, msgFormat.get(), msgPos, msgBrush.get());
    }
    
    // 绘制操作按钮
    if (!m_actionText.empty()) {
        auto actionBrush = context->CreateSolidColorBrush(m_actionColor);
        auto actionFormat = context->CreateTextFormat(L"Microsoft YaHei", m_fontSize);
        
        if (actionBrush && actionFormat) {
            actionFormat->SetTextAlignment(rendering::TextAlignment::Trailing);
            rendering::Point actionPos(rect.x + rect.width - m_padding, 
                                       rect.y + (rect.height - m_fontSize) / 2);
            context->DrawTextString(m_actionText, actionFormat.get(), actionPos, actionBrush.get());
        }
    }
}

} // namespace controls
} // namespace luaui
