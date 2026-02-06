#pragma once

#include "Types.h"
#include "IBrush.h"
#include "IGeometry.h"
#include "IBitmap.h"
#include "ITextFormat.h"
#include <memory>
#include <stack>

namespace luaui {
namespace rendering {

// Rendering state
struct RenderState {
    Transform transform;
    float opacity = 1.0f;
    bool antialias = true;
    Rect clipRect;
    bool clipEnabled = false;
};

// Render context - main drawing interface
class IRenderContext {
public:
    virtual ~IRenderContext() = default;
    
    // ============ Lifecycle ============
    virtual bool BeginDraw() = 0;
    virtual bool EndDraw() = 0;
    virtual void Clear(const Color& color) = 0;
    virtual void Flush() = 0;
    
    // ============ State Management ============
    virtual void PushState() = 0;
    virtual void PopState() = 0;
    virtual void ResetState() = 0;
    
    virtual void SetTransform(const Transform& transform) = 0;
    virtual void MultiplyTransform(const Transform& transform) = 0;
    virtual Transform GetTransform() const = 0;
    
    virtual void SetOpacity(float opacity) = 0;
    virtual float GetOpacity() const = 0;
    
    virtual void SetAntialias(bool enabled) = 0;
    virtual bool GetAntialias() const = 0;
    
    // ============ Clipping ============
    virtual void PushClip(const Rect& rect) = 0;
    virtual void PushClip(const IGeometry& geometry) = 0;
    virtual void PopClip() = 0;
    virtual void ResetClip() = 0;
    virtual Rect GetClipBounds() const = 0;
    
    // ============ Primitive Drawing ============
    // Lines
    virtual void DrawLine(const Point& p1, const Point& p2, IBrush* brush, float strokeWidth = 1.0f, 
                          const StrokeStyle* strokeStyle = nullptr) = 0;
    
    // Rectangles
    virtual void DrawRectangle(const Rect& rect, IBrush* brush, float strokeWidth = 1.0f,
                               const StrokeStyle* strokeStyle = nullptr) = 0;
    virtual void FillRectangle(const Rect& rect, IBrush* brush) = 0;
    
    // Rounded rectangles
    virtual void DrawRoundedRectangle(const Rect& rect, const CornerRadius& radius, IBrush* brush,
                                      float strokeWidth = 1.0f, const StrokeStyle* strokeStyle = nullptr) = 0;
    virtual void FillRoundedRectangle(const Rect& rect, const CornerRadius& radius, IBrush* brush) = 0;
    
    // Ellipses
    virtual void DrawEllipse(const Point& center, float radiusX, float radiusY, IBrush* brush,
                             float strokeWidth = 1.0f, const StrokeStyle* strokeStyle = nullptr) = 0;
    virtual void FillEllipse(const Point& center, float radiusX, float radiusY, IBrush* brush) = 0;
    
    // Circles (convenience)
    void DrawCircle(const Point& center, float radius, IBrush* brush,
                    float strokeWidth = 1.0f, const StrokeStyle* strokeStyle = nullptr) {
        DrawEllipse(center, radius, radius, brush, strokeWidth, strokeStyle);
    }
    void FillCircle(const Point& center, float radius, IBrush* brush) {
        FillEllipse(center, radius, radius, brush);
    }
    
    // ============ Geometry Drawing ============
    virtual void DrawGeometry(const IGeometry& geometry, IBrush* brush, float strokeWidth = 1.0f,
                              const StrokeStyle* strokeStyle = nullptr) = 0;
    virtual void FillGeometry(const IGeometry& geometry, IBrush* brush) = 0;
    
    // ============ Bitmap Drawing ============
    virtual void DrawBitmap(IBitmap* bitmap, const Point& destination, float opacity = 1.0f) = 0;
    virtual void DrawBitmap(IBitmap* bitmap, const Rect& destination, float opacity = 1.0f) = 0;
    virtual void DrawBitmap(IBitmap* bitmap, const Rect& destination, const Rect& source, 
                            float opacity = 1.0f) = 0;
    
    // ============ Text Drawing ============
    virtual void DrawTextString(const std::wstring& text, ITextFormat* format, const Point& position, IBrush* brush) = 0;
    virtual void DrawTextString(const std::wstring& text, ITextFormat* format, const Rect& rect, IBrush* brush) = 0;
    
    // ============ Layer/Opacity Groups ============
    virtual void PushLayer(float opacity = 1.0f) = 0;
    virtual void PopLayer() = 0;
    
    // ============ Factory Methods ============
    // Brushes
    virtual ISolidColorBrushPtr CreateSolidColorBrush(const Color& color) = 0;
    virtual ILinearGradientBrushPtr CreateLinearGradientBrush(const Point& start, const Point& end,
                                                               const std::vector<GradientStop>& stops) = 0;
    virtual IRadialGradientBrushPtr CreateRadialGradientBrush(const Point& center, float rx, float ry,
                                                               const std::vector<GradientStop>& stops) = 0;
    
    // Geometries
    virtual std::shared_ptr<IRectangleGeometry> CreateRectangleGeometry(const Rect& rect) = 0;
    virtual std::shared_ptr<IRoundedRectangleGeometry> CreateRoundedRectangleGeometry(const Rect& rect, 
                                                                                       const CornerRadius& radius) = 0;
    virtual std::shared_ptr<IEllipseGeometry> CreateEllipseGeometry(const Point& center, float rx, float ry) = 0;
    virtual std::shared_ptr<IPathGeometry> CreatePathGeometry() = 0;
    virtual std::shared_ptr<ICombinedGeometry> CreateCombinedGeometry(IGeometry* g1, IGeometry* g2, 
                                                                       CombineMode mode) = 0;
    
    // Text
    virtual ITextFormatPtr CreateTextFormat(const std::wstring& fontFamily, float fontSize) = 0;
    virtual ITextLayoutPtr CreateTextLayout(const std::wstring& text, ITextFormat* format, 
                                             const Size& maxSize) = 0;
    
    // Bitmaps
    virtual IBitmapPtr CreateBitmap(int width, int height, PixelFormat format) = 0;
    virtual IBitmapPtr LoadBitmapFromFile(const std::wstring& filePath) = 0;
    virtual IBitmapPtr LoadBitmapFromMemory(const void* data, size_t size) = 0;
};

using IRenderContextPtr = std::shared_ptr<IRenderContext>;

} // namespace rendering
} // namespace luaui
