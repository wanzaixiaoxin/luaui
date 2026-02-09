#pragma once
#include "LayoutPanel.h"

namespace luaui {
namespace controls {

enum class Dock { Left, Top, Right, Bottom };

// DockPanel - Docks children to edges
class DockPanel : public Panel {
public:
    DockPanel() = default;
    
    std::string GetTypeName() const override { return "DockPanel"; }
    
    // Attached property
    static void SetDock(Control* control, Dock dock);
    static Dock GetDock(Control* control);
    
    bool GetLastChildFill() const { return m_lastChildFill; }
    void SetLastChildFill(bool fill) { 
        m_lastChildFill = fill; 
        InvalidateArrange(); 
    }
    
protected:
    Size MeasureOverride(const Size& availableSize) override;
    Size ArrangeOverride(const Size& finalSize) override;
    
private:
    bool m_lastChildFill = true;
};

} // namespace controls
} // namespace luaui
