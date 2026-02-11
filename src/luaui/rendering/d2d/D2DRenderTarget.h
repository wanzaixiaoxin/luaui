#pragma once

#include "IRenderTarget.h"
#include "D2DRenderContext.h"
#include <d2d1.h>
#include <wincodec.h>
#include <wrl/client.h>

namespace luaui {
namespace rendering {

using Microsoft::WRL::ComPtr;

// D2D off-screen render target implementation
class D2DRenderTarget : public IRenderTarget {
public:
    D2DRenderTarget(ID2D1Factory* factory, IWICImagingFactory* wicFactory,
                    int width, int height, bool useAlpha);
    ~D2DRenderTarget() override;
    
    // IRenderTarget implementation
    int GetWidth() const override { return m_width; }
    int GetHeight() const override { return m_height; }
    Size GetSize() const override { return Size(static_cast<float>(m_width), static_cast<float>(m_height)); }
    PixelFormat GetFormat() const override { return PixelFormat::BGRA8; }
    
    IRenderContext* GetContext() override;
    const IRenderContext* GetContext() const override;
    
    bool BeginDraw() override;
    bool EndDraw() override;
    void Clear(const Color& color) override;
    
    IBitmapPtr ToBitmap() const override;
    bool SaveToFile(const std::wstring& filePath) const override;
    bool Resize(int width, int height) override;
    
    void* GetNativeTarget() const override { return m_bitmapTarget.Get(); }
    
    // Internal access
    ID2D1BitmapRenderTarget* GetBitmapTarget() const { return m_bitmapTarget.Get(); }
    bool Initialize();
    bool CreateResources(ID2D1RenderTarget* parentTarget);
    
private:
    bool CreateResources();
    void ReleaseResources();
    
    ID2D1Factory* m_d2dFactory = nullptr;
    IWICImagingFactory* m_wicFactory = nullptr;
    
    ComPtr<ID2D1BitmapRenderTarget> m_bitmapTarget;
    std::unique_ptr<D2DRenderContext> m_context;
    
    int m_width = 0;
    int m_height = 0;
    bool m_useAlpha = true;
    bool m_isDrawing = false;
};

} // namespace rendering
} // namespace luaui
