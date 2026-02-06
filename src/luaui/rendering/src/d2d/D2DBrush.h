#pragma once

#include "luaui/rendering/IBrush.h"
#include <d2d1.h>
#include <wrl/client.h>

namespace luaui {
namespace rendering {

class D2DRenderContext;
using Microsoft::WRL::ComPtr;

// Direct2D solid color brush
class D2DSolidColorBrush : public ISolidColorBrush {
public:
    D2DSolidColorBrush();
    ~D2DSolidColorBrush() override;
    
    bool Initialize(D2DRenderContext* context, const Color& color);
    
    // IBrush
    BrushType GetType() const override { return BrushType::Solid; }
    void* GetNativeBrush(IRenderContext* context) override;
    
    // ISolidColorBrush
    void SetColor(const Color& color) override;
    Color GetColor() const override;
    
private:
    ComPtr<ID2D1SolidColorBrush> m_brush;
    Color m_color;
};

// Direct2D linear gradient brush
class D2DLinearGradientBrush : public ILinearGradientBrush {
public:
    D2DLinearGradientBrush();
    ~D2DLinearGradientBrush() override;
    
    bool Initialize(D2DRenderContext* context, const Point& start, const Point& end,
                    const std::vector<GradientStop>& stops);
    
    // IBrush
    BrushType GetType() const override { return BrushType::LinearGradient; }
    void* GetNativeBrush(IRenderContext* context) override;
    
    // ILinearGradientBrush
    void SetStartPoint(const Point& point) override;
    void SetEndPoint(const Point& point) override;
    void SetGradientStops(const std::vector<GradientStop>& stops) override;
    Point GetStartPoint() const override;
    Point GetEndPoint() const override;
    
private:
    bool CreateGradientStopCollection(D2DRenderContext* context, const std::vector<GradientStop>& stops);
    
    ComPtr<ID2D1LinearGradientBrush> m_brush;
    ComPtr<ID2D1GradientStopCollection> m_stopCollection;
    Point m_start;
    Point m_end;
};

// Direct2D radial gradient brush
class D2DRadialGradientBrush : public IRadialGradientBrush {
public:
    D2DRadialGradientBrush();
    ~D2DRadialGradientBrush() override;
    
    bool Initialize(D2DRenderContext* context, const Point& center, float rx, float ry,
                    const std::vector<GradientStop>& stops);
    
    // IBrush
    BrushType GetType() const override { return BrushType::RadialGradient; }
    void* GetNativeBrush(IRenderContext* context) override;
    
    // IRadialGradientBrush
    void SetCenter(const Point& point) override;
    void SetRadius(float radiusX, float radiusY) override;
    void SetGradientStops(const std::vector<GradientStop>& stops) override;
    Point GetCenter() const override;
    float GetRadiusX() const override;
    float GetRadiusY() const override;
    
private:
    bool CreateGradientStopCollection(D2DRenderContext* context, const std::vector<GradientStop>& stops);
    
    ComPtr<ID2D1RadialGradientBrush> m_brush;
    ComPtr<ID2D1GradientStopCollection> m_stopCollection;
    Point m_center;
    float m_radiusX;
    float m_radiusY;
};

} // namespace rendering
} // namespace luaui
