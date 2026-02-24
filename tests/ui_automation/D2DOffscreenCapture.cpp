// D2DOffscreenCapture.cpp - Offscreen rendering for screenshot capture

#include "UIAutomationFramework.h"
#include "TestHooks.h"
#include "Window.h"
#include "IRenderContext.h"
#include "d2d/D2DRenderEngine.h"
#include "d2d/D2DRenderContext.h"
#include <windows.h>
#include <d2d1.h>
#include <wincodec.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "windowscodecs.lib")

namespace luaui {
namespace test {
namespace automation {

using namespace luaui::rendering;

// ============================================================================
// Implementation Class
// ============================================================================
class D2DOffscreenCapture::Impl {
public:
    ComPtr<ID2D1Factory> d2dFactory;
    ComPtr<IWICImagingFactory> wicFactory;
    bool initialized = false;
};

// ============================================================================
// Constructor / Destructor
// ============================================================================
D2DOffscreenCapture::D2DOffscreenCapture() 
    : m_impl(std::make_unique<Impl>()) {}

D2DOffscreenCapture::~D2DOffscreenCapture() {
    Shutdown();
}

// ============================================================================
// Initialization
// ============================================================================
bool D2DOffscreenCapture::Initialize() {
    if (m_impl->initialized) return true;
    
    // Create D2D factory
    D2D1_FACTORY_OPTIONS options = {};
    options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
    
    HRESULT hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        options,
        m_impl->d2dFactory.GetAddressOf()
    );
    
    if (FAILED(hr)) return false;
    
    // Create WIC factory
    hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IWICImagingFactory,
        reinterpret_cast<void**>(m_impl->wicFactory.GetAddressOf())
    );
    
    if (FAILED(hr)) return false;
    
    m_impl->initialized = true;
    return true;
}

void D2DOffscreenCapture::Shutdown() {
    m_impl->d2dFactory.Reset();
    m_impl->wicFactory.Reset();
    m_impl->initialized = false;
}

// ============================================================================
// Capture Implementation
// ============================================================================
bool D2DOffscreenCapture::CaptureWindow(Window* window, const std::filesystem::path& outputPath) {
    if (!m_impl->initialized || !window) return false;
    
    auto* tw = dynamic_cast<TestWindow*>(window);
    if (!tw) return false;
    
    // Get window size
    auto size = tw->GetWindowSize();
    int width = static_cast<int>(size.width);
    int height = static_cast<int>(size.height);
    
    if (width <= 0 || height <= 0) return false;
    
    // Create WIC bitmap
    ComPtr<IWICBitmap> wicBitmap;
    HRESULT hr = m_impl->wicFactory->CreateBitmap(
        static_cast<UINT>(width),
        static_cast<UINT>(height),
        GUID_WICPixelFormat32bppBGRA,
        WICBitmapCacheOnDemand,
        wicBitmap.GetAddressOf()
    );
    
    if (FAILED(hr)) return false;
    
    // Create D2D render target from WIC bitmap
    ComPtr<ID2D1RenderTarget> renderTarget;
    D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
    );
    
    hr = m_impl->d2dFactory->CreateWicBitmapRenderTarget(
        wicBitmap.Get(),
        rtProps,
        renderTarget.GetAddressOf()
    );
    
    if (FAILED(hr)) return false;
    
    // Render the window content
    // This requires access to the window's render logic
    // For now, we'll capture the window's current render target
    
    renderTarget->BeginDraw();
    renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
    
    // Get the window's root control and render it
    auto* root = tw->GetRootControl();
    if (root) {
        if (auto* renderable = root->AsRenderable()) {
            // We need to render using the renderTarget
            // This is complex as it requires creating a temporary render context
            // For now, we'll use the window's existing render method
            
            // Alternative: Copy from existing render target if available
        }
    }
    
    hr = renderTarget->EndDraw();
    if (FAILED(hr)) return false;
    
    // Save to file
    return SaveWICBitmap(wicBitmap.Get(), outputPath);
}

bool D2DOffscreenCapture::CaptureControl(Control* control, const std::filesystem::path& outputPath) {
    if (!m_impl->initialized || !control) return false;
    
    auto rect = ControlTestHook::GetRenderRect(control);
    return CaptureRegion(rect, outputPath);
}

bool D2DOffscreenCapture::CaptureRegion(const luaui::rendering::Rect& region, 
                                         const std::filesystem::path& outputPath) {
    if (!m_impl->initialized) return false;
    
    int width = static_cast<int>(region.width);
    int height = static_cast<int>(region.height);
    
    if (width <= 0 || height <= 0) return false;
    
    // Create WIC bitmap for region
    ComPtr<IWICBitmap> wicBitmap;
    HRESULT hr = m_impl->wicFactory->CreateBitmap(
        static_cast<UINT>(width),
        static_cast<UINT>(height),
        GUID_WICPixelFormat32bppBGRA,
        WICBitmapCacheOnDemand,
        wicBitmap.GetAddressOf()
    );
    
    if (FAILED(hr)) return false;
    
    // Create render target
    ComPtr<ID2D1RenderTarget> renderTarget;
    D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
    );
    
    hr = m_impl->d2dFactory->CreateWicBitmapRenderTarget(
        wicBitmap.Get(),
        rtProps,
        renderTarget.GetAddressOf()
    );
    
    if (FAILED(hr)) return false;
    
    // Render (placeholder - would need proper render context setup)
    renderTarget->BeginDraw();
    renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
    hr = renderTarget->EndDraw();
    
    if (FAILED(hr)) return false;
    
    return SaveWICBitmap(wicBitmap.Get(), outputPath);
}

