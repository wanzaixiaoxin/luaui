#pragma once

#include "IRenderContext.h"
#include "Types.h"
#include <memory>
#include <unordered_map>
#include <string>

namespace luaui {
namespace rendering {

/**
 * @brief 资源缓存池
 * 
 * 缓存渲染资源（画刷、文本格式等）以避免重复创建，提升性能
 * 
 * 使用示例：
 *   ResourceCache cache(context);
 *   auto brush = cache.GetSolidColorBrush(Color::Red());
 *   auto format = cache.GetTextFormat(L"Arial", 14.0f);
 *   context->FillRectangle(rect, brush);
 */
class ResourceCache {
public:
    explicit ResourceCache(IRenderContext* context);
    ~ResourceCache();

    // 禁止拷贝
    ResourceCache(const ResourceCache&) = delete;
    ResourceCache& operator=(const ResourceCache&) = delete;

    // 允许移动
    ResourceCache(ResourceCache&&) = default;
    ResourceCache& operator=(ResourceCache&&) = default;

    // ========== 画刷缓存 ==========
    
    /**
     * @brief 获取纯色画刷（带缓存）
     * @param color 颜色
     * @return 画刷指针（缓存中的实例，不要手动释放）
     */
    ISolidColorBrush* GetSolidColorBrush(const Color& color);
    
    /**
     * @brief 清除画刷缓存
     */
    void ClearBrushes();

    // ========== 文本格式缓存 ==========
    
    /**
     * @brief 获取文本格式（带缓存）
     * @param fontFamily 字体名称
     * @param fontSize 字体大小
     * @return 文本格式指针（缓存中的实例，不要手动释放）
     */
    ITextFormat* GetTextFormat(const std::wstring& fontFamily, float fontSize);
    
    /**
     * @brief 获取默认文本格式（14px 微软雅黑）
     * @return 文本格式指针
     */
    ITextFormat* GetDefaultTextFormat();
    
    /**
     * @brief 清除文本格式缓存
     */
    void ClearTextFormats();

    // ========== 全局清理 ==========
    
    /**
     * @brief 清除所有缓存
     */
    void ClearAll();

    // ========== 统计信息 ==========
    
    size_t GetBrushCacheSize() const { return m_brushCache.size(); }
    size_t GetTextFormatCacheSize() const { return m_textFormatCache.size(); }

private:
    IRenderContext* m_context;
    
    // 画刷缓存：颜色哈希 -> 画刷
    std::unordered_map<uint32_t, ISolidColorBrushPtr> m_brushCache;
    
    // 文本格式缓存：(字体, 大小) -> 文本格式
    struct TextFormatKey {
        std::wstring fontFamily;
        float fontSize;
        
        bool operator==(const TextFormatKey& other) const {
            return fontFamily == other.fontFamily && 
                   fontSize == other.fontSize;
        }
    };
    
    struct TextFormatKeyHash {
        size_t operator()(const TextFormatKey& key) const {
            size_t h1 = std::hash<std::wstring>{}(key.fontFamily);
            size_t h2 = std::hash<float>{}(key.fontSize);
            return h1 ^ (h2 << 1);
        }
    };
    
    std::unordered_map<TextFormatKey, ITextFormatPtr, TextFormatKeyHash> m_textFormatCache;
    
    // 颜色转哈希
    static uint32_t ColorToHash(const Color& color);
};

} // namespace rendering
} // namespace luaui
