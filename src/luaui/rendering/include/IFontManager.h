#pragma once

#include "Types.h"
#include <string>
#include <memory>
#include <vector>

namespace luaui {
namespace rendering {

// Font family information
struct FontFamily {
    std::wstring name;
    std::wstring path;
    bool isSystemFont = true;
};

// Font metrics
struct FontMetrics {
    float ascent = 0;        // Distance from baseline to top
    float descent = 0;       // Distance from baseline to bottom
    float lineGap = 0;       // Additional space between lines
    float capHeight = 0;     // Height of capital letters
    float xHeight = 0;       // Height of lowercase 'x'
    float underlinePosition = 0;
    float underlineThickness = 0;
    float strikethroughPosition = 0;
    float strikethroughThickness = 0;
};

// Font manager interface
class IFontManager {
public:
    virtual ~IFontManager() = default;
    
    // Initialization
    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    
    // System fonts
    virtual std::vector<FontFamily> GetSystemFonts() const = 0;
    virtual std::vector<FontFamily> GetSystemFontsWithStyle(const std::wstring& style) const = 0;
    
    // Custom font loading
    virtual bool LoadFontFromFile(const std::wstring& filePath) = 0;
    virtual bool LoadFontFromMemory(const void* data, size_t size, const std::wstring& name) = 0;
    virtual void UnloadCustomFont(const std::wstring& name) = 0;
    
    // Font lookup
    virtual bool HasFont(const std::wstring& familyName) const = 0;
    virtual FontMetrics GetFontMetrics(const std::wstring& familyName, float size) const = 0;
    
    // Default fonts
    virtual std::wstring GetDefaultFontFamily() const = 0;
    virtual void SetDefaultFontFamily(const std::wstring& family) = 0;
    
    // Fallback fonts for missing glyphs
    virtual void AddFallbackFont(const std::wstring& familyName) = 0;
    virtual void ClearFallbackFonts() = 0;
};

using IFontManagerPtr = std::unique_ptr<IFontManager>;

// Factory function
IFontManagerPtr CreateFontManager();

} // namespace rendering
} // namespace luaui
