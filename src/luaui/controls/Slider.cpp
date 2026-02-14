#include "Slider.h"
#include "IRenderContext.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include "Components/InputComponent.h"
#include "Logger.h"
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
    (void)availableSize;
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

void Slider::UpdateValueFromPosition(float x, float y) {
    auto* render = GetRender();
    if (!render) {
        utils::Logger::Trace("[Slider] UpdateValue: No render component!");
        return;
    }
    
    // 计算控件的全局位置（累加所有父容器的位置）
    float globalX = 0, globalY = 0;
    Control* current = this;
    int depth = 0;
    while (current) {
        if (auto* r = current->GetRender()) {
            globalX += r->GetRenderRect().x;
            globalY += r->GetRenderRect().y;
        }
        auto parent = current->GetParent();
        if (parent) {
            current = dynamic_cast<Control*>(parent.get());
        } else {
            current = nullptr;
        }
        depth++;
    }
    
    // 传入的是全局坐标，转换为本地坐标
    float localX = x - globalX;
    float localY = y - globalY;
    
    static int count = 0;
    if (++count % 10 == 0) {
        utils::Logger::TraceF("[Slider] Value=%.2f localX=%.2f", m_value, localX);
    }
    
    float thumbSize = 12.0f;
    
    if (m_isVertical) {
        float trackLength = render->GetRenderRect().height - thumbSize;
        float trackY = thumbSize / 2;
        
        // 限制在轨道范围内
        localY = std::max(trackY, std::min(localY, trackY + trackLength));
        
        // 计算比例（从底部开始）
        double ratio = 1.0 - (localY - trackY) / trackLength;
        ratio = std::max(0.0, std::min(1.0, ratio));
        
        double newValue = m_minimum + ratio * (m_maximum - m_minimum);
        SetValue(newValue);
    } else {
        float trackLength = render->GetRenderRect().width - thumbSize;
        float trackX = thumbSize / 2;
        
        // 限制在轨道范围内
        localX = std::max(trackX, std::min(localX, trackX + trackLength));
        
        // 计算比例
        double ratio = (localX - trackX) / trackLength;
        ratio = std::max(0.0, std::min(1.0, ratio));
        
        double newValue = m_minimum + ratio * (m_maximum - m_minimum);
        SetValue(newValue);
    }
}

void Slider::OnMouseDown(MouseEventArgs& args) {
    (void)args;
    utils::Logger::Trace("[Slider] MouseDown -> Drag START");
    m_isDragging = true;
    UpdateValueFromPosition(args.x, args.y);
}

void Slider::OnMouseUp(MouseEventArgs& args) {
    (void)args;
    utils::Logger::Trace("[Slider] MouseUp -> Drag STOP");
    m_isDragging = false;
}

void Slider::OnMouseMove(MouseEventArgs& args) {
    if (m_isDragging) {
        static int count = 0;
        if (++count % 5 == 0) {
            utils::Logger::TraceF("[Slider] OnMouseMove x=%.2f y=%.2f", args.x, args.y);
        }
        UpdateValueFromPosition(args.x, args.y);
    }
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

} // namespace controls
} // namespace luaui
