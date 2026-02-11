#include "DirtyRegion.h"
#include <cmath>

namespace luaui {
namespace rendering {

DirtyRegion::DirtyRegion() = default;

void DirtyRegion::AddRect(const Rect& rect) {
    // 忽略空矩形
    if (rect.width <= 0 || rect.height <= 0) {
        return;
    }
    
    // 添加新矩形
    m_rects.push_back(rect);
    
    // 如果矩形数量过多，进行合并
    if (m_rects.size() > 10) {
        MergeRects();
    }
}

void DirtyRegion::Clear() {
    m_rects.clear();
}

void DirtyRegion::InvalidateAll(float width, float height) {
    m_rects.clear();
    m_rects.push_back(Rect(0, 0, width, height));
}

bool DirtyRegion::Intersects(const Rect& rect) const {
    for (const auto& dirty : m_rects) {
        if (RectsIntersect(dirty, rect)) {
            return true;
        }
    }
    return false;
}

void DirtyRegion::MergeRects() {
    if (m_rects.size() < 2) return;
    
    bool merged = true;
    while (merged && m_rects.size() > 1) {
        merged = false;
        
        for (size_t i = 0; i < m_rects.size() && !merged; ++i) {
            for (size_t j = i + 1; j < m_rects.size(); ++j) {
                // 检查是否可以合并（重叠或相邻）
                const Rect& a = m_rects[i];
                const Rect& b = m_rects[j];
                
                // 计算合并后的矩形
                float left = std::min(a.x, b.x);
                float top = std::min(a.y, b.y);
                float right = std::max(a.x + a.width, b.x + b.width);
                float bottom = std::max(a.y + a.height, b.y + b.height);
                
                float mergedWidth = right - left;
                float mergedHeight = bottom - top;
                
                // 合并条件：如果合并后的面积小于两个矩形面积之和的 1.5 倍，则合并
                float areaA = a.width * a.height;
                float areaB = b.width * b.height;
                float mergedArea = mergedWidth * mergedHeight;
                
                if (mergedArea < (areaA + areaB) * 1.5f) {
                    // 执行合并
                    m_rects[i] = Rect(left, top, mergedWidth, mergedHeight);
                    m_rects.erase(m_rects.begin() + j);
                    merged = true;
                    break;
                }
            }
        }
    }
}

bool DirtyRegion::RectsIntersect(const Rect& a, const Rect& b) {
    return (a.x < b.x + b.width) &&
           (a.x + a.width > b.x) &&
           (a.y < b.y + b.height) &&
           (a.y + a.height > b.y);
}

Rect DirtyRegion::MergeTwoRects(const Rect& a, const Rect& b) {
    float left = std::min(a.x, b.x);
    float top = std::min(a.y, b.y);
    float right = std::max(a.x + a.width, b.x + b.width);
    float bottom = std::max(a.y + a.height, b.y + b.height);
    
    return Rect(left, top, right - left, bottom - top);
}

} // namespace rendering
} // namespace luaui
