#include "D2DRenderEngine.h"
#include "D2DRenderTarget.h"
#include "D2DTextLayoutAdvanced.h"
#include <iostream>

namespace luaui {
namespace rendering {

D2DRenderEngine::D2DRenderEngine() = default;

D2DRenderEngine::~D2DRenderEngine() {
    Shutdown();
}

bool D2DRenderEngine::Initialize(RenderAPI api) {
    if (m_initialized) return true;
    
    m_api = api;
    
    // Create D2D factory
    D2D1_FACTORY_OPTIONS options = {};
    #ifdef _DEBUG
    options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
    #endif
    
    D2D1_FACTORY_TYPE factoryType = (api == RenderAPI::Direct2D_WARP) ? 
        D2D1_FACTORY_TYPE_SINGLE_THREADED : D2D1_FACTORY_TYPE_MULTI_THREADED;
    
    HRESULT hr = D2D1CreateFactory(
        factoryType,
        __uuidof(ID2D1Factory),
        &options,
        &m_d2dFactory
    );
    
    if (FAILED(hr)) {
        std::cerr << "Failed to create D2D factory: 0x" << std::hex << hr << std::endl;
        return false;
    }
    
    // Create DirectWrite factory
    IDWriteFactory* dwriteFactory = nullptr;
    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&dwriteFactory)
    );
    if (SUCCEEDED(hr) && dwriteFactory) {
        m_dwriteFactory = dwriteFactory;
        dwriteFactory->Release();
    }
    
    if (FAILED(hr)) {
        std::cerr << "Failed to create DWrite factory: 0x" << std::hex << hr << std::endl;
        return false;
    }
    
    // Create WIC factory
    hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IWICImagingFactory,
        &m_wicFactory
    );
    
    if (FAILED(hr)) {
        std::cerr << "Failed to create WIC factory: 0x" << std::hex << hr << std::endl;
        return false;
    }
    
    // Create render context
    m_context = std::make_unique<D2DRenderContext>();
    
    m_initialized = true;
    return true;
}

void D2DRenderEngine::Shutdown() {
    m_context.reset();
    DiscardDeviceResources();
    
    m_dwriteFactory.Reset();
    m_d2dFactory.Reset();
    
    m_initialized = false;
}

bool D2DRenderEngine::IsInitialized() const {
    return m_initialized;
}

bool D2DRenderEngine::CreateRenderTarget(const RenderTargetDesc& desc) {
    if (!m_initialized) return false;
    
    if (desc.type == RenderTargetType::Window) {
        HWND hwnd = static_cast<HWND>(desc.nativeHandle);
        if (!hwnd) return false;
        
        m_hwnd = hwnd;
        m_width = desc.width;
        m_height = desc.height;
        m_dpiX = desc.dpiX;
        m_dpiY = desc.dpiY;
        
        if (!CreateHwndRenderTarget(hwnd, desc.width, desc.height)) {
            return false;
        }
        
        // Initialize context with render target
        if (!m_context->Initialize(m_d2dFactory.Get(), m_hwndTarget.Get(), m_dwriteFactory.Get())) {
            return false;
        }
        
        return true;
    }
    
    // TODO: Support bitmap and device context render targets
    return false;
}

void D2DRenderEngine::DestroyRenderTarget() {
    m_context->Shutdown();
    DiscardDeviceResources();
    m_hwnd = nullptr;
    m_width = 0;
    m_height = 0;
}

bool D2DRenderEngine::ResizeRenderTarget(int width, int height) {
    if (!m_hwndTarget) return false;
    
    m_width = width;
    m_height = height;
    
    HRESULT hr = m_hwndTarget->Resize(D2D1::SizeU(width, height));
    return SUCCEEDED(hr);
}

int D2DRenderEngine::GetWidth() const {
    return m_width;
}

int D2DRenderEngine::GetHeight() const {
    return m_height;
}

Size D2DRenderEngine::GetSize() const {
    return Size(static_cast<float>(m_width), static_cast<float>(m_height));
}

float D2DRenderEngine::GetDpiX() const {
    return m_dpiX;
}

float D2DRenderEngine::GetDpiY() const {
    return m_dpiY;
}

void D2DRenderEngine::SetDpi(float dpiX, float dpiY) {
    m_dpiX = dpiX;
    m_dpiY = dpiY;
    if (m_hwndTarget) {
        m_hwndTarget->SetDpi(dpiX, dpiY);
    }
}

IRenderContext* D2DRenderEngine::GetContext() {
    return m_context.get();
}

const IRenderContext* D2DRenderEngine::GetContext() const {
    return m_context.get();
}

bool D2DRenderEngine::BeginFrame() {
    if (!m_context || !m_hwndTarget) return false;
    if (m_inFrame) return true;
    
    m_context->BeginDraw();
    m_context->Clear(Color(1, 1, 1, 1)); // Clear to white
    
    m_inFrame = true;
    return true;
}

void D2DRenderEngine::Present() {
    if (!m_inFrame) return;
    
    HRESULT hr = m_context->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET) {
        // Device lost
        RecoverDevice();
    }
    
    ValidateRect(m_hwnd, nullptr);
    m_inFrame = false;
}

