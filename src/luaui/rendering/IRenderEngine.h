#pragma once

#include "IRenderContext.h"
#include <memory>
#include <string>
#include <functional>

namespace luaui {
namespace rendering {

// Rendering API types
enum class RenderAPI {
    Direct2D,
    Direct2D_WARP,  // Software fallback
    Software        // WIC/GDI+ fallback
};

// Device/feature capabilities
struct RenderCapabilities {
    bool hardwareAcceleration = false;
    bool supportsEffects = false;
    bool supportsGeometryRealization = false;
    bool supportsSpriteBatch = false;
    int maxTextureSize = 8192;
    int maxTextureUnits = 16;
};

// Forward declarations
class IRenderTarget;
class ITextLayoutAdvanced;

// Frame statistics
struct FrameStats {
    int drawCallCount = 0;
    int triangleCount = 0;
    float frameTime = 0;        // milliseconds
    float cpuTime = 0;          // milliseconds
    float gpuTime = 0;          // milliseconds (if available)
};

// Render target type
enum class RenderTargetType {
    Window,     // HWND render target
    Bitmap,     // Off-screen bitmap
    DeviceContext  // Device context for composition
};

// Render target description
struct RenderTargetDesc {
    RenderTargetType type;
    void* nativeHandle = nullptr;  // HWND for window, ID2D1Bitmap for bitmap, etc.
    int width = 0;
    int height = 0;
    float dpiX = 96.0f;
    float dpiY = 96.0f;
    bool useTransparency = false;
};

// Main render engine interface
class IRenderEngine {
public:
    virtual ~IRenderEngine() = default;
    
    // ============ Initialization ============
    virtual bool Initialize(RenderAPI api = RenderAPI::Direct2D) = 0;
    virtual void Shutdown() = 0;
    virtual bool IsInitialized() const = 0;
    
    // ============ Render Target Management ============
    virtual bool CreateRenderTarget(const RenderTargetDesc& desc) = 0;
    virtual void DestroyRenderTarget() = 0;
    virtual bool ResizeRenderTarget(int width, int height) = 0;
    
    virtual int GetWidth() const = 0;
    virtual int GetHeight() const = 0;
    virtual Size GetSize() const = 0;
    virtual float GetDpiX() const = 0;
    virtual float GetDpiY() const = 0;
    virtual void SetDpi(float dpiX, float dpiY) = 0;
    
    // ============ Rendering ============
    virtual IRenderContext* GetContext() = 0;
    virtual const IRenderContext* GetContext() const = 0;
    
    virtual bool BeginFrame() = 0;
    virtual void Present() = 0;
    virtual void Present(const Rect& dirtyRect) = 0;  // Partial present
    
    // ============ Capabilities ============
    virtual RenderCapabilities GetCapabilities() const = 0;
    virtual RenderAPI GetAPI() const = 0;
    virtual std::string GetAPIName() const = 0;
    virtual std::string GetGPUName() const = 0;
    
    // ============ Statistics ============
    virtual FrameStats GetStats() const = 0;
    virtual void ResetStats() = 0;
    virtual void EnableStats(bool enable) = 0;
    
    // ============ Resource Management ============
    virtual void SetResourceCacheSize(size_t maxBytes) = 0;
    virtual void ClearResourceCache() = 0;
    virtual void TrimResourceCache() = 0;  // Release unused resources
    
    // ============ Device Lost Recovery ============
    virtual bool IsDeviceLost() const = 0;
    virtual bool RecoverDevice() = 0;
    virtual void OnDeviceLost(std::function<void()> callback) = 0;
    virtual void OnDeviceRestored(std::function<void()> callback) = 0;
    
    // ============ Advanced Features ============
    // Off-screen render target
    virtual std::unique_ptr<IRenderTarget> CreateRenderTarget(int width, int height, 
                                                               bool useAlpha = true) = 0;
    
    // Advanced text layout
    virtual std::unique_ptr<ITextLayoutAdvanced> CreateTextLayoutAdvanced() = 0;
};

using IRenderEnginePtr = std::unique_ptr<IRenderEngine>;

// Factory function
IRenderEnginePtr CreateRenderEngine();

} // namespace rendering
} // namespace luaui
