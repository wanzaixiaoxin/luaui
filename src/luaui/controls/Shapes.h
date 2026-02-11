#pragma once

#include "Control.h"
#include <string>

namespace luaui {
namespace controls {

/**
 * @brief Rectangle 矩形形状（新架构）
 */
class Rectangle : public Control {
public:
    Rectangle();
    
    std::string GetTypeName() const override { return "Rectangle"; }
    
    // 填充
    rendering::Color GetFill() const { return m_fill; }
    void SetFill(const rendering::Color& color);
    
    // 描边
    rendering::Color GetStroke() const { return m_stroke; }
    void SetStroke(const rendering::Color& color);
    
    float GetStrokeThickness() const { return m_strokeThickness; }
    void SetStrokeThickness(float thickness);
    
    // 圆角
    float GetRadiusX() const { return m_radiusX; }
    void SetRadiusX(float radius);
    
    float GetRadiusY() const { return m_radiusY; }
    void SetRadiusY(float radius);

protected:
    void InitializeComponents() override;

private:
    rendering::Color m_fill = rendering::Color::White();
    rendering::Color m_stroke = rendering::Color::Transparent();
    float m_strokeThickness = 1.0f;
    float m_radiusX = 0.0f;
    float m_radiusY = 0.0f;
};

/**
 * @brief Ellipse 椭圆形状（新架构）
 */
class Ellipse : public Control {
public:
    Ellipse();
    
    std::string GetTypeName() const override { return "Ellipse"; }
    
    // 填充
    rendering::Color GetFill() const { return m_fill; }
    void SetFill(const rendering::Color& color);
    
    // 描边
    rendering::Color GetStroke() const { return m_stroke; }
    void SetStroke(const rendering::Color& color);
    
    float GetStrokeThickness() const { return m_strokeThickness; }
    void SetStrokeThickness(float thickness);

protected:
    void InitializeComponents() override;

private:
    rendering::Color m_fill = rendering::Color::White();
    rendering::Color m_stroke = rendering::Color::Transparent();
    float m_strokeThickness = 1.0f;
};

/**
 * @brief Line 线条形状（新架构）
 */
class Line : public Control {
public:
    Line();
    
    std::string GetTypeName() const override { return "Line"; }
    
    // 起点
    float GetX1() const { return m_x1; }
    void SetX1(float x);
    
    float GetY1() const { return m_y1; }
    void SetY1(float y);
    
    // 终点
    float GetX2() const { return m_x2; }
    void SetX2(float x);
    
    float GetY2() const { return m_y2; }
    void SetY2(float y);
    
    // 颜色
    rendering::Color GetStroke() const { return m_stroke; }
    void SetStroke(const rendering::Color& color);
    
    // 粗细
    float GetStrokeThickness() const { return m_strokeThickness; }
    void SetStrokeThickness(float thickness);

protected:
    void InitializeComponents() override;

private:
    float m_x1 = 0.0f;
    float m_y1 = 0.0f;
    float m_x2 = 100.0f;
    float m_y2 = 100.0f;
    rendering::Color m_stroke = rendering::Color::Black();
    float m_strokeThickness = 1.0f;
};

} // namespace controls
} // namespace luaui
