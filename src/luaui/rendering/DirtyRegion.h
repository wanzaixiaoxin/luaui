#pragma once

#include "Types.h"
#include <vector>
#include <algorithm>

namespace luaui {
namespace rendering {

/**
 * @brief 脏矩形区域管理器
 * 
 * 管理和合并多个脏矩形区域，优化渲染性能
 */
class DirtyRegion {
public:
    DirtyRegion();
    
    /**
     * @brief 添加脏矩形区域
     * @param rect 需要重绘的矩形区域
     */
    void AddRect(const Rect& rect);
    
    /**
     * @brief 获取所有脏矩形区域（已合并优化）
     * @return 脏矩形列表
     */
    const std::vector<Rect>& GetRects() const { return m_rects; }
    
    /**
     * @brief 检查是否需要重绘指定区域
     * @param rect 要检查的区域
     * @return 是否需要重绘
     */
    bool Intersects(const Rect& rect) const;
    
    /**
     * @brief 清空所有脏矩形
     */
    void Clear();
    
    /**
     * @brief 检查是否有脏矩形
     */
    bool IsEmpty() const { return m_rects.empty(); }
    
    /**
     * @brief 使整个区域变脏（强制全屏重绘）
     * @param width 区域宽度
     * @param height 区域高度
     */
    void InvalidateAll(float width, float height);

private:
    std::vector<Rect> m_rects;
    
    // 合并重叠的矩形
    void MergeRects();
    
    // 检查两个矩形是否重叠
    static bool RectsIntersect(const Rect& a, const Rect& b);
    
    // 合并两个矩形
    static Rect MergeTwoRects(const Rect& a, const Rect& b);
};

} // namespace rendering
} // namespace luaui
