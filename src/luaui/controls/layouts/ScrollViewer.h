#pragma once
#include "Panel.h"
#include <string>

namespace luaui {
namespace controls {

enum class ScrollBarVisibility { Auto, Visible, Hidden, Disabled };

/**
 * @brief ScrollViewer - 滚动视图面板（新架构）
 */
class ScrollViewer : public Panel {
public:
    ScrollViewer();
    
    std::string GetTypeName() const override { return "ScrollViewer"; }
    
    // 滚动条可见性
    ScrollBarVisibility GetHorizontalScrollBarVisibility() const { return m_horizontalScrollBarVisibility; }
    void SetHorizontalScrollBarVisibility(ScrollBarVisibility visibility);
    
    ScrollBarVisibility GetVerticalScrollBarVisibility() const { return m_verticalScrollBarVisibility; }
    void SetVerticalScrollBarVisibility(ScrollBarVisibility visibility);
    
    // 滚动偏移
    float GetHorizontalOffset() const { return m_horizontalOffset; }
    float GetVerticalOffset() const { return m_verticalOffset; }
    void ScrollToHorizontalOffset(float offset);
    void ScrollToVerticalOffset(float offset);
    
    // 内容大小
    float GetExtentWidth() const { return m_extentWidth; }
    float GetExtentHeight() const { return m_extentHeight; }
    float GetViewportWidth() const { return m_viewportWidth; }
    float GetViewportHeight() const { return m_viewportHeight; }

protected:
    rendering::Size OnMeasureChildren(const rendering::Size& availableSize) override;
    rendering::Size OnArrangeChildren(const rendering::Size& finalSize) override;

private:
    ScrollBarVisibility m_horizontalScrollBarVisibility;
    ScrollBarVisibility m_verticalScrollBarVisibility;
    float m_horizontalOffset;
    float m_verticalOffset;
    float m_extentWidth;
    float m_extentHeight;
    float m_viewportWidth;
    float m_viewportHeight;
};

} // namespace controls
} // namespace luaui
