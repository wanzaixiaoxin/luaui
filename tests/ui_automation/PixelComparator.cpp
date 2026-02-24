// PixelComparator.cpp - Image comparison implementation

#include "PixelComparator.h"
#include <windows.h>
#include <wincodec.h>
#include <cmath>
#include <fstream>
#include <iostream>

#pragma comment(lib, "windowscodecs.lib")

namespace luaui {
namespace test {
namespace automation {

// WIC Helper class for image I/O
class WICHelper {
public:
    static WICHelper& Instance() {
        static WICHelper instance;
        return instance;
    }
    
    bool Initialize() {
        if (m_initialized) return true;
        HRESULT hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_IWICImagingFactory,
            reinterpret_cast<void**>(&m_factory)
        );
        m_initialized = SUCCEEDED(hr);
        return m_initialized;
    }
    
    void Shutdown() {
        if (m_factory) {
            m_factory->Release();
            m_factory = nullptr;
        }
        m_initialized = false;
    }
    
    IWICImagingFactory* GetFactory() const { return m_factory; }
    
private:
    WICHelper() = default;
    ~WICHelper() { Shutdown(); }
    
    IWICImagingFactory* m_factory = nullptr;
    bool m_initialized = false;
};

// ============================================================================
// Pixel Comparison Implementation
// ============================================================================
PixelComparisonResult PixelComparator::CompareImages(
    const std::filesystem::path& baseline,
    const std::filesystem::path& current,
    const PixelComparisonOptions& options) {
    
    PixelComparisonResult result = {};
    
    auto img1 = LoadImage(baseline);
    auto img2 = LoadImage(current);
    
    if (!img1.IsValid() || !img2.IsValid()) {
        result.matches = false;
        result.diffPercentage = 1.0f;
        return result;
    }
    
    if (img1.width != img2.width || img1.height != img2.height) {
        result.matches = false;
        result.diffPercentage = 1.0f;
        return result;
    }
    
    result = CompareImages(img1, img2, options);
    
    if (!result.matches && options.generateDiffImage) {
        std::filesystem::path diffPath = current.parent_path() / (current.stem().string() + "_diff.png");
        GenerateDiffImage(img1, img2, diffPath, options.diffColor);
        result.diffImagePath = diffPath;
    }
    
    return result;
}

PixelComparisonResult PixelComparator::CompareImages(
    const ImageData& baseline,
    const ImageData& current,
    const PixelComparisonOptions& options) {
    
    PixelComparisonResult result = {};
    result.totalPixels = baseline.width * baseline.height;
    
    if (baseline.width != current.width || baseline.height != current.height) {
        result.matches = false;
        result.diffPercentage = 1.0f;
        return result;
    }
    
    int diffCount = 0;
    
    for (int y = 0; y < baseline.height; y++) {
        for (int x = 0; x < baseline.width; x++) {
            // Check if this pixel should be ignored
            bool ignore = false;
            if (options.ignoreRegions) {
                for (const auto& rect : options.ignoreRects) {
                    if (rect.Contains(x, y)) {
                        ignore = true;
                        break;
                    }
                }
            }
            
            if (ignore) continue;
            
            auto* p1 = baseline.GetPixel(x, y);
            auto* p2 = current.GetPixel(x, y);
            
            if (!p1 || !p2) continue;
            
            float diff = CalculatePixelDiff(p1, p2, baseline.channels);
            
            if (diff > options.threshold) {
                diffCount++;
            }
        }
    }
    
    result.diffPixelCount = diffCount;
    result.diffPercentage = static_cast<float>(diffCount) / result.totalPixels;
    result.matches = result.diffPercentage <= options.threshold;
    
    return result;
}

bool PixelComparator::AreEqual(const ImageData& img1, const ImageData& img2, float tolerance) {
    PixelComparisonOptions options;
    options.threshold = tolerance;
    auto result = CompareImages(img1, img2, options);
    return result.matches;
}

bool PixelComparator::GenerateDiffImage(
    const ImageData& baseline,
    const ImageData& current,
    const std::filesystem::path& outputPath,
    uint32_t diffColor) {
    
    if (baseline.width != current.width || baseline.height != current.height) {
        return false;
    }
    
    ImageData diffImage;
    diffImage.width = baseline.width;
    diffImage.height = baseline.height;
    diffImage.channels = 4;  // RGBA
    diffImage.pixels.resize(diffImage.width * diffImage.height * 4);
    
    uint8_t r = (diffColor >> 16) & 0xFF;
    uint8_t g = (diffColor >> 8) & 0xFF;
    uint8_t b = diffColor & 0xFF;
    uint8_t a = (diffColor >> 24) & 0xFF;
    
    for (int y = 0; y < baseline.height; y++) {
        for (int x = 0; x < baseline.width; x++) {
            auto* p1 = baseline.GetPixel(x, y);
            auto* p2 = current.GetPixel(x, y);
            auto* pd = diffImage.GetPixel(x, y);
            
            if (!p1 || !p2 || !pd) continue;
            
            bool isDiff = false;
            for (int c = 0; c < baseline.channels && c < current.channels; c++) {
                if (std::abs(p1[c] - p2[c]) > 5) {
                    isDiff = true;
                    break;
                }
            }
            
            if (isDiff) {
                pd[0] = r;
                pd[1] = g;
                pd[2] = b;
                pd[3] = a;
            } else {
                // Use baseline pixel with reduced opacity
                pd[0] = p1[0];
                pd[1] = p1[1];
                pd[2] = p1[2];
                pd[3] = 128;
            }
        }
    }
    
    return SaveImage(outputPath, diffImage);
}

// ============================================================================
// Image I/O
// ============================================================================
ImageData PixelComparator::LoadImage(const std::filesystem::path& path) {
    ImageData result = {};
    
    if (!WICHelper::Instance().Initialize()) {
        return result;
    }
    
    IWICBitmapDecoder* decoder = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    IWICFormatConverter* converter = nullptr;
    
    auto factory = WICHelper::Instance().GetFactory();
    
    HRESULT hr = factory->CreateDecoderFromFilename(
        path.wstring().c_str(),
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &decoder
    );
    
    if (FAILED(hr)) goto cleanup;
    
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) goto cleanup;
    
