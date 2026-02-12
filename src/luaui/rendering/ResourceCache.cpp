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
    m_brushCache[hash] = std::move(brush);
    return ptr;
}

void ResourceCache::ClearBrushes() {
    m_brushCache.clear();
}

ITextFormat* ResourceCache::GetTextFormat(const std::wstring& fontFamily, 
                                           float fontSize) {
    if (!m_context) return nullptr;
    
    TextFormatKey key{fontFamily, fontSize};
    
    auto it = m_textFormatCache.find(key);
    if (it != m_textFormatCache.end()) {
        return it->second.get();
    }
    
    // 创建新文本格式并缓存
    auto format = m_context->CreateTextFormat(fontFamily, fontSize);
    if (!format) return nullptr;
    
    ITextFormat* ptr = format.get();
    m_textFormatCache[key] = std::move(format);
    return ptr;
}

ITextFormat* ResourceCache::GetDefaultTextFormat() {
    // 默认：微软雅黑 14px
    return GetTextFormat(L"Microsoft YaHei", 14.0f);
}

void ResourceCache::ClearTextFormats() {
    m_textFormatCache.clear();
}

void ResourceCache::ClearAll() {
    ClearBrushes();
    ClearTextFormats();
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
