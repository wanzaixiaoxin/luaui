#pragma once

#include "IBitmap.h"
#include <d2d1.h>
#include <wrl/client.h>

namespace luaui {
namespace rendering {

class D2DRenderContext;
using Microsoft::WRL::ComPtr;

class D2DBitmap : public IBitmap {
public:
    D2DBitmap();
    ~D2DBitmap() override;
    
    // Creation
    bool Initialize(D2DRenderContext* context, int width, int height, PixelFormat format);
    bool LoadFromFile(D2DRenderContext* context, const std::wstring& filePath);
    bool LoadFromMemory(D2DRenderContext* context, const void* data, size_t size);
    
    // IBitmap
    int GetWidth() const override;
    int GetHeight() const override;
    Size GetSize() const override;
    PixelFormat GetPixelFormat() const override;
    int GetBytesPerPixel() const override;
    float GetDpiX() const override;
    float GetDpiY() const override;
    void SetDpi(float dpiX, float dpiY) override;
    void* GetNativeBitmap(IRenderContext* context) override;
    bool Lock(const Rect* rect, void** pixels, int* pitch) override;
    void Unlock() override;
    bool CopyFromMemory(const void* src, int srcPitch) override;
    
    ID2D1Bitmap* GetD2DBitmap() const { return m_bitmap.Get(); }
    
private:
    ComPtr<ID2D1Bitmap> m_bitmap;
    PixelFormat m_format = PixelFormat::BGRA8;
    float m_dpiX = 96.0f;
    float m_dpiY = 96.0f;
};

} // namespace rendering
} // namespace luaui
