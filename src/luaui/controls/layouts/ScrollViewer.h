#pragma once
#include "Panel.h"
#include <string>

namespace luaui {
namespace controls {

enum class ScrollBarVisibility { Auto, Visible, Hidden, Disabled };

/**
 * @brief ScrollViewer - 滚动视图面板
 *
 * 当子内容超出视口大小时，支持鼠标滚轮滚动。
 * 自动显示垂直滚动条指示器。
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
    void InitializeComponents() override;
    rendering::Size OnMeasureChildren(const rendering::Size& availableSize) override;
    rendering::Size OnArrangeChildren(const rendering::Size& finalSize) override;
    void OnRenderChildren(rendering::IRenderContext* context) override;

    // 鼠标滚轮
    void OnMouseWheel(MouseEventArgs& args) override;

private:
    float ClampVerticalOffset(float offset);
    float ClampHorizontalOffset(float offset);

    ScrollBarVisibility m_horizontalScrollBarVisibility = ScrollBarVisibility::Auto;
    ScrollBarVisibility m_verticalScrollBarVisibility = ScrollBarVisibility::Auto;
    float m_horizontalOffset = 0;
    float m_verticalOffset = 0;
    float m_extentWidth = 0;
    float m_extentHeight = 0;
    float m_viewportWidth = 0;
    float m_viewportHeight = 0;

    // scrollbar appearance
    float m_scrollbarMinThumb = 30.0f;
    float m_sbWidth = 8.0f;
    rendering::Color m_scrollbarTrackColor = rendering::Color::FromHex(0x00000011);
    rendering::Color m_scrollbarThumbColor = rendering::Color::FromHex(0x00000044);
};

} // namespace controls
} // namespace luaui
