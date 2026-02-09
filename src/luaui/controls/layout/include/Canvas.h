#pragma once
#include "LayoutPanel.h"

namespace luaui {
namespace controls {

// Canvas - Absolute positioning panel
class Canvas : public Panel {
public:
    Canvas() = default;
    
    std::string GetTypeName() const override { return "Canvas"; }
    
    // Attached properties
    static void SetLeft(Control* control, float left);
    static void SetTop(Control* control, float top);
    static void SetRight(Control* control, float right);
    static void SetBottom(Control* control, float bottom);
    static float GetLeft(Control* control);
    static float GetTop(Control* control);
    static float GetRight(Control* control);
    static float GetBottom(Control* control);
    
protected:
    Size MeasureOverride(const Size& availableSize) override;
    Size ArrangeOverride(const Size& finalSize) override;
};

} // namespace controls
} // namespace luaui
