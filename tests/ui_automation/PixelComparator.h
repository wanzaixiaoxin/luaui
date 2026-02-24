// PixelComparator.h - Pixel-level image comparison for UI testing

#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <functional>
#include <cstdint>
#include <sstream>

namespace luaui {
namespace test {
namespace automation {

// Forward declarations
struct Rect;
struct PixelComparisonOptions;
struct PixelComparisonResult;
struct ImageData;

// Pixel comparison algorithms
enum class ComparisonAlgorithm {
    Exact,           // Bit-for-bit exact match
    Fuzzy,           // Allow small color variations
    Perceptual,      // Perceptual diff (human-like comparison)
    IgnoreAntiAliasing // Ignore AA differences
};

struct PixelComparisonOptions {
    ComparisonAlgorithm algorithm = ComparisonAlgorithm::Fuzzy;
    float threshold = 0.01f;  // Maximum allowed difference (0-1)
    bool ignoreRegions = false;
    std::vector<struct Rect> ignoreRects;  // Regions to ignore
    bool generateDiffImage = true;
    bool highlightDiffs = true;
    uint32_t diffColor = 0xFF0000FF;  // Red for differences (ARGB)
};

struct PixelComparisonResult {
    bool matches = false;
    float diffPercentage = 0.0f;
    int diffPixelCount = 0;
    int totalPixels = 0;
    std::filesystem::path diffImagePath;  // Path to diff visualization
    
    std::string ToString() const {
        std::stringstream ss;
        ss << "PixelComparison: " << (matches ? "MATCH" : "MISMATCH")
           << " | Diff: " << diffPercentage * 100.0f << "%"
           << " | Pixels: " << diffPixelCount << "/" << totalPixels;
        return ss.str();
    }
};

struct ImageData {
    int width = 0;
    int height = 0;
    int channels = 4;  // 3 = RGB, 4 = RGBA
    std::vector<uint8_t> pixels;
    
    bool IsValid() const { return width > 0 && height > 0 && !pixels.empty(); }
    size_t PixelCount() const { return width * height; }
    
    uint8_t* GetPixel(int x, int y) {
        if (x < 0 || x >= width || y < 0 || y >= height) return nullptr;
        return &pixels[(y * width + x) * channels];
    }
    
    const uint8_t* GetPixel(int x, int y) const {
        if (x < 0 || x >= width || y < 0 || y >= height) return nullptr;
        return &pixels[(y * width + x) * channels];
    }
};

// ============================================================================
// Region of Interest for selective comparison
// ============================================================================
struct Rect {
    int x = 0, y = 0, width = 0, height = 0;
    
    bool Contains(int px, int py) const {
        return px >= x && px < x + width && py >= y && py < y + height;
    }
    
    bool IsEmpty() const { return width <= 0 || height <= 0; }
};

class PixelComparator {
public:
    // Compare two images from file paths
    static PixelComparisonResult CompareImages(
        const std::filesystem::path& baseline,
        const std::filesystem::path& current,
        const PixelComparisonOptions& options = {}
    );
    
    // Compare two images from memory
    static PixelComparisonResult CompareImages(
        const ImageData& baseline,
        const ImageData& current,
        const PixelComparisonOptions& options = {}
    );
    
    // Compare with tolerance
    static bool AreEqual(const ImageData& img1, const ImageData& img2, float tolerance = 0.01f);
    
    // Generate diff visualization
    static bool GenerateDiffImage(
        const ImageData& baseline,
        const ImageData& current,
        const std::filesystem::path& outputPath,
        uint32_t diffColor = 0xFF0000FF
    );
    
    // Load image from file (PNG/BMP)
    static ImageData LoadImage(const std::filesystem::path& path);
    
    // Save image to file
    static bool SaveImage(const std::filesystem::path& path, const ImageData& image);
    
    // Create image from raw pixel data
    static ImageData CreateImage(int width, int height, int channels, const uint8_t* data);
    
private:
    static float CalculatePixelDiff(const uint8_t* p1, const uint8_t* p2, int channels);
    static float CalculateMSE(const ImageData& img1, const ImageData& img2);
    static float CalculatePSNR(const ImageData& img1, const ImageData& img2);
    static float CalculateSSIM(const ImageData& img1, const ImageData& img2);
};

// ============================================================================
// Baseline Manager - Manage test baseline images
// ============================================================================
class BaselineManager {
public:
    BaselineManager(const std::filesystem::path& baselineDir);
    
    // Check if baseline exists
    bool HasBaseline(const std::string& testName) const;
    
    // Get baseline path
    std::filesystem::path GetBaselinePath(const std::string& testName) const;
    
    // Get output path for current test run
    std::filesystem::path GetOutputPath(const std::string& testName) const;
    
    // Update baseline with current result
    bool UpdateBaseline(const std::string& testName, const std::filesystem::path& currentImage);
    
    // Approve current result as new baseline
    bool ApproveAsBaseline(const std::string& testName);
    
    // Compare against baseline
    PixelComparisonResult CompareWithBaseline(
        const std::string& testName,
        const std::filesystem::path& currentImage,
        const PixelComparisonOptions& options = {}
    );
    
    // List all baselines
    std::vector<std::string> ListBaselines() const;
    
    // Clean up old outputs
    void CleanOutputs(int keepLastN = 10);
    
private:
    std::filesystem::path m_baselineDir;
    std::filesystem::path m_outputDir;
};

} // namespace automation
} // namespace test
} // namespace luaui
