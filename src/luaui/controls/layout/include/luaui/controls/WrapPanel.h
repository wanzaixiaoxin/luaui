#pragma once
#include "LayoutPanel.h"

namespace luaui {
namespace controls {

// WrapPanel - Flow layout that wraps
class WrapPanel : public Panel {
public:
    WrapPanel() = default;
    
    std::string GetTypeName() const override { return "WrapPanel"; }
    
    enum class Orientation { Horizontal, Vertical };
    
    Orientation GetOrientation() const { return m_orientation; }
    void SetOrientation(Orientation orient) { 
        m_orientation = orient; 
        InvalidateMeasure(); 
    }
    
    float GetItemWidth() const { return m_itemWidth; }
    void SetItemWidth(float width) { 
        m_itemWidth = width; 
        InvalidateMeasure(); 
    }
    
    float GetItemHeight() const { return m_itemHeight; }
    void SetItemHeight(float height) { 
        m_itemHeight = height; 
        InvalidateMeasure(); 
    }
    
protected:
    Size MeasureOverride(const Size& availableSize) override;
    Size ArrangeOverride(const Size& finalSize) override;
    
private:
    Orientation m_orientation = Orientation::Horizontal;
    float m_itemWidth = 0;   // 0 = use child's desired width
    float m_itemHeight = 0;  // 0 = use child's desired height
};

} // namespace controls
} // namespace luaui
