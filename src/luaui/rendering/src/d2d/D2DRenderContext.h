#pragma once

#include "luaui/rendering/IRenderContext.h"
#include <d2d1.h>
#include <dwrite.h>
#include <wrl/client.h>
#include <stack>

namespace luaui {
namespace rendering {

using Microsoft::WRL::ComPtr;

// Direct2D implementation of render context
class D2DRenderContext : public IRenderContext {
public:
    D2DRenderContext();
    ~D2DRenderContext() override;
    
    // Initialize with Direct2D resources
    bool Initialize(ID2D1Factory* factory, ID2D1RenderTarget* renderTarget, IDWriteFactory* dwriteFactory);
    void Shutdown();
    
    // IRenderContext implementation
    bool BeginDraw() override;
    bool EndDraw() override;
    void Clear(const Color& color) override;
    void Flush() override;
    
    void PushState() override;
    void PopState() override;
    void ResetState() override;
    
    void SetTransform(const Transform& transform) override;
    void MultiplyTransform(const Transform& transform) override;
    Transform GetTransform() const override;
    
    void SetOpacity(float opacity) override;
    float GetOpacity() const override;
    
    void SetAntialias(bool enabled) override;
    bool GetAntialias() const override;
    
    void PushClip(const Rect& rect) override;
    void PushClip(const IGeometry& geometry) override;
    void PopClip() override;
    void ResetClip() override;
    Rect GetClipBounds() const override;
    
    void DrawLine(const Point& p1, const Point& p2, IBrush* brush, float strokeWidth = 1.0f,
                  const StrokeStyle* strokeStyle = nullptr) override;
    
    void DrawRectangle(const Rect& rect, IBrush* brush, float strokeWidth = 1.0f,
                       const StrokeStyle* strokeStyle = nullptr) override;
    void FillRectangle(const Rect& rect, IBrush* brush) override;
    
    void DrawRoundedRectangle(const Rect& rect, const CornerRadius& radius, IBrush* brush,
                              float strokeWidth = 1.0f, const StrokeStyle* strokeStyle = nullptr) override;
    void FillRoundedRectangle(const Rect& rect, const CornerRadius& radius, IBrush* brush) override;
    
    void DrawEllipse(const Point& center, float radiusX, float radiusY, IBrush* brush,
                     float strokeWidth = 1.0f, const StrokeStyle* strokeStyle = nullptr) override;
    void FillEllipse(const Point& center, float radiusX, float radiusY, IBrush* brush) override;
    
    void DrawGeometry(const IGeometry& geometry, IBrush* brush, float strokeWidth = 1.0f,
                      const StrokeStyle* strokeStyle = nullptr) override;
    void FillGeometry(const IGeometry& geometry, IBrush* brush) override;
    
    void DrawBitmap(IBitmap* bitmap, const Point& destination, float opacity = 1.0f) override;
    void DrawBitmap(IBitmap* bitmap, const Rect& destination, float opacity = 1.0f) override;
    void DrawBitmap(IBitmap* bitmap, const Rect& destination, const Rect& source, float opacity = 1.0f) override;
    
    void DrawTextString(const std::wstring& text, ITextFormat* format, const Point& position, IBrush* brush) override;
    void DrawTextString(const std::wstring& text, ITextFormat* format, const Rect& rect, IBrush* brush) override;
    
    void PushLayer(float opacity = 1.0f) override;
    void PopLayer() override;
    
    // Factory methods
    ISolidColorBrushPtr CreateSolidColorBrush(const Color& color) override;
    ILinearGradientBrushPtr CreateLinearGradientBrush(const Point& start, const Point& end,
                                                       const std::vector<GradientStop>& stops) override;
    IRadialGradientBrushPtr CreateRadialGradientBrush(const Point& center, float rx, float ry,
                                                       const std::vector<GradientStop>& stops) override;
    
    std::shared_ptr<IRectangleGeometry> CreateRectangleGeometry(const Rect& rect) override;
    std::shared_ptr<IRoundedRectangleGeometry> CreateRoundedRectangleGeometry(const Rect& rect,
                                                                               const CornerRadius& radius) override;
    std::shared_ptr<IEllipseGeometry> CreateEllipseGeometry(const Point& center, float rx, float ry) override;
    std::shared_ptr<IPathGeometry> CreatePathGeometry() override;
    std::shared_ptr<ICombinedGeometry> CreateCombinedGeometry(IGeometry* g1, IGeometry* g2, 
                                                               CombineMode mode) override;
    
    ITextFormatPtr CreateTextFormat(const std::wstring& fontFamily, float fontSize) override;
    ITextLayoutPtr CreateTextLayout(const std::wstring& text, ITextFormat* format, const Size& maxSize) override;
    
    IBitmapPtr CreateBitmap(int width, int height, PixelFormat format) override;
    IBitmapPtr LoadBitmapFromFile(const std::wstring& filePath) override;
    IBitmapPtr LoadBitmapFromMemory(const void* data, size_t size) override;
    
    // Direct2D specific access
    ID2D1Factory* GetD2DFactory() const { return m_d2dFactory.Get(); }
    ID2D1RenderTarget* GetRenderTarget() const { return m_renderTarget.Get(); }
    IDWriteFactory* GetDWriteFactory() const { return m_dwriteFactory.Get(); }
    
private:
    // Convert our types to D2D types
    D2D1_COLOR_F ToD2DColor(const Color& color) const;
    D2D1_POINT_2F ToD2DPoint(const Point& point) const;
    D2D1_RECT_F ToD2DRect(const Rect& rect) const;
    D2D1_SIZE_F ToD2DSize(const Size& size) const;
    D2D1_MATRIX_3X2_F ToD2DMatrix(const Transform& transform) const;
    
    // Get or create stroke style
    ID2D1StrokeStyle* GetStrokeStyle(const StrokeStyle* style);
    
    // State stack
    struct State {
        Transform transform;
        float opacity = 1.0f;
        bool antialias = true;
    };
    std::stack<State> m_stateStack;
    State m_currentState;
    
    // Layer stack
    struct LayerEntry {
        ComPtr<ID2D1Layer> layer;
        float opacity;
    };
    std::stack<LayerEntry> m_layerStack;
    
    // Direct2D resources
    ComPtr<ID2D1Factory> m_d2dFactory;
    ComPtr<ID2D1RenderTarget> m_renderTarget;
    ComPtr<IDWriteFactory> m_dwriteFactory;
    
    // Stroke style cache
    std::vector<ComPtr<ID2D1StrokeStyle>> m_strokeStyles;
};

} // namespace rendering
} // namespace luaui
