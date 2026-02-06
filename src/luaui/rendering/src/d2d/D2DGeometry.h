#pragma once

#include "luaui/rendering/IGeometry.h"
#include <d2d1.h>
#include <wrl/client.h>

namespace luaui {
namespace rendering {

class D2DRenderContext;
using Microsoft::WRL::ComPtr;

// Rectangle geometry
class D2DRectangleGeometry : public IRectangleGeometry {
public:
    bool Initialize(D2DRenderContext* context, const Rect& rect);
    
    GeometryType GetType() const override { return GeometryType::Rectangle; }
    void* GetNativeGeometry(IRenderContext* context) const override { return m_geometry.Get(); }
    
    void SetRect(const Rect& rect) override;
    Rect GetRect() const override { return m_rect; }
    
    Rect GetBounds() const override;
    Rect GetBoundsWithStroke(const StrokeStyle& stroke) const override;
    bool FillContains(const Point& point) const override;
    bool StrokeContains(const Point& point, const StrokeStyle& stroke) const override;
    
private:
    Rect m_rect;
    ComPtr<ID2D1RectangleGeometry> m_geometry;
};

// Rounded rectangle geometry
class D2DRoundedRectangleGeometry : public IRoundedRectangleGeometry {
public:
    bool Initialize(D2DRenderContext* context, const Rect& rect, const CornerRadius& radius);
    
    GeometryType GetType() const override { return GeometryType::RoundedRectangle; }
    void* GetNativeGeometry(IRenderContext* context) const override { return m_geometry.Get(); }
    
    void SetRect(const Rect& rect) override;
    void SetCornerRadius(const CornerRadius& radius) override;
    Rect GetRect() const override { return m_rect; }
    CornerRadius GetCornerRadius() const override { return m_radius; }
    
    Rect GetBounds() const override;
    Rect GetBoundsWithStroke(const StrokeStyle& stroke) const override;
    bool FillContains(const Point& point) const override;
    bool StrokeContains(const Point& point, const StrokeStyle& stroke) const override;
    
private:
    Rect m_rect;
    CornerRadius m_radius;
    ComPtr<ID2D1RoundedRectangleGeometry> m_geometry;
};

// Ellipse geometry
class D2DEllipseGeometry : public IEllipseGeometry {
public:
    bool Initialize(D2DRenderContext* context, const Point& center, float rx, float ry);
    
    GeometryType GetType() const override { return GeometryType::Ellipse; }
    void* GetNativeGeometry(IRenderContext* context) const override { return m_geometry.Get(); }
    
    void SetCenter(const Point& center) override;
    void SetRadius(float rx, float ry) override;
    Point GetCenter() const override { return m_center; }
    float GetRadiusX() const override { return m_radiusX; }
    float GetRadiusY() const override { return m_radiusY; }
    
    Rect GetBounds() const override;
    Rect GetBoundsWithStroke(const StrokeStyle& stroke) const override;
    bool FillContains(const Point& point) const override;
    bool StrokeContains(const Point& point, const StrokeStyle& stroke) const override;
    
private:
    Point m_center;
    float m_radiusX;
    float m_radiusY;
    ComPtr<ID2D1EllipseGeometry> m_geometry;
};

// Path geometry
class D2DPathGeometry : public IPathGeometry {
public:
    D2DPathGeometry();
    ~D2DPathGeometry();
    
    bool Initialize(D2DRenderContext* context);
    void Close();
    void Clear() override;
    
    GeometryType GetType() const override { return GeometryType::Path; }
    void* GetNativeGeometry(IRenderContext* context) const override { return m_geometry.Get(); }
    
    void BeginFigure(const Point& startPoint, bool filled = true) override;
    void EndFigure(bool closed = true) override;
    
    void AddLine(const Point& point) override;
    void AddQuadraticBezier(const Point& control, const Point& end) override;
    void AddCubicBezier(const Point& control1, const Point& control2, const Point& end) override;
    void AddArc(const Point& end, const Size& size, float rotation, bool isLargeArc, bool sweepClockwise) override;
    
    void AddRectangle(const Rect& rect) override;
    void AddRoundedRectangle(const Rect& rect, const CornerRadius& radius) override;
    void AddEllipse(const Point& center, float rx, float ry) override;
    
    Rect GetBounds() const override;
    Rect GetBoundsWithStroke(const StrokeStyle& stroke) const override;
    bool FillContains(const Point& point) const override;
    bool StrokeContains(const Point& point, const StrokeStyle& stroke) const override;
    
private:
    ComPtr<ID2D1PathGeometry> m_geometry;
    ID2D1GeometrySink* m_sink = nullptr;
    bool m_figureOpen = false;
};

// Combined geometry
class D2DCombinedGeometry : public ICombinedGeometry {
public:
    bool Initialize(D2DRenderContext* context, IGeometry* g1, IGeometry* g2, CombineMode mode);
    
    GeometryType GetType() const override { return GeometryType::Combined; }
    void* GetNativeGeometry(IRenderContext* context) const override { return m_geometry.Get(); }
    
    void SetGeometries(IGeometry* g1, IGeometry* g2, CombineMode mode) override;
    
    Rect GetBounds() const override;
    Rect GetBoundsWithStroke(const StrokeStyle& stroke) const override;
    bool FillContains(const Point& point) const override;
    bool StrokeContains(const Point& point, const StrokeStyle& stroke) const override;
    
private:
    ComPtr<ID2D1PathGeometry> m_geometry;
};

} // namespace rendering
} // namespace luaui
