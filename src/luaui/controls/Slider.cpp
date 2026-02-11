#include "Slider.h"
#include "IRenderContext.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include "Components/InputComponent.h"
#include <algorithm>
#include <cmath>

namespace luaui {
namespace controls {

// ============================================================================
// Slider
// ============================================================================
Slider::Slider() {}

void Slider::InitializeComponents() {
    auto* layout = GetComponents().AddComponent<components::LayoutComponent>(this);
    GetComponents().AddComponent<components::RenderComponent>(this);
    
    auto* input = GetComponents().AddComponent<components::InputComponent>(this);
    input->SetIsFocusable(true);
    
    if (m_isVertical) {
        layout->SetWidth(16);
        layout->SetHeight(100);
    } else {
        layout->SetWidth(200);
        layout->SetHeight(16);
    }
}

rendering::Size Slider::OnMeasure(const rendering::Size& availableSize) {
    if (m_isVertical) {
        return rendering::Size(16, 100);
    } else {
        return rendering::Size(200, 16);
    }
}

void Slider::SetValue(double value) {
    double oldValue = m_value;
    m_value = value;
    ClampValue();
    
    if (m_value != oldValue) {
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
        ValueChanged.Invoke(this, m_value);
    }
}

void Slider::SetMinimum(double min) {
    if (m_minimum != min) {
        m_minimum = min;
        ClampValue();
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void Slider::SetMaximum(double max) {
    if (m_maximum != max) {
        m_maximum = max;
        ClampValue();
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void Slider::SetIsVertical(bool vertical) {
    if (m_isVertical != vertical) {
        m_isVertical = vertical;
        if (auto* layout = GetLayout()) {
            if (m_isVertical) {
                layout->SetMinWidth(16);
                layout->SetMinHeight(100);
            } else {
                layout->SetMinWidth(100);
                layout->SetMinHeight(16);
            }
        }
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void Slider::ClampValue() {
    if (m_value < m_minimum) m_value = m_minimum;
    if (m_value > m_maximum) m_value = m_maximum;
}

void Slider::OnRender(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    rendering::Rect localRect(0, 0, render->GetRenderRect().width, render->GetRenderRect().height);
    
    // 颜色定义
    rendering::Color trackColor = rendering::Color::FromHex(0xE0E0E0);      // 轨道颜色
    rendering::Color progressColor = rendering::Color::FromHex(0x2196F3);   // 进度颜色
    rendering::Color thumbColor = rendering::Color::White();                 // 滑块颜色
    rendering::Color thumbBorder = rendering::Color::FromHex(0x1976D2);     // 滑块边框
    
    float trackHeight = 4.0f;    // 轨道高度
    float thumbSize = 12.0f;     // 滑块大小
    
    if (m_isVertical) {
        // 垂直滑块
        float trackX = localRect.width / 2;
        float trackY = thumbSize / 2;
        float trackLength = localRect.height - thumbSize;
        
        // 绘制轨道
        auto trackBrush = context->CreateSolidColorBrush(trackColor);
        if (trackBrush) {
            rendering::Rect trackRect(trackX - trackHeight/2, trackY, trackHeight, trackLength);
            context->FillRectangle(trackRect, trackBrush.get());
        }
        
        // 计算进度位置
        double range = m_maximum - m_minimum;
        double ratio = range > 0 ? (m_value - m_minimum) / range : 0;
        float progressY = trackY + trackLength * (1.0f - static_cast<float>(ratio));
        
        // 绘制进度条（从底部到当前位置）
        auto progressBrush = context->CreateSolidColorBrush(progressColor);
        if (progressBrush) {
            rendering::Rect progressRect(trackX - trackHeight/2, progressY, trackHeight, 
                                         trackY + trackLength - progressY);
            context->FillRectangle(progressRect, progressBrush.get());
        }
        
        // 绘制滑块
        auto thumbBgBrush = context->CreateSolidColorBrush(thumbColor);
        auto thumbBorderBrush = context->CreateSolidColorBrush(thumbBorder);
        if (thumbBgBrush && thumbBorderBrush) {
            rendering::Point thumbCenter(trackX, progressY);
            context->FillEllipse(thumbCenter, thumbSize/2, thumbSize/2, thumbBgBrush.get());
            context->DrawEllipse(thumbCenter, thumbSize/2, thumbSize/2, thumbBorderBrush.get(), 1.5f);
        }
    } else {
        // 水平滑块
        float trackY = localRect.height / 2;
        float trackX = thumbSize / 2;
        float trackLength = localRect.width - thumbSize;
        
        // 绘制轨道
        auto trackBrush = context->CreateSolidColorBrush(trackColor);
        if (trackBrush) {
            rendering::Rect trackRect(trackX, trackY - trackHeight/2, trackLength, trackHeight);
            context->FillRectangle(trackRect, trackBrush.get());
        }
        
        // 计算进度位置
        double range = m_maximum - m_minimum;
        double ratio = range > 0 ? (m_value - m_minimum) / range : 0;
        float progressX = trackX + trackLength * static_cast<float>(ratio);
        
        // 绘制进度条
        auto progressBrush = context->CreateSolidColorBrush(progressColor);
        if (progressBrush) {
            rendering::Rect progressRect(trackX, trackY - trackHeight/2, progressX - trackX, trackHeight);
            context->FillRectangle(progressRect, progressBrush.get());
        }
        
        // 绘制滑块
        auto thumbBgBrush = context->CreateSolidColorBrush(thumbColor);
        auto thumbBorderBrush = context->CreateSolidColorBrush(thumbBorder);
        if (thumbBgBrush && thumbBorderBrush) {
            rendering::Point thumbCenter(progressX, trackY);
            context->FillEllipse(thumbCenter, thumbSize/2, thumbSize/2, thumbBgBrush.get());
            context->DrawEllipse(thumbCenter, thumbSize/2, thumbSize/2, thumbBorderBrush.get(), 1.5f);
        }
    }
}

// ============================================================================
// ProgressBar
// ============================================================================
ProgressBar::ProgressBar() {}

void ProgressBar::InitializeComponents() {
    auto* layout = GetComponents().AddComponent<components::LayoutComponent>(this);
    GetComponents().AddComponent<components::RenderComponent>(this);
    
    if (m_isVertical) {
        layout->SetWidth(8);
        layout->SetHeight(100);
    } else {
        layout->SetWidth(200);
        layout->SetHeight(8);
    }
}

rendering::Size ProgressBar::OnMeasure(const rendering::Size& availableSize) {
    if (m_isVertical) {
        return rendering::Size(8, 100);
    } else {
        return rendering::Size(200, 8);
    }
}

void ProgressBar::SetValue(double value) {
    if (m_value != value && !m_isIndeterminate) {
        m_value = value;
        if (m_value < m_minimum) m_value = m_minimum;
        if (m_value > m_maximum) m_value = m_maximum;
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void ProgressBar::SetMinimum(double min) {
    if (m_minimum != min) {
        m_minimum = min;
        if (m_value < m_minimum) {
            m_value = m_minimum;
        }
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void ProgressBar::SetMaximum(double max) {
    if (m_maximum != max) {
        m_maximum = max;
        if (m_value > m_maximum) {
            m_value = m_maximum;
        }
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void ProgressBar::SetIsIndeterminate(bool indeterminate) {
    if (m_isIndeterminate != indeterminate) {
        m_isIndeterminate = indeterminate;
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void ProgressBar::SetIsVertical(bool vertical) {
    if (m_isVertical != vertical) {
        m_isVertical = vertical;
        if (auto* layout = GetLayout()) {
            if (m_isVertical) {
                layout->SetWidth(8);
                layout->SetHeight(0);
                layout->SetMinHeight(100);
            } else {
                layout->SetWidth(0);
                layout->SetHeight(8);
                layout->SetMinWidth(100);
            }
        }
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void ProgressBar::OnRender(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    rendering::Rect localRect(0, 0, render->GetRenderRect().width, render->GetRenderRect().height);
    
    // 颜色定义
    rendering::Color trackColor = rendering::Color::FromHex(0xE0E0E0);      // 轨道颜色
    rendering::Color progressColor = rendering::Color::FromHex(0x4CAF50);   // 进度颜色（绿色）
    
    if (m_isIndeterminate) {
        // 不确定状态：绘制动画条（简化版，静态显示）
        auto trackBrush = context->CreateSolidColorBrush(trackColor);
        if (trackBrush) {
            context->FillRectangle(localRect, trackBrush.get());
        }
        
        // 绘制移动中的进度块（简化：固定在中间）
        auto progressBrush = context->CreateSolidColorBrush(progressColor);
        if (progressBrush) {
            if (m_isVertical) {
                float blockHeight = localRect.height / 3;
                rendering::Rect blockRect(0, localRect.height / 3, localRect.width, blockHeight);
                context->FillRectangle(blockRect, progressBrush.get());
            } else {
                float blockWidth = localRect.width / 3;
                rendering::Rect blockRect(localRect.width / 3, 0, blockWidth, localRect.height);
                context->FillRectangle(blockRect, progressBrush.get());
            }
        }
    } else {
        // 确定状态
        // 计算进度比例
        double range = m_maximum - m_minimum;
        double ratio = range > 0 ? (m_value - m_minimum) / range : 0;
        
        // 绘制轨道背景
        auto trackBrush = context->CreateSolidColorBrush(trackColor);
        if (trackBrush) {
            context->FillRectangle(localRect, trackBrush.get());
        }
        
        // 绘制进度
        auto progressBrush = context->CreateSolidColorBrush(progressColor);
        if (progressBrush) {
            if (m_isVertical) {
                float progressHeight = localRect.height * static_cast<float>(ratio);
                rendering::Rect progressRect(0, localRect.height - progressHeight, 
                                             localRect.width, progressHeight);
                context->FillRectangle(progressRect, progressBrush.get());
            } else {
                float progressWidth = localRect.width * static_cast<float>(ratio);
                rendering::Rect progressRect(0, 0, progressWidth, localRect.height);
                context->FillRectangle(progressRect, progressBrush.get());
            }
        }
    }
}

} // namespace controls
} // namespace luaui
