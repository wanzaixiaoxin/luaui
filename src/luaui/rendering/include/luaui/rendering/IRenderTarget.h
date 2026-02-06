#pragma once

#include "Types.h"
#include "IBitmap.h"
#include <memory>

namespace luaui {
namespace rendering {

class IRenderContext;

// Render target types
enum class RenderTargetUsage {
    Default,        // General purpose rendering
    Static,         // Rarely changing content (caching)
    Dynamic,        // Frequently updated
    Readable        // CPU readable (for screenshots)
};

// Render target properties
struct RenderTargetProperties {
    int width = 0;
    int height = 0;
    PixelFormat format = PixelFormat::BGRA8;
    float dpiX = 96.0f;
    float dpiY = 96.0f;
    RenderTargetUsage usage = RenderTargetUsage::Default;
    bool useAlpha = true;
    bool useHardware = true;
};

// Off-screen render target interface
class IRenderTarget {
public:
    virtual ~IRenderTarget() = default;
    
    // Properties
    virtual int GetWidth() const = 0;
    virtual int GetHeight() const = 0;
    virtual Size GetSize() const = 0;
    virtual PixelFormat GetFormat() const = 0;
    
    // Context for rendering to this target
    virtual IRenderContext* GetContext() = 0;
    virtual const IRenderContext* GetContext() const = 0;
    
    // Drawing lifecycle
    virtual bool BeginDraw() = 0;
    virtual bool EndDraw() = 0;
    virtual void Clear(const Color& color) = 0;
    
    // Copy to bitmap
    virtual IBitmapPtr ToBitmap() const = 0;
    
    // Save to file (convenience)
    virtual bool SaveToFile(const std::wstring& filePath) const = 0;
    
    // Resize (may recreate resources)
    virtual bool Resize(int width, int height) = 0;
    
    // Native access
    virtual void* GetNativeTarget() const = 0;
};

using IRenderTargetPtr = std::unique_ptr<IRenderTarget>;

// Screenshot/Screen capture utility
class IScreenCapture {
public:
    virtual ~IScreenCapture() = default;
    
    // Capture window or screen region
    virtual IBitmapPtr CaptureWindow(void* nativeWindowHandle) = 0;
    virtual IBitmapPtr CaptureScreen(int x, int y, int width, int height) = 0;
    virtual IBitmapPtr CaptureRenderTarget(IRenderTarget* target) = 0;
    
    // Save screenshot
    virtual bool SaveScreenshot(const std::wstring& filePath, IBitmapPtr bitmap) = 0;
};

using IScreenCapturePtr = std::unique_ptr<IScreenCapture>;

} // namespace rendering
} // namespace luaui
