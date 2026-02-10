#include "Shapes.h"
#include <algorithm>

namespace luaui {
namespace controls {

// ==================== Rectangle ====================
Size Rectangle::MeasureOverride(const Size& availableSize) {
    // 如果设置了固定宽高，使用固定�?
    float width = m_width > 0 ? m_width : availableSize.width;
    float height = m_height > 0 ? m_height : availableSize.height;
    
    // 如果没有固定值且可用尺寸无限，给一个默认尺�?
    if (width <= 0 || width == std::numeric_limits<float>::max()) width = 100;
    if (height <= 0 || height == std::numeric_limits<float>::max()) height = 100;
    
    return Size(width, height);
}

void Rectangle::Render(IRenderContext* context) {
    // 填充
    if (m_fill.a > 0) {
        auto brush = context->CreateSolidColorBrush(m_fill);
        if (m_radiusX > 0 || m_radiusY > 0) {
            context->FillRoundedRectangle(m_renderRect, 
                CornerRadius(m_radiusY, m_radiusX, m_radiusY, m_radiusX), brush.get());
        } else {
            context->FillRectangle(m_renderRect, brush.get());
        }
    }
    
    // 描边
    if (m_strokeThickness > 0 && m_stroke.a > 0) {
        auto brush = context->CreateSolidColorBrush(m_stroke);
        if (m_radiusX > 0 || m_radiusY > 0) {
            context->DrawRoundedRectangle(m_renderRect, 
                CornerRadius(m_radiusY, m_radiusX, m_radiusY, m_radiusX),
                brush.get(), m_strokeThickness);
        } else {
            context->DrawRectangle(m_renderRect, brush.get(), m_strokeThickness);
        }
    }
}

// ==================== Ellipse ====================
Size Ellipse::MeasureOverride(const Size& availableSize) {
    float width = m_width > 0 ? m_width : availableSize.width;
    float height = m_height > 0 ? m_height : availableSize.height;
    
    if (width <= 0 || width == std::numeric_limits<float>::max()) width = 100;
    if (height <= 0 || height == std::numeric_limits<float>::max()) height = 100;
    
    return Size(width, height);
}

void Ellipse::Render(IRenderContext* context) {
    // 计算椭圆中心点和半径
    float centerX = m_renderRect.x + m_renderRect.width / 2;
    float centerY = m_renderRect.y + m_renderRect.height / 2;
    float radiusX = m_renderRect.width / 2;
    float radiusY = m_renderRect.height / 2;
    
    // 填充
    if (m_fill.a > 0) {
        auto brush = context->CreateSolidColorBrush(m_fill);
        context->FillEllipse(Point(centerX, centerY), radiusX, radiusY, brush.get());
    }
    
    // 描边
    if (m_strokeThickness > 0 && m_stroke.a > 0) {
        auto brush = context->CreateSolidColorBrush(m_stroke);
        context->DrawEllipse(Point(centerX, centerY), radiusX, radiusY, brush.get(), m_strokeThickness);
    }
}

// ==================== Line ====================
Size Line::MeasureOverride(const Size& /*availableSize*/) {
    // 计算线段包围�?
    float minX = std::min(m_x1, m_x2);
    float maxX = std::max(m_x1, m_x2);
    float minY = std::min(m_y1, m_y2);
    float maxY = std::max(m_y1, m_y2);
    
    float width = maxX - minX;
    float height = maxY - minY;
    
    // 添加描边宽度
    width += m_strokeThickness;
    height += m_strokeThickness;
    
    return Size(width, height);
}

Size Line::ArrangeOverride(const Size& finalSize) {
    // 保存当前�?arrange 位置，用于后续渲�?
    return finalSize;
}

void Line::Render(IRenderContext* context) {
    if (m_strokeThickness <= 0 || m_stroke.a <= 0) return;
    
    // 将相对坐标转换为绝对坐标
    float absX1 = m_renderRect.x + m_x1;
    float absY1 = m_renderRect.y + m_y1;
    float absX2 = m_renderRect.x + m_x2;
    float absY2 = m_renderRect.y + m_y2;
    
    auto brush = context->CreateSolidColorBrush(m_stroke);
    context->DrawLine(Point(absX1, absY1), Point(absX2, absY2), brush.get(), m_strokeThickness);
}

// ==================== Polygon ====================
void Polygon::SetPoints(const std::vector<std::pair<float, float>>& points) {
    m_points = points;
    UpdateBounds();
    InvalidateMeasure();
}

void Polygon::UpdateBounds() {
    if (m_points.empty()) {
        m_minX = m_minY = m_maxX = m_maxY = 0;
        return;
    }
    
    m_minX = m_maxX = m_points[0].first;
    m_minY = m_maxY = m_points[0].second;
    
    for (const auto& pt : m_points) {
        m_minX = std::min(m_minX, pt.first);
        m_maxX = std::max(m_maxX, pt.first);
        m_minY = std::min(m_minY, pt.second);
        m_maxY = std::max(m_maxY, pt.second);
    }
}

Size Polygon::MeasureOverride(const Size& /*availableSize*/) {
    if (m_points.empty()) return Size(0, 0);
    
    float width = m_maxX - m_minX;
    float height = m_maxY - m_minY;
    
    // 添加描边宽度
    width += m_strokeThickness;
    height += m_strokeThickness;
    
    return Size(width, height);
}

void Polygon::Render(IRenderContext* context) {
    if (m_points.size() < 3) return;
    
    // TODO: 使用路径几何绘制多边�?
    // 由于当前渲染接口可能没有完整的路径支持，这里简化为绘制边线
    
    // 填充（简化实现：使用扇形填充或需要路径支持）
    if (m_fill.a > 0) {
        // 填充多边形需要路径几何支持，这里暂时不实�?
        // 或者可以分解为多个三角形填�?
    }
    
    // 描边
    if (m_strokeThickness > 0 && m_stroke.a > 0) {
        auto brush = context->CreateSolidColorBrush(m_stroke);
        
        // 将相对坐标转换为绝对坐标并绘制边�?
        for (size_t i = 0; i < m_points.size(); ++i) {
            size_t j = (i + 1) % m_points.size();
            
            float absX1 = m_renderRect.x + m_points[i].first - m_minX;
            float absY1 = m_renderRect.y + m_points[i].second - m_minY;
            float absX2 = m_renderRect.x + m_points[j].first - m_minX;
            float absY2 = m_renderRect.y + m_points[j].second - m_minY;
            
            context->DrawLine(Point(absX1, absY1), Point(absX2, absY2), 
                            brush.get(), m_strokeThickness);
        }
    }
}

} // namespace controls
} // namespace luaui