std::vector<uint8_t> D2DOffscreenCapture::GetPixelData(Window* window) {
    std::vector<uint8_t> result;
    
    if (!m_impl->initialized || !window) return result;
    
    auto* tw = dynamic_cast<TestWindow*>(window);
    if (!tw) return result;
    
    auto size = tw->GetWindowSize();
    int width = static_cast<int>(size.width);
    int height = static_cast<int>(size.height);
    
    // Create temporary file for capture
    auto tempPath = std::filesystem::temp_directory_path() / "luaui_capture_temp.png";
    
    if (CaptureWindow(window, tempPath)) {
        // Load the image data
        ComPtr<IWICBitmapDecoder> decoder;
        HRESULT hr = m_impl->wicFactory->CreateDecoderFromFilename(
            tempPath.wstring().c_str(),
            nullptr,
            GENERIC_READ,
            WICDecodeMetadataCacheOnLoad,
            decoder.GetAddressOf()
        );
        
        if (SUCCEEDED(hr)) {
            ComPtr<IWICBitmapFrameDecode> frame;
            hr = decoder->GetFrame(0, frame.GetAddressOf());
            
            if (SUCCEEDED(hr)) {
                ComPtr<IWICFormatConverter> converter;
                hr = m_impl->wicFactory->CreateFormatConverter(converter.GetAddressOf());
                
                if (SUCCEEDED(hr)) {
                    hr = converter->Initialize(
                        frame.Get(),
                        GUID_WICPixelFormat32bppBGRA,
                        WICBitmapDitherTypeNone,
                        nullptr,
                        0.0,
                        WICBitmapPaletteTypeCustom
                    );
                    
                    if (SUCCEEDED(hr)) {
                        result.resize(width * height * 4);
                        hr = converter->CopyPixels(
                            nullptr,
                            width * 4,
                            static_cast<UINT>(result.size()),
                            result.data()
                        );
                        
                        if (FAILED(hr)) {
                            result.clear();
                        }
                    }
                }
            }
        }
        
        // Clean up temp file
        std::filesystem::remove(tempPath);
    }
    
    return result;
}

// ============================================================================
// Helper Methods
// ============================================================================
bool D2DOffscreenCapture::SaveWICBitmap(IWICBitmap* bitmap, const std::filesystem::path& path) {
    if (!bitmap || !m_impl->wicFactory) return false;
    
    ComPtr<IWICStream> stream;
    HRESULT hr = m_impl->wicFactory->CreateStream(stream.GetAddressOf());
    if (FAILED(hr)) return false;
    
    hr = stream->InitializeFromFilename(path.wstring().c_str(), GENERIC_WRITE);
    if (FAILED(hr)) return false;
    
    // Determine encoder from extension
    GUID containerFormat = GUID_ContainerFormatPng;
    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if (ext == ".bmp") {
        containerFormat = GUID_ContainerFormatBmp;
    } else if (ext == ".jpg" || ext == ".jpeg") {
        containerFormat = GUID_ContainerFormatJpeg;
    }
    
    ComPtr<IWICBitmapEncoder> encoder;
    hr = m_impl->wicFactory->CreateEncoder(containerFormat, nullptr, encoder.GetAddressOf());
    if (FAILED(hr)) return false;
    
    hr = encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache);
    if (FAILED(hr)) return false;
    
    ComPtr<IWICBitmapFrameEncode> frame;
    IPropertyBag2* props = nullptr;
    hr = encoder->CreateNewFrame(frame.GetAddressOf(), &props);
    if (FAILED(hr)) return false;
    
    hr = frame->Initialize(props);
    if (props) props->Release();
    if (FAILED(hr)) return false;
    
    UINT width, height;
    bitmap->GetSize(&width, &height);
    hr = frame->SetSize(width, height);
    if (FAILED(hr)) return false;
    
    WICPixelFormatGUID format = GUID_WICPixelFormat32bppBGRA;
    hr = frame->SetPixelFormat(&format);
    if (FAILED(hr)) return false;
    
    hr = frame->WriteSource(bitmap, nullptr);
    if (FAILED(hr)) return false;
    
    hr = frame->Commit();
    if (FAILED(hr)) return false;
    
    hr = encoder->Commit();
    
    return SUCCEEDED(hr);
}

} // namespace automation
} // namespace test
} // namespace luaui
