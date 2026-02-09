#pragma once
#include "LayoutPanel.h"

namespace luaui {
namespace controls {

// StackPanel - Arranges children in a single line
class StackPanel : public Panel {
public:
    StackPanel() = default;
    
    std::string GetTypeName() const override { return "StackPanel"; }
    
    enum class Orientation { Horizontal, Vertical };
    
    Orientation GetOrientation() const { return m_orientation; }
    void SetOrientation(Orientation orient) { 
        m_orientation = orient; 
        InvalidateMeasure(); 
    }
    
    float GetSpacing() const { return m_spacing; }
    void SetSpacing(float spacing) { 
        m_spacing = spacing; 
        InvalidateMeasure(); 
    }
    
protected:
    Size MeasureOverride(const Size& availableSize) override;
    Size ArrangeOverride(const Size& finalSize) override;
    
private:
    Orientation m_orientation = Orientation::Vertical;
    float m_spacing = 0;
};

} // namespace controls
} // namespace luaui
