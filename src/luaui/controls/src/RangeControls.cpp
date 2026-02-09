#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "RangeControls.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace luaui {
namespace controls {

// ==================== Slider ====================
Slider::Slider() {
    SetIsFocusable(true);
    SetWidth(200);
    SetHeight(30);
}

void Slider::SetMinimum(double minimum) {
    if (!std::isfinite(minimum)) return;
    if (m_minimum != minimum) {
        m_minimum = minimum;
        // 确保 maximum >= minimum
        if (m_maximum < m_minimum) {
            m_maximum = m_minimum;
        }
        SetValue(m_value);  // 重新约束�?
        Invalidate();
    }
}

void Slider::SetMaximum(double maximum) {
    if (!std::isfinite(maximum)) return;
    if (m_maximum != maximum) {
        m_maximum = maximum;
        // 确保 maximum >= minimum
        if (m_maximum < m_minimum) {
            m_minimum = m_maximum;
        }
        SetValue(m_value);  // 重新约束�?
        Invalidate();
    }
}

void Slider::SetValue(double value) {
    if (!std::isfinite(value)) return;
    
    // 约束值在范围�?
    value = (std::max)(m_minimum, (std::min)(value, m_maximum));
    
    // 应用步长
    if (m_step > 0 && std::isfinite(m_step)) {
        double steps = std::round((value - m_minimum) / m_step);
        value = m_minimum + steps * m_step;
        // 再次约束，确保步长计算后仍在范围�?
        value = (std::max)(m_minimum, (std::min)(value, m_maximum));
    }
    
    if (m_value != value) {
        m_value = value;
        RequestRedraw();  // 使用 RequestRedraw 触发重绘
        if (m_valueChangedHandler) {
            m_valueChangedHandler(this, m_value);
        }
    }
}

void Slider::SetOrientation(Orientation orient) {
    if (m_orientation != orient) {
        m_orientation = orient;
        InvalidateMeasure();
    }
}

Size Slider::MeasureOverride(const Size& availableSize) {
    if (m_orientation == Orientation::Horizontal) {
        return Size(200, ThumbSize);
    } else {
        return Size(ThumbSize, 200);
    }
}

void Slider::Render(IRenderContext* context) {
    if (!context) return;
    if (m_actualWidth <= 0 || m_actualHeight <= 0) return;
    
    bool isHorizontal = (m_orientation == Orientation::Horizontal);
    
    // 计算轨道�?thumb 位置
    float trackLength = isHorizontal ? m_actualWidth : m_actualHeight;
    float thumbPos = CalculateThumbPosition();
    
    // 轨道颜色
    Color trackColor = Color::FromHex(0xE0E0E0);
    Color fillColor = Color::FromHex(0x0078D4);
    
    // 绘制轨道背景
    auto trackBrush = context->CreateSolidColorBrush(trackColor);
    if (isHorizontal) {
        float trackY = m_renderRect.y + (m_actualHeight - TrackThickness) / 2;
        Rect trackRect(m_renderRect.x, trackY, m_actualWidth, TrackThickness);
        context->FillRectangle(trackRect, trackBrush.get());
        
        // 绘制已填充部�?
        auto fillBrush = context->CreateSolidColorBrush(fillColor);
        Rect fillRect(m_renderRect.x, trackY, thumbPos, TrackThickness);
        context->FillRectangle(fillRect, fillBrush.get());
    } else {
        float trackX = m_renderRect.x + (m_actualWidth - TrackThickness) / 2;
        Rect trackRect(trackX, m_renderRect.y, TrackThickness, m_actualHeight);
        context->FillRectangle(trackRect, trackBrush.get());
        
        // 绘制已填充部�?
        auto fillBrush = context->CreateSolidColorBrush(fillColor);
        float fillHeight = thumbPos;
        Rect fillRect(trackX, m_renderRect.y + m_actualHeight - fillHeight, 
                     TrackThickness, fillHeight);
        context->FillRectangle(fillRect, fillBrush.get());
    }
    
    // 绘制 thumb
    Color thumbColor = GetIsFocused() ? Color::FromHex(0x106EBE) : Color::FromHex(0xFFFFFF);
    auto thumbBrush = context->CreateSolidColorBrush(thumbColor);
    auto thumbBorderBrush = context->CreateSolidColorBrush(Color::FromHex(0x808080));
    
    if (isHorizontal) {
        float thumbX = m_renderRect.x + thumbPos - ThumbSize / 2;
        float thumbY = m_renderRect.y + (m_actualHeight - ThumbSize) / 2;
        Rect thumbRect(thumbX, thumbY, ThumbSize, ThumbSize);
        context->FillRoundedRectangle(thumbRect, CornerRadius(ThumbSize/2), thumbBrush.get());
        context->DrawRoundedRectangle(thumbRect, CornerRadius(ThumbSize/2), 
                                     thumbBorderBrush.get(), 1.0f);
    } else {
        float thumbY = m_renderRect.y + m_actualHeight - thumbPos - ThumbSize / 2;
        float thumbX = m_renderRect.x + (m_actualWidth - ThumbSize) / 2;
        Rect thumbRect(thumbX, thumbY, ThumbSize, ThumbSize);
        context->FillRoundedRectangle(thumbRect, CornerRadius(ThumbSize/2), thumbBrush.get());
        context->DrawRoundedRectangle(thumbRect, CornerRadius(ThumbSize/2), 
                                     thumbBorderBrush.get(), 1.0f);
    }
}

float Slider::CalculateThumbPosition() const {
    bool isHorizontal = (m_orientation == Orientation::Horizontal);
    float trackLength = isHorizontal ? m_actualWidth : m_actualHeight;
    
    double range = m_maximum - m_minimum;
    if (range <= 0) return 0;
    
    double ratio = (m_value - m_minimum) / range;
    return static_cast<float>(ratio * trackLength);
}

double Slider::ValueFromPosition(float position, float trackLength) {
    if (trackLength <= 0.0f || !std::isfinite(trackLength)) return m_minimum;
    if (!std::isfinite(position)) return m_value;
    
    double ratio = std::max(0.0f, std::min(position / trackLength, 1.0f));
    double value = m_minimum + ratio * (m_maximum - m_minimum);
    return value;
}

void Slider::HandleMouseDown(const Point& pt) {
    m_isDragging = true;
    
    bool isHorizontal = (m_orientation == Orientation::Horizontal);
    float pos = isHorizontal ? pt.x - m_renderRect.x 
                            : m_actualHeight - (pt.y - m_renderRect.y);
    float trackLength = isHorizontal ? m_actualWidth : m_actualHeight;
    
    // 约束位置在有效范围内
    pos = (std::max)(0.0f, (std::min)(pos, trackLength));
    
    SetValue(ValueFromPosition(pos, trackLength));
}

void Slider::HandleMouseMove(const Point& pt) {
    if (m_isDragging) {
        bool isHorizontal = (m_orientation == Orientation::Horizontal);
        float pos = isHorizontal ? pt.x - m_renderRect.x 
                                : m_actualHeight - (pt.y - m_renderRect.y);
        float trackLength = isHorizontal ? m_actualWidth : m_actualHeight;
        
        // 约束位置在有效范围内
        pos = (std::max)(0.0f, (std::min)(pos, trackLength));
        
        SetValue(ValueFromPosition(pos, trackLength));
    }
}

void Slider::HandleMouseUp(const Point& /*pt*/) {
    m_isDragging = false;
}

// ==================== ProgressBar ====================
ProgressBar::ProgressBar() {
    SetWidth(200);
    SetHeight(20);
}

void ProgressBar::SetValue(double value) {
    value = (std::max)(0.0, (std::min)(value, 100.0));
    if (m_value != value) {
        m_value = value;
        Invalidate();
    }
}

Size ProgressBar::MeasureOverride(const Size& availableSize) {
    if (m_orientation == Orientation::Horizontal) {
        return Size(200, 20);
    } else {
        return Size(20, 200);
    }
}

void ProgressBar::Render(IRenderContext* context) {
    bool isHorizontal = (m_orientation == Orientation::Horizontal);
    
    // 背景
    auto bgBrush = context->CreateSolidColorBrush(Color::FromHex(0xF0F0F0));
    context->FillRectangle(m_renderRect, bgBrush.get());
    
    // 边框
    auto borderBrush = context->CreateSolidColorBrush(Color::FromHex(0xCCCCCC));
    context->DrawRectangle(m_renderRect, borderBrush.get(), 1.0f);
    
    // 填充
    auto fillBrush = context->CreateSolidColorBrush(m_fillColor);
    if (isHorizontal) {
        float fillWidth = static_cast<float>(m_value / 100.0 * m_actualWidth);
        if (fillWidth > 0) {
            Rect fillRect(m_renderRect.x, m_renderRect.y, fillWidth, m_actualHeight);
            context->FillRectangle(fillRect, fillBrush.get());
        }
    } else {
        float fillHeight = static_cast<float>(m_value / 100.0 * m_actualHeight);
        if (fillHeight > 0) {
            Rect fillRect(m_renderRect.x, m_renderRect.y + m_actualHeight - fillHeight,
                         m_actualWidth, fillHeight);
            context->FillRectangle(fillRect, fillBrush.get());
        }
    }
    
    // 文本
    if (m_showText) {
        std::wstringstream ss;
        ss << std::fixed << std::setprecision(0) << m_value << L"%";
        
        auto textBrush = context->CreateSolidColorBrush(Color::Black());
        auto textFormat = context->CreateTextFormat(L"Segoe UI", 12.0f);
        
        // 居中显示
        Size textSize = textFormat->MeasureText(ss.str(), 100);
        float textX = m_renderRect.x + (m_actualWidth - textSize.width) / 2;
        float textY = m_renderRect.y + (m_actualHeight - textSize.height) / 2;
        
        context->DrawTextString(ss.str(), textFormat.get(), 
                               Point(textX, textY), textBrush.get());
    }
}

} // namespace controls
} // namespace luaui
