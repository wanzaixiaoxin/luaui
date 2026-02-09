#pragma once

#include "IRenderEngine.h"
#include "D2DRenderContext.h"
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

namespace luaui {
namespace rendering {

using Microsoft::WRL::ComPtr;

class D2DRenderEngine : public IRenderEngine {
public:
    D2DRenderEngine();
    ~D2DRenderEngine() override;
    
    // IRenderEngine
    bool Initialize(RenderAPI api = RenderAPI::Direct2D) override;
    void Shutdown() override;
    bool IsInitialized() const override;
    
    bool CreateRenderTarget(const RenderTargetDesc& desc) override;
    void DestroyRenderTarget() override;
    bool ResizeRenderTarget(int width, int height) override;
    
    int GetWidth() const override;
    int GetHeight() const override;
    Size GetSize() const override;
    float GetDpiX() const override;
    float GetDpiY() const override;
    void SetDpi(float dpiX, float dpiY) override;
    
    IRenderContext* GetContext() override;
    const IRenderContext* GetContext() const override;
    
    bool BeginFrame() override;
    void Present() override;
    void Present(const Rect& dirtyRect) override;
    
    RenderCapabilities GetCapabilities() const override;
    RenderAPI GetAPI() const override;
    std::string GetAPIName() const override;
    std::string GetGPUName() const override;
    
    FrameStats GetStats() const override;
    void ResetStats() override;
    void EnableStats(bool enable) override;
    
    void SetResourceCacheSize(size_t maxBytes) override;
    void ClearResourceCache() override;
    void TrimResourceCache() override;
    
    bool IsDeviceLost() const override;
    bool RecoverDevice() override;
    void OnDeviceLost(std::function<void()> callback) override;
    void OnDeviceRestored(std::function<void()> callback) override;
    
    // Advanced features
    std::unique_ptr<IRenderTarget> CreateRenderTarget(int width, int height, 
                                                       bool useAlpha = true) override;
    std::unique_ptr<ITextLayoutAdvanced> CreateTextLayoutAdvanced() override;
    
    // D2D specific
    ID2D1Factory* GetD2DFactory() const { return m_d2dFactory.Get(); }
    IDWriteFactory* GetDWriteFactory() const { return m_dwriteFactory.Get(); }
    IWICImagingFactory* GetWICFactory() const { return m_wicFactory.Get(); }
    
private:
    bool CreateDeviceIndependentResources();
    bool CreateDeviceResources();
    void DiscardDeviceResources();
    bool CreateHwndRenderTarget(HWND hwnd, int width, int height);
    bool CheckHRESULT(HRESULT hr, const char* msg);
    
    // State
    bool m_initialized = false;
    bool m_inFrame = false;
    bool m_statsEnabled = false;
    RenderAPI m_api = RenderAPI::Direct2D;
    
    // Render target info
    HWND m_hwnd = nullptr;
    int m_width = 0;
    int m_height = 0;
    float m_dpiX = 96.0f;
    float m_dpiY = 96.0f;
    
    // D2D/DirectWrite/WIC
    ComPtr<ID2D1Factory> m_d2dFactory;
    ComPtr<IDWriteFactory> m_dwriteFactory;
    ComPtr<IWICImagingFactory> m_wicFactory;
    ComPtr<ID2D1HwndRenderTarget> m_hwndTarget;
    ComPtr<IDXGIDevice> m_dxgiDevice;
    
    // Render context
    std::unique_ptr<D2DRenderContext> m_context;
    
    // Stats
    FrameStats m_stats;
    
    // Callbacks
    std::vector<std::function<void()>> m_deviceLostCallbacks;
    std::vector<std::function<void()>> m_deviceRestoredCallbacks;
};

} // namespace rendering
} // namespace luaui
