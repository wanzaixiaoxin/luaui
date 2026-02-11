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
 * 缓存渲染资源（画刷等）以避免重复创建，提升性能
 * 
 * 使用示例：
 *   ResourceCache cache(context);
 *   auto brush = cache.GetSolidColorBrush(Color::Red());
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

    // ========== 全局清理 ==========
    
    /**
     * @brief 清除所有缓存
     */
    void ClearAll();

    // ========== 统计信息 ==========
    
    size_t GetBrushCacheSize() const { return m_brushCache.size(); }

private:
    IRenderContext* m_context;
    
    // 画刷缓存：颜色哈希 -> 画刷
    std::unordered_map<uint32_t, ISolidColorBrushPtr> m_brushCache;
    
    // 颜色转哈希
    static uint32_t ColorToHash(const Color& color);
};

} // namespace rendering
} // namespace luaui
