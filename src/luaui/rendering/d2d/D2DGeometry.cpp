#include "D2DGeometry.h"
#include "D2DRenderContext.h"

namespace luaui {
namespace rendering {

// Helper to convert stroke style
static D2D1_MATRIX_3X2_F IdentityMatrix() {
    return D2D1::Matrix3x2F::Identity();
}

// D2DRectangleGeometry
bool D2DRectangleGeometry::Initialize(D2DRenderContext* context, const Rect& rect) {
    ID2D1Factory* factory = context->GetD2DFactory();
    if (!factory) return false;
    
    ID2D1RectangleGeometry* geom = nullptr;
    HRESULT hr = factory->CreateRectangleGeometry(
        D2D1::RectF(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height),
        &geom
    );
    
    if (SUCCEEDED(hr) && geom) {
        m_geometry = geom;
        m_rect = rect;
        geom->Release();
        return true;
    }
    return false;
}

void D2DRectangleGeometry::SetRect(const Rect& rect) {
    m_rect = rect;
    // Note: D2D geometries are immutable, would need to recreate
}

Rect D2DRectangleGeometry::GetBounds() const {
    return m_rect;
}

Rect D2DRectangleGeometry::GetBoundsWithStroke(const StrokeStyle& stroke) const {
    float halfWidth = stroke.width / 2;
    return Rect(m_rect.x - halfWidth, m_rect.y - halfWidth,
                m_rect.width + stroke.width, m_rect.height + stroke.width);
}

bool D2DRectangleGeometry::FillContains(const Point& point) const {
    return m_rect.Contains(point);
}

bool D2DRectangleGeometry::StrokeContains(const Point& point, const StrokeStyle& stroke) const {
    float halfWidth = stroke.width / 2;
    Rect outer(m_rect.x - halfWidth, m_rect.y - halfWidth,
               m_rect.width + stroke.width, m_rect.height + stroke.width);
    Rect inner(m_rect.x + halfWidth, m_rect.y + halfWidth,
               m_rect.width - stroke.width, m_rect.height - stroke.width);
    return outer.Contains(point) && !inner.Contains(point);
}

// D2DRoundedRectangleGeometry
bool D2DRoundedRectangleGeometry::Initialize(D2DRenderContext* context, const Rect& rect, 
                                              const CornerRadius& radius) {
    ID2D1Factory* factory = context->GetD2DFactory();
    if (!factory) return false;
    
    D2D1_ROUNDED_RECT rr;
    rr.rect = D2D1::RectF(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);
    rr.radiusX = radius.topLeft;
    rr.radiusY = radius.topLeft;
    
    ID2D1RoundedRectangleGeometry* geom = nullptr;
    HRESULT hr = factory->CreateRoundedRectangleGeometry(&rr, &geom);
    
    if (SUCCEEDED(hr) && geom) {
        m_geometry = geom;
        m_rect = rect;
        m_radius = radius;
        geom->Release();
        return true;
    }
    return false;
}

void D2DRoundedRectangleGeometry::SetRect(const Rect& rect) {
    m_rect = rect;
}

void D2DRoundedRectangleGeometry::SetCornerRadius(const CornerRadius& radius) {
    m_radius = radius;
}

Rect D2DRoundedRectangleGeometry::GetBounds() const {
    return m_rect;
}

Rect D2DRoundedRectangleGeometry::GetBoundsWithStroke(const StrokeStyle& stroke) const {
    float halfWidth = stroke.width / 2;
    return Rect(m_rect.x - halfWidth, m_rect.y - halfWidth,
                m_rect.width + stroke.width, m_rect.height + stroke.width);
}

bool D2DRoundedRectangleGeometry::FillContains(const Point& point) const {
    return m_rect.Contains(point);
}

bool D2DRoundedRectangleGeometry::StrokeContains(const Point& point, const StrokeStyle& stroke) const {
    (void)stroke;
    return m_rect.Contains(point); // Simplified
}

// D2DEllipseGeometry
bool D2DEllipseGeometry::Initialize(D2DRenderContext* context, const Point& center, 
                                     float rx, float ry) {
    ID2D1Factory* factory = context->GetD2DFactory();
    if (!factory) return false;
    
    D2D1_ELLIPSE ellipse;
    ellipse.point = D2D1::Point2F(center.x, center.y);
    ellipse.radiusX = rx;
    ellipse.radiusY = ry;
    
    ID2D1EllipseGeometry* geom = nullptr;
    HRESULT hr = factory->CreateEllipseGeometry(&ellipse, &geom);
    
    if (SUCCEEDED(hr) && geom) {
        m_geometry = geom;
        m_center = center;
        m_radiusX = rx;
        m_radiusY = ry;
        geom->Release();
        return true;
    }
    return false;
}

void D2DEllipseGeometry::SetCenter(const Point& center) {
    m_center = center;
}

void D2DEllipseGeometry::SetRadius(float rx, float ry) {
    m_radiusX = rx;
    m_radiusY = ry;
}

Rect D2DEllipseGeometry::GetBounds() const {
    return Rect(m_center.x - m_radiusX, m_center.y - m_radiusY,
                m_radiusX * 2, m_radiusY * 2);
}

Rect D2DEllipseGeometry::GetBoundsWithStroke(const StrokeStyle& stroke) const {
    float halfWidth = stroke.width / 2;
    return Rect(m_center.x - m_radiusX - halfWidth, m_center.y - m_radiusY - halfWidth,
                (m_radiusX + halfWidth) * 2, (m_radiusY + halfWidth) * 2);
}

bool D2DEllipseGeometry::FillContains(const Point& point) const {
    float dx = point.x - m_center.x;
    float dy = point.y - m_center.y;
    return (dx * dx) / (m_radiusX * m_radiusX) + (dy * dy) / (m_radiusY * m_radiusY) <= 1.0f;
}

bool D2DEllipseGeometry::StrokeContains(const Point& point, const StrokeStyle& stroke) const {
    float dx = point.x - m_center.x;
    float dy = point.y - m_center.y;
    float normalizedDist = (dx * dx) / (m_radiusX * m_radiusX) + (dy * dy) / (m_radiusY * m_radiusY);
    float tolerance = stroke.width / (m_radiusX < m_radiusY ? m_radiusX : m_radiusY);
    return normalizedDist <= 1.0f + tolerance && normalizedDist >= 1.0f - tolerance;
}

// D2DPathGeometry - Simplified implementation
D2DPathGeometry::D2DPathGeometry() = default;
D2DPathGeometry::~D2DPathGeometry() {
    if (m_sink) {
        m_sink->Release();
    }
}

bool D2DPathGeometry::Initialize(D2DRenderContext* context) {
    ID2D1Factory* factory = context->GetD2DFactory();
    if (!factory) return false;
    
    ID2D1PathGeometry* path = nullptr;
    HRESULT hr = factory->CreatePathGeometry(&path);
    
    if (SUCCEEDED(hr) && path) {
        m_geometry = path;
        m_geometry = path;
        path->Release();
        return true;
    }
    return false;
}

void D2DPathGeometry::Close() {
    if (m_sink) {
        m_sink->Close();
        m_sink->Release();
        m_sink = nullptr;
    }
}

void D2DPathGeometry::Clear() {
    if (m_sink) {
        m_sink->Release();
        m_sink = nullptr;
    }
    m_geometry.Reset();
    m_geometry.Reset();
}

void D2DPathGeometry::BeginFigure(const Point& startPoint, bool filled) {
    if (!m_geometry) return;
    
    if (!m_sink) {
        m_geometry->Open(&m_sink);
    }
    
    if (m_sink) {
        m_sink->BeginFigure(D2D1::Point2F(startPoint.x, startPoint.y),
                            filled ? D2D1_FIGURE_BEGIN_FILLED : D2D1_FIGURE_BEGIN_HOLLOW);
        m_figureOpen = true;
    }
}

void D2DPathGeometry::EndFigure(bool closed) {
    if (m_sink && m_figureOpen) {
        m_sink->EndFigure(closed ? D2D1_FIGURE_END_CLOSED : D2D1_FIGURE_END_OPEN);
        m_figureOpen = false;
    }
}

void D2DPathGeometry::AddLine(const Point& point) {
    if (m_sink) {
        m_sink->AddLine(D2D1::Point2F(point.x, point.y));
    }
}

void D2DPathGeometry::AddQuadraticBezier(const Point& control, const Point& end) {
    if (m_sink) {
        m_sink->AddQuadraticBezier(D2D1::QuadraticBezierSegment(
            D2D1::Point2F(control.x, control.y),
            D2D1::Point2F(end.x, end.y)));
    }
}

void D2DPathGeometry::AddCubicBezier(const Point& control1, const Point& control2, const Point& end) {
    if (m_sink) {
        m_sink->AddBezier(D2D1::BezierSegment(
            D2D1::Point2F(control1.x, control1.y),
            D2D1::Point2F(control2.x, control2.y),
            D2D1::Point2F(end.x, end.y)));
    }
}

void D2DPathGeometry::AddArc(const Point& end, const Size& size, float rotation, 
                              bool isLargeArc, bool sweepClockwise) {
    if (m_sink) {
        m_sink->AddArc(D2D1::ArcSegment(
            D2D1::Point2F(end.x, end.y),
            D2D1::SizeF(size.width, size.height),
            rotation,
            sweepClockwise ? D2D1_SWEEP_DIRECTION_CLOCKWISE : D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
            isLargeArc ? D2D1_ARC_SIZE_LARGE : D2D1_ARC_SIZE_SMALL));
    }
}

void D2DPathGeometry::AddRectangle(const Rect& rect) {
    // Simplified: add as four lines
    BeginFigure(Point(rect.x, rect.y), true);
    AddLine(Point(rect.x + rect.width, rect.y));
    AddLine(Point(rect.x + rect.width, rect.y + rect.height));
    AddLine(Point(rect.x, rect.y + rect.height));
    EndFigure(true);
}

void D2DPathGeometry::AddRoundedRectangle(const Rect& rect, const CornerRadius& radius) {
    (void)radius;
    // Simplified implementation
    AddRectangle(rect);
}

void D2DPathGeometry::AddEllipse(const Point& center, float rx, float ry) {
    // Approximate ellipse with bezier curves
    float kappa = 0.5522848f;
    float ox = rx * kappa;
    float oy = ry * kappa;
    
    BeginFigure(Point(center.x, center.y - ry), true);
    AddCubicBezier(Point(center.x + ox, center.y - ry),
                   Point(center.x + rx, center.y - oy),
                   Point(center.x + rx, center.y));
    AddCubicBezier(Point(center.x + rx, center.y + oy),
                   Point(center.x + ox, center.y + ry),
                   Point(center.x, center.y + ry));
    AddCubicBezier(Point(center.x - ox, center.y + ry),
                   Point(center.x - rx, center.y + oy),
                   Point(center.x - rx, center.y));
    AddCubicBezier(Point(center.x - rx, center.y - oy),
                   Point(center.x - ox, center.y - ry),
                   Point(center.x, center.y - ry));
    EndFigure(true);
}

Rect D2DPathGeometry::GetBounds() const {
    if (!m_geometry) return Rect();
    
    D2D1_RECT_F bounds;
    if (SUCCEEDED(m_geometry->GetBounds(IdentityMatrix(), &bounds))) {
        return Rect(bounds.left, bounds.top, 
                    bounds.right - bounds.left, bounds.bottom - bounds.top);
    }
    return Rect();
}

Rect D2DPathGeometry::GetBoundsWithStroke(const StrokeStyle& stroke) const {
    (void)stroke;
    if (!m_geometry) return Rect();
    
    // Simplified
    return GetBounds();
}

bool D2DPathGeometry::FillContains(const Point& point) const {
    if (!m_geometry) return false;
    
    BOOL contains = FALSE;
    m_geometry->FillContainsPoint(D2D1::Point2F(point.x, point.y), 
                                   IdentityMatrix(), &contains);
    return contains != FALSE;
}

bool D2DPathGeometry::StrokeContains(const Point& point, const StrokeStyle& stroke) const {
    if (!m_geometry) return false;
    
    BOOL contains = FALSE;
    m_geometry->StrokeContainsPoint(D2D1::Point2F(point.x, point.y), 
                                     stroke.width, nullptr, IdentityMatrix(), &contains);
    return contains != FALSE;
}

// D2DCombinedGeometry
bool D2DCombinedGeometry::Initialize(D2DRenderContext* context, IGeometry* g1, IGeometry* g2, 
                                      CombineMode mode) {
    ID2D1Factory* factory = context->GetD2DFactory();
    if (!factory || !g1 || !g2) return false;
    
    ID2D1Geometry* d2dG1 = static_cast<ID2D1Geometry*>(g1->GetNativeGeometry(context));
    ID2D1Geometry* d2dG2 = static_cast<ID2D1Geometry*>(g2->GetNativeGeometry(context));
    if (!d2dG1 || !d2dG2) return false;
    
    D2D1_COMBINE_MODE d2dMode = D2D1_COMBINE_MODE_UNION;
    switch (mode) {
        case CombineMode::Union: d2dMode = D2D1_COMBINE_MODE_UNION; break;
        case CombineMode::Intersect: d2dMode = D2D1_COMBINE_MODE_INTERSECT; break;
        case CombineMode::Xor: d2dMode = D2D1_COMBINE_MODE_XOR; break;
        case CombineMode::Exclude: d2dMode = D2D1_COMBINE_MODE_EXCLUDE; break;
    }
    
    ID2D1PathGeometry* pathGeom = nullptr;
    HRESULT hr = factory->CreatePathGeometry(&pathGeom);
    if (FAILED(hr) || !pathGeom) return false;
    
    ID2D1GeometrySink* sink = nullptr;
    hr = pathGeom->Open(&sink);
    if (FAILED(hr) || !sink) {
        pathGeom->Release();
        return false;
    }
    
    hr = d2dG1->CombineWithGeometry(d2dG2, d2dMode, IdentityMatrix(), sink);
    sink->Close();
    sink->Release();
    
    if (SUCCEEDED(hr)) {
        m_geometry = pathGeom;
        pathGeom->Release();
        return true;
    }
    
    pathGeom->Release();
    return false;
}

void D2DCombinedGeometry::SetGeometries(IGeometry* g1, IGeometry* g2, CombineMode mode) {
    (void)g1;
    (void)g2;
    (void)mode;
    // Geometries are immutable in D2D, would need to recreate
}

Rect D2DCombinedGeometry::GetBounds() const {
    if (!m_geometry) return Rect();
    
    D2D1_RECT_F bounds;
    if (SUCCEEDED(m_geometry->GetBounds(IdentityMatrix(), &bounds))) {
        return Rect(bounds.left, bounds.top,
                    bounds.right - bounds.left, bounds.bottom - bounds.top);
    }
    return Rect();
}

Rect D2DCombinedGeometry::GetBoundsWithStroke(const StrokeStyle& stroke) const {
    (void)stroke;
    return GetBounds();
}

bool D2DCombinedGeometry::FillContains(const Point& point) const {
    if (!m_geometry) return false;
    
    BOOL contains = FALSE;
    m_geometry->FillContainsPoint(D2D1::Point2F(point.x, point.y),
                                   IdentityMatrix(), &contains);
    return contains != FALSE;
}

bool D2DCombinedGeometry::StrokeContains(const Point& point, const StrokeStyle& stroke) const {
    if (!m_geometry) return false;
    
    BOOL contains = FALSE;
    m_geometry->StrokeContainsPoint(D2D1::Point2F(point.x, point.y),
                                     stroke.width, nullptr, IdentityMatrix(), &contains);
    return contains != FALSE;
}

} // namespace rendering
} // namespace luaui
