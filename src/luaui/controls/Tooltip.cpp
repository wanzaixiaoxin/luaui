#include "Tooltip.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include "Interfaces/IRenderable.h"
#include "IRenderContext.h"
#include "Window.h"
#include <algorithm>

namespace luaui {
namespace controls {

// 静态成员初始化
Tooltip* Tooltip::s_defaultTooltip = nullptr;
int ToolTipService::s_showDelayMs = 500;
bool ToolTipService::s_initialized = false;

// ============================================================================
// Tooltip
// ============================================================================
Tooltip::Tooltip() {}

void Tooltip::InitializeComponents() {
    GetComponents().AddComponent<components::LayoutComponent>(this);
    GetComponents().AddComponent<components::RenderComponent>(this);
    
    // Tooltip 默认不可见
    SetIsVisible(false);
    
    // Z-index not supported in current LayoutComponent
    // TODO: Implement z-index support
}

void Tooltip::SetText(const std::wstring& text) {
    if (m_text != text) {
        m_text = text;
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void Tooltip::Show() {
    if (!m_text.empty() && !m_isVisible) {
        m_isVisible = true;
        SetIsVisible(true);
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void Tooltip::ShowAt(float x, float y) {
    if (auto* layoutable = AsLayoutable()) {
        layoutable->Arrange(rendering::Rect(x, y, 0, 0));
    }
    Show();
}

void Tooltip::ShowRelativeTo(Control* target, float offsetX, float offsetY) {
    if (!target) return;
    
    // 获取目标控件的位置
    rendering::Rect targetRect;
    if (auto* renderable = target->AsRenderable()) {
        targetRect = renderable->GetRenderRect();
    }
    
    // 计算 Tooltip 尺寸
    auto size = MeasureText(m_text, m_maxWidth);
    float width = size.width + m_padding * 2;
    float height = size.height + m_padding * 2;
    
    // 默认显示在目标上方
    float x = targetRect.x + targetRect.width / 2 - width / 2 + offsetX;
    float y = targetRect.y - height + offsetY;
    
    // 边界检查：如果上方空间不足，显示在下方
    // 这里简化处理，实际应该检查窗口边界
    if (y < 0) {
        y = targetRect.y + targetRect.height - offsetY;
    }
    
    if (auto* layoutable = AsLayoutable()) {
        layoutable->Arrange(rendering::Rect(x, y, width, height));
    }
    
    Show();
}

void Tooltip::Hide() {
    if (m_isVisible) {
        m_isVisible = false;
        SetIsVisible(false);
    }
}

Tooltip* Tooltip::GetDefault() {
    if (!s_defaultTooltip) {
        // 创建默认实例
        // 注意：实际应用中需要确保生命周期管理
        // 这里简化处理
    }
    return s_defaultTooltip;
}

void Tooltip::SetToolTip(Control* target, const std::wstring& text) {
    ToolTipService::SetToolTip(reinterpret_cast<controls::Control*>(target), text);
}

std::wstring Tooltip::GetToolTip(Control* target) {
    return ToolTipService::GetToolTip(reinterpret_cast<controls::Control*>(target));
}

rendering::Size Tooltip::MeasureText(const std::wstring& text, float maxWidth) {
    // 简化实现：估算文本尺寸
    // 实际应使用字体测量 API
    if (text.empty()) {
        return rendering::Size(0, 0);
    }
    
    // 按换行符分割
    size_t lineCount = 1;
    size_t maxLineLength = 0;
    size_t currentLineLength = 0;
    
    for (wchar_t ch : text) {
        if (ch == L'\n') {
            lineCount++;
            maxLineLength = std::max(maxLineLength, currentLineLength);
            currentLineLength = 0;
        } else {
            currentLineLength++;
        }
    }
    maxLineLength = std::max(maxLineLength, currentLineLength);
    
    // 估算宽度和高度（假设平均字符宽度为字体大小的0.6倍）
    float charWidth = m_fontSize * 0.6f;
    float estimatedWidth = std::min(static_cast<float>(maxLineLength) * charWidth, maxWidth);
    float estimatedHeight = static_cast<float>(lineCount) * (m_fontSize * 1.2f);
    
    return rendering::Size(estimatedWidth, estimatedHeight);
}

rendering::Size Tooltip::OnMeasure(const rendering::Size& availableSize) {
    (void)availableSize;
    
    auto textSize = MeasureText(m_text, m_maxWidth);
    return rendering::Size(textSize.width + m_padding * 2, 
                           textSize.height + m_padding * 2);
}

void Tooltip::OnRender(rendering::IRenderContext* context) {
    if (!context || m_text.empty()) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    auto rect = render->GetRenderRect();
    
    // 绘制背景（带圆角）
    auto bgBrush = context->CreateSolidColorBrush(m_bgColor);
    auto borderBrush = context->CreateSolidColorBrush(m_borderColor);
    
    if (bgBrush) {
        // 简化：使用普通矩形，实际应使用圆角矩形
        context->FillRectangle(rect, bgBrush.get());
    }
    
    // 绘制边框
    if (borderBrush) {
        context->DrawRectangle(rect, borderBrush.get(), 1.0f);
    }
    
    // 绘制文本
    auto textBrush = context->CreateSolidColorBrush(m_textColor);
    auto textFormat = context->CreateTextFormat(L"Microsoft YaHei", m_fontSize);
    
    if (textBrush && textFormat) {
        textFormat->SetWordWrapping(rendering::WordWrapping::Wrap);
        
        rendering::Point textPos(rect.x + m_padding, rect.y + m_padding);
        context->DrawTextString(m_text, textFormat.get(), textPos, textBrush.get());
    }
}

// ============================================================================
// ToolTipService
// ============================================================================
void ToolTipService::Initialize() {
    if (s_initialized) return;
    s_initialized = true;
    // 初始化全局事件处理等
}

void ToolTipService::SetToolTip(Control* control, const std::wstring& text) {
    (void)control;
    (void)text;
    Initialize();
    // 实际实现：在 Control 上附加 Tooltip 数据
    // 可以使用附加属性或映射表存储
}

std::wstring ToolTipService::GetToolTip(Control* control) {
    (void)control;
    Initialize();
    return L"";
}

void ToolTipService::SetIsEnabled(Control* control, bool enabled) {
    (void)control;
    (void)enabled;
    Initialize();
}

bool ToolTipService::GetIsEnabled(Control* control) {
    (void)control;
    Initialize();
    return true;
}

void ToolTipService::SetShowDelay(int ms) {
    s_showDelayMs = ms;
}

int ToolTipService::GetShowDelay() {
    return s_showDelayMs;
}

} // namespace controls
} // namespace luaui
