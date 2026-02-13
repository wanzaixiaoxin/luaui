#include "D2DBrush.h"
#include "D2DRenderContext.h"

namespace luaui {
namespace rendering {

// D2DSolidColorBrush
D2DSolidColorBrush::D2DSolidColorBrush() = default;
D2DSolidColorBrush::~D2DSolidColorBrush() = default;

bool D2DSolidColorBrush::Initialize(D2DRenderContext* context, const Color& color) {
    ID2D1RenderTarget* rt = context->GetRenderTarget();
    if (!rt) return false;
    
    Color premul = color.Premultiply();
    D2D1_COLOR_F d2dColor = D2D1::ColorF(premul.r, premul.g, premul.b, premul.a);
    
    HRESULT hr = rt->CreateSolidColorBrush(d2dColor, &m_brush);
    if (SUCCEEDED(hr)) {
        m_color = color;
        return true;
    }
    return false;
}

void* D2DSolidColorBrush::GetNativeBrush(IRenderContext* context) {
    (void)context;
    return m_brush.Get();
}

void D2DSolidColorBrush::SetColor(const Color& color) {
    if (m_brush) {
        Color premul = color.Premultiply();
        m_brush->SetColor(D2D1::ColorF(premul.r, premul.g, premul.b, premul.a));
        m_color = color;
    }
}

Color D2DSolidColorBrush::GetColor() const {
    return m_color;
}

// D2DLinearGradientBrush
D2DLinearGradientBrush::D2DLinearGradientBrush() = default;
D2DLinearGradientBrush::~D2DLinearGradientBrush() = default;

bool D2DLinearGradientBrush::Initialize(D2DRenderContext* context, const Point& start, 
                                         const Point& end, const std::vector<GradientStop>& stops) {
    if (!CreateGradientStopCollection(context, stops)) {
        return false;
    }
    
    ID2D1RenderTarget* rt = context->GetRenderTarget();
    if (!rt) return false;
    
    D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES props;
    props.startPoint = D2D1::Point2F(start.x, start.y);
    props.endPoint = D2D1::Point2F(end.x, end.y);
    
    HRESULT hr = rt->CreateLinearGradientBrush(
        &props,
        nullptr,
        m_stopCollection.Get(),
        &m_brush
    );
    
    if (SUCCEEDED(hr)) {
        m_start = start;
        m_end = end;
        return true;
    }
    return false;
}

bool D2DLinearGradientBrush::CreateGradientStopCollection(D2DRenderContext* context, 
                                                           const std::vector<GradientStop>& stops) {
    ID2D1RenderTarget* rt = context->GetRenderTarget();
    if (!rt) return false;
    
    std::vector<D2D1_GRADIENT_STOP> d2dStops;
    d2dStops.reserve(stops.size());
    
    for (const auto& stop : stops) {
        D2D1_GRADIENT_STOP d2dStop;
        Color premul = stop.color.Premultiply();
        d2dStop.color = D2D1::ColorF(premul.r, premul.g, premul.b, premul.a);
        d2dStop.position = stop.position;
        d2dStops.push_back(d2dStop);
    }
    
    HRESULT hr = rt->CreateGradientStopCollection(
        d2dStops.data(),
        static_cast<UINT32>(d2dStops.size()),
        &m_stopCollection
    );
    
    return SUCCEEDED(hr);
}

void* D2DLinearGradientBrush::GetNativeBrush(IRenderContext* context) {
    (void)context;
    return m_brush.Get();
}

void D2DLinearGradientBrush::SetStartPoint(const Point& point) {
    if (m_brush) {
        m_brush->SetStartPoint(D2D1::Point2F(point.x, point.y));
        m_start = point;
    }
}

void D2DLinearGradientBrush::SetEndPoint(const Point& point) {
    if (m_brush) {
        m_brush->SetEndPoint(D2D1::Point2F(point.x, point.y));
        m_end = point;
    }
}

void D2DLinearGradientBrush::SetGradientStops(const std::vector<GradientStop>& stops) {
    (void)stops;
    // Note: In Direct2D, gradient stops cannot be changed after creation
    // We would need to recreate the brush
}

Point D2DLinearGradientBrush::GetStartPoint() const {
    return m_start;
}

Point D2DLinearGradientBrush::GetEndPoint() const {
    return m_end;
}

// D2DRadialGradientBrush
D2DRadialGradientBrush::D2DRadialGradientBrush() : m_radiusX(0), m_radiusY(0) {}
D2DRadialGradientBrush::~D2DRadialGradientBrush() = default;

bool D2DRadialGradientBrush::Initialize(D2DRenderContext* context, const Point& center,
                                         float rx, float ry, const std::vector<GradientStop>& stops) {
    if (!CreateGradientStopCollection(context, stops)) {
        return false;
    }
    
    ID2D1RenderTarget* rt = context->GetRenderTarget();
    if (!rt) return false;
    
    D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES props;
    props.center = D2D1::Point2F(center.x, center.y);
    props.gradientOriginOffset = D2D1::Point2F(0, 0);
    props.radiusX = rx;
    props.radiusY = ry;
    
    HRESULT hr = rt->CreateRadialGradientBrush(
        &props,
        nullptr,
        m_stopCollection.Get(),
        &m_brush
    );
    
    if (SUCCEEDED(hr)) {
        m_center = center;
        m_radiusX = rx;
        m_radiusY = ry;
        return true;
    }
    return false;
}

bool D2DRadialGradientBrush::CreateGradientStopCollection(D2DRenderContext* context,
                                                           const std::vector<GradientStop>& stops) {
    ID2D1RenderTarget* rt = context->GetRenderTarget();
    if (!rt) return false;
    
    std::vector<D2D1_GRADIENT_STOP> d2dStops;
    d2dStops.reserve(stops.size());
    
    for (const auto& stop : stops) {
        D2D1_GRADIENT_STOP d2dStop;
        Color premul = stop.color.Premultiply();
        d2dStop.color = D2D1::ColorF(premul.r, premul.g, premul.b, premul.a);
        d2dStop.position = stop.position;
        d2dStops.push_back(d2dStop);
    }
    
    HRESULT hr = rt->CreateGradientStopCollection(
        d2dStops.data(),
        static_cast<UINT32>(d2dStops.size()),
        &m_stopCollection
    );
    
    return SUCCEEDED(hr);
}

void* D2DRadialGradientBrush::GetNativeBrush(IRenderContext* context) {
    (void)context;
    return m_brush.Get();
}

void D2DRadialGradientBrush::SetCenter(const Point& point) {
    if (m_brush) {
        m_brush->SetCenter(D2D1::Point2F(point.x, point.y));
        m_center = point;
    }
}

void D2DRadialGradientBrush::SetRadius(float radiusX, float radiusY) {
    if (m_brush) {
        m_brush->SetRadiusX(radiusX);
        m_brush->SetRadiusY(radiusY);
        m_radiusX = radiusX;
        m_radiusY = radiusY;
    }
}

void D2DRadialGradientBrush::SetGradientStops(const std::vector<GradientStop>& stops) {
    (void)stops;
    // Note: In Direct2D, gradient stops cannot be changed after creation
}

Point D2DRadialGradientBrush::GetCenter() const {
    return m_center;
}

float D2DRadialGradientBrush::GetRadiusX() const {
    return m_radiusX;
}

float D2DRadialGradientBrush::GetRadiusY() const {
    return m_radiusY;
}

} // namespace rendering
} // namespace luaui
