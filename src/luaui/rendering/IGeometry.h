#pragma once

#include "Types.h"
#include <memory>
#include <vector>

namespace luaui {
namespace rendering {

class IRenderContext;

// Geometry types
enum class GeometryType {
    Rectangle,
    RoundedRectangle,
    Ellipse,
    Path,
    Combined
};

// Path commands
enum class PathCommand {
    Move,
    Line,
    QuadraticBezier,
    CubicBezier,
    Arc,
    Close
};

// Stroke style
struct StrokeStyle {
    float width = 1.0f;
    float miterLimit = 10.0f;
    float dashOffset = 0.0f;
    
    enum class CapStyle { Flat, Square, Round };
    enum class LineJoin { Miter, Bevel, Round };
    
    CapStyle startCap = CapStyle::Flat;
    CapStyle endCap = CapStyle::Flat;
    LineJoin lineJoin = LineJoin::Miter;
    std::vector<float> dashes;
};

// Base geometry interface
class IGeometry {
public:
    virtual ~IGeometry() = default;
    
    virtual GeometryType GetType() const = 0;
    virtual void* GetNativeGeometry(IRenderContext* context) const = 0;
    
    // Bounds
    virtual Rect GetBounds() const = 0;
    virtual Rect GetBoundsWithStroke(const StrokeStyle& stroke) const = 0;
    
    // Hit testing
    virtual bool FillContains(const Point& point) const = 0;
    virtual bool StrokeContains(const Point& point, const StrokeStyle& stroke) const = 0;
};

using IGeometryPtr = std::shared_ptr<IGeometry>;

// Rectangle geometry
class IRectangleGeometry : public IGeometry {
public:
    virtual void SetRect(const Rect& rect) = 0;
    virtual Rect GetRect() const = 0;
};

// Rounded rectangle geometry
class IRoundedRectangleGeometry : public IGeometry {
public:
    virtual void SetRect(const Rect& rect) = 0;
    virtual void SetCornerRadius(const CornerRadius& radius) = 0;
    virtual Rect GetRect() const = 0;
    virtual CornerRadius GetCornerRadius() const = 0;
};

// Ellipse geometry
class IEllipseGeometry : public IGeometry {
public:
    virtual void SetCenter(const Point& center) = 0;
    virtual void SetRadius(float rx, float ry) = 0;
    virtual Point GetCenter() const = 0;
    virtual float GetRadiusX() const = 0;
    virtual float GetRadiusY() const = 0;
};

// Path geometry builder
class IPathGeometry : public IGeometry {
public:
    // Begin building
    virtual void BeginFigure(const Point& startPoint, bool filled = true) = 0;
    virtual void EndFigure(bool closed = true) = 0;
    
    // Drawing commands
    virtual void AddLine(const Point& point) = 0;
    virtual void AddQuadraticBezier(const Point& control, const Point& end) = 0;
    virtual void AddCubicBezier(const Point& control1, const Point& control2, const Point& end) = 0;
    virtual void AddArc(const Point& end, const Size& size, float rotation, bool isLargeArc, bool sweepClockwise) = 0;
    
    // Shorthand commands
    virtual void AddRectangle(const Rect& rect) = 0;
    virtual void AddRoundedRectangle(const Rect& rect, const CornerRadius& radius) = 0;
    virtual void AddEllipse(const Point& center, float rx, float ry) = 0;
    
    // Finish building
    virtual void Close() = 0;
    virtual void Clear() = 0;
};

// Combined geometry (union, intersect, etc.)
enum class CombineMode {
    Union,
    Intersect,
    Xor,
    Exclude
};

class ICombinedGeometry : public IGeometry {
public:
    virtual void SetGeometries(IGeometry* geom1, IGeometry* geom2, CombineMode mode) = 0;
};

} // namespace rendering
} // namespace luaui
