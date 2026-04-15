#pragma once
#include "Panel.h"
#include <memory>

namespace luaui {
namespace controls {

/**
 * @brief WrapPanel - auto-wrapping panel
 */
class WrapPanel : public Panel {
public:
    enum class Orientation { Horizontal, Vertical };
    
    WrapPanel();
    
    std::string GetTypeName() const override { return "WrapPanel"; }
    
    // Orientation property
    Orientation GetOrientation() const { return m_orientation; }
    void SetOrientation(Orientation orient);
    
    // ItemWidth/ItemHeight for uniform item sizing
    float GetItemWidth() const { return m_itemWidth; }
    void SetItemWidth(float width);
    
    float GetItemHeight() const { return m_itemHeight; }
    void SetItemHeight(float height);
    
    // Spacing between items
    float GetSpacing() const { return m_spacing; }
    void SetSpacing(float spacing);

protected:
    rendering::Size OnMeasureChildren(const rendering::Size& availableSize) override;
    rendering::Size OnArrangeChildren(const rendering::Size& finalSize) override;

private:
    Orientation m_orientation = Orientation::Horizontal;
    float m_itemWidth = 0;   // 0 means use child's desired width
    float m_itemHeight = 0;  // 0 means use child's desired height
    float m_spacing = 0;     // Spacing between items
};

} // namespace controls
} // namespace luaui
