#pragma once

#include "Types.h"
#include <memory>
#include <vector>

namespace luaui {
namespace rendering {

class IRenderContext;

// Brush types
enum class BrushType {
    Solid,
    LinearGradient,
    RadialGradient,
    Bitmap
};

// Gradient stop
struct GradientStop {
    Color color;
    float position; // 0.0 to 1.0
    
    GradientStop() : color(), position(0) {}
    GradientStop(const Color& c, float pos) : color(c), position(pos) {}
};

// Base brush interface
class IBrush {
public:
    virtual ~IBrush() = default;
    
    virtual BrushType GetType() const = 0;
    virtual void* GetNativeBrush(IRenderContext* context) = 0;
};

using IBrushPtr = std::shared_ptr<IBrush>;

// Solid color brush
class ISolidColorBrush : public IBrush {
public:
    virtual void SetColor(const Color& color) = 0;
    virtual Color GetColor() const = 0;
};

using ISolidColorBrushPtr = std::shared_ptr<ISolidColorBrush>;

// Linear gradient brush
class ILinearGradientBrush : public IBrush {
public:
    virtual void SetStartPoint(const Point& point) = 0;
    virtual void SetEndPoint(const Point& point) = 0;
    virtual void SetGradientStops(const std::vector<GradientStop>& stops) = 0;
    virtual Point GetStartPoint() const = 0;
    virtual Point GetEndPoint() const = 0;
};

using ILinearGradientBrushPtr = std::shared_ptr<ILinearGradientBrush>;

// Radial gradient brush
class IRadialGradientBrush : public IBrush {
public:
    virtual void SetCenter(const Point& point) = 0;
    virtual void SetRadius(float radiusX, float radiusY) = 0;
    virtual void SetGradientStops(const std::vector<GradientStop>& stops) = 0;
    virtual Point GetCenter() const = 0;
    virtual float GetRadiusX() const = 0;
    virtual float GetRadiusY() const = 0;
};

using IRadialGradientBrushPtr = std::shared_ptr<IRadialGradientBrush>;

} // namespace rendering
} // namespace luaui
