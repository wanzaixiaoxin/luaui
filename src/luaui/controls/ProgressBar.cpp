#include "ProgressBar.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include "Interfaces/IRenderable.h"
#include "IRenderContext.h"
#include <algorithm>
#include <cmath>

namespace luaui {
namespace controls {

// ============================================================================
// ProgressBar
// ============================================================================
ProgressBar::ProgressBar() {}

void ProgressBar::InitializeComponents() {
    GetComponents().AddComponent<components::LayoutComponent>(this);
    GetComponents().AddComponent<components::RenderComponent>(this);
    
    // 设置默认大小
    if (auto* layout = GetLayout()) {
        if (m_orientation == Orientation::Horizontal) {
            layout->SetMinWidth(100);
            layout->SetMinHeight(16);
            layout->SetHeight(16);
        } else {
            layout->SetMinWidth(16);
            layout->SetMinHeight(100);
            layout->SetWidth(16);
        }
    }
}

void ProgressBar::SetMinimum(double value) {
    if (m_minimum != value) {
        m_minimum = value;
        if (m_value < m_minimum) {
            m_value = m_minimum;
        }
        UpdateVisualState();
    }
}

void ProgressBar::SetMaximum(double value) {
    if (m_maximum != value) {
        m_maximum = value;
        if (m_value > m_maximum) {
            m_value = m_maximum;
        }
        UpdateVisualState();
    }
}

void ProgressBar::SetValue(double value) {
    value = std::max(m_minimum, std::min(m_maximum, value));
    if (m_value != value) {
        m_value = value;
        UpdateVisualState();
    }
}

double ProgressBar::GetPercent() const {
    if (m_maximum <= m_minimum) return 0.0;
    return (m_value - m_minimum) / (m_maximum - m_minimum);
}

void ProgressBar::SetIsIndeterminate(bool indeterminate) {
    if (m_isIndeterminate != indeterminate) {
        m_isIndeterminate = indeterminate;
        UpdateVisualState();
    }
}

void ProgressBar::SetOrientation(Orientation orientation) {
    if (m_orientation != orientation) {
        m_orientation = orientation;
        
        // 交换宽高约束
        if (auto* layout = GetLayout()) {
            if (orientation == Orientation::Horizontal) {
                layout->SetMinWidth(100);
                layout->SetMinHeight(16);
                layout->ClearWidth();
                layout->SetHeight(16);
            } else {
                layout->SetMinWidth(16);
                layout->SetMinHeight(100);
                layout->SetWidth(16);
                layout->ClearHeight();
            }
            layout->InvalidateMeasure();
        }
    }
}

std::wstring ProgressBar::GetDisplayText() const {
    if (m_textFormat.empty()) return L"";
    
    std::wstring result = m_textFormat;
    
    // 替换 {0} 为百分比
    double percent = GetPercent() * 100.0;
    std::wstring percentStr = std::to_wstring(static_cast<int>(percent));
    size_t pos = result.find(L"{0}");
    if (pos != std::wstring::npos) {
        result.replace(pos, 3, percentStr);
    }
    
    // 替换 {1} 为当前值
    std::wstring valueStr = std::to_wstring(static_cast<int>(m_value));
    pos = result.find(L"{1}");
    if (pos != std::wstring::npos) {
        result.replace(pos, 3, valueStr);
    }
    
    // 替换 {2} 为最大值
    std::wstring maxStr = std::to_wstring(static_cast<int>(m_maximum));
    pos = result.find(L"{2}");
    if (pos != std::wstring::npos) {
        result.replace(pos, 3, maxStr);
    }
    
    return result;
}

void ProgressBar::UpdateVisualState() {
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

rendering::Size ProgressBar::OnMeasure(const rendering::Size& availableSize) {
    (void)availableSize;
    
    if (m_orientation == Orientation::Horizontal) {
        return rendering::Size(
            availableSize.width > 0 ? availableSize.width : 200,
            m_showText ? 24.0f : 16.0f);
    } else {
        return rendering::Size(
            m_showText ? 24.0f : 16.0f,
            availableSize.height > 0 ? availableSize.height : 200);
    }
}

void ProgressBar::OnRender(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    auto rect = render->GetRenderRect();
    
    // 绘制背景
    auto bgBrush = context->CreateSolidColorBrush(m_backgroundColor);
    if (bgBrush) {
        context->FillRectangle(rect, bgBrush.get());
    }
    
    // 绘制进度
    if (m_isIndeterminate) {
        // 不确定模式：绘制动画块
        auto fgBrush = context->CreateSolidColorBrush(m_foregroundColor);
        if (fgBrush) {
            if (m_orientation == Orientation::Horizontal) {
                // 水平不确定动画
                float blockWidth = rect.width * 0.3f; // 30% 宽度
                float offset = m_animationOffset * (rect.width + blockWidth) - blockWidth;
                rendering::Rect blockRect(rect.x + offset, rect.y, blockWidth, rect.height);
                
                // 限制在背景区域内
                float visibleX = std::max(blockRect.x, rect.x);
                float visibleRight = std::min(blockRect.x + blockRect.width, rect.x + rect.width);
                if (visibleRight > visibleX) {
                    context->FillRectangle(
                        rendering::Rect(visibleX, blockRect.y, 
                                       visibleRight - visibleX, blockRect.height),
                        fgBrush.get());
                }
            } else {
                // 垂直不确定动画
                float blockHeight = rect.height * 0.3f;
                float offset = m_animationOffset * (rect.height + blockHeight) - blockHeight;
                rendering::Rect blockRect(rect.x, rect.y + offset, rect.width, blockHeight);
                
                float visibleY = std::max(blockRect.y, rect.y);
                float visibleBottom = std::min(blockRect.y + blockRect.height, rect.y + rect.height);
                if (visibleBottom > visibleY) {
                    context->FillRectangle(
                        rendering::Rect(blockRect.x, visibleY,
                                       blockRect.width, visibleBottom - visibleY),
                        fgBrush.get());
                }
            }
        }
    } else {
        // 确定模式
        double percent = GetPercent();
        auto fgBrush = context->CreateSolidColorBrush(m_foregroundColor);
        if (fgBrush) {
            if (m_orientation == Orientation::Horizontal) {
                float progressWidth = static_cast<float>(rect.width * percent);
                if (progressWidth > 0) {
                    rendering::Rect progressRect(rect.x, rect.y, progressWidth, rect.height);
                    context->FillRectangle(progressRect, fgBrush.get());
                }
            } else {
                float progressHeight = static_cast<float>(rect.height * percent);
                if (progressHeight > 0) {
                    float progressY = rect.y + rect.height - progressHeight; // 从底部开始
                    rendering::Rect progressRect(rect.x, progressY, rect.width, progressHeight);
                    context->FillRectangle(progressRect, fgBrush.get());
                }
            }
        }
    }
    
    // 绘制边框
    auto borderBrush = context->CreateSolidColorBrush(m_borderColor);
    if (borderBrush) {
        context->DrawRectangle(rect, borderBrush.get(), 1.0f);
    }
    
    // 绘制文本
    if (m_showText) {
        std::wstring text = GetDisplayText();
        if (!text.empty()) {
            auto textBrush = context->CreateSolidColorBrush(m_textColor);
            auto textFormat = context->CreateTextFormat(L"Microsoft YaHei", m_fontSize);
            
            if (textBrush && textFormat) {
                textFormat->SetTextAlignment(rendering::TextAlignment::Center);
                textFormat->SetParagraphAlignment(rendering::ParagraphAlignment::Center);
                
                rendering::Point textPos(rect.x + rect.width / 2, rect.y + rect.height / 2 - m_fontSize / 2);
                context->DrawTextString(text, textFormat.get(), textPos, textBrush.get());
            }
        }
    }
}

// ============================================================================
// ProgressRing
// ============================================================================
ProgressRing::ProgressRing() {}

void ProgressRing::InitializeComponents() {
    GetComponents().AddComponent<components::LayoutComponent>(this);
    GetComponents().AddComponent<components::RenderComponent>(this);
    
    // 默认正方形
    if (auto* layout = GetLayout()) {
        layout->SetWidth(40);
        layout->SetHeight(40);
    }
}

void ProgressRing::SetIsIndeterminate(bool indeterminate) {
    if (m_isIndeterminate != indeterminate) {
        m_isIndeterminate = indeterminate;
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void ProgressRing::SetValue(double value) {
    value = std::max(0.0, std::min(1.0, value));
    if (m_value != value) {
        m_value = value;
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

rendering::Size ProgressRing::OnMeasure(const rendering::Size& availableSize) {
    (void)availableSize;
    // 保持正方形
    float size = std::min(availableSize.width > 0 ? availableSize.width : 40.0f,
                          availableSize.height > 0 ? availableSize.height : 40.0f);
    return rendering::Size(size, size);
}

void ProgressRing::OnRender(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    auto rect = render->GetRenderRect();
    
    float centerX = rect.x + rect.width / 2;
    float centerY = rect.y + rect.height / 2;
    float radius = std::min(rect.width, rect.height) / 2 - m_ringThickness;
    
    // 绘制背景环
    auto bgBrush = context->CreateSolidColorBrush(m_backgroundColor);
    if (bgBrush && radius > 0) {
        // 绘制圆形轮廓（简化）
        int segments = 64;
        for (int i = 0; i < segments; ++i) {
            float angle1 = 2.0f * 3.14159f * i / segments;
            float angle2 = 2.0f * 3.14159f * (i + 1) / segments;
            context->DrawLine(
                rendering::Point(centerX + cos(angle1) * radius, centerY + sin(angle1) * radius),
                rendering::Point(centerX + cos(angle2) * radius, centerY + sin(angle2) * radius),
                bgBrush.get(), m_ringThickness);
        }
    }
    
    // 绘制前景环
    auto fgBrush = context->CreateSolidColorBrush(m_foregroundColor);
    if (fgBrush && radius > 0) {
        if (m_isIndeterminate) {
            // 不确定模式：绘制旋转的弧形
            int segments = 32;
            float startAngle = m_animationAngle;
            float sweepAngle = 2.0f * 3.14159f * 0.75f; // 270度弧
            
            for (int i = 0; i < segments; ++i) {
                float t1 = static_cast<float>(i) / segments;
                float t2 = static_cast<float>(i + 1) / segments;
                float angle1 = startAngle + sweepAngle * t1;
                float angle2 = startAngle + sweepAngle * t2;
                
                context->DrawLine(
                    rendering::Point(centerX + cos(angle1) * radius, centerY + sin(angle1) * radius),
                    rendering::Point(centerX + cos(angle2) * radius, centerY + sin(angle2) * radius),
                    fgBrush.get(), m_ringThickness);
            }
        } else {
            // 确定模式：绘制进度弧
            int segments = static_cast<int>(64 * m_value);
            for (int i = 0; i < segments; ++i) {
                float angle1 = -3.14159f / 2 + 2.0f * 3.14159f * i / 64; // 从顶部开始
                float angle2 = -3.14159f / 2 + 2.0f * 3.14159f * (i + 1) / 64;
                context->DrawLine(
                    rendering::Point(centerX + cos(angle1) * radius, centerY + sin(angle1) * radius),
                    rendering::Point(centerX + cos(angle2) * radius, centerY + sin(angle2) * radius),
                    fgBrush.get(), m_ringThickness);
            }
        }
    }
}

} // namespace controls
} // namespace luaui
