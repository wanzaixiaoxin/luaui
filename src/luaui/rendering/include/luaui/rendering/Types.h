#pragma once

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <algorithm>

namespace luaui {
namespace rendering {

// Color with premultiplied alpha support
struct Color {
    float r, g, b, a;
    
    Color() : r(0), g(0), b(0), a(1.0f) {}
    Color(float r, float g, float b, float a = 1.0f) 
        : r(std::clamp(r, 0.0f, 1.0f))
        , g(std::clamp(g, 0.0f, 1.0f))
        , b(std::clamp(b, 0.0f, 1.0f))
        , a(std::clamp(a, 0.0f, 1.0f)) {}
    
    // Preset colors
    static Color Transparent() { return Color(0, 0, 0, 0); }
    static Color Black() { return Color(0, 0, 0); }
    static Color White() { return Color(1, 1, 1); }
    static Color Red() { return Color(1, 0, 0); }
    static Color Green() { return Color(0, 1, 0); }
    static Color Blue() { return Color(0, 0, 1); }
    
    // Premultiply alpha for Direct2D
    Color Premultiply() const {
        return Color(r * a, g * a, b * a, a);
    }
};

// 2D Point
struct Point {
    float x, y;
    
    Point() : x(0), y(0) {}
    Point(float x, float y) : x(x), y(y) {}
    
    Point operator+(const Point& other) const { return Point(x + other.x, y + other.y); }
    Point operator-(const Point& other) const { return Point(x - other.x, y - other.y); }
    Point operator*(float scale) const { return Point(x * scale, y * scale); }
};

// 2D Size
struct Size {
    float width, height;
    
    Size() : width(0), height(0) {}
    Size(float w, float h) : width(w), height(h) {}
    
    bool IsEmpty() const { return width <= 0 || height <= 0; }
    
    Size operator*(float scale) const { return Size(width * scale, height * scale); }
};

// Rectangle
struct Rect {
    float x, y, width, height;
    
    Rect() : x(0), y(0), width(0), height(0) {}
    Rect(float x, float y, float w, float h) : x(x), y(y), width(w), height(h) {}
    Rect(const Point& pos, const Size& size) : x(pos.x), y(pos.y), width(size.width), height(size.height) {}
    
    float Left() const { return x; }
    float Right() const { return x + width; }
    float Top() const { return y; }
    float Bottom() const { return y + height; }
    
    Point Position() const { return Point(x, y); }
    Size GetSize() const { return Size(width, height); }
    
    bool IsEmpty() const { return width <= 0 || height <= 0; }
    
    bool Contains(const Point& p) const {
        return p.x >= x && p.x < Right() && p.y >= y && p.y < Bottom();
    }
    
    bool Intersects(const Rect& other) const {
        return !(Right() < other.Left() || Left() > other.Right() ||
                 Bottom() < other.Top() || Top() > other.Bottom());
    }
    
    Rect Intersect(const Rect& other) const {
        float x1 = (std::max)(Left(), other.Left());
        float y1 = (std::max)(Top(), other.Top());
        float x2 = (std::min)(Right(), other.Right());
        float y2 = (std::min)(Bottom(), other.Bottom());
        
        if (x2 > x1 && y2 > y1) {
            return Rect(x1, y1, x2 - x1, y2 - y1);
        }
        return Rect();
    }
};

// Corner radius for rounded rectangles
struct CornerRadius {
    float topLeft, topRight, bottomRight, bottomLeft;
    
    CornerRadius() : topLeft(0), topRight(0), bottomRight(0), bottomLeft(0) {}
    CornerRadius(float all) : topLeft(all), topRight(all), bottomRight(all), bottomLeft(all) {}
    CornerRadius(float tl, float tr, float br, float bl) 
        : topLeft(tl), topRight(tr), bottomRight(br), bottomLeft(bl) {}
};

// 3x2 transformation matrix (2D affine transform)
class Transform {
public:
    Transform() {
        // Identity matrix
        m[0] = 1; m[1] = 0;
        m[2] = 0; m[3] = 1;
        m[4] = 0; m[5] = 0;
    }
    
    static Transform Identity() { return Transform(); }
    
    static Transform Translation(float x, float y) {
        Transform t;
        t.m[4] = x;
        t.m[5] = y;
        return t;
    }
    
    static Transform Scale(float sx, float sy, float cx = 0, float cy = 0) {
        Transform t;
        t.m[0] = sx;
        t.m[3] = sy;
        t.m[4] = cx - sx * cx;
        t.m[5] = cy - sy * cy;
        return t;
    }
    
    static Transform Rotation(float angleDegrees, float cx = 0, float cy = 0) {
        float rad = angleDegrees * 3.14159265f / 180.0f;
        float c = std::cos(rad);
        float s = std::sin(rad);
        
        Transform t;
        t.m[0] = c;  t.m[1] = s;
        t.m[2] = -s; t.m[3] = c;
        t.m[4] = cx - c * cx + s * cy;
        t.m[5] = cy - s * cx - c * cy;
        return t;
    }
    
    Transform operator*(const Transform& other) const {
        Transform result;
        result.m[0] = m[0] * other.m[0] + m[1] * other.m[2];
        result.m[1] = m[0] * other.m[1] + m[1] * other.m[3];
        result.m[2] = m[2] * other.m[0] + m[3] * other.m[2];
        result.m[3] = m[2] * other.m[1] + m[3] * other.m[3];
        result.m[4] = m[4] * other.m[0] + m[5] * other.m[2] + other.m[4];
        result.m[5] = m[4] * other.m[1] + m[5] * other.m[3] + other.m[5];
        return result;
    }
    
    Point TransformPoint(const Point& p) const {
        return Point(
            p.x * m[0] + p.y * m[2] + m[4],
            p.x * m[1] + p.y * m[3] + m[5]
        );
    }
    
    const float* GetMatrix() const { return m; }
    
private:
    float m[6]; // [sx, shy, shx, sy, tx, ty]
};

} // namespace rendering
} // namespace luaui