void D2DRenderEngine::Present(const Rect& dirtyRect) {
    // Partial present not implemented for HWND target
    Present();
}

RenderCapabilities D2DRenderEngine::GetCapabilities() const {
    RenderCapabilities caps;
    caps.hardwareAcceleration = (m_api == RenderAPI::Direct2D);
    caps.supportsEffects = true;
    caps.supportsGeometryRealization = true;
    caps.supportsSpriteBatch = false;
    caps.maxTextureSize = 8192;
    caps.maxTextureUnits = 16;
    return caps;
}

RenderAPI D2DRenderEngine::GetAPI() const {
    return m_api;
}

std::string D2DRenderEngine::GetAPIName() const {
    switch (m_api) {
        case RenderAPI::Direct2D: return "Direct2D";
        case RenderAPI::Direct2D_WARP: return "Direct2D (WARP)";
        case RenderAPI::Software: return "Software";
        default: return "Unknown";
    }
}

std::string D2DRenderEngine::GetGPUName() const {
    return "Unknown"; // Would need DXGI adapter query
}

FrameStats D2DRenderEngine::GetStats() const {
    return m_stats;
}

void D2DRenderEngine::ResetStats() {
    m_stats = FrameStats();
}

void D2DRenderEngine::EnableStats(bool enable) {
    m_statsEnabled = enable;
}

void D2DRenderEngine::SetResourceCacheSize(size_t maxBytes) {
    // TODO: Implement resource cache
}

void D2DRenderEngine::ClearResourceCache() {
    // TODO: Implement resource cache
}

void D2DRenderEngine::TrimResourceCache() {
    // TODO: Implement resource cache
}

bool D2DRenderEngine::IsDeviceLost() const {
    // Check by attempting to query interface
    if (!m_hwndTarget) return true;
    
    ComPtr<ID2D1Resource> resource;
    HRESULT hr = m_hwndTarget.As(&resource);
    return FAILED(hr);
}

bool D2DRenderEngine::RecoverDevice() {
    DiscardDeviceResources();
    
    // Notify callbacks
    for (auto& callback : m_deviceLostCallbacks) {
        callback();
    }
    
    // Recreate
    if (!CreateHwndRenderTarget(m_hwnd, m_width, m_height)) {
        return false;
    }
    
    // Reinitialize context
    m_context->Initialize(m_d2dFactory.Get(), m_hwndTarget.Get(), m_dwriteFactory.Get());
    
    // Notify restored
    for (auto& callback : m_deviceRestoredCallbacks) {
        callback();
    }
    
    return true;
}

void D2DRenderEngine::OnDeviceLost(std::function<void()> callback) {
    m_deviceLostCallbacks.push_back(callback);
}

void D2DRenderEngine::OnDeviceRestored(std::function<void()> callback) {
    m_deviceRestoredCallbacks.push_back(callback);
}

// Advanced features
std::unique_ptr<IRenderTarget> D2DRenderEngine::CreateRenderTarget(int width, int height, bool useAlpha) {
    if (!m_initialized || !m_d2dFactory || !m_wicFactory || !m_hwndTarget) return nullptr;
    
    auto target = std::make_unique<D2DRenderTarget>(
        m_d2dFactory.Get(),
        m_wicFactory.Get(),
        width,
        height,
        useAlpha
    );
    
    if (!target->CreateResources(m_hwndTarget.Get())) {
        return nullptr;
    }
    
    return target;
}

std::unique_ptr<ITextLayoutAdvanced> D2DRenderEngine::CreateTextLayoutAdvanced() {
    if (!m_initialized || !m_dwriteFactory) return nullptr;
    
    return std::make_unique<D2DTextLayoutAdvanced>(m_dwriteFactory.Get());
}

// Private helpers
bool D2DRenderEngine::CreateHwndRenderTarget(HWND hwnd, int width, int height) {
    if (!m_d2dFactory) return false;
    
    RECT rc;
    GetClientRect(hwnd, &rc);
    
    D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
    if (width > 0 && height > 0) {
        size = D2D1::SizeU(width, height);
    }
    
    D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
    rtProps.dpiX = m_dpiX;
    rtProps.dpiY = m_dpiY;
    
    D2D1_HWND_RENDER_TARGET_PROPERTIES hwndProps = D2D1::HwndRenderTargetProperties(
        hwnd,
        size
    );
    
    ID2D1HwndRenderTarget* target = nullptr;
    HRESULT hr = m_d2dFactory->CreateHwndRenderTarget(
        &rtProps,
        &hwndProps,
        &target
    );
    
    if (SUCCEEDED(hr) && target) {
        m_hwndTarget = target;
        target->Release();
        return true;
    }
    
    return false;
}

void D2DRenderEngine::DiscardDeviceResources() {
    m_hwndTarget.Reset();
}

bool D2DRenderEngine::CheckHRESULT(HRESULT hr, const char* msg) {
    if (FAILED(hr)) {
        std::cerr << msg << " failed: 0x" << std::hex << hr << std::endl;
        return false;
    }
    return true;
}

// Factory function
IRenderEnginePtr CreateRenderEngine() {
    return std::make_unique<D2DRenderEngine>();
}

} // namespace rendering
} // namespace luaui