    hr = factory->CreateFormatConverter(&converter);
    if (FAILED(hr)) goto cleanup;
    
    hr = converter->Initialize(
        frame,
        GUID_WICPixelFormat32bppBGRA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0,
        WICBitmapPaletteTypeCustom
    );
    
    if (FAILED(hr)) goto cleanup;
    
    converter->GetSize(reinterpret_cast<UINT*>(&result.width), reinterpret_cast<UINT*>(&result.height));
    
    result.channels = 4;
    result.pixels.resize(result.width * result.height * result.channels);
    
    hr = converter->CopyPixels(
        nullptr,
        result.width * result.channels,
        static_cast<UINT>(result.pixels.size()),
        result.pixels.data()
    );
    
cleanup:
    if (converter) converter->Release();
    if (frame) frame->Release();
    if (decoder) decoder->Release();
    
    return result;
}

bool PixelComparator::SaveImage(const std::filesystem::path& path, const ImageData& image) {
    if (!WICHelper::Instance().Initialize()) {
        return false;
    }
    
    IWICBitmapEncoder* encoder = nullptr;
    IWICBitmapFrameEncode* frame = nullptr;
    IPropertyBag2* props = nullptr;
    IWICStream* stream = nullptr;
    
    auto factory = WICHelper::Instance().GetFactory();
    
    HRESULT hr = factory->CreateStream(&stream);
    if (FAILED(hr)) goto cleanup;
    
    hr = stream->InitializeFromFilename(path.wstring().c_str(), GENERIC_WRITE);
    if (FAILED(hr)) goto cleanup;
    
    GUID containerFormat = path.extension() == ".png" ? GUID_ContainerFormatPng : GUID_ContainerFormatBmp;
    hr = factory->CreateEncoder(containerFormat, nullptr, &encoder);
    if (FAILED(hr)) goto cleanup;
    
    hr = encoder->Initialize(stream, WICBitmapEncoderNoCache);
    if (FAILED(hr)) goto cleanup;
    
    hr = encoder->CreateNewFrame(&frame, &props);
    if (FAILED(hr)) goto cleanup;
    
    hr = frame->Initialize(props);
    if (FAILED(hr)) goto cleanup;
    
    hr = frame->SetSize(static_cast<UINT>(image.width), static_cast<UINT>(image.height));
    if (FAILED(hr)) goto cleanup;
    
    WICPixelFormatGUID format = GUID_WICPixelFormat32bppBGRA;
    hr = frame->SetPixelFormat(&format);
    if (FAILED(hr)) goto cleanup;
    
    hr = frame->WritePixels(
        static_cast<UINT>(image.height),
        static_cast<UINT>(image.width * image.channels),
        static_cast<UINT>(image.pixels.size()),
        const_cast<BYTE*>(image.pixels.data())
    );
    
    if (FAILED(hr)) goto cleanup;
    
    hr = frame->Commit();
    if (FAILED(hr)) goto cleanup;
    
    hr = encoder->Commit();
    
cleanup:
    if (props) props->Release();
    if (frame) frame->Release();
    if (encoder) encoder->Release();
    if (stream) stream->Release();
    
    return SUCCEEDED(hr);
}

ImageData PixelComparator::CreateImage(int width, int height, int channels, const uint8_t* data) {
    ImageData result;
    result.width = width;
    result.height = height;
    result.channels = channels;
    result.pixels.assign(data, data + width * height * channels);
    return result;
}

// ============================================================================
// Pixel Difference Algorithms
// ============================================================================
float PixelComparator::CalculatePixelDiff(const uint8_t* p1, const uint8_t* p2, int channels) {
    float diff = 0.0f;
    int compareChannels = (channels >= 3) ? 3 : channels;  // Ignore alpha for comparison
    
    for (int c = 0; c < compareChannels; c++) {
        float d = static_cast<float>(p1[c]) - static_cast<float>(p2[c]);
        diff += d * d;
    }
    
    return std::sqrt(diff / (compareChannels * 255.0f * 255.0f));
}

float PixelComparator::CalculateMSE(const ImageData& img1, const ImageData& img2) {
    if (img1.width != img2.width || img1.height != img2.height) {
        return -1.0f;
    }
    
    double mse = 0.0;
    int channels = std::min(img1.channels, img2.channels);
    
    for (int i = 0; i < img1.width * img1.height; i++) {
        for (int c = 0; c < channels; c++) {
            double diff = static_cast<double>(img1.pixels[i * img1.channels + c]) - 
                         static_cast<double>(img2.pixels[i * img2.channels + c]);
            mse += diff * diff;
        }
    }
    
    return static_cast<float>(mse / (img1.width * img1.height * channels));
}

float PixelComparator::CalculatePSNR(const ImageData& img1, const ImageData& img2) {
    float mse = CalculateMSE(img1, img2);
    if (mse <= 0.0f) return 100.0f;  // Identical images
    
    return 10.0f * std::log10((255.0f * 255.0f) / mse);
}

float PixelComparator::CalculateSSIM(const ImageData& img1, const ImageData& img2) {
    // Simplified SSIM implementation
    // Full implementation would use windowing
    (void)img1;
    (void)img2;
    return 1.0f;  // Placeholder
}

// ============================================================================
// Baseline Manager
// ============================================================================
BaselineManager::BaselineManager(const std::filesystem::path& baselineDir) 
    : m_baselineDir(baselineDir) {
    m_outputDir = baselineDir.parent_path() / "output";
}

bool BaselineManager::HasBaseline(const std::string& testName) const {
    return std::filesystem::exists(GetBaselinePath(testName));
}

std::filesystem::path BaselineManager::GetBaselinePath(const std::string& testName) const {
    return m_baselineDir / (testName + ".png");
}

std::filesystem::path BaselineManager::GetOutputPath(const std::string& testName) const {
    return m_outputDir / (testName + ".png");
}

bool BaselineManager::UpdateBaseline(const std::string& testName, const std::filesystem::path& currentImage) {
    try {
        std::filesystem::create_directories(m_baselineDir);
        std::filesystem::copy_file(currentImage, GetBaselinePath(testName), 
                                   std::filesystem::copy_options::overwrite_existing);
        return true;
    } catch (...) {
        return false;
    }
}

bool BaselineManager::ApproveAsBaseline(const std::string& testName) {
    auto currentPath = GetOutputPath(testName);
    if (!std::filesystem::exists(currentPath)) {
        return false;
    }
    return UpdateBaseline(testName, currentPath);
}

PixelComparisonResult BaselineManager::CompareWithBaseline(
    const std::string& testName,
    const std::filesystem::path& currentImage,
    const PixelComparisonOptions& options) {
    
    auto baselinePath = GetBaselinePath(testName);
    
    if (!std::filesystem::exists(baselinePath)) {
        // No baseline exists, create one
        UpdateBaseline(testName, currentImage);
        
        PixelComparisonResult result;
        result.matches = true;
        result.diffPercentage = 0.0f;
        return result;
    }
    
    return PixelComparator::CompareImages(baselinePath, currentImage, options);
}

std::vector<std::string> BaselineManager::ListBaselines() const {
    std::vector<std::string> result;
    
    if (!std::filesystem::exists(m_baselineDir)) {
        return result;
    }
    
    for (const auto& entry : std::filesystem::directory_iterator(m_baselineDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".png") {
            result.push_back(entry.path().stem().string());
        }
    }
    
    return result;
}

void BaselineManager::CleanOutputs(int keepLastN) {
    (void)keepLastN;
    // Implementation would sort by date and keep only the N most recent
}

} // namespace automation
} // namespace test
} // namespace luaui
