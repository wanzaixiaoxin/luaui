#pragma once
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace luaui {

// NaN constant for Auto sizing
constexpr float NaN = std::numeric_limits<float>::quiet_NaN();
inline bool IsNaN(float v) { return std::isnan(v); }
inline bool IsAuto(float v) { return std::isnan(v); }

// Basic types
struct Point {
    float X = 0;
    float Y = 0;
    
    Point() = default;
    Point(float x, float y) : X(x), Y(y) {}
};

struct Size {
    float Width = 0;
    float Height = 0;
    
    Size() = default;
    Size(float w, float h) : Width(w), Height(h) {}
    
    bool IsEmpty() const { return Width <= 0 || Height <= 0; }
    
    static Size Infinite() { return Size(Infinity, Infinity); }
    static constexpr float Infinity = 1e30f;
};

struct Rect {
    float X = 0;
    float Y = 0;
    float Width = 0;
    float Height = 0;
    
    Rect() = default;
    Rect(float x, float y, float w, float h) : X(x), Y(y), Width(w), Height(h) {}
    Rect(const Point& pos, const Size& size) 
        : X(pos.X), Y(pos.Y), Width(size.Width), Height(size.Height) {}
    
    float Left() const { return X; }
    float Top() const { return Y; }
    float Right() const { return X + Width; }
    float Bottom() const { return Y + Height; }
    
    Point Position() const { return Point(X, Y); }
    Size GetSize() const { return Size(Width, Height); }
};

struct Thickness {
    float Left = 0;
    float Top = 0;
    float Right = 0;
    float Bottom = 0;
    
    Thickness() = default;
    explicit Thickness(float uniform) 
        : Left(uniform), Top(uniform), Right(uniform), Bottom(uniform) {}
    Thickness(float left, float top, float right, float bottom)
        : Left(left), Top(top), Right(right), Bottom(bottom) {}
    
    float Horizontal() const { return Left + Right; }
    float Vertical() const { return Top + Bottom; }
};

// Alignment enums
enum class HorizontalAlignment { Left, Center, Right, Stretch };
enum class VerticalAlignment { Top, Center, Bottom, Stretch };
enum class Orientation { Horizontal, Vertical };

// Color struct (simplified)
struct Color {
    float R = 0, G = 0, B = 0, A = 1;
    
    Color() = default;
    Color(float r, float g, float b, float a = 1) : R(r), G(g), B(b), A(a) {}
    
    // Create from RGB int (0xRRGGBB)
    static Color FromRGB(int rgb) {
        return Color(
            ((rgb >> 16) & 0xFF) / 255.0f,
            ((rgb >> 8) & 0xFF) / 255.0f,
            (rgb & 0xFF) / 255.0f,
            1.0f
        );
    }
    
    // Create from ARGB int (0xAARRGGBB)
    static Color FromARGB(int argb) {
        return Color(
            ((argb >> 16) & 0xFF) / 255.0f,
            ((argb >> 8) & 0xFF) / 255.0f,
            (argb & 0xFF) / 255.0f,
            ((argb >> 24) & 0xFF) / 255.0f
        );
    }
    
    static Color Transparent() { return Color(0, 0, 0, 0); }
    static Color White() { return Color(1, 1, 1); }
    static Color Black() { return Color(0, 0, 0); }
    static Color Red() { return Color(1, 0, 0); }
    static Color Green() { return Color(0, 1, 0); }
    static Color Blue() { return Color(0, 0, 1); }
    static Color Yellow() { return Color(1, 1, 0); }
    static Color Cyan() { return Color(0, 1, 1); }
    static Color Magenta() { return Color(1, 0, 1); }
    static Color Gray(float v) { return Color(v, v, v); }
};

// Layout helper functions
inline Size ConstrainSize(const Size& desired, const Size& available) {
    return Size{
        std::min(desired.Width, available.Width),
        std::min(desired.Height, available.Height)
    };
}

inline Rect AlignRect(const Size& size, const Rect& slot, 
                      HorizontalAlignment hAlign, VerticalAlignment vAlign) {
    float x = slot.X;
    float y = slot.Y;
    float w = size.Width;
    float h = size.Height;
    
    // Horizontal alignment
    if (hAlign == HorizontalAlignment::Stretch && !IsNaN(slot.Width)) {
        w = slot.Width;
    } else if (hAlign == HorizontalAlignment::Center) {
        x += (slot.Width - w) / 2;
    } else if (hAlign == HorizontalAlignment::Right) {
        x += slot.Width - w;
    }
    
    // Vertical alignment
    if (vAlign == VerticalAlignment::Stretch && !IsNaN(slot.Height)) {
        h = slot.Height;
    } else if (vAlign == VerticalAlignment::Center) {
        y += (slot.Height - h) / 2;
    } else if (vAlign == VerticalAlignment::Bottom) {
        y += slot.Height - h;
    }
    
    return Rect(x, y, w, h);
}

} // namespace luaui
