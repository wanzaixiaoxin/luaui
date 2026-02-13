#include "D2DRenderContext.h"
#include "D2DBrush.h"
#include "D2DGeometry.h"
#include "D2DBitmap.h"
#include "D2DTextFormat.h"

namespace luaui {
namespace rendering {

// Constructor / Destructor
D2DRenderContext::D2DRenderContext() = default;
D2DRenderContext::~D2DRenderContext() { Shutdown(); }

// Initialization
bool D2DRenderContext::Initialize(ID2D1Factory* factory, ID2D1RenderTarget* rt, IDWriteFactory* dw) {
    m_d2dFactory = factory;
    m_renderTarget = rt;
    m_dwriteFactory = dw;
    if (!m_d2dFactory || !m_renderTarget) return false;
    ResetState();
    return true;
}

void D2DRenderContext::Shutdown() {
    m_strokeStyles.clear();
    while (!m_layerStack.empty()) m_layerStack.pop();
    while (!m_stateStack.empty()) m_stateStack.pop();
    m_renderTarget.Reset();
    m_dwriteFactory.Reset();
    m_d2dFactory.Reset();
}

// Drawing Lifecycle
bool D2DRenderContext::BeginDraw() { 
    if (!m_renderTarget) return false; 
    m_renderTarget->BeginDraw(); 
    return true; 
}

bool D2DRenderContext::EndDraw() { 
    if (!m_renderTarget) return false; 
    return SUCCEEDED(m_renderTarget->EndDraw()); 
}

void D2DRenderContext::Clear(const Color& color) { 
    if (m_renderTarget) m_renderTarget->Clear(ToD2DColor(color)); 
}

void D2DRenderContext::Flush() { 
    if (m_renderTarget) m_renderTarget->Flush(); 
}

// State Management
void D2DRenderContext::PushState() { m_stateStack.push(m_currentState); }

void D2DRenderContext::PopState() {
    if (m_stateStack.empty()) return;
    m_currentState = m_stateStack.top();
    m_stateStack.pop();
    SetTransform(m_currentState.transform);
}

void D2DRenderContext::ResetState() {
    m_currentState = State();
    SetTransform(Transform::Identity());
    SetOpacity(1.0f);
    SetAntialias(true);
}

void D2DRenderContext::SetTransform(const Transform& t) {
    m_currentState.transform = t;
    if (m_renderTarget) m_renderTarget->SetTransform(ToD2DMatrix(t));
}

void D2DRenderContext::MultiplyTransform(const Transform& t) {
    m_currentState.transform = m_currentState.transform * t;
    SetTransform(m_currentState.transform);
}

Transform D2DRenderContext::GetTransform() const { return m_currentState.transform; }
void D2DRenderContext::SetOpacity(float o) { m_currentState.opacity = (o < 0) ? 0 : (o > 1 ? 1 : o); }
float D2DRenderContext::GetOpacity() const { return m_currentState.opacity; }

void D2DRenderContext::SetAntialias(bool enabled) {
    m_currentState.antialias = enabled;
    if (m_renderTarget) m_renderTarget->SetAntialiasMode(enabled ? D2D1_ANTIALIAS_MODE_PER_PRIMITIVE : D2D1_ANTIALIAS_MODE_ALIASED);
}

bool D2DRenderContext::GetAntialias() const { return m_currentState.antialias; }

// Clipping
void D2DRenderContext::PushClip(const Rect& rect) {
    if (!m_renderTarget) return;
    m_renderTarget->PushAxisAlignedClip(ToD2DRect(rect), D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}

void D2DRenderContext::PushClip(const IGeometry& geom) {
    if (!m_renderTarget) return;
    ID2D1Geometry* g = static_cast<ID2D1Geometry*>(geom.GetNativeGeometry(this));
    if (!g) return;
    ID2D1Layer* layer = nullptr;
    if (SUCCEEDED(m_renderTarget->CreateLayer(&layer))) {
        m_renderTarget->PushLayer(D2D1::LayerParameters(D2D1::InfiniteRect(), g), layer);
        m_layerStack.push({ComPtr<ID2D1Layer>(layer), 1.0f});
    }
}

void D2DRenderContext::PopClip() {
    if (!m_renderTarget) return;
    if (!m_layerStack.empty()) {
        m_renderTarget->PopLayer();
        m_layerStack.pop();
    } else {
        m_renderTarget->PopAxisAlignedClip();
    }
}

void D2DRenderContext::ResetClip() {
    if (!m_renderTarget) return;
    while (!m_layerStack.empty()) { m_renderTarget->PopLayer(); m_layerStack.pop(); }
}

Rect D2DRenderContext::GetClipBounds() const {
    if (!m_renderTarget) return Rect();
    D2D1_SIZE_U size = m_renderTarget->GetPixelSize();
    float dpiX, dpiY;
    m_renderTarget->GetDpi(&dpiX, &dpiY);
    return Rect(0, 0, size.width * 96.0f / dpiX, size.height * 96.0f / dpiY);
}

// Type Conversions
D2D1_COLOR_F D2DRenderContext::ToD2DColor(const Color& c) const {
    Color p = c.Premultiply();
    return D2D1::ColorF(p.r, p.g, p.b, p.a);
}

D2D1_POINT_2F D2DRenderContext::ToD2DPoint(const Point& p) const { return D2D1::Point2F(p.x, p.y); }
D2D1_RECT_F D2DRenderContext::ToD2DRect(const Rect& r) const { return D2D1::RectF(r.x, r.y, r.x + r.width, r.y + r.height); }
D2D1_SIZE_F D2DRenderContext::ToD2DSize(const Size& s) const { return D2D1::SizeF(s.width, s.height); }

D2D1_MATRIX_3X2_F D2DRenderContext::ToD2DMatrix(const Transform& t) const {
    const float* m = t.GetMatrix();
    return D2D1::Matrix3x2F(m[0], m[1], m[2], m[3], m[4], m[5]);
}

// Drawing Functions
void D2DRenderContext::DrawLine(const Point& p1, const Point& p2, IBrush* brush, float sw, const StrokeStyle* style) {
    if (!m_renderTarget || !brush) return;
    ID2D1Brush* b = static_cast<ID2D1Brush*>(brush->GetNativeBrush(this));
    if (!b) return;
    m_renderTarget->DrawLine(ToD2DPoint(p1), ToD2DPoint(p2), b, sw * m_currentState.opacity, GetStrokeStyle(style));
}

void D2DRenderContext::DrawRectangle(const Rect& r, IBrush* brush, float sw, const StrokeStyle* style) {
    if (!m_renderTarget || !brush) return;
    ID2D1Brush* b = static_cast<ID2D1Brush*>(brush->GetNativeBrush(this));
    if (!b) return;
    m_renderTarget->DrawRectangle(ToD2DRect(r), b, sw * m_currentState.opacity, GetStrokeStyle(style));
}

void D2DRenderContext::FillRectangle(const Rect& r, IBrush* brush) {
    if (!m_renderTarget || !brush) return;
    ID2D1Brush* b = static_cast<ID2D1Brush*>(brush->GetNativeBrush(this));
    if (b) m_renderTarget->FillRectangle(ToD2DRect(r), b);
}

void D2DRenderContext::DrawRoundedRectangle(const Rect& r, const CornerRadius& cr, IBrush* brush, float sw, const StrokeStyle* style) {
    if (!m_renderTarget || !brush) return;
    ID2D1Brush* b = static_cast<ID2D1Brush*>(brush->GetNativeBrush(this));
    if (!b) return;
    D2D1_ROUNDED_RECT rr = { ToD2DRect(r), cr.topLeft, cr.topLeft };
    m_renderTarget->DrawRoundedRectangle(&rr, b, sw * m_currentState.opacity, GetStrokeStyle(style));
}

void D2DRenderContext::FillRoundedRectangle(const Rect& r, const CornerRadius& cr, IBrush* brush) {
    if (!m_renderTarget || !brush) return;
    ID2D1Brush* b = static_cast<ID2D1Brush*>(brush->GetNativeBrush(this));
    if (!b) return;
    D2D1_ROUNDED_RECT rr = { ToD2DRect(r), cr.topLeft, cr.topLeft };
    m_renderTarget->FillRoundedRectangle(&rr, b);
}

void D2DRenderContext::DrawEllipse(const Point& c, float rx, float ry, IBrush* brush, float sw, const StrokeStyle* style) {
    if (!m_renderTarget || !brush) return;
    ID2D1Brush* b = static_cast<ID2D1Brush*>(brush->GetNativeBrush(this));
    if (!b) return;
    D2D1_ELLIPSE e = { ToD2DPoint(c), rx, ry };
    m_renderTarget->DrawEllipse(&e, b, sw * m_currentState.opacity, GetStrokeStyle(style));
}

void D2DRenderContext::FillEllipse(const Point& c, float rx, float ry, IBrush* brush) {
    if (!m_renderTarget || !brush) return;
    ID2D1Brush* b = static_cast<ID2D1Brush*>(brush->GetNativeBrush(this));
    if (b) { D2D1_ELLIPSE e = { ToD2DPoint(c), rx, ry }; m_renderTarget->FillEllipse(&e, b); }
}

void D2DRenderContext::DrawGeometry(const IGeometry& g, IBrush* brush, float sw, const StrokeStyle* style) {
    if (!m_renderTarget || !brush) return;
    ID2D1Geometry* geom = static_cast<ID2D1Geometry*>(g.GetNativeGeometry(this));
    ID2D1Brush* b = static_cast<ID2D1Brush*>(brush->GetNativeBrush(this));
    if (geom && b) m_renderTarget->DrawGeometry(geom, b, sw * m_currentState.opacity, GetStrokeStyle(style));
}

void D2DRenderContext::FillGeometry(const IGeometry& g, IBrush* brush) {
    if (!m_renderTarget || !brush) return;
    ID2D1Geometry* geom = static_cast<ID2D1Geometry*>(g.GetNativeGeometry(this));
    ID2D1Brush* b = static_cast<ID2D1Brush*>(brush->GetNativeBrush(this));
    if (geom && b) m_renderTarget->FillGeometry(geom, b);
}

void D2DRenderContext::DrawBitmap(IBitmap* bmp, const Point& pos, float op) {
    if (!m_renderTarget || !bmp) return;
    ID2D1Bitmap* b = static_cast<ID2D1Bitmap*>(bmp->GetNativeBitmap(this));
    if (!b) return;
    D2D1_SIZE_F size = b->GetSize();
    D2D1_RECT_F dest = { pos.x, pos.y, pos.x + size.width, pos.y + size.height };
    m_renderTarget->DrawBitmap(b, &dest, op * m_currentState.opacity);
}

void D2DRenderContext::DrawBitmap(IBitmap* bmp, const Rect& dest, float op) {
    if (!m_renderTarget || !bmp) return;
    ID2D1Bitmap* b = static_cast<ID2D1Bitmap*>(bmp->GetNativeBitmap(this));
    if (b) {
        D2D1_RECT_F d2dDest = ToD2DRect(dest);
        m_renderTarget->DrawBitmap(b, &d2dDest, op * m_currentState.opacity);
    }
}

void D2DRenderContext::DrawBitmap(IBitmap* bmp, const Rect& dest, const Rect& src, float op) {
    if (!m_renderTarget || !bmp) return;
    ID2D1Bitmap* b = static_cast<ID2D1Bitmap*>(bmp->GetNativeBitmap(this));
    if (b) {
        D2D1_RECT_F d2dDest = ToD2DRect(dest);
        D2D1_RECT_F d2dSrc = ToD2DRect(src);
        m_renderTarget->DrawBitmap(b, &d2dDest, op * m_currentState.opacity, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &d2dSrc);
    }
}

void D2DRenderContext::DrawTextString(const std::wstring& text, ITextFormat* format, const Point& pos, IBrush* brush) {
    if (!m_renderTarget || !format || !brush) return;
    IDWriteTextFormat* f = static_cast<IDWriteTextFormat*>(format->GetNativeFormat(this));
    ID2D1Brush* b = static_cast<ID2D1Brush*>(brush->GetNativeBrush(this));
    if (f && b) {
        D2D1_RECT_F rect = D2D1::RectF(pos.x, pos.y, pos.x + 10000, pos.y + 10000);
        m_renderTarget->DrawText(text.c_str(), (UINT32)text.length(), f, &rect, b);
    }
}

void D2DRenderContext::DrawTextString(const std::wstring& text, ITextFormat* format, const Rect& rect, IBrush* brush) {
    if (!m_renderTarget || !format || !brush) return;
    IDWriteTextFormat* f = static_cast<IDWriteTextFormat*>(format->GetNativeFormat(this));
    ID2D1Brush* b = static_cast<ID2D1Brush*>(brush->GetNativeBrush(this));
    if (f && b) {
        D2D1_RECT_F d2dRect = ToD2DRect(rect);
        m_renderTarget->DrawText(text.c_str(), (UINT32)text.length(), f, &d2dRect, b);
    }
}

void D2DRenderContext::PushLayer(float op) {
    if (!m_renderTarget) return;
    ID2D1Layer* layer = nullptr;
    if (SUCCEEDED(m_renderTarget->CreateLayer(&layer))) {
        m_renderTarget->PushLayer(D2D1::LayerParameters(D2D1::InfiniteRect(), nullptr, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE, D2D1::IdentityMatrix(), op), layer);
        m_layerStack.push({ComPtr<ID2D1Layer>(layer), op});
    }
}

void D2DRenderContext::PopLayer() {
    if (!m_renderTarget || m_layerStack.empty()) return;
    m_renderTarget->PopLayer();
    m_layerStack.pop();
}

ID2D1StrokeStyle* D2DRenderContext::GetStrokeStyle(const StrokeStyle* s) {
    if (!s) return nullptr;
    D2D1_STROKE_STYLE_PROPERTIES p = {};
    p.startCap = (D2D1_CAP_STYLE)s->startCap;
    p.endCap = (D2D1_CAP_STYLE)s->endCap;
    p.lineJoin = (D2D1_LINE_JOIN)s->lineJoin;
    p.miterLimit = s->miterLimit;
    p.dashStyle = s->dashes.empty() ? D2D1_DASH_STYLE_SOLID : D2D1_DASH_STYLE_CUSTOM;
    p.dashOffset = s->dashOffset;
    ID2D1StrokeStyle* style = nullptr;
    if (SUCCEEDED(m_d2dFactory->CreateStrokeStyle(&p, s->dashes.empty() ? nullptr : s->dashes.data(), (UINT32)s->dashes.size(), &style)) && style) {
        m_strokeStyles.emplace_back(style);
    }
    return style;
}

// Factory Methods
ISolidColorBrushPtr D2DRenderContext::CreateSolidColorBrush(const Color& c) {
    auto b = std::make_shared<D2DSolidColorBrush>();
    return b->Initialize(this, c) ? b : nullptr;
}

ILinearGradientBrushPtr D2DRenderContext::CreateLinearGradientBrush(const Point& s, const Point& e, const std::vector<GradientStop>& stops) {
    auto b = std::make_shared<D2DLinearGradientBrush>();
    return b->Initialize(this, s, e, stops) ? b : nullptr;
}

IRadialGradientBrushPtr D2DRenderContext::CreateRadialGradientBrush(const Point& c, float rx, float ry, const std::vector<GradientStop>& stops) {
    auto b = std::make_shared<D2DRadialGradientBrush>();
    return b->Initialize(this, c, rx, ry, stops) ? b : nullptr;
}

std::shared_ptr<IRectangleGeometry> D2DRenderContext::CreateRectangleGeometry(const Rect& r) {
    auto g = std::make_shared<D2DRectangleGeometry>();
    return g->Initialize(this, r) ? g : nullptr;
}

std::shared_ptr<IRoundedRectangleGeometry> D2DRenderContext::CreateRoundedRectangleGeometry(const Rect& r, const CornerRadius& cr) {
    auto g = std::make_shared<D2DRoundedRectangleGeometry>();
    return g->Initialize(this, r, cr) ? g : nullptr;
}

std::shared_ptr<IEllipseGeometry> D2DRenderContext::CreateEllipseGeometry(const Point& c, float rx, float ry) {
    auto g = std::make_shared<D2DEllipseGeometry>();
    return g->Initialize(this, c, rx, ry) ? g : nullptr;
}

std::shared_ptr<IPathGeometry> D2DRenderContext::CreatePathGeometry() {
    auto g = std::make_shared<D2DPathGeometry>();
    return g->Initialize(this) ? g : nullptr;
}

std::shared_ptr<ICombinedGeometry> D2DRenderContext::CreateCombinedGeometry(IGeometry* g1, IGeometry* g2, CombineMode m) {
    auto g = std::make_shared<D2DCombinedGeometry>();
    return g->Initialize(this, g1, g2, m) ? g : nullptr;
}

ITextFormatPtr D2DRenderContext::CreateTextFormat(const std::wstring& family, float size) {
    auto f = std::make_shared<D2DTextFormat>();
    return f->Initialize(this, family, size) ? f : nullptr;
}

ITextLayoutPtr D2DRenderContext::CreateTextLayout(const std::wstring& text, ITextFormat* format, const Size& size) {
    auto l = std::make_shared<D2DTextLayout>();
    return l->Initialize(this, text, format, size) ? l : nullptr;
}

IBitmapPtr D2DRenderContext::CreateBitmap(int w, int h, PixelFormat f) {
    auto b = std::make_shared<D2DBitmap>();
    return b->Initialize(this, w, h, f) ? b : nullptr;
}

IBitmapPtr D2DRenderContext::LoadBitmapFromFile(const std::wstring& path) {
    auto b = std::make_shared<D2DBitmap>();
    return b->LoadFromFile(this, path) ? b : nullptr;
}

IBitmapPtr D2DRenderContext::LoadBitmapFromMemory(const void* data, size_t size) {
    auto b = std::make_shared<D2DBitmap>();
    return b->LoadFromMemory(this, data, size) ? b : nullptr;
}

} // namespace rendering
} // namespace luaui
