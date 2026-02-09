#pragma once

#include "Control.h"

namespace luaui {
namespace controls {

// ==================== Shape 基类 ====================
class Shape : public Control {
public:
    // 填充画刷
    Color GetFill() const { return m_fill; }
    void SetFill(const Color& color) { m_fill = color; Invalidate(); }
    
    // 描边画刷
    Color GetStroke() const { return m_stroke; }
    void SetStroke(const Color& color) { m_stroke = color; Invalidate(); }
    
    // 描边粗细
    float GetStrokeThickness() const { return m_strokeThickness; }
    void SetStrokeThickness(float thickness) { m_strokeThickness = thickness; Invalidate(); }

protected:
    Shape() = default;
    
    Color m_fill = Color::Transparent();
    Color m_stroke = Color::Black();
    float m_strokeThickness = 1.0f;
};

// ==================== Rectangle ====================
class Rectangle : public Shape {
public:
    std::string GetTypeName() const override { return "Rectangle"; }
    
    // 圆角半径
    float GetRadiusX() const { return m_radiusX; }
    void SetRadiusX(float radius) { m_radiusX = radius; Invalidate(); }
    
    float GetRadiusY() const { return m_radiusY; }
    void SetRadiusY(float radius) { m_radiusY = radius; Invalidate(); }
    
    void Render(IRenderContext* context) override;

protected:
    Size MeasureOverride(const Size& availableSize) override;

private:
    float m_radiusX = 0;
    float m_radiusY = 0;
};

// ==================== Ellipse ====================
class Ellipse : public Shape {
public:
    std::string GetTypeName() const override { return "Ellipse"; }
    
    void Render(IRenderContext* context) override;

protected:
    Size MeasureOverride(const Size& availableSize) override;
};

// ==================== Line ====================
class Line : public Shape {
public:
    std::string GetTypeName() const override { return "Line"; }
    
    // 起点
    float GetX1() const { return m_x1; }
    void SetX1(float x) { m_x1 = x; InvalidateMeasure(); }
    
    float GetY1() const { return m_y1; }
    void SetY1(float y) { m_y1 = y; InvalidateMeasure(); }
    
    // 终点
    float GetX2() const { return m_x2; }
    void SetX2(float x) { m_x2 = x; InvalidateMeasure(); }
    
    float GetY2() const { return m_y2; }
    void SetY2(float y) { m_y2 = y; InvalidateMeasure(); }
    
    void Render(IRenderContext* context) override;

protected:
    Size MeasureOverride(const Size& availableSize) override;
    Size ArrangeOverride(const Size& finalSize) override;

private:
    float m_x1 = 0, m_y1 = 0;
    float m_x2 = 100, m_y2 = 100;
};

// ==================== Polygon ====================
class Polygon : public Shape {
public:
    std::string GetTypeName() const override { return "Polygon"; }
    
    // 顶点列表
    void SetPoints(const std::vector<std::pair<float, float>>& points);
    const std::vector<std::pair<float, float>>& GetPoints() const { return m_points; }
    
    void Render(IRenderContext* context) override;

protected:
    Size MeasureOverride(const Size& availableSize) override;

private:
    std::vector<std::pair<float, float>> m_points;
    float m_minX = 0, m_minY = 0, m_maxX = 0, m_maxY = 0;
    void UpdateBounds();
};

} // namespace controls
} // namespace luaui
