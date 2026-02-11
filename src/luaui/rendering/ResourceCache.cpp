#include "ResourceCache.h"

namespace luaui {
namespace rendering {

ResourceCache::ResourceCache(IRenderContext* context) 
    : m_context(context) {
}

ResourceCache::~ResourceCache() {
    ClearAll();
}

ISolidColorBrush* ResourceCache::GetSolidColorBrush(const Color& color) {
    if (!m_context) return nullptr;
    
    uint32_t hash = ColorToHash(color);
    
    auto it = m_brushCache.find(hash);
    if (it != m_brushCache.end()) {
        return it->second.get();
    }
    
    // 创建新画刷并缓存
    auto brush = m_context->CreateSolidColorBrush(color);
    if (!brush) return nullptr;
    
    ISolidColorBrush* ptr = brush.get();
    m_brushCache[hash] = brush;
    return ptr;
}

void ResourceCache::ClearBrushes() {
    m_brushCache.clear();
}

void ResourceCache::ClearAll() {
    ClearBrushes();
}

uint32_t ResourceCache::ColorToHash(const Color& color) {
    // 将颜色打包为 32 位哈希值 (ARGB)
    uint32_t r = static_cast<uint32_t>(color.r * 255.0f + 0.5f) & 0xFF;
    uint32_t g = static_cast<uint32_t>(color.g * 255.0f + 0.5f) & 0xFF;
    uint32_t b = static_cast<uint32_t>(color.b * 255.0f + 0.5f) & 0xFF;
    uint32_t a = static_cast<uint32_t>(color.a * 255.0f + 0.5f) & 0xFF;
    return (a << 24) | (r << 16) | (g << 8) | b;
}

} // namespace rendering
} // namespace luaui
