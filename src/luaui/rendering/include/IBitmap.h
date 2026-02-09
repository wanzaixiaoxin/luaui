#pragma once

#include "Types.h"
#include <memory>
#include <string>

namespace luaui {
namespace rendering {

class IRenderContext;

// Bitmap pixel format
enum class PixelFormat {
    Unknown,
    BGRA8,      // 32-bit BGRA
    RGBA8,      // 32-bit RGBA
    BGR8,       // 24-bit BGR
    RGB8,       // 24-bit RGB
    A8,         // 8-bit alpha
    BC1,        // DXT1
    BC2,        // DXT3
    BC3         // DXT5
};

// Bitmap interface
class IBitmap {
public:
    virtual ~IBitmap() = default;
    
    // Dimensions
    virtual int GetWidth() const = 0;
    virtual int GetHeight() const = 0;
    virtual Size GetSize() const = 0;
    
    // Format
    virtual PixelFormat GetPixelFormat() const = 0;
    virtual int GetBytesPerPixel() const = 0;
    
    // DPI
    virtual float GetDpiX() const = 0;
    virtual float GetDpiY() const = 0;
    virtual void SetDpi(float dpiX, float dpiY) = 0;
    
    // Native access
    virtual void* GetNativeBitmap(IRenderContext* context) = 0;
    
    // Lock for pixel access
    virtual bool Lock(const Rect* rect, void** pixels, int* pitch) = 0;
    virtual void Unlock() = 0;
    
    // Copy from memory
    virtual bool CopyFromMemory(const void* src, int srcPitch) = 0;
};

using IBitmapPtr = std::shared_ptr<IBitmap>;

} // namespace rendering
} // namespace luaui
