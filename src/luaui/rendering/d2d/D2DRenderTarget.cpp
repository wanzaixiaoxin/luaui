#include "D2DRenderTarget.h"
#include "D2DHelpers.h"
#include "D2DBitmap.h"
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

namespace luaui {
namespace rendering {

D2DRenderTarget::D2DRenderTarget(ID2D1Factory* factory, IWICImagingFactory* wicFactory,
                                 int width, int height, bool useAlpha)
    : m_d2dFactory(factory)
    , m_wicFactory(wicFactory)
    , m_width(width)
    , m_height(height)
    , m_useAlpha(useAlpha) {
}

D2DRenderTarget::~D2DRenderTarget() {
    ReleaseResources();
}

bool D2DRenderTarget::Initialize() {
    // Note: D2DRenderTarget needs an existing render target to create a compatible one
    // We'll create resources when first used with a parent target
    return true;
}

bool D2DRenderTarget::CreateResources(ID2D1RenderTarget* parentTarget) {
    ReleaseResources();
    
    if (!parentTarget || m_width <= 0 || m_height <= 0) return false;
    
    // Create compatible render target
    D2D1_SIZE_F size = D2D1::SizeF(static_cast<float>(m_width), static_cast<float>(m_height));
    D2D1_PIXEL_FORMAT pixelFormat = D2D1::PixelFormat(
        DXGI_FORMAT_B8G8R8A8_UNORM,
        m_useAlpha ? D2D1_ALPHA_MODE_PREMULTIPLIED : D2D1_ALPHA_MODE_IGNORE
    );
    
    HRESULT hr = parentTarget->CreateCompatibleRenderTarget(
        &size,
        nullptr,
        &pixelFormat,
        D2D1_COMPATIBLE_RENDER_TARGET_OPTIONS_NONE,
        &m_bitmapTarget
    );
    
    if (FAILED(hr)) return false;
    
    // Create context wrapper and initialize it
    m_context = std::make_unique<D2DRenderContext>();
    if (!m_context->Initialize(m_d2dFactory, m_bitmapTarget.Get(), nullptr)) {
        return false;
    }
    
    return true;
}

void D2DRenderTarget::ReleaseResources() {
    m_context.reset();
    m_bitmapTarget.Reset();
}

IRenderContext* D2DRenderTarget::GetContext() {
    return m_context.get();
}

const IRenderContext* D2DRenderTarget::GetContext() const {
    return m_context.get();
}

bool D2DRenderTarget::BeginDraw() {
    if (!m_bitmapTarget || m_isDrawing) return false;
    
    m_bitmapTarget->BeginDraw();
    m_isDrawing = true;
    return true;
}

bool D2DRenderTarget::EndDraw() {
    if (!m_bitmapTarget || !m_isDrawing) return false;
    
    HRESULT hr = m_bitmapTarget->EndDraw();
    m_isDrawing = false;
    
    return SUCCEEDED(hr);
}

void D2DRenderTarget::Clear(const Color& color) {
    if (!m_bitmapTarget) return;
    m_bitmapTarget->Clear(ToD2DColor(color));
}

IBitmapPtr D2DRenderTarget::ToBitmap() const {
    if (!m_bitmapTarget) return nullptr;
    
    ComPtr<ID2D1Bitmap> d2dBitmap;
    HRESULT hr = m_bitmapTarget->GetBitmap(&d2dBitmap);
    if (FAILED(hr)) return nullptr;
    
    // Get bitmap size and properties
    D2D1_SIZE_U size = d2dBitmap->GetPixelSize();
    D2D1_BITMAP_PROPERTIES props;
    d2dBitmap->GetDpi(&props.dpiX, &props.dpiY);
    
    // Create WIC bitmap and copy data
    ComPtr<IWICBitmap> wicBitmap;
    hr = m_wicFactory->CreateBitmap(
        size.width,
        size.height,
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapCacheOnDemand,
        &wicBitmap
    );
    if (FAILED(hr)) return nullptr;
    
    // Copy pixels from D2D bitmap
    ComPtr<IWICBitmapLock> lock;
    WICRect rect = { 0, 0, static_cast<int>(size.width), static_cast<int>(size.height) };
    hr = wicBitmap->Lock(&rect, WICBitmapLockWrite, &lock);
    if (FAILED(hr)) return nullptr;
    
    UINT bufferSize = 0;
    BYTE* buffer = nullptr;
    UINT stride = 0;
    lock->GetStride(&stride);
    lock->GetDataPointer(&bufferSize, &buffer);
    
    hr = d2dBitmap->CopyFromMemory(nullptr, buffer, stride);
    lock.Reset();
    
    if (FAILED(hr)) return nullptr;
    
    // Create bitmap - need to return as IBitmapPtr
    // For now, return nullptr as we need to load it from a file/memory
    // A proper implementation would create a D2DBitmap and copy the pixels
    return nullptr;
}

bool D2DRenderTarget::SaveToFile(const std::wstring& filePath) const {
    if (!m_bitmapTarget || !m_wicFactory) return false;
    
    ComPtr<ID2D1Bitmap> d2dBitmap;
    HRESULT hr = m_bitmapTarget->GetBitmap(&d2dBitmap);
    if (FAILED(hr)) return false;
    
    D2D1_SIZE_U size = d2dBitmap->GetPixelSize();
    
    // Create WIC bitmap
    ComPtr<IWICBitmap> wicBitmap;
    hr = m_wicFactory->CreateBitmap(
        size.width,
        size.height,
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapCacheOnLoad,
        &wicBitmap
    );
    if (FAILED(hr)) return false;
    
    // Copy pixels
    ComPtr<IWICBitmapLock> lock;
    WICRect rect = {0, 0, static_cast<int>(size.width), static_cast<int>(size.height)};
    hr = wicBitmap->Lock(&rect, WICBitmapLockWrite, &lock);
    if (FAILED(hr)) return false;
    
    UINT bufferSize = 0;
    BYTE* buffer = nullptr;
    UINT stride = 0;
    lock->GetStride(&stride);
    lock->GetDataPointer(&bufferSize, &buffer);
    
    hr = d2dBitmap->CopyFromMemory(nullptr, buffer, stride);
    lock.Reset();
    
    if (FAILED(hr)) return false;
    
    // Create encoder
    ComPtr<IWICBitmapEncoder> encoder;
    GUID containerFormat = GUID_ContainerFormatPng;
    if (filePath.size() > 4) {
        std::wstring ext = filePath.substr(filePath.size() - 4);
        for (auto& c : ext) c = static_cast<wchar_t>(std::tolower(static_cast<unsigned char>(c)));
        if (ext == L".jpg" || ext == L"jpeg") {
            containerFormat = GUID_ContainerFormatJpeg;
        } else if (ext == L".bmp") {
            containerFormat = GUID_ContainerFormatBmp;
        }
    }
    
    hr = m_wicFactory->CreateEncoder(containerFormat, nullptr, &encoder);
    if (FAILED(hr)) return false;
    
    ComPtr<IWICStream> stream;
    hr = m_wicFactory->CreateStream(&stream);
    if (FAILED(hr)) return false;
    
    hr = stream->InitializeFromFilename(filePath.c_str(), GENERIC_WRITE);
    if (FAILED(hr)) return false;
    
    hr = encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache);
    if (FAILED(hr)) return false;
    
    ComPtr<IWICBitmapFrameEncode> frame;
    hr = encoder->CreateNewFrame(&frame, nullptr);
    if (FAILED(hr)) return false;
    
    hr = frame->Initialize(nullptr);
    if (FAILED(hr)) return false;
    
    hr = frame->SetSize(size.width, size.height);
    if (FAILED(hr)) return false;
    
    WICPixelFormatGUID pixelFormat = GUID_WICPixelFormat32bppPBGRA;
    hr = frame->SetPixelFormat(&pixelFormat);
    if (FAILED(hr)) return false;
    
    hr = frame->WriteSource(wicBitmap.Get(), nullptr);
    if (FAILED(hr)) return false;
    
    hr = frame->Commit();
    if (FAILED(hr)) return false;
    
    hr = encoder->Commit();
    return SUCCEEDED(hr);
}

bool D2DRenderTarget::Resize(int width, int height) {
    if (m_width == width && m_height == height) return true;
    
    m_width = width;
    m_height = height;
    
    // Release current resources - they will be recreated when needed
    ReleaseResources();
    return true;
}

} // namespace rendering
} // namespace luaui
