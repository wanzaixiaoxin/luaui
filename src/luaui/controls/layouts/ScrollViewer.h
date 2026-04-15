#pragma once
#include "Panel.h"
#include <string>

namespace luaui {
namespace controls {

enum class ScrollBarVisibility { Auto, Visible, Hidden, Disabled };

/**
 * @brief ScrollViewer - scrollable viewport panel
 *
 * Supports mouse wheel scrolling when content exceeds viewport.
 * Scrollbar interaction aligns with mainstream browser behavior:
 * - Expand on hover
 * - Click track to jump
 * - Drag thumb for precise scrolling
 * - Hover / pressed visual feedback
 */
class ScrollViewer : public Panel {
public:
    ScrollViewer();

    std::string GetTypeName() const override { return "ScrollViewer"; }

    ScrollBarVisibility GetHorizontalScrollBarVisibility() const { return m_horizontalScrollBarVisibility; }
    void SetHorizontalScrollBarVisibility(ScrollBarVisibility visibility);

    ScrollBarVisibility GetVerticalScrollBarVisibility() const { return m_verticalScrollBarVisibility; }
    void SetVerticalScrollBarVisibility(ScrollBarVisibility visibility);

    float GetHorizontalOffset() const { return m_horizontalOffset; }
    float GetVerticalOffset() const { return m_verticalOffset; }
    void ScrollToHorizontalOffset(float offset);
    void ScrollToVerticalOffset(float offset);

    float GetExtentWidth() const { return m_extentWidth; }
    float GetExtentHeight() const { return m_extentHeight; }
    float GetViewportWidth() const { return m_viewportWidth; }
    float GetViewportHeight() const { return m_viewportHeight; }

protected:
    void InitializeComponents() override;
    rendering::Size OnMeasureChildren(const rendering::Size& availableSize) override;
    rendering::Size OnArrangeChildren(const rendering::Size& finalSize) override;
    void OnRenderChildren(rendering::IRenderContext* context) override;

    void OnMouseDown(MouseEventArgs& args) override;
    void OnMouseMove(MouseEventArgs& args) override;
    void OnMouseUp(MouseEventArgs& args) override;
    void OnMouseWheel(MouseEventArgs& args) override;

private:
    /** Convert global screen coords to local coords */
    void GlobalToLocal(float gx, float gy, float& lx, float& ly);

    /** Whether vertical scrollbar is needed */
    bool NeedVScroll() const;
    
    /** Whether horizontal scrollbar is needed */
    bool NeedHScroll() const;

    /** Test if global coords hit the thumb */
    bool HitTestThumb(float gx, float gy);

    /** Test if global coords hit the track (including thumb) */
    bool HitTestTrack(float gx, float gy);

    /** Compute thumb y position and height in local coords */
    void CalcThumb(float& y, float& h);

    /** Apply offset and refresh */
    void ApplyOffset(float offset);

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

    // scrollbar layout constants
    static constexpr float SB_COLLAPSED = 6.0f;
    static constexpr float SB_EXPANDED  = 12.0f;
    static constexpr float SB_MARGIN    = 3.0f;
    static constexpr float THUMB_MIN    = 30.0f;
    static constexpr float WHEEL_STEP   = 48.0f;
    static constexpr float TRACK_CLICK_RATIO = 0.8f;

    // scrollbar state
    bool m_sbHovered = false;
    bool m_sbPressed = false;
    bool m_dragging = false;
    float m_dragStartY = 0;
    float m_dragStartOffset = 0;
    float m_currentSbWidth = SB_COLLAPSED;
};

} // namespace controls
} // namespace luaui
