#include "D2DBitmap.h"
#include "D2DRenderContext.h"
#include <wincodec.h>

namespace luaui {
namespace rendering {

// WIC Helper for image loading
static IWICImagingFactory* GetWICFactory() {
    static IWICImagingFactory* factory = nullptr;
    if (!factory) {
        CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                         IID_PPV_ARGS(&factory));
    }
    return factory;
}

// Convert PixelFormat to D2D format
static DXGI_FORMAT ToDXGIFormat(PixelFormat format) {
    switch (format) {
        case PixelFormat::BGRA8: return DXGI_FORMAT_B8G8R8A8_UNORM;
        case PixelFormat::RGBA8: return DXGI_FORMAT_R8G8B8A8_UNORM;
        case PixelFormat::A8: return DXGI_FORMAT_A8_UNORM;
        default: return DXGI_FORMAT_B8G8R8A8_UNORM;
    }
}

D2DBitmap::D2DBitmap() = default;
D2DBitmap::~D2DBitmap() = default;

bool D2DBitmap::Initialize(D2DRenderContext* context, int width, int height, PixelFormat format) {
    ID2D1RenderTarget* rt = context->GetRenderTarget();
    if (!rt) return false;
    
    D2D1_BITMAP_PROPERTIES props;
    props.pixelFormat.format = ToDXGIFormat(format);
    props.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
    props.dpiX = m_dpiX;
    props.dpiY = m_dpiY;
    
    ID2D1Bitmap* bitmap = nullptr;
    HRESULT hr = rt->CreateBitmap(
        D2D1::SizeU(width, height),
        nullptr, 0,
        &props,
        &bitmap
    );
    
    if (SUCCEEDED(hr) && bitmap) {
        m_bitmap = bitmap;
        m_format = format;
        bitmap->Release();
        return true;
    }
    return false;
}

bool D2DBitmap::LoadFromFile(D2DRenderContext* context, const std::wstring& filePath) {
    ID2D1RenderTarget* rt = context->GetRenderTarget();
    if (!rt) return false;
    
    IWICImagingFactory* wic = GetWICFactory();
    if (!wic) return false;
    
    // Load image using WIC
    IWICBitmapDecoder* decoder = nullptr;
    HRESULT hr = wic->CreateDecoderFromFilename(
        filePath.c_str(),
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &decoder
    );
    
    if (FAILED(hr) || !decoder) return false;
    
    IWICBitmapFrameDecode* frame = nullptr;
    hr = decoder->GetFrame(0, &frame);
    decoder->Release();
    
    if (FAILED(hr) || !frame) return false;
    
    // Convert to 32bpp BGRA
    IWICFormatConverter* converter = nullptr;
    hr = wic->CreateFormatConverter(&converter);
    if (SUCCEEDED(hr) && converter) {
        hr = converter->Initialize(
            frame,
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapDitherTypeNone,
            nullptr,
            0.0,
            WICBitmapPaletteTypeMedianCut
        );
    }
    frame->Release();
    
    if (FAILED(hr) || !converter) {
        converter->Release();
        return false;
    }
    
    // Create D2D bitmap from WIC source
    hr = rt->CreateBitmapFromWicBitmap(converter, nullptr, &m_bitmap);
    converter->Release();
    
    return SUCCEEDED(hr) && m_bitmap;
}

bool D2DBitmap::LoadFromMemory(D2DRenderContext* context, const void* data, size_t size) {
    ID2D1RenderTarget* rt = context->GetRenderTarget();
    if (!rt || !data || size == 0) return false;
    
    IWICImagingFactory* wic = GetWICFactory();
    if (!wic) return false;
    
    // Create stream from memory
    IWICStream* stream = nullptr;
    HRESULT hr = wic->CreateStream(&stream);
    if (FAILED(hr) || !stream) return false;
    
    hr = stream->InitializeFromMemory(
        reinterpret_cast<BYTE*>(const_cast<void*>(data)),
        static_cast<DWORD>(size)
    );
    
    if (FAILED(hr)) {
        stream->Release();
        return false;
    }
    
    // Create decoder
    IWICBitmapDecoder* decoder = nullptr;
    hr = wic->CreateDecoderFromStream(
        stream,
        nullptr,
        WICDecodeMetadataCacheOnLoad,
        &decoder
    );
    stream->Release();
    
    if (FAILED(hr) || !decoder) return false;
    
    // Get frame
    IWICBitmapFrameDecode* frame = nullptr;
    hr = decoder->GetFrame(0, &frame);
    decoder->Release();
    
    if (FAILED(hr) || !frame) return false;
    
    // Convert to 32bpp BGRA
    IWICFormatConverter* converter = nullptr;
    hr = wic->CreateFormatConverter(&converter);
    if (SUCCEEDED(hr) && converter) {
        hr = converter->Initialize(
            frame,
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapDitherTypeNone,
            nullptr,
            0.0,
            WICBitmapPaletteTypeMedianCut
        );
    }
    frame->Release();
    
    if (FAILED(hr) || !converter) {
        if (converter) converter->Release();
        return false;
    }
    
    // Create D2D bitmap
    hr = rt->CreateBitmapFromWicBitmap(converter, nullptr, &m_bitmap);
    converter->Release();
    
    return SUCCEEDED(hr) && m_bitmap;
}

int D2DBitmap::GetWidth() const {
    if (!m_bitmap) return 0;
    D2D1_SIZE_F size = m_bitmap->GetSize();
    return static_cast<int>(size.width);
}

int D2DBitmap::GetHeight() const {
    if (!m_bitmap) return 0;
    D2D1_SIZE_F size = m_bitmap->GetSize();
    return static_cast<int>(size.height);
}

Size D2DBitmap::GetSize() const {
    if (!m_bitmap) return Size();
    D2D1_SIZE_F size = m_bitmap->GetSize();
    return Size(size.width, size.height);
}

PixelFormat D2DBitmap::GetPixelFormat() const {
    return m_format;
}

int D2DBitmap::GetBytesPerPixel() const {
    switch (m_format) {
        case PixelFormat::BGRA8:
        case PixelFormat::RGBA8:
            return 4;
        case PixelFormat::BGR8:
        case PixelFormat::RGB8:
            return 3;
        case PixelFormat::A8:
            return 1;
        default:
            return 4;
    }
}

float D2DBitmap::GetDpiX() const {
    if (!m_bitmap) return m_dpiX;
    float dpiX, dpiY;
    m_bitmap->GetDpi(&dpiX, &dpiY);
    return dpiX;
}

float D2DBitmap::GetDpiY() const {
    if (!m_bitmap) return m_dpiY;
    float dpiX, dpiY;
    m_bitmap->GetDpi(&dpiX, &dpiY);
    return dpiY;
}

void D2DBitmap::SetDpi(float dpiX, float dpiY) {
    m_dpiX = dpiX;
    m_dpiY = dpiY;
    // Note: ID2D1Bitmap doesn't have SetDpi, we just store the values
}

void* D2DBitmap::GetNativeBitmap(IRenderContext* context) {
    return m_bitmap.Get();
}

bool D2DBitmap::Lock(const Rect* rect, void** pixels, int* pitch) {
    // Direct2D bitmaps don't support direct pixel access
    // Would need to use WIC or staging texture
    return false;
}

void D2DBitmap::Unlock() {
    // Not implemented
}

bool D2DBitmap::CopyFromMemory(const void* src, int srcPitch) {
    if (!m_bitmap || !src) return false;
    
    D2D1_SIZE_U size = m_bitmap->GetPixelSize();
    D2D1_RECT_U rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = size.width;
    rect.bottom = size.height;
    
    HRESULT hr = m_bitmap->CopyFromMemory(&rect, src, static_cast<UINT32>(srcPitch));
    return SUCCEEDED(hr);
}

} // namespace rendering
} // namespace luaui
